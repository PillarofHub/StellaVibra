local effect = {}
function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=20, pwm=false}}
    context:output("Airframe", {"Stick:1"})
end
function effect:update(context, telemetry, parameters)
    local limit = telemetry["sim/aircraft/view/acf_Vne"]
    if context:input_valid() and limit and limit > 0 and telemetry["sim/flightmodel/position/indicated_airspeed"] and telemetry["sim/flightmodel/position/indicated_airspeed"] > limit then
        context:vibrate("Airframe", parameters.Power, 100)
    else context:vibrate("Airframe", off()) end
end
return effect
