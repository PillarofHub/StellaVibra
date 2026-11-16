local effect = {}

function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=2, pwm=false, on_ms=20, period_ms=80}}
    context:parameter {name="Deployment threshold", type="float", default=0.01, min=0, max=1, step=0.01}
    context:output("Airbrake", {"Throttle:1", "Throttle:2"})
end

function effect:update(context, telemetry, parameters)
    local dcs = telemetry.dcs or {}
    local mech = dcs.LoGetMechInfo
    local brake = type(mech) == "table" and mech.speedbrakes or nil
    local value = type(brake) == "table" and brake.value or nil
    -- Synthetic deployment feedback, not measured buffet or a universal spoiler signal.
    if context:input_valid() and dcs.LoGetInAir == true and type(value) == "number"
        and value > parameters.Deployment_threshold then
        context:vibrate("Airbrake", parameters.Power, 100)
    else
        context:vibrate("Airbrake", off())
    end
end

return effect
