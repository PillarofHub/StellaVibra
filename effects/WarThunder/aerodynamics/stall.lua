-- Configurable AoA cue, not a universal aircraft stall detector.
-- Tune the threshold for the aircraft; WT supplies no standard stall warning.
local effect = {}
function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=20,pwm=false}}
    context:parameter {name="AoA threshold degrees",type="float",default=15,min=1,max=45,step=0.5}
    context:parameter {name="Minimum IAS kmh",type="float",default=80,min=0,max=1000,step=5}
    context:output("Airframe", {"Stick:1"})
end
function effect:update(context, telemetry, parameters)
    local s = telemetry.state or {}
    local aoa, speed = s["AoA, deg"], s["IAS, km/h"]
    if context:input_valid() and aoa and speed and speed >= parameters.Minimum_IAS_kmh
        and math.abs(aoa) >= parameters.AoA_threshold_degrees then
        context:vibrate("Airframe",parameters.Power,100)
    else context:vibrate("Airframe",off()) end
end
return effect
