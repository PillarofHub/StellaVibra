local effect = {}
function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=20, pwm=true}}
    context:parameter {name="Fade ms", type="int", default=3000, min=100, max=30000, step=100}
    self.engines = {}
    for i = 1, 4 do context:output("Engine " .. i); self.engines[i] = {} end
end
function effect:update(context, telemetry, parameters)
    for i = 1, 4 do
        local state = self.engines[i]
        local combustion = telemetry["general_eng_combustion_" .. i]
        local exists = telemetry.number_of_engines and i <= telemetry.number_of_engines
        if context:sample_is_new() and exists and combustion ~= nil then
            if state.combustion == true and not combustion then state.stopped = context:now() end
            if combustion then state.stopped = nil end
            state.combustion = combustion
        end
        local fraction = state.stopped and math.max(0, 1 - (context:now() - state.stopped) / parameters.Fade_ms) or 0
        if context:input_valid() and exists and fraction > 0 then
            local power = parameters.Power
            local strength = math.floor(power.strength * fraction + 0.5)
            local specification = power.pwm and pwm {strength=strength, on_ms=power.on_ms, period_ms=power.period_ms} or constant(strength)
            context:vibrate("Engine " .. i, specification, math.min(150, parameters.Fade_ms * fraction))
        else context:vibrate("Engine " .. i, off()) end
    end
end
return effect
