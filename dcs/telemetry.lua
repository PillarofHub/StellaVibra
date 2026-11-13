-- Stella Vibra DCS relay v1; DCS Lua 5.1 compatible. No effect decisions here.
if _G.StellaVibraTelemetryLoaded then return end
local root = lfs.writedir() .. "Scripts/StellaVibra Telemetry/"
local config = dofile(root .. "config.lua")
assert(type(config) == "table" and type(config.interval_ms) == "number"
    and config.interval_ms >= 10 and config.interval_ms <= 100, "Invalid Stella Vibra interval")
package.path = package.path .. ";./LuaSocket/?.lua"
package.cpath = package.cpath .. ";./LuaSocket/?.dll"
local socket = require("socket")
local udp = assert(socket.udp())
assert(udp:settimeout(0))
assert(udp:setpeername("127.0.0.1", 43891))
local interval = config.interval_ms / 1000
local session, sequence, nextSend, active = "", 0, 0, false
local generation = 0
local errors = {}
local function report(key, err)
    if errors[key] then return end
    errors[key] = true
    if log and log.write then log.write("StellaVibra", log.ERROR, key .. ": " .. tostring(err)) end
end
local function call(name)
    if type(_G[name]) ~= "function" then return nil end
    local ok, value = pcall(_G[name])
    if ok then return value end
    report(name, value)
end
local function quote(s)
    return '"' .. s:gsub('[%z\1-\31\\"]', function(c)
        return string.format("\\u%04x", string.byte(c))
    end) .. '"'
end
local function encode(value, depth, budget)
    if type(value) == "boolean" then return value and "true" or "false" end
    if type(value) == "number" then
        if value == value and value ~= math.huge and value ~= -math.huge then return string.format("%.17g", value) end
    elseif type(value) == "string" then
        if #value <= 512 then return quote(value) end
    elseif type(value) == "table" and depth < 6 then
        local fields = {}
        for key, item in pairs(value) do
            budget.n = budget.n + 1
            if budget.n > 256 then error("Telemetry structure limit") end
            if type(key) == "string" and #key <= 64 then
                local encoded = encode(item, depth + 1, budget)
                if encoded then fields[#fields + 1] = quote(key) .. ":" .. encoded end
            end
        end
        return "{" .. table.concat(fields, ",") .. "}"
    end
    -- Unsupported, absent and non-finite values are omitted, never measured zero.
end
local function newSession()
    generation = generation + 1
    session = string.format("%.6f", socket.gettime()) .. ":" .. tostring(udp) .. ":" .. generation
    sequence, nextSend = 0, 0
end
local function publish(stopped)
    local modelTime = call("LoGetModelTime")
    if type(modelTime) ~= "number" then return end
    local data = {}
    data.LoIsOwnshipExportAllowed = call("LoIsOwnshipExportAllowed")
    data.LoSimulationOnPause = call("LoSimulationOnPause")
    data.LoSimulationOnActivePause = call("LoSimulationOnActivePause")
    if not stopped and data.LoIsOwnshipExportAllowed == true then
        local self = call("LoGetSelfData")
        if type(self) == "table" then data.LoGetSelfData = {Name = self.Name} end
        for _, name in ipairs({"LoGetPlayerPlaneId", "LoGetInAir", "LoGetIndicatedAirSpeed",
            "LoGetTrueAirSpeed", "LoGetMachNumber", "LoGetAngleOfAttack", "LoGetAccelerationUnits",
            "LoGetVectorVelocity", "LoGetAngularVelocity", "LoGetShakeAmplitude"}) do data[name] = call(name) end
        local mech = call("LoGetMechInfo")
        if type(mech) == "table" then data.LoGetMechInfo = {gear=mech.gear, flaps=mech.flaps, speedbrakes=mech.speedbrakes} end
        local engine = call("LoGetEngineInfo")
        if type(engine) == "table" then data.LoGetEngineInfo = {RPM=engine.RPM} end
    end
    sequence = sequence + 1
    local packet = encode({protocol=1, session=session, sequence=sequence, sim_time=modelTime,
        sent_ms=math.floor(socket.gettime()*1000), interval_ms=config.interval_ms,
        stopped=stopped, dcs=data}, 0, {n=0})
    if #packet <= 16384 then udp:send(packet) else report("packet", "Telemetry exceeds 16 KiB") end
end
local function chain(name, own)
    local previous = _G[name]
    _G[name] = function(...)
        if type(previous) == "function" then
            local ok, err = pcall(previous, ...)
            if not ok then report(name .. " previous exporter", err) end
        end
        local ok, err = pcall(own, ...)
        if not ok then report(name, err) end
    end
end
chain("LuaExportStart", function() newSession(); active = true end)
chain("LuaExportOnHumanStart", function() newSession(); active = true end)
chain("LuaExportOnHumanStop", function() publish(true); active = false end)
chain("LuaExportAfterNextFrame", function()
    if not active then return end
    local now = socket.gettime()
    if now < nextSend then return end
    nextSend = now + interval -- No catch-up bursts or per-frame serialization.
    publish(false)
end)
chain("LuaExportStop", function() publish(true); active = false end)
_G.StellaVibraTelemetryLoaded = true
