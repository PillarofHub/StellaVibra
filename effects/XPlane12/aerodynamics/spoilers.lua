local effect = {}
function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=20, pwm=false}}
    context:parameter {name="Deployment percent", type="float", default=5, min=0, max=100, step=1}

    context:output("Airframe", {"Stick:1"})
end
function effect:update(context, telemetry, parameters)
    local ratio = telemetry["sim/flightmodel2/controls/speedbrake_ratio"]
    if context:input_valid() and ratio and telemetry["sim/flightmodel/position/indicated_airspeed"] and telemetry["sim/flightmodel/position/indicated_airspeed"] > 20
        and ratio * 100 > parameters.Deployment_percent
        then
        context:vibrate("Airframe", parameters.Power, 100)
    else context:vibrate("Airframe", off()) end
end
return effect
