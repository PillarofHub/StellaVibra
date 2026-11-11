-- Aircraft-specific speed limits are not present in /state. Set a manual limit.
local effect = {}
function effect:init(context)
    context:parameter {name="Power",type="haptic_power",default={strength=20,pwm=false}}
    context:parameter {name="Limit IAS kmh",type="float",default=700,min=50,max=3000,step=5}
    context:output("Airframe", {"Stick:1"})
end
function effect:update(context,telemetry,parameters)
    local speed = (telemetry.state or {})["IAS, km/h"]
    if context:input_valid() and speed and speed > parameters.Limit_IAS_kmh then
        context:vibrate("Airframe",parameters.Power,100)
    else context:vibrate("Airframe",off()) end
end
return effect
