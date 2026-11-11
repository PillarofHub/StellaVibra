-- RPM-scaled vibration, not a measurement of engine thrust.
local effect = {}
function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=20, pwm=false}}
    context:parameter {name="Full scale RPM", type="float", default=2500, min=100, max=50000, step=100}
    context:output("Engine 1", {"Throttle:1"})
    context:output("Engine 2", {"Throttle:2"})
    context:output("Engine 3", {"Throttle:3"})
    context:output("Engine 4", {"Throttle:4"})
end
function effect:update(context, telemetry, parameters)
    for i=1,4 do
        local rpm = telemetry["sim/cockpit2/engine/indicators/engine_speed_rpm[" .. (i-1) .. "]"]
        local burning = telemetry["sim/flightmodel2/engines/engine_is_burning_fuel[" .. (i-1) .. "]"]
        if context:input_valid() and rpm and burning and burning > 0 then
            local p = {}
            for key, value in pairs(parameters.Power) do p[key] = value end
            local strength = math.floor(p.strength * math.min(1, math.max(0, rpm / parameters.Full_scale_RPM)) + 0.5)
            p.strength = strength
            context:vibrate("Engine " .. i, p, 100)
        else context:vibrate("Engine " .. i, off()) end
    end
end
return effect
