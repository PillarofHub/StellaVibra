-- Stella Vibra DCS loader v1. Keep Export.lua independent of relay changes.
if not _G.StellaVibraTelemetryLoaded then
    local ok, err = pcall(dofile, lfs.writedir() .. "Scripts/StellaVibra Telemetry/telemetry.lua")
    if not ok and log and log.write then log.write("StellaVibra", log.ERROR, tostring(err)) end
end
