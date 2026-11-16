local effect = {}
local gears = {
    {key="nose", arg="101", output="Center Gear", slot="Stick:1"},
    {key="left", arg="103", output="Left Gear", slot="Throttle:1"},
    {key="right", arg="102", output="Right Gear", slot="Throttle:2"}
}
local epsilon = 0.000001

function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=1, pwm=true, on_ms=20, period_ms=80}}
    for _, gear in ipairs(gears) do context:output(gear.output, {gear.slot}) end
    self.gears = {}
end

function effect:update(context, telemetry, parameters)
    local dcs = telemetry.dcs or {}
    local sim = (telemetry.transport or {}).sim_time
    local rotation = dcs.LoGetAircraftDrawArgumentValue
    local mech = dcs.LoGetMechInfo
    local gear_info = type(mech) == "table" and mech.gear or nil
    local main = type(gear_info) == "table" and gear_info.main or nil
    for _, gear in ipairs(gears) do
        local parent = gear.key == "nose" and gear_info or main
        local source = type(parent) == "table" and parent[gear.key] or nil
        local rod = type(source) == "table" and source.rod or nil
        local value = type(rotation) == "table" and rotation[gear.arg] or nil
        local state = self.gears[gear.key] or {}
        if not context:input_valid() or dcs.LoGetInAir ~= false or type(sim) ~= "number"
            or type(rod) ~= "number" or rod <= epsilon or type(value) ~= "number" then
            state = {}
        elseif context:sample_is_new() then
            -- Rotation phase, not RPM. Cyclic sampling may miss whole revolutions.
            -- A positive rod is an experimental per-gear contact proxy.
            state.rolling = state.value ~= nil and state.sim ~= nil
                and sim > state.sim and sim - state.sim < 0.5
                and math.abs(value - state.value) > epsilon
            state.value, state.sim = value, sim
        end
        self.gears[gear.key] = state
        if state.rolling then context:vibrate(gear.output, parameters.Power, 100)
        else context:vibrate(gear.output, off()) end
    end
end

return effect
