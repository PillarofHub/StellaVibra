local effect = {}
function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=20, pwm=false}}
    context:parameter {name="Deployment percent", type="float", default=5, min=0, max=100, step=1}
    context:parameter {name="Symmetry tolerance percent", type="float", default=10, min=0, max=100, step=1}
    context:output("Airframe", {"Stick:1"})
end
function effect:update(context, telemetry, parameters)
    local left, right = telemetry.spoilers_left_position, telemetry.spoilers_right_position
    if context:input_valid() and left and right and telemetry.airspeed_indicated and telemetry.airspeed_indicated > 20
        and math.min(left, right) > parameters.Deployment_percent
        and math.abs(left - right) <= parameters.Symmetry_tolerance_percent then
        context:vibrate("Airframe", parameters.Power, 100)
    else context:vibrate("Airframe", off()) end
end
return effect
