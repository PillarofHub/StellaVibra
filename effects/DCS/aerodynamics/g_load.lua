local effect = {}

function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=25, pwm=false, on_ms=20, period_ms=80}}
    context:parameter {name="Direction", type="enum", default="Positive", choices={"Positive", "Negative", "Both"}}
    context:parameter {name="Start G", type="float", default=2, min=0, max=20, step=0.1}
    context:parameter {name="Full power G", type="float", default=6, min=0.1, max=30, step=0.1}
    context:output("G Load", {"Throttle:1", "Throttle:2"})
end

function effect:update(context, telemetry, parameters)
    local low, high = parameters.Start_G, parameters.Full_power_G
    if high <= low then error("Full power G must exceed Start G") end
    local dcs = telemetry.dcs or {}
    local acceleration = dcs.LoGetAccelerationUnits
    local g = type(acceleration) == "table" and acceleration.y or nil
    if context:input_valid() and dcs.LoGetInAir == true and type(g) == "number" then
        -- Use the native Ny component. Thresholds are magnitudes for Negative/Both.
        local load = parameters.Direction == "Both" and math.abs(g)
            or parameters.Direction == "Negative" and -g or g
        local ratio = math.max(0, math.min(1, (load - low) / (high - low)))
        local power = parameters.Power
        local strength = math.floor(power.strength * ratio + 0.5)
        if strength == 0 then context:vibrate("G Load", off()); return end
        context:vibrate("G Load", {strength=strength,
            pwm=power.pwm, on_ms=power.on_ms, period_ms=power.period_ms}, 100)
    else
        context:vibrate("G Load", off())
    end
end

return effect
