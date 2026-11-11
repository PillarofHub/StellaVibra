local effect = {}
function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=20, pwm=true}}
    context:parameter {name="Use manual limit", type="boolean", default=false}
    context:parameter {name="Manual limit knots", type="float", default=250, min=20, max=1000, step=1}
    context:output("Airframe")
end
function effect:update(context, telemetry, parameters)
    local limit = parameters.Use_manual_limit and parameters.Manual_limit_knots or telemetry.airspeed_barber_pole
    if context:input_valid() and limit and limit > 0 and telemetry.airspeed_indicated and telemetry.airspeed_indicated > limit then
        context:vibrate("Airframe", parameters.Power, 150)
    else context:vibrate("Airframe", off()) end
end
return effect
