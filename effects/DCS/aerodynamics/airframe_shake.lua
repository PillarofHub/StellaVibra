local effect = {}

function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=30, pwm=false, on_ms=20, period_ms=80}}
    context:parameter {name="Minimum amplitude", type="float", default=0, min=0, max=10, step=0.01}
    context:parameter {name="Full power amplitude", type="float", default=1, min=0.01, max=10, step=0.01}
    context:output("Airframe", {"Stick:1"})
end

function effect:update(context, telemetry, parameters)
    local low, high = parameters.Minimum_amplitude, parameters.Full_power_amplitude
    if high <= low then error("Full power amplitude must exceed Minimum amplitude") end
    local amplitude = (telemetry.dcs or {}).LoGetShakeAmplitude
    if context:input_valid() and type(amplitude) == "number" and amplitude > low then
        -- Native shake has no guaranteed scale or separation into engine/buffet sources.
        local ratio = math.min(1, (amplitude - low) / (high - low))
        local power = parameters.Power
        local strength = math.floor(power.strength * ratio + 0.5)
        if strength == 0 then context:vibrate("Airframe", off()); return end
        context:vibrate("Airframe", {strength=strength,
            pwm=power.pwm, on_ms=power.on_ms, period_ms=power.period_ms}, 100)
    else
        context:vibrate("Airframe", off())
    end
end

return effect
