local effect = {}

local function count(value)
    if type(value) == "number" and value >= 0 and value < math.huge and value % 1 == 0 then return value end
end

local function inventory(payload)
    local stations = {}
    if type(payload.Stations) ~= "table" then return stations end
    for id, station in pairs(payload.Stations) do
        if type(station) == "table" and type(station.weapon) == "table" then
            local weapon, parts = station.weapon, {}
            for _, key in ipairs({"level1", "level2", "level3", "level4"}) do
                local value = count(weapon[key])
                if value == nil then break end
                parts[#parts + 1] = tostring(value)
            end
            local remaining = count(station.count)
            if #parts == 4 and remaining ~= nil then
                stations[id] = {kind=table.concat(parts, ":") .. ":" .. tostring(station.container), count=remaining}
            end
        end
    end
    return stations
end

function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=40, pwm=false, on_ms=20, period_ms=80}}
    context:parameter {name="Gun ms", type="int", default=80, min=20, max=1000, step=1}
    context:parameter {name="Release ms", type="int", default=150, min=20, max=2000, step=1}
    context:output("Gun", {"Stick:1"})
    context:output("Release", {"Stick:1"})
end

function effect:update(context, telemetry, parameters)
    local payload = (telemetry.dcs or {}).LoGetPayloadInfo
    local now = context:now()
    if not context:input_valid() or type(payload) ~= "table" then
        self.stations, self.shells, self.gun_end, self.release_end, self.settle_until = nil, nil, nil, nil, nil
        context:vibrate("Gun", off()); context:vibrate("Release", off()); return
    end
    if context:sample_is_new() then
        local stations = inventory(payload)
        local shells = type(payload.Cannon) == "table" and count(payload.Cannon.shells) or nil
        local gun, release, changed = false, false, false
        if shells ~= nil and self.shells ~= nil then
            gun = shells < self.shells
            changed = shells > self.shells
        end
        if self.stations then
            for id, station in pairs(stations) do
                local old = self.stations[id]
                if not old or old.kind ~= station.kind or station.count > old.count then
                    changed = true
                elseif station.count < old.count then release = true end
            end
        end
        -- A missing station is unavailable, never a synthesized zero or release.
        if changed then
            self.settle_until = now + 500
            self.gun_end, self.release_end = nil, nil
        elseif not self.settle_until or now >= self.settle_until then
            if gun then self.gun_end = now + parameters.Gun_ms end
            if release then self.release_end = now + parameters.Release_ms end
        end
        self.stations, self.shells = stations, shells
        if shells == nil then self.gun_end = nil end
        if type(payload.Stations) ~= "table" then self.release_end = nil end
    end
    -- Repeated snapshots retain event deadlines; new decreases may extend a burst.
    local gun_ms = self.gun_end and self.gun_end - now or 0
    local release_ms = self.release_end and self.release_end - now or 0
    if gun_ms > 0 then context:vibrate("Gun", parameters.Power, gun_ms) else context:vibrate("Gun", off()) end
    if release_ms > 0 then context:vibrate("Release", parameters.Power, release_ms) else context:vibrate("Release", off()) end
end

return effect
