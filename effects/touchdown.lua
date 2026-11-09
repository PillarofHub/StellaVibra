local effect = {}
local ports = {"Center Gear", "Left Gear", "Right Gear"}

function effect:init(api)
    api:parameter {name="Strength", default=40, min=0, max=255, step=1}
    api:parameter {name="Minimum ms", default=100, min=20, max=3000, step=1}
    api:parameter {name="Maximum ms", default=1000, min=20, max=5000, step=1}
    api:parameter {name="Time multiplier", default=1, min=0.1, max=10, step=0.1}
    api:parameter {name="Minimum speed knots", default=5, min=0, max=100, step=0.5}
    api:parameter {name="PWM", default=false}
    api:parameter {name="ON ms", default=20, min=0, max=1000, step=1}
    api:parameter {name="Period ms", default=80, min=20, max=1000, step=1}
    self.gears = {}
    for index, name in ipairs(ports) do
        api:output(name)
        -- Initial on-ground data must not look like a landing.
        self.gears[index] = {armed=false, contact=nil, rising=false}
    end
end

function effect:update(context)
    local telemetry = context.telemetry
    local parameters = context.parameters
    local minimum = parameters["Minimum ms"]
    local maximum = parameters["Maximum ms"]
    if maximum < minimum then error("Maximum ms must be >= Minimum ms") end
    for index, name in ipairs(ports) do
        local state = self.gears[index]
        local gear = telemetry.gears and telemetry.gears[index]
        if not context.telemetry_valid or not gear then
            context:vibrate(name, off())
        else
            if context.is_new_sample then
                if not gear.contact then
                    state.airborne = state.airborne or context.now_ms
                    if context.now_ms - state.airborne >= minimum then
                        state.armed = true
                        state.deadline = nil
                        state.rising = false
                    end
                else
                    if state.contact == false and state.armed
                        and telemetry.ground_speed_knots >= parameters["Minimum speed knots"] then
                        state.started = context.now_ms
                        state.deadline = state.started + minimum
                        state.last_animation = gear.animation_percent
                        state.last_time = telemetry.sim_time_s
                        state.rising = true
                        state.saw_rise = false
                        state.armed = false
                        context:log(name .. " touchdown")
                    elseif state.rising and telemetry.sim_time_s > state.last_time then
                        local elapsed = context.now_ms - state.started
                        local total = math.min(maximum, math.max(minimum,
                            elapsed * parameters["Time multiplier"]))
                        state.deadline = state.started + total
                        -- Only the first rise is measured; later peaks are ignored.
                        if not gear.animation_percent or not state.last_animation
                            or gear.animation_percent <= state.last_animation or elapsed >= maximum then
                            if not state.saw_rise then state.deadline = state.started + minimum end
                            state.rising = false
                        else
                            state.saw_rise = true
                            state.last_animation = gear.animation_percent
                        end
                        state.last_time = telemetry.sim_time_s
                    end
                    state.airborne = nil
                end
                state.contact = gear.contact
            end
            local remaining = state.deadline and state.deadline - context.now_ms or 0
            if remaining > 0 then
                local specification = constant(parameters.Strength)
                if parameters.PWM then
                    specification = pwm {strength=parameters.Strength,
                        on_ms=parameters["ON ms"], period_ms=parameters["Period ms"]}
                end
                context:vibrate(name, specification, remaining)
            else
                context:vibrate(name, off())
            end
        end
    end
end

return effect
