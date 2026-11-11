local effect = {}
local ports = {"Center Gear", "Left Gear", "Right Gear"}

function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=40, pwm=false, on_ms=20, period_ms=80}}
    context:parameter {name="Minimum ms", type="int", default=200, min=20, max=3000, step=1}
    context:parameter {name="Maximum ms", type="int", default=2000, min=20, max=5000, step=1}
    context:parameter {name="Time multiplier", type="float", default=2, min=0.1, max=10, step=0.1}
    context:parameter {name="Minimum speed mps", type="float", default=5, min=0, max=100, step=0.5}
    context:output("Center Gear", {"Stick:1"})
    context:output("Left Gear", {"Throttle:1"})
    context:output("Right Gear", {"Throttle:2"})
    self.gears = {}
    for index, name in ipairs(ports) do
        -- Initial on-ground data must not look like a landing.
        self.gears[index] = {armed=false, contact=nil, rising=false}
    end
end

function effect:update(context, telemetry, parameters)
    local speed = telemetry["sim/flightmodel/position/groundspeed"]
    local minimum = parameters.Minimum_ms
    local maximum = parameters.Maximum_ms
    if maximum < minimum then error("Maximum ms must be >= Minimum ms") end
    for index, name in ipairs(ports) do
        local state = self.gears[index]
        local deflection = telemetry["sim/flightmodel2/gear/tire_vertical_deflection_mtr[" .. (index - 1) .. "]"]
        local gear = deflection and {contact=deflection > 0, compression_m=deflection}
        if not context:input_valid() or not gear then
            state.armed, state.rising, state.deadline, state.contact, state.airborne = false, false, nil, nil, nil
            context:vibrate(name, off())
        else
            if context:sample_is_new() then
                if not gear.contact then
                    state.airborne = state.airborne or context:now()
                    if context:now() - state.airborne >= minimum then
                        state.armed = true
                        state.deadline = nil
                        state.rising = false
                    end
                else
                    if state.contact == false and state.armed
                        and speed and speed >= parameters.Minimum_speed_mps then
                        state.started = context:now()
                        state.deadline = state.started + minimum
                        state.last_animation = gear.compression_m
                        state.last_time = telemetry["sim/network/misc/network_time_sec"]
                        state.rising = true
                        state.saw_rise = false
                        state.armed = false
                        context:log(name .. " touchdown")
                    elseif state.rising and telemetry["sim/network/misc/network_time_sec"] > state.last_time then
                        local elapsed = context:now() - state.started
                        local total = math.min(maximum, math.max(minimum,
                            elapsed * parameters.Time_multiplier))
                        state.deadline = state.started + total
                        -- Only the first rise is measured; later peaks are ignored.
                        if not gear.compression_m or not state.last_animation
                            or gear.compression_m <= state.last_animation or elapsed >= maximum then
                            if not state.saw_rise then state.deadline = state.started + minimum end
                            state.rising = false
                        else
                            state.saw_rise = true
                            state.last_animation = gear.compression_m
                        end
                        state.last_time = telemetry["sim/network/misc/network_time_sec"]
                    end
                    state.airborne = nil
                end
                state.contact = gear.contact
            end
            local remaining = state.deadline and state.deadline - context:now() or 0
            if remaining > 0 then
                context:vibrate(name, parameters.Power, remaining)
            else
                context:vibrate(name, off())
            end
        end
    end
end

return effect
