local effect = {}
local ports = {"Center Gear", "Left Gear", "Right Gear"}

function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=20, pwm=false, on_ms=20, period_ms=80}}
    context:parameter {name="Minimum speed knots", type="float", default=2, min=0, max=100, step=0.5}
    for _, name in ipairs(ports) do context:output(name) end
end

function effect:update(context, telemetry, parameters)
    for index, name in ipairs(ports) do
        local gear = telemetry.gears and telemetry.gears[index]
        if context:input_valid() and gear and gear.contact
            and telemetry.ground_speed_knots >= parameters.Minimum_speed_knots then
            -- This effect owns its renewal margin; the host never extends it.
            context:vibrate(name, parameters.Power, 150)
        else
            context:vibrate(name, off())
        end
    end
end

return effect
