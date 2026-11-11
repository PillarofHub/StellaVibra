-- Standard SimVars only; no aircraft-specific starter L-vars.
local effect = {}
function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=20, pwm=true}}
    context:parameter {name="Ignition pulse ms", type="int", default=400, min=20, max=3000, step=1}
    context:parameter {name="Startup limit ms", type="int", default=30000, min=1000, max=60000, step=100}
    self.engines = {}
    for i = 1, 4 do context:output("Engine " .. i); self.engines[i] = {} end
end
function effect:update(context, telemetry, parameters)
    for i = 1, 4 do
        local state = self.engines[i]
        local starter = telemetry["general_eng_starter_" .. i]
        local combustion = telemetry["general_eng_combustion_" .. i]
        local exists = telemetry.number_of_engines and i <= telemetry.number_of_engines
        if context:sample_is_new() and exists and starter ~= nil and combustion ~= nil then
            -- First sample only establishes state, never invents a startup edge.
            if state.starter == false and starter then state.started = context:now() end
            if state.combustion == false and combustion then state.ignition = context:now() + parameters.Ignition_pulse_ms end
            if not starter then state.started = nil end
            state.starter, state.combustion = starter, combustion
        end
        local cranking = state.started and starter and context:now() - state.started < parameters.Startup_limit_ms
        if context:input_valid() and exists and (cranking or (state.ignition and state.ignition > context:now())) then
            context:vibrate("Engine " .. i, parameters.Power, 150)
        else context:vibrate("Engine " .. i, off()) end
    end
end
return effect
