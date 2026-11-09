#include "engine.h"
#include "simconnect.h"
#include "hid.h"
#include <algorithm>
#include <tuple>
namespace sv {
Engine::Engine() { runtimeThread=std::thread([this]{runtimeLoop();}); hidThread=std::thread([this]{hidLoop();}); }
Engine::~Engine() { stop(); quitting=true; runtimeThread.join(); hidThread.join(); }
void Engine::log(const QString& text) { std::lock_guard lock(mutex); logs<<text; while(logs.size()>1000) logs.removeFirst(); }
void Engine::apply(Bundle bundle) { std::lock_guard lock(mutex); pending=std::make_unique<Bundle>(std::move(bundle)); }
void Engine::clearProject() { stop(); ready=false; ++projectRevision; std::lock_guard lock(mutex); clearRequested=true; pending.reset(); }
void Engine::run() { require(ready,"Apply a valid project before Run"); ++generation; running=true; }
void Engine::stop() { running=false; ++generation; std::lock_guard lock(mutex); requests.clear(); testMotor.clear(); execution="Stopped"; }
void Engine::disable(const QString& id) { std::lock_guard lock(mutex); disabled.insert(id); for(auto it=requests.begin();it!=requests.end();) if(it.key().startsWith(id+"/")) it=requests.erase(it); else ++it; }
void Engine::connectSim(QString path) { stop(); std::lock_guard lock(mutex); dllPath=std::move(path); connectRequested=true; }
void Engine::scanHid() { stop(); std::lock_guard lock(mutex); scanRequested=true; }
void Engine::loadMappings(QJsonObject values) { std::lock_guard lock(mutex); mappings=std::move(values); }
void Engine::mapMotor(QString id,QStringList destinations) { stop(); std::lock_guard lock(mutex); QJsonArray list; for(const auto& s:destinations) if(logicalSlots().contains(s)) list<<s; mappings[id]=QJsonObject{{"slots",list},{"manual",true}}; for(auto& motor:motors) if(motor.id==id) motor.destinations=destinations; }
void Engine::test(QString motor,Mode mode,int strength,int on,int period,int duration) {
    require(strength>=0 && strength<=255 && duration>0 && duration<=5000 && on>=0 && period>=20 && on<=period,"Invalid motor test");
    std::lock_guard lock(mutex); bool found=false; for(const auto& m:motors) if(m.id==motor && m.connected) found=true; require(found,"Motor not connected");
    testMotor=std::move(motor); testRequest={"test",{},mode,strength,int(std::floor(on/20.0+0.5))*20,int(std::floor(period/20.0+0.5))*20,nowMs()+duration,nowMs()};
}
void Engine::stopTest() { std::lock_guard lock(mutex); testMotor.clear(); }
Snapshot Engine::snapshot() { std::lock_guard lock(mutex); Snapshot s{simStatus,aircraftKey,execution,motors,logs,mappings}; logs.clear(); return s; }
static std::vector<std::unique_ptr<LuaEffect>> instantiate(const Bundle& b,const QString& key) {
    std::vector<std::unique_ptr<LuaEffect>> result;
    for(auto value:b.project.data["effects"].toArray()) { auto e=value.toObject(); auto path=e["script"].toString(); auto instance=std::make_unique<LuaEffect>(e["id"].toString(),b.sources.value(path),path); instance->configure(b.project.resolved(e,key)); result.push_back(std::move(instance)); }
    return result;
}
void Engine::runtimeLoop() {
    SimConnect sim; Bundle active; bool hasActive=false,wasValid=false; Telemetry telemetry; quint64 localGeneration=0; qint64 lastUpdate=0;
    std::vector<std::unique_ptr<LuaEffect>> effects;
    while(!quitting) {
        auto now=nowMs(); heartbeat=now;
        std::unique_ptr<Bundle> change; bool connect=false,clear=false; QString dll; QSet<QString> muted; quint64 revision;
        { std::lock_guard lock(mutex); change=std::move(pending); revision=projectRevision; connect=connectRequested; connectRequested=false; clear=clearRequested; clearRequested=false; dll=dllPath; muted=disabled; }
        if(clear) { effects.clear(); active={}; hasActive=false; }
        try {
            if(connect) { sim.connect(dll); telemetry={}; ++generation; }
            if(change) {
                // Validate every profile before replacing the active project.
                auto prepared=instantiate(*change,telemetry.aircraft);
                auto globalValidation=instantiate(*change,QString());
                for(auto it=change->project.aircraft.begin();it!=change->project.aircraft.end();++it) { auto validation=instantiate(*change,it.key()); }
                require(revision==projectRevision.load(),"Discarded Apply from previous project");
                active=std::move(*change); effects=std::move(prepared); hasActive=true; localGeneration=++generation;
                { std::lock_guard lock(mutex); requests.clear(); disabled.clear(); ready=revision==projectRevision.load(); }
                log("Apply succeeded");
            }
        } catch(const std::exception& e) { log(QString("Apply/connection error: ")+e.what()); }
        try {
            auto samples=sim.poll();
            if(samples.isEmpty() && now-lastUpdate>=100) samples<<telemetry;
            for(auto t:samples) {
                bool fresh=t.sequence!=telemetry.sequence;
                bool reset=t.aircraft!=telemetry.aircraft || t.simTime<telemetry.simTime || generation.load()!=localGeneration;
                telemetry=t; now=nowMs(); bool valid=t.valid(now) && sim.connected(); inputDeadline=valid?std::min(t.received+500,t.advancedAt+500):0;
                if(wasValid && !valid) reset=true; wasValid=valid;
                if(reset) { localGeneration=generation.load(); { std::lock_guard lock(mutex); requests.clear(); } if(hasActive) effects=instantiate(active,t.aircraft); }
                { std::lock_guard lock(mutex); aircraftKey=t.aircraft; execution=running?(hasActive?(valid?"Running":"Running / waiting for valid telemetry"):"Run unavailable / apply a project"):"Stopped"; }
                if(running && ready && hasActive && valid) for(auto& effect:effects) {
                    if(!effect || muted.contains(effect->id) || !effect->settings["enabled"].toBool()) continue;
                    try {
                        auto commands=effect->update(t,fresh,now); for(const auto& message:effect->takeLogs()) log(effect->id+": "+message);
                        std::lock_guard lock(mutex);
                        if(generation.load()!=localGeneration || !running || disabled.contains(effect->id)) continue;
                        for(auto request:commands) {
                            auto old=requests.value(request.key);
                            if(request.mode==Mode::Pwm && old.mode==Mode::Pwm && old.deadline>now) request.phase=old.phase;
                            if(request.mode==Mode::Off || request.strength==0 || (request.mode==Mode::Pwm && request.on==0)) requests.remove(request.key); else requests[request.key]=request;
                        }
                    } catch(const std::exception& e) { auto id=effect->id; log("ERROR "+id+": "+e.what()); disable(id); effect.reset(); }
                }
                lastUpdate=now;
            }
        } catch(const std::exception& e) { log(QString("Runtime: ")+e.what()); sim.disconnect(); telemetry={}; inputDeadline=0; ++generation; }
        { std::lock_guard lock(mutex); simStatus=sim.status; }
        heartbeat=nowMs(); std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}
static bool stronger(const Request& a,const Request& b) {
    return std::tuple{a.mode==Mode::Constant,a.strength,-a.period,a.on}>std::tuple{b.mode==Mode::Constant,b.strength,-b.period,b.on};
}
void Engine::hidLoop() {
    Hid hid; QMap<QString,QString> winners; QSet<QString> touched; auto next=std::chrono::steady_clock::now();
    while(!quitting) {
        next+=std::chrono::milliseconds(20); bool scan=false; { std::lock_guard lock(mutex); scan=scanRequested; scanRequested=false; }
        if(scan) {
            for(const auto& id:touched) { QString error; hid.write(id,0,error); } touched.clear();
            try { auto discovered=hid.scan(); std::lock_guard lock(mutex);
                QSet<QString> occupied; for(auto it=mappings.begin();it!=mappings.end();++it) for(auto v:it.value().toObject()["slots"].toArray()) occupied.insert(v.toString());
                for(auto& motor:discovered) {
                    if(!mappings.contains(motor.id) && !motor.ambiguous) {
                        QJsonArray assigned; for(int i=1;i<=4;++i) { auto s="Throttle:"+QString::number(i); if(!occupied.contains(s)) { assigned<<s; occupied.insert(s); break; } }
                        mappings[motor.id]=QJsonObject{{"slots",assigned},{"manual",false}};
                    }
                    if(!motor.ambiguous) for(auto v:mappings[motor.id].toObject()["slots"].toArray()) motor.destinations<<v.toString();
                }
                for(auto old:motors) { bool found=false;for(const auto& motor:discovered)if(motor.id==old.id)found=true;if(!found){old.connected=false;old.sent=0;old.error="Disconnected / mapping reserved";discovered<<old;} }
                motors=discovered;
            } catch(const std::exception& e) { log(QString("HID: ")+e.what()); }
        }
        auto now=nowMs(); QList<Motor> currentMotors; QMap<QString,Request> currentRequests; QString testId; Request test;
        { std::lock_guard lock(mutex);
            for(auto it=requests.begin();it!=requests.end();) if(it->deadline<=now) it=requests.erase(it); else ++it;
            if(!running || now-heartbeat.load()>250 || now>=inputDeadline.load()) requests.clear();
            if(testRequest.deadline<=now) testMotor.clear(); currentMotors=motors; currentRequests=requests; testId=testMotor; test=testRequest;
        }
        for(auto& motor:currentMotors) {
            if(!motor.connected) continue;
            const Request* best=nullptr;
            if(testId==motor.id) best=&test;
            else for(auto it=currentRequests.begin();it!=currentRequests.end();++it) {
                bool routed=false; for(const auto& s:motor.destinations) if(it->destinations.contains(s)) routed=true; if(!routed) continue;
                if(!best || stronger(*it,*best) || (!stronger(*best,*it) && it.key()==winners.value(motor.id))) best=&*it;
            }
            int strength=0; if(best) { winners[motor.id]=best->key; if(best->mode==Mode::Constant || (best->mode==Mode::Pwm && (now-best->phase)%best->period<best->on)) strength=best->strength; } else winners.remove(motor.id);
            // No report is sent merely because a device was opened. Once driven, send zero on release.
            if(strength || touched.contains(motor.id)) {
                QString error; bool ok=hid.write(motor.id,strength,error); if(strength && ok) touched.insert(motor.id); if(!strength || !ok) touched.remove(motor.id);
                std::lock_guard lock(mutex); for(auto& stored:motors) if(stored.id==motor.id) { stored.sent=ok?strength:0; if(!ok) {stored.connected=false;stored.error=error;logs<<motor.label+": "+error;} }
            }
        }
        if(next<std::chrono::steady_clock::now()) next=std::chrono::steady_clock::now(); std::this_thread::sleep_until(next);
    }
    for(const auto& id:touched) {QString error;hid.write(id,0,error);}
}
}
