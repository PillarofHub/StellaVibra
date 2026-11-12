#include "engine.h"
#include "simconnect.h"
#include "network_telemetry.h"
#include "game_catalog.h"
#include "hid.h"
#include <algorithm>
#include <tuple>
namespace sv {
Engine::Engine() { runtimeThread=std::thread([this]{runtimeLoop();}); hidThread=std::thread([this]{hidLoop();}); }
Engine::~Engine() { stop(); quitting=true; if(runtimeThread.joinable())runtimeThread.join();if(hidThread.joinable())hidThread.join(); }
void Engine::log(const QString& text) { std::lock_guard lock(mutex); logs<<text; while(logs.size()>1000) logs.removeFirst(); }
void Engine::apply(Bundle bundle) { std::lock_guard lock(mutex); if(pending)bundle.reloadIds.unite(pending->reloadIds); pending=std::make_unique<Bundle>(std::move(bundle)); }
void Engine::clearProject() { ready=false; ++projectRevision; ++generation; std::lock_guard lock(mutex); requests.clear(); disabled.clear(); testMotor.clear(); clearRequested=true; pending.reset(); }
void Engine::run() { ++generation; running=true; }
void Engine::stop() { running=false; ++generation; std::lock_guard lock(mutex); requests.clear(); testMotor.clear(); execution="Stopped"; }
void Engine::disable(const QString& id) { std::lock_guard lock(mutex); disabled.insert(id); for(auto it=requests.begin();it!=requests.end();) if(it.key().startsWith(id+"/")) it=requests.erase(it); else ++it; }
void Engine::loadMappings(QJsonObject values) { std::lock_guard lock(mutex); mappings=std::move(values); }
void Engine::mapMotor(QString id,QStringList destinations) { std::lock_guard lock(mutex); QJsonArray list; for(const auto& s:destinations) if(logicalSlots().contains(s)) list<<s; mappings[id]=QJsonObject{{"slots",list},{"manual",true}}; for(auto& motor:motors) if(motor.id==id) motor.destinations=destinations; }
void Engine::test(QString motor,Mode mode,int strength,int on,int period,int duration) {
    require(strength>=0 && strength<=255 && duration>0 && duration<=5000 && on>=0 && period>=20 && on<=period,"Invalid motor test");
    std::lock_guard lock(mutex); bool found=false; for(const auto& m:motors) if(m.id==motor && m.connected) found=true; require(found,"Motor not connected");
    testMotor=std::move(motor); testRequest={"test",{},mode,strength,on,period,nowMs()+duration,nowMs()};
}
void Engine::setTick(int milliseconds) {require(milliseconds>=5 && milliseconds<=100,"Tick must be 5..100 ms");tickMilliseconds=milliseconds;++generation;std::lock_guard lock(mutex);requests.clear();testMotor.clear();}
void Engine::setMaximumUpdatePeriod(int milliseconds) {require(milliseconds>=5 && milliseconds<=500,"Maximum update period must be 5..500 ms");maximumUpdateMilliseconds=milliseconds;}
void Engine::stopTest() { std::lock_guard lock(mutex); testMotor.clear(); }
Snapshot Engine::snapshot() { std::lock_guard lock(mutex); Snapshot s{simStatus,aircraftKey,execution,motors,logs,mappings,latestTelemetry,telemetryConnected}; logs.clear(); return s; }
static std::vector<std::unique_ptr<LuaEffect>> instantiate(const Bundle& b,const QString& key) {
    std::vector<std::unique_ptr<LuaEffect>> result;
    for(auto value:b.project.data["effects"].toArray()) { auto e=value.toObject(); auto path=e["script"].toString(); if(!b.sources.contains(path))continue; auto instance=std::make_unique<LuaEffect>(e["id"].toString(),b.sources.value(path),path); instance->configure(b.project.resolved(e,key),b.channelCounts); result.push_back(std::move(instance)); }
    return result;
}
void Engine::runtimeLoop() {
    SimConnect sim; std::unique_ptr<NetworkTelemetry> network; QString selectedGame; Bundle active; bool hasActive=false,wasValid=false,loggedFirstUpdate=false; Telemetry telemetry; quint64 localGeneration=0; qint64 lastUpdate=0;
    std::vector<std::unique_ptr<LuaEffect>> effects; qint64 nextConnect=0; QString lastConnectionError;
    while(!quitting) {
        auto now=nowMs(); heartbeat=now;
        std::unique_ptr<Bundle> change; bool clear=false; QSet<QString> muted; quint64 revision;
        { std::lock_guard lock(mutex); change=std::move(pending); revision=projectRevision; clear=clearRequested; clearRequested=false; muted=disabled; }
        if(clear) { effects.clear(); active={}; hasActive=false; }
        const auto desiredGame=change?change->project.data["game"].toString():hasActive?active.project.data["game"].toString():QString();
        if(desiredGame!=selectedGame){sim.disconnect();network.reset();selectedGame=desiredGame;telemetry={};wasValid=false;inputDeadline=0;nextConnect=0;++generation;
            std::lock_guard lock(mutex);requests.clear();}
        if(!selectedGame.isEmpty() && !sim.connected() && !network && now>=nextConnect){
            nextConnect=now+2000;
            try{if(selectedGame.startsWith("MSFS"))sim.connect(selectedGame);else network=std::make_unique<NetworkTelemetry>(selectedGame);
                telemetry={};++generation;lastConnectionError.clear();log(gameLabel(selectedGame)+" connection started");}
            catch(const std::exception& e){sim.status="Disconnected / retrying";if(lastConnectionError!=e.what()){lastConnectionError=e.what();log(lastConnectionError);}}
        }
        try {
            if(change) {
                Bundle accepted=*change; QJsonArray acceptedEntries;
                std::vector<std::unique_ptr<LuaEffect>> prepared;
                QSet<QString> retained,reloaded;
                for(auto value:change->project.data["effects"].toArray()) {
                    auto e=value.toObject();auto id=e["id"].toString();auto path=e["script"].toString();
                    if(!change->sources.contains(path))continue;
                    QJsonObject oldEntry;
                    for(auto v:active.project.data["effects"].toArray())if(v.toObject()["id"]==id)oldEntry=v.toObject();
                    auto oldPath=oldEntry["script"].toString();
                    auto old=std::find_if(effects.begin(),effects.end(),[&](const auto& item){return item && item->id==id;});
                    bool sameSource=!change->reloadIds.contains(id) && !oldEntry.isEmpty() && active.sources.value(oldPath)==change->sources.value(path);
                    bool sameSettings=e==oldEntry && change->channelCounts==active.channelCounts;
                    auto keys=change->project.aircraft.keys();for(auto key:active.project.aircraft.keys())if(!keys.contains(key))keys<<key;
                    for(const auto& key:keys)if(change->project.resolved(e,key)!=active.project.resolved(oldEntry,key))sameSettings=false;
                    if(sameSource && sameSettings) {
                        acceptedEntries<<e;
                        if(old!=effects.end()){retained.insert(id);prepared.push_back(std::move(*old));}
                        continue;
                    }
                    try {
                        auto candidate=std::make_unique<LuaEffect>(id,change->sources[path],path);
                        candidate->configure(change->project.resolved(e,QString()),change->channelCounts);
                        for(auto key:change->project.aircraft.keys())candidate->configure(change->project.resolved(e,key),change->channelCounts);
                        candidate->configure(change->project.resolved(e,telemetry.aircraft),change->channelCounts);
                        if(sameSource && old!=effects.end()) {
                            bool keep=(*old)->settings["enabled"]==candidate->settings["enabled"] && (*old)->settings["outputs"]==candidate->settings["outputs"];
                            (*old)->configure(candidate->settings);if(keep)retained.insert(id);candidate=std::move(*old);
                        } else if(!sameSource) reloaded.insert(id);
                        if(!sameSource || !muted.contains(id))prepared.push_back(std::move(candidate));
                        acceptedEntries<<e;
                    } catch(const std::exception& error) {
                        log("ERROR "+id+" (saved script/settings not activated): "+error.what());
                        if(!oldEntry.isEmpty()) {
                            // Keep only this effect's last valid source/settings; other effects still update.
                            oldEntry["script"]=path;acceptedEntries<<oldEntry;accepted.sources[path]=active.sources.value(oldPath);
                            for(auto key:accepted.project.aircraft.keys())accepted.project.setOverride(key,id,active.project.aircraft.value(key)["effects"].toObject()[id].toObject());
                            if(old!=effects.end()){retained.insert(id);prepared.push_back(std::move(*old));}
                        }
                    }
                }
                require(revision==projectRevision.load(),"Discarded update from previous project");
                accepted.project.data["effects"]=acceptedEntries;active=std::move(accepted);effects=std::move(prepared);hasActive=true;
                // Do not consume a concurrent Stop/Resume generation; the next update must reset state.
                { std::lock_guard lock(mutex);
                    for(auto it=requests.begin();it!=requests.end();) {if(!retained.contains(it.key().section('/',0,0)))it=requests.erase(it);else ++it;}
                    for(const auto& id:reloaded){disabled.remove(id);muted.remove(id);}ready=revision==projectRevision.load();
                }

            }
        } catch(const std::exception& e) { log(QString("Update error: ")+e.what()); }
        try {
            auto samples=network?network->poll():sim.poll();
            if(samples.isEmpty() && now-lastUpdate>=maximumUpdateMilliseconds.load()) samples<<telemetry;
            for(auto t:samples) {
                bool fresh=t.sequence!=telemetry.sequence;
                bool reset=t.aircraft!=telemetry.aircraft || t.simTime<telemetry.simTime || generation.load()!=localGeneration;
                telemetry=t; now=nowMs(); bool valid=t.valid(now) && (network?network->connected():sim.connected()); inputDeadline=valid?std::min(t.received+500,t.advancedAt+500):0;
                if(wasValid && !valid) reset=true; wasValid=valid;
                if(reset) { loggedFirstUpdate=false;localGeneration=generation.load(); { std::lock_guard lock(mutex); requests.clear(); } if(hasActive) effects=instantiate(active,t.aircraft); }
                { std::lock_guard lock(mutex); aircraftKey=t.aircraft; execution=running?(hasActive?(valid?"Running":"Running / waiting for valid telemetry"):"Waiting for a project"):"Stopped"; }
                if(running && ready && hasActive && valid) for(auto& effect:effects) {
                    if(!effect || muted.contains(effect->id) || !effect->settings["enabled"].toBool()) continue;
                    try {
                        if(!loggedFirstUpdate){qInfo().noquote()<<"First Lua update in execution generation"<<localGeneration<<"effect"<<effect->id;loggedFirstUpdate=true;}auto commands=effect->update(t,fresh,now); for(const auto& message:effect->takeLogs()) log(effect->id+": "+message);
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
        } catch(const std::exception& e) { log(QString("Runtime: ")+e.what()); sim.disconnect(); network.reset(); telemetry={}; inputDeadline=0; ++generation; }
        { std::lock_guard lock(mutex); simStatus=selectedGame.isEmpty()?"No project":network?network->status:sim.status;latestTelemetry=telemetry;telemetryConnected=network?network->connected():sim.connected(); }
        heartbeat=nowMs(); std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}
static bool stronger(const Request& a,const Request& b) {
    return std::tuple{a.mode==Mode::Constant,a.strength,-a.period,a.on}>std::tuple{b.mode==Mode::Constant,b.strength,-b.period,b.on};
}
void Engine::hidLoop() {
    Hid hid; QMap<QString,QString> winners; QSet<QString> touched,loggedMotors; auto next=std::chrono::steady_clock::now(); qint64 nextScan=0;
    while(!quitting) {
        const int tick=tickMilliseconds.load();next+=std::chrono::milliseconds(tick); bool scan=false;
        if(nowMs()>=nextScan)scan=true;
        if(scan) {
            nextScan=nowMs()+1000;
            try { auto discovered=hid.scan();auto messages=hid.takeLogs(); std::lock_guard lock(mutex);logs.append(messages);
                QSet<QString> occupied; for(auto it=mappings.begin();it!=mappings.end();++it) for(auto v:it.value().toObject()["slots"].toArray()) occupied.insert(v.toString());
                for(auto& motor:discovered) {
                    if(!mappings.contains(motor.id) && !motor.ambiguous) {
                        QJsonArray assigned; for(int i=1;i<=4;++i) { auto s=motor.category+":"+QString::number(i); if(!occupied.contains(s)) { assigned<<s; occupied.insert(s); break; } }
                        mappings[motor.id]=QJsonObject{{"slots",assigned},{"manual",false}};
                    }
                    if(!motor.ambiguous) for(auto v:mappings[motor.id].toObject()["slots"].toArray()) motor.destinations<<v.toString();
                    for(const auto& old:motors) if(old.id==motor.id && old.connected) { motor.sent=old.sent; if(motor.ambiguous)motor.destinations=old.destinations; }
                    bool wasConnected=false;for(const auto& old:motors)if(old.id==motor.id && old.connected)wasConnected=true;
                    if(!wasConnected)logs<<"HID connected: "+motor.label+" ["+motor.id+"]";
                }
                for(auto old:motors) { bool found=false;for(const auto& motor:discovered)if(motor.id==old.id)found=true;if(!found){if(old.connected)logs<<"HID disconnected: "+old.label;old.connected=false;old.sent=0;old.error="Disconnected / mapping reserved";discovered<<old;} }
                motors=discovered;
            } catch(const std::exception& e) { log(QString("HID: ")+e.what()); }
        }
        auto now=nowMs(); QList<Motor> currentMotors; QMap<QString,Request> currentRequests; QString testId; Request test;
        { std::lock_guard lock(mutex);
            for(auto it=requests.begin();it!=requests.end();) if(it->deadline<=now) it=requests.erase(it); else ++it;
            if(!running || now-heartbeat.load()>250 || now>=inputDeadline.load()) requests.clear();
            if(testRequest.deadline<=now) testMotor.clear(); currentMotors=motors; currentRequests=requests; testId=testMotor; test=testRequest;
        }
        auto quantize=[tick](Request& r){if(r.mode==Mode::Pwm){r.on=int(std::floor(double(r.on)/tick+0.5))*tick;r.period=int(std::floor(double(r.period)/tick+0.5))*tick;if(!r.on || !r.period)r.mode=Mode::Off;}};
        quantize(test);for(auto& r:currentRequests)quantize(r);
        for(auto& motor:currentMotors) {
            if(!motor.connected) continue;
            const Request* best=nullptr;
            if(testId==motor.id){if(test.mode!=Mode::Off)best=&test;}
            else for(auto it=currentRequests.begin();it!=currentRequests.end();++it) {
                if(it->mode==Mode::Off)continue;bool routed=false; for(const auto& s:motor.destinations) if(it->destinations.contains(s)) routed=true; if(!routed) continue;
                if(!best || stronger(*it,*best) || (!stronger(*best,*it) && it.key()==winners.value(motor.id))) best=&*it;
            }
            int strength=0; if(best) { winners[motor.id]=best->key; if(best->mode==Mode::Constant || (best->mode==Mode::Pwm && (now-best->phase)%best->period<best->on)) strength=best->strength; } else winners.remove(motor.id);
            // No report is sent merely because a device was opened. Once driven, send zero on release.
            if(strength || touched.contains(motor.id)) {
                if(strength && !loggedMotors.contains(motor.id)){qInfo().noquote()<<"First HID output:"<<motor.id<<"strength"<<strength;loggedMotors.insert(motor.id);}QString error; bool ok=hid.write(motor.id,strength,error);
                // A failed multi-report write can have partially reached the device. Retry zero after reconnect.
                if(strength)touched.insert(motor.id);if(!strength && ok)touched.remove(motor.id);
                std::lock_guard lock(mutex); for(auto& stored:motors) if(stored.id==motor.id) { stored.sent=ok?strength:0; if(!ok) {stored.connected=false;stored.error=error;logs<<motor.label+": "+error;} }
            }
        }
        if(next<std::chrono::steady_clock::now()) next=std::chrono::steady_clock::now(); std::this_thread::sleep_until(next);
    }
    for(const auto& id:touched) {QString error;hid.write(id,0,error);}
}
}
