-- Native weapon flags differ by aircraft. No inferred ammunition counters.
local effect = {}
function effect:init(context)
    context:parameter {name="Power",type="haptic_power",default={strength=30,pwm=false}}
    context:parameter {name="Weapon channel",type="int",default=1,min=1,max=4,step=1}
    context:output("Airframe", {"Stick:1"})
end
function effect:update(context,telemetry,parameters)
    local flag = (telemetry.indicators or {})["weapon" .. parameters.Weapon_channel]
    if context:input_valid() and type(flag)=="number" and flag > 0.5 then
        context:vibrate("Airframe",parameters.Power,100)
    else context:vibrate("Airframe",off()) end
end
return effect
