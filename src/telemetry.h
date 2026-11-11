#pragma once
#include "model.h"
namespace sv {
struct SimVariable { QString key,name,unit; bool boolean=false; };
// One list drives registration, decoding, completion and the live inspector.
inline const QList<SimVariable>& simVariables() {
    static const QList<SimVariable> fields=[] {
        QList<SimVariable> f{
            {"sim_time_s","ABSOLUTE TIME","Seconds"},
            {"ground_speed_knots","GROUND VELOCITY","Knots"},
            {"on_ground","SIM ON GROUND","Bool",true},
            {"gear_contact_0","GEAR IS ON GROUND:0","Bool",true},
            {"gear_contact_1","GEAR IS ON GROUND:1","Bool",true},
            {"gear_contact_2","GEAR IS ON GROUND:2","Bool",true},
            {"gear_animation_0","GEAR ANIMATION POSITION:0","Percent"},
            {"gear_animation_1","GEAR ANIMATION POSITION:1","Percent"},
            {"gear_animation_2","GEAR ANIMATION POSITION:2","Percent"},
            {"airspeed_indicated","AIRSPEED INDICATED","Knots"},
            {"airspeed_barber_pole","AIRSPEED BARBER POLE","Knots"},
            {"plane_bank_degrees","PLANE BANK DEGREES","Degrees"},
            {"stall_warning","STALL WARNING","Bool",true},
            {"spoilers_left_position","SPOILERS LEFT POSITION","Percent"},
            {"spoilers_right_position","SPOILERS RIGHT POSITION","Percent"},
            {"trailing_edge_flaps_left_percent","TRAILING EDGE FLAPS LEFT PERCENT","Percent"},
            {"trailing_edge_flaps_right_percent","TRAILING EDGE FLAPS RIGHT PERCENT","Percent"},
            {"leading_edge_flaps_left_percent","LEADING EDGE FLAPS LEFT PERCENT","Percent"},
            {"leading_edge_flaps_right_percent","LEADING EDGE FLAPS RIGHT PERCENT","Percent"},
            {"number_of_engines","NUMBER OF ENGINES","Number"}
        };
        for(int i=1;i<=4;++i)for(const auto& field:QList<SimVariable>{
            {"general_eng_combustion","GENERAL ENG COMBUSTION","Bool",true},
            {"general_eng_starter","GENERAL ENG STARTER","Bool",true},
            {"general_eng_pct_max_rpm","GENERAL ENG PCT MAX RPM","Percent"},
            {"turb_eng_n1","TURB ENG N1","Percent"},
            {"turb_eng_n2","TURB ENG N2","Percent"},
            {"turb_eng_jet_thrust","TURB ENG JET THRUST","Pounds"},
            {"prop_thrust","PROP THRUST","Pounds"}
        })f.append({field.key+"_"+QString::number(i),field.name+":"+QString::number(i),field.unit,field.boolean});
        return f;
    }();return fields;
}
inline QJsonObject telemetryObject(const Telemetry& t) {
    if(!t.sequence)return {};
    auto result=t.values;QJsonArray gears;
    for(int i=0;i<3;++i)gears.append(QJsonObject{{"contact",t.contact[i]},{"animation_percent",t.animation[i]}});
    result["sample_id"]=double(t.sequence);result["received_ms"]=double(t.received);
    result["sim_time_advanced"]=t.advanced;result["aircraft_key"]=t.aircraft;
    result["gears"]=gears;result["msfs"]=QJsonObject{{"paused",t.paused}};return result;
}
inline QString parameterAlias(QString name) {name.replace(QRegularExpression("\\s"),"_");return name;}
inline bool luaIdentifier(const QString& name) {
    static const QStringList keywords=QString("and break do else elseif end false for function goto if in local nil not or repeat return then true until while").split(' ');
    return QRegularExpression("^[A-Za-z_][A-Za-z_0-9]*$").match(name).hasMatch() && !keywords.contains(name);
}
}
