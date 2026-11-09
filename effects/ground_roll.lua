local effect = {}
local ports = {"Center Gear", "Left Gear", "Right Gear"}

function effect:init(api)
    api:parameter {name="Strength", default=20, min=0, max=255, step=1}
    api:parameter {name="Minimum speed knots", default=2, min=0, max=100, step=0.5}
    api:parameter {name="PWM", default=false}
    api:parameter {name="ON ms", default=20, min=0, max=1000, step=1}
    api:parameter {name="Period ms", default=80, min=20, max=1000, step=1}
    for _, name in ipairs(ports) do api:output(name) end
end

function effect:update(context)
    local telemetry = context.telemetry
    local parameters = context.parameters
    for index, name in ipairs(ports) do
        local gear = telemetry.gears and telemetry.gears[index]
        if context.telemetry_valid and gear and gear.contact
            and telemetry.ground_speed_knots >= parameters["Minimum speed knots"] then
            local specification = constant(parameters.Strength)
            if parameters.PWM then
                specification = pwm {strength=parameters.Strength,
                    on_ms=parameters["ON ms"], period_ms=parameters["Period ms"]}
            end
            -- This effect owns its renewal margin; the host never extends it.
            context:vibrate(name, specification, 150)
        else
            context:vibrate(name, off())
        end
    end
end

return effect
