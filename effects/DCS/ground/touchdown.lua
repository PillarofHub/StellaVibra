local effect = {}
local gears = {{key="nose", output="Center Gear"}, {key="left", output="Left Gear"}, {key="right", output="Right Gear"}}

function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=40, pwm=false, on_ms=20, period_ms=80}}
    context:parameter {name="Minimum ms", type="int", default=200, min=20, max=3000, step=1}
    context:parameter {name="Maximum ms", type="int", default=2000, min=20, max=5000, step=1}
    context:parameter {name="Time multiplier", type="float", default=2, min=0.1, max=10, step=0.1}
    context:parameter {name="Rearm ms", type="int", default=200, min=100, max=3000, step=1}
    context:parameter {name="Landing window ms", type="int", default=1500, min=100, max=5000, step=1}
    context:parameter {name="Minimum rod rise", type="float", default=0.0001, min=0, max=1, step=0.0001}
    context:output("Center Gear", {"Stick:1"})
    context:output("Left Gear", {"Throttle:1"})
    context:output("Right Gear", {"Throttle:2"})
    self.gears = {}
end

function effect:update(context, telemetry, parameters)
    local minimum, maximum = parameters.Minimum_ms, parameters.Maximum_ms
    if maximum < minimum then error("Maximum ms must be >= Minimum ms") end
    local dcs = telemetry.dcs or {}
    local air = dcs.LoGetInAir
    local sim = (telemetry.transport or {}).sim_time
    local mech = dcs.LoGetMechInfo
    local main = type(mech) == "table" and type(mech.gear) == "table" and mech.gear.main or nil
    local now = context:now()
    if not context:input_valid() or type(air) ~= "boolean" or type(sim) ~= "number" then
        self.gears, self.previous_air, self.landing_until = {}, nil, nil
        for _, gear in ipairs(gears) do context:vibrate(gear.output, off()) end
        return
    end
    local fresh = context:sample_is_new()
    if fresh and not air and self.previous_air == true then
        self.landing_until = now + parameters.Landing_window_ms
    end
    for _, gear in ipairs(gears) do
        local source = type(main) == "table" and main[gear.key] or nil
        local rod = type(source) == "table" and source.rod or nil
        local state = self.gears[gear.key] or {}
        if type(rod) ~= "number" then
            state = {} -- Missing rods do not become zero or a fallback touchdown.
        elseif fresh then
            if air then
                state.air_since = state.air_since or now
                if state.rod ~= nil and now - state.air_since >= parameters.Rearm_ms then
                    state.armed, state.rising, state.deadline = true, false, nil
                end
            else
                state.air_since = nil
                if not self.landing_until or now > self.landing_until then state.armed = false end
                local rising = state.rod ~= nil and rod > state.rod + parameters.Minimum_rod_rise
                if state.armed and rising and state.sim ~= nil and sim > state.sim then
                    -- Native rod increase is an experimental compression proxy, not force.
                    state.armed, state.rising = false, true
                    state.started, state.start_sim = now, state.sim
                    local duration = math.min(maximum, math.max(minimum, (sim - state.start_sim)*1000*parameters.Time_multiplier))
                    state.deadline = now + duration
                    context:log(gear.output .. " first rod rise")
                elseif state.rising and state.sim ~= nil and sim > state.sim then
                    if rising and now - state.started < maximum then
                        local duration = math.min(maximum, math.max(minimum, (sim - state.start_sim)*1000*parameters.Time_multiplier))
                        state.deadline = state.started + duration
                    else
                        state.rising = false -- Later peaks cannot restart this landing.
                    end
                end
            end
            state.rod, state.sim = rod, sim
        end
        self.gears[gear.key] = state
        local remaining = state.deadline and state.deadline - now or 0
        if remaining > 0 then context:vibrate(gear.output, parameters.Power, remaining)
        else context:vibrate(gear.output, off()) end
    end
    if fresh then self.previous_air = air end
end

return effect
