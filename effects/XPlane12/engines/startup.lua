-- X-Plane native datarefs. Starter and combustion transitions are evaluated in Lua.
local effect = {}
function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=20, pwm=false}}
    context:parameter {name="Ignition pulse ms", type="int", default=400, min=20, max=3000, step=1}
    context:parameter {name="Startup limit ms", type="int", default=30000, min=1000, max=60000, step=100}
    context:output("Engine 1", {"Throttle:1"})
    context:output("Engine 2", {"Throttle:2"})
    context:output("Engine 3", {"Throttle:3"})
    context:output("Engine 4", {"Throttle:4"})
    self.engines = {{}, {}, {}, {}}
end
function effect:update(context, telemetry, parameters)
    for i = 1, 4 do
        local state = self.engines[i]
        local raw_starter = telemetry["sim/cockpit2/engine/actuators/starter_hit[" .. (i - 1) .. "]"]
        local starter = raw_starter ~= nil and raw_starter > 0
        local raw_combustion = telemetry["sim/flightmodel2/engines/engine_is_burning_fuel[" .. (i - 1) .. "]"]
        local combustion = raw_combustion ~= nil and raw_combustion > 0
        local exists = telemetry["sim/aircraft/engine/acf_num_engines"] and i <= telemetry["sim/aircraft/engine/acf_num_engines"]
        if context:sample_is_new() and exists and raw_starter ~= nil and raw_combustion ~= nil then
            -- First sample only establishes state, never invents a startup edge.
            if state.starter == false and starter then state.started = context:now() end
            if state.combustion == false and combustion then state.ignition = context:now() + parameters.Ignition_pulse_ms end
            if not starter then state.started = nil end
            state.starter, state.combustion = starter, combustion
        end
        local cranking = state.started and starter and context:now() - state.started < parameters.Startup_limit_ms
        if context:input_valid() and exists and raw_starter ~= nil and raw_combustion ~= nil and (cranking or (state.ignition and state.ignition > context:now())) then
            context:vibrate("Engine " .. i, parameters.Power, 100)
        else context:vibrate("Engine " .. i, off()) end
    end
end
return effect
