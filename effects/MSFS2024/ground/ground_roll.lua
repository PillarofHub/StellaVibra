local effect = {}
local ports = {"Center Gear", "Left Gear", "Right Gear"}

function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=1, pwm=true, on_ms=20, period_ms=80}}
    context:parameter {name="Minimum speed knots", type="float", default=0.1, min=0, max=100, step=0.1}
    context:output("Center Gear", {"Stick:1"})
    context:output("Left Gear", {"Throttle:1"})
    context:output("Right Gear", {"Throttle:2"})
end

function effect:update(context, telemetry, parameters)
    for index, name in ipairs(ports) do
        local gear = telemetry.gears and telemetry.gears[index]
        if context:input_valid() and gear and gear.contact
            and telemetry.ground_speed_knots >= parameters.Minimum_speed_knots then
            -- This effect owns its renewal margin; the host never extends it.
            context:vibrate(name, parameters.Power, 100)
        else
            context:vibrate(name, off())
        end
    end
end

return effect
