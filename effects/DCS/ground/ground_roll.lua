local effect = {}

function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=1, pwm=true, on_ms=20, period_ms=80}}
    context:parameter {name="Minimum speed", type="float", default=0.5, min=0, max=100, step=0.1}
    context:output("Ground", {"Stick:1", "Throttle:1", "Throttle:2"})
end

function effect:update(context, telemetry, parameters)
    local dcs = telemetry.dcs or {}
    local velocity = dcs.LoGetVectorVelocity
    -- Horizontal world velocity in native DCS units, not carrier-relative speed.
    if context:input_valid() and dcs.LoGetInAir == false and type(velocity) == "table"
        and type(velocity.x) == "number" and type(velocity.z) == "number"
        and math.sqrt(velocity.x^2 + velocity.z^2) > parameters.Minimum_speed then
        context:vibrate("Ground", parameters.Power, 100)
    else
        context:vibrate("Ground", off())
    end
end

return effect
