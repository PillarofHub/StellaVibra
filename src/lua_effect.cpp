#include "lua_effect.h"
#include "telemetry.h"
#include <cmath>
#include <cstdlib>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
namespace sv {
void* LuaEffect::allocate(void* ud,void* pointer,size_t oldSize,size_t newSize) {
    auto* self=static_cast<LuaEffect*>(ud); if(!pointer) oldSize=0;
    if(!newSize) { self->allocated-=oldSize; free(pointer); return nullptr; }
    if(newSize>8*1024*1024 || self->allocated-oldSize+newSize>8*1024*1024) return nullptr;
    auto* result=realloc(pointer,newSize); if(result) self->allocated=self->allocated-oldSize+newSize; return result;
}
void LuaEffect::hook(lua_State* L,lua_Debug*) {
    auto* self=*static_cast<LuaEffect**>(lua_getextraspace(L)); self->budget-=1000;
    if(self->budget<=0 || nowMs()-self->started>20) luaL_error(L,"Lua instruction/time limit exceeded");
}
void LuaEffect::protectedCall(int args,int results) {
    budget=200000; started=nowMs(); lua_sethook(state,hook,LUA_MASKCOUNT,1000);
    int status=lua_pcall(state,args,results,0); lua_sethook(state,nullptr,0,0);
    if(status!=LUA_OK) { auto error=QString::fromUtf8(lua_tostring(state,-1)); lua_pop(state,1); throw std::runtime_error(error.toStdString()); }
}
LuaEffect::LuaEffect(QString identity,QString source,QString filename):id(std::move(identity)) {
    require(source.toUtf8().size()<=256*1024,"Lua source exceeds 256 KiB");
    state=lua_newstate(allocate,this); require(state!=nullptr,"Cannot allocate Lua state");
    *static_cast<LuaEffect**>(lua_getextraspace(state))=this;
    sourceText=std::move(source); sourceName=std::move(filename);
    try {
        lua_pushcfunction(state,[](lua_State* L)->int {
            auto* self=*static_cast<LuaEffect**>(lua_getextraspace(L));
            try { self->initialize(); return 0; }
            catch(const std::exception& e) { self->callbackError=e.what(); }
            return luaL_error(L,"%s",self->callbackError.constData());
        });
        protectedCall(0,0);
    } catch(...) { lua_close(state); state=nullptr; throw; }
}
void LuaEffect::initialize() {
        for(auto lib: {std::pair{"_G",luaopen_base}, {"math",luaopen_math},{"string",luaopen_string},{"table",luaopen_table},{"utf8",luaopen_utf8}}) { luaL_requiref(state,lib.first,lib.second,1); lua_pop(state,1); }
        for(auto name:{"dofile","loadfile","load","collectgarbage","print","warn","pcall","xpcall","setmetatable","getmetatable"}) { lua_pushnil(state); lua_setglobal(state,name); }
        const char* helpers="function constant(s) return {mode='constant',strength=s} end function pwm(s) s.mode='pwm'; return s end function off() return {mode='off'} end";
        require(luaL_loadstring(state,helpers)==LUA_OK,"Cannot load helpers"); lua_call(state,0,0);
        auto bytes=sourceText.toUtf8(); auto name=("@"+sourceName).toUtf8();
        if(luaL_loadbufferx(state,bytes.constData(),bytes.size(),name.constData(),"t")!=LUA_OK) { auto e=QString::fromUtf8(lua_tostring(state,-1)); throw std::runtime_error(e.toStdString()); }
        lua_call(state,0,1); require(lua_istable(state,-1),"Script must return an effect table"); reference=luaL_ref(state,LUA_REGISTRYINDEX);
        lua_rawgeti(state,LUA_REGISTRYINDEX,reference); lua_getfield(state,-1,"init"); require(lua_isfunction(state,-1),"Missing init(context)"); lua_pushvalue(state,-2); registering=true; pushApi(); lua_call(state,2,0); registering=false; lua_pop(state,1);
        lua_rawgeti(state,LUA_REGISTRYINDEX,reference); lua_getfield(state,-1,"update"); require(lua_isfunction(state,-1),"Missing update(context, telemetry, parameters)"); lua_pop(state,2);
}
LuaEffect::~LuaEffect() { if(state) lua_close(state); }
void LuaEffect::pushApi() {
    lua_newtable(state); int operation=0;
    for(auto name:{"parameter","output","log","vibrate","now","elapsed","sample_is_new","input_valid"}) { const int code=operation++;if(registering?code>2:code<2)continue;lua_pushlightuserdata(state,this); lua_pushinteger(state,code); lua_pushcclosure(state,dispatch,2); lua_setfield(state,-2,name); }
}
int LuaEffect::dispatch(lua_State* L) {
    auto* self=static_cast<LuaEffect*>(lua_touserdata(L,lua_upvalueindex(1)));
    try { return self->api(L,static_cast<int>(lua_tointeger(L,lua_upvalueindex(2)))); }
    catch(const std::exception& e) { self->callbackError=e.what(); }
    lua_pushstring(L,self->callbackError.constData()); return lua_error(L);
}
static QString stringAt(lua_State* L,int i) { require(lua_type(L,i)==LUA_TSTRING,"Expected string"); size_t size=0; auto p=lua_tolstring(L,i,&size); require(size>0 && size<=512,"Invalid string length"); return QString::fromUtf8(p,static_cast<qsizetype>(size)); }
static double numberAt(lua_State* L,int i) { require(lua_type(L,i)==LUA_TNUMBER && std::isfinite(lua_tonumber(L,i)),"Expected finite number"); return lua_tonumber(L,i); }
static double numberField(lua_State* L,int i,const char* name) { lua_getfield(L,i,name); auto v=numberAt(L,-1); lua_pop(L,1); return v; }
int LuaEffect::api(lua_State* L,int operation) {
    if(operation>=4){require(!registering,"Update information is unavailable in init");if(operation==4)lua_pushnumber(L,double(current));else if(operation==5)lua_pushnumber(L,double(previous?current-previous:0));else if(operation==6)lua_pushboolean(L,updateFresh);else lua_pushboolean(L,updateTelemetry.valid(current));return 1;}
    if(operation==0) {
        require(registering && lua_istable(L,2) && parameters.size()<64,"Invalid parameter registration"); Parameter p;
        lua_getfield(L,2,"name"); p.name=stringAt(L,-1); lua_pop(L,1);
        for(const auto& old:parameters) require(old.name!=p.name,"Duplicate parameter");
        lua_getfield(L,2,"type"); if(!lua_isnil(L,-1)) p.type=stringAt(L,-1); lua_pop(L,1);
        lua_getfield(L,2,"default");
        require(p.type=="boolean" || p.type=="int" || p.type=="float" || p.type=="haptic_power" || p.type=="enum","Unknown parameter type");
        p.boolean=p.type=="boolean";
        if(p.boolean) { require(lua_isboolean(L,-1),"Expected boolean default"); p.initial=bool(lua_toboolean(L,-1)); }
        else if(p.type=="enum") {
            p.initial=stringAt(L,-1);lua_getfield(L,2,"choices");require(lua_istable(L,-1),"Expected enum choices");
            const auto count=lua_rawlen(L,-1);require(count>0 && count<=64,"Expected 1..64 enum choices");
            const int choices=lua_absindex(L,-1);size_t entries=0;lua_pushnil(L);
            while(lua_next(L,choices)){require(lua_isinteger(L,-2) && lua_tointeger(L,-2)>=1 && lua_tointeger(L,-2)<=static_cast<lua_Integer>(count),"Enum choices must be a dense array");++entries;lua_pop(L,1);}
            require(entries==count,"Enum choices must be a dense array");
            for(size_t i=1;i<=count;++i){lua_rawgeti(L,-1,static_cast<lua_Integer>(i));auto choice=stringAt(L,-1);lua_pop(L,1);require(!p.choices.contains(choice),"Duplicate enum choice");p.choices<<choice;}
            lua_pop(L,1);require(p.choices.contains(p.initial.toString()),"Enum default is not a choice");
        } else if(p.type=="haptic_power") {
            require(lua_istable(L,-1),"haptic_power default must be a table");
            int table=lua_absindex(L,-1); QJsonObject h;
            for(auto field:{"strength","on_ms","period_ms"}) { lua_getfield(L,table,field); h[field]=lua_isnil(L,-1)?(QString(field)=="period_ms"?80:QString(field)=="on_ms"?20:0):numberAt(L,-1); lua_pop(L,1); }
            lua_getfield(L,table,"pwm"); require(lua_isnil(L,-1)||lua_isboolean(L,-1),"Expected PWM boolean"); h["pwm"]=bool(lua_toboolean(L,-1)); lua_pop(L,1); p.initial=h;
        } else p.initial=numberAt(L,-1);
        lua_pop(L,1);
        if(!p.boolean && p.type!="enum") {
            if(p.type=="haptic_power") {
                lua_getfield(L,2,"min"); p.minimum=lua_isnil(L,-1)?0:numberAt(L,-1); lua_pop(L,1);
                lua_getfield(L,2,"max"); p.maximum=lua_isnil(L,-1)?255:numberAt(L,-1); lua_pop(L,1);
                validateSettings({{"enabled",true},{"parameters",QJsonObject{{p.name,p.initial}}}},false);
                require(p.minimum>=0 && p.maximum<=255,"Haptic strength range must be within 0..255");
            } else { p.minimum=numberField(L,2,"min"); p.maximum=numberField(L,2,"max"); p.step=numberField(L,2,"step"); }
            double initial=p.type=="haptic_power"?p.initial.toObject()["strength"].toDouble():p.initial.toDouble();
            require(p.minimum<=p.maximum && p.step>0 && initial>=p.minimum && initial<=p.maximum && (p.maximum-p.minimum)/p.step<=1000000,"Invalid parameter range");
            if(p.type=="int" || p.type=="haptic_power") require(floor(initial)==initial && floor(p.minimum)==p.minimum && floor(p.maximum)==p.maximum && floor(p.step)==p.step,"Expected integer range/default/step");
        }
        auto alias=parameterAlias(p.name);
        require(luaIdentifier(alias),"Parameter name must form a Lua identifier after replacing spaces: "+alias);
        for(const auto& old:parameters)require(parameterAlias(old.name)!=alias,"Parameter access name collision: "+alias);
        parameters<<p; return 0;
    }
    if(operation==1) {
        require(registering && outputs.size()<64,"Output registration only in init");
        require(lua_gettop(L)<=3,"Expected output(name, optional default slots)");
        auto name=stringAt(L,2);require(!outputs.contains(name),"Duplicate output");QJsonArray defaults;
        if(!lua_isnoneornil(L,3)) {
            require(lua_istable(L,3),"Default output slots must be an array");
            auto count=lua_rawlen(L,3);require(count<=20,"Too many default output slots");
            size_t entries=0;lua_pushnil(L);
            while(lua_next(L,3)) {
                require(lua_isinteger(L,-2) && lua_tointeger(L,-2)>=1 && lua_tointeger(L,-2)<=static_cast<lua_Integer>(count),"Default output slots must be a dense array");
                ++entries;lua_pop(L,1);
            }
            require(entries==count,"Default output slots must be a dense array");
            for(size_t i=1;i<=count;++i){lua_rawgeti(L,3,static_cast<lua_Integer>(i));auto slot=stringAt(L,-1);lua_pop(L,1);
                require(logicalSlots().contains(slot),"Unknown default output slot: "+slot);require(!defaults.contains(slot),"Duplicate default output slot: "+slot);defaults.append(slot);}
        }
        outputs<<name;outputDefaults[name]=defaults;return 0;
    }
    if(operation==2) { if(logs.size()<8) logs<<stringAt(L,2); return 0; }
    require(!registering && pending.size()<128,"Invalid output call"); Request r; auto name=stringAt(L,2); require(outputs.contains(name),"Unknown output: "+name); r.key=id+"/"+name;
    require(lua_istable(L,3),"Expected output specification"); lua_getfield(L,3,"mode"); QString mode;
    if(lua_isnil(L,-1)) {
        lua_pop(L,1);lua_getfield(L,3,"pwm");require(lua_isboolean(L,-1),"Expected output mode or haptic_power PWM boolean");
        mode=lua_toboolean(L,-1)?"pwm":"constant";
    } else mode=stringAt(L,-1);
    lua_pop(L,1);
    if(mode=="off") { pending<<r; return 0; }
    require(mode=="constant" || mode=="pwm","Unknown output mode"); r.mode=mode=="constant"?Mode::Constant:Mode::Pwm;
    auto strength=numberField(L,3,"strength"); require(strength>=0 && strength<=255 && floor(strength)==strength,"Strength must be integer 0..255"); r.strength=static_cast<int>(strength);
    auto duration=numberAt(L,4); require(duration>0 && duration<=60000,"Duration must be 0..60000 ms"); r.deadline=current+static_cast<qint64>(duration); r.phase=current;
    if(r.mode==Mode::Pwm) { auto on=numberField(L,3,"on_ms"), period=numberField(L,3,"period_ms"); require(period>=20 && period<=60000 && on>=0 && on<=period,"Invalid PWM timing"); r.on=static_cast<int>(on); r.period=static_cast<int>(period); }
    for(auto v:settings["outputs"].toObject()[name].toArray()) r.destinations<<v.toString();
    pending<<r; return 0;
}
void LuaEffect::configure(const QJsonObject& values,const QJsonObject& channelCounts) {
    validateSettings(values,false); settings=values; unresolved.clear(); auto stored=values["parameters"].toObject(); QJsonObject merged; QSet<QString> names;
    for(const auto& p:parameters) {
        auto value=stored.contains(p.name)?stored[p.name]:p.initial;
        auto number=p.type=="haptic_power"?value.toObject()["strength"]:value;
        require(p.type=="enum" ? (value.isString() && p.choices.contains(value.toString())) : p.boolean ? value.isBool() : (number.isDouble() && number.toDouble()>=p.minimum && number.toDouble()<=p.maximum && (p.type=="float" || floor(number.toDouble())==number.toDouble())),"Invalid value: "+p.name);
        if(p.type=="haptic_power") require(value.isObject(),"Expected haptic_power object");
        merged[p.name]=value; names.insert(p.name);
    }
    for(auto it=stored.begin();it!=stored.end();++it) if(!names.contains(it.key())) unresolved<<"Parameter: "+it.key();
    auto routes=values["outputs"].toObject(); for(auto it=routes.begin();it!=routes.end();++it) if(!outputs.contains(it.key())) unresolved<<"Output: "+it.key(); settings["parameters"]=merged;
    const auto defaults=fitOutputDefaults(outputDefaults,channelCounts);
    for(const auto& name:outputs)if(!routes.contains(name))routes[name]=defaults[name];
    settings["outputs"]=routes;
}
void LuaEffect::pushJson(const QJsonValue& v) {
    if(v.isBool()) lua_pushboolean(state,v.toBool()); else if(v.isDouble()) lua_pushnumber(state,v.toDouble()); else if(v.isString()) { auto b=v.toString().toUtf8(); lua_pushlstring(state,b.constData(),b.size()); }
    else if(v.isObject()) { lua_newtable(state); auto o=v.toObject(); for(auto it=o.begin();it!=o.end();++it) { pushJson(it.value()); lua_setfield(state,-2,it.key().toUtf8().constData()); } }
    else if(v.isArray()) { lua_newtable(state); int i=1; for(auto item:v.toArray()) { pushJson(item); lua_rawseti(state,-2,i++); } } else lua_pushnil(state);
}
QList<Request> LuaEffect::update(const Telemetry& t,bool fresh,qint64 now) {
    current=now; updateTelemetry=t; updateFresh=fresh; pending.clear(); logs.clear(); lua_settop(state,0);
    lua_pushcfunction(state,[](lua_State* L)->int {
        auto* self=*static_cast<LuaEffect**>(lua_getextraspace(L));
        self->performUpdate(); return 0;
    });
    protectedCall(0,0); previous=now; return pending;
}
void LuaEffect::performUpdate() {
    const auto& t=updateTelemetry; const bool fresh=updateFresh;
    lua_rawgeti(state,LUA_REGISTRYINDEX,reference); lua_getfield(state,-1,"update"); lua_pushvalue(state,-2); pushApi();
    auto telemetry=telemetryObject(t);if(t.game.startsWith("MSFS"))telemetry["sim_time_advanced"]=fresh && t.advanced;
    QJsonObject values;const auto stored=settings["parameters"].toObject();
    for(const auto& p:parameters)values[parameterAlias(p.name)]=stored[p.name];
    pushJson(telemetry);pushJson(values);
    lua_call(state,4,0);
}
QStringList LuaEffect::takeLogs() { auto result=logs; logs.clear(); return result; }
}
