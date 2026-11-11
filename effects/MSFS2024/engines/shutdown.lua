local effect = {}
function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=20, pwm=false}}
    context:parameter {name="Fade ms", type="int", default=3000, min=100, max=30000, step=100}
    context:output("Engine 1", {"Throttle:1"})
    context:output("Engine 2", {"Throttle:2"})
    context:output("Engine 3", {"Throttle:3"})
    context:output("Engine 4", {"Throttle:4"})
    self.engines = {{}, {}, {}, {}}
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
            local power = {}
            for key, value in pairs(parameters.Power) do power[key] = value end
            local strength = math.floor(power.strength * fraction + 0.5)
            power.strength = strength
            context:vibrate("Engine " .. i, power, math.min(100, parameters.Fade_ms * fraction))
        else context:vibrate("Engine " .. i, off()) end
    end
end
return effect
