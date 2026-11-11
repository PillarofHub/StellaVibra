local effect = {}
function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=20, pwm=true}}
    context:output("Airframe")
end
function effect:update(context, telemetry, parameters)
    if context:input_valid() and telemetry.stall_warning then
        context:vibrate("Airframe", parameters.Power, 150)
    else context:vibrate("Airframe", off()) end
end
return effect
