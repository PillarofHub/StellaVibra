-- Horsepower-scaled vibration. WT does not provide SimConnect thrust in pounds.
local effect = {}
function effect:init(context)
    context:parameter {name="Power",type="haptic_power",default={strength=20,pwm=false}}
    context:parameter {name="Full scale hp",type="float",default=1500,min=1,max=100000,step=100}
    context:output("Engine 1", {"Throttle:1"})
end
function effect:update(context,telemetry,parameters)
    local hp=(telemetry.state or {})["power 1, hp"]
    if context:input_valid() and hp and hp > 0 then
        local p = {}
        for key, value in pairs(parameters.Power) do p[key] = value end
        local strength=math.floor(p.strength*math.min(1,hp/parameters.Full_scale_hp)+0.5)
        p.strength = strength
        context:vibrate("Engine 1", p, 100)
    else context:vibrate("Engine 1",off()) end
end
return effect
