-- Virtual slab boundaries, not measured joints or wheel positions.
-- 4.6 m corresponds to the 15 ft spacing in FAA AC 150/5320-6G, Table 3-7.
local effect = {}
local ports = {"Center Gear", "Left Gear", "Right Gear"}

function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=30, pwm=false}}
    context:parameter {name="Slab length metres", type="float", default=4.6, min=0.5, max=30, step=0.1}
    context:parameter {name="Minimum speed knots", type="float", default=0.1, min=0.1, max=100, step=0.1}
    context:parameter {name="Maximum speed knots", type="float", default=160, min=1, max=300, step=1}
    context:parameter {name="Pulse ms", type="int", default=100, min=1, max=1000, step=1}
    context:parameter {name="Ground type", type="enum", default="Concrete", choices={"Concrete", "Asphalt", "Concrete or asphalt"}}
    context:output("Center Gear", {"Stick:1"})
    context:output("Left Gear", {"Throttle:1"})
    context:output("Right Gear", {"Throttle:2"})
    self.gears = {{}, {}, {}}
end

function effect:update(context, telemetry, parameters)
    if parameters.Maximum_speed_knots < parameters.Minimum_speed_knots then error("Maximum speed must be >= minimum") end
    local speed = telemetry.ground_speed_knots
    local surface = telemetry.surface_type
    local selected = parameters.Ground_type
    local allowed = (surface == 0 and selected ~= "Asphalt") or (surface == 4 and selected ~= "Concrete")
    local valid = context:input_valid() and telemetry.on_any_runway and allowed and speed
        and speed >= parameters.Minimum_speed_knots and speed <= parameters.Maximum_speed_knots
    local time = telemetry.sim_time_s
    for index, name in ipairs(ports) do
        local state = self.gears[index]
        local gear = telemetry.gears and telemetry.gears[index]
        if not valid or not gear or not gear.contact then
            self.gears[index] = {}
            context:vibrate(name, off())
        elseif context:sample_is_new() then
            local now = context:now()
            local dt = state.time and time - state.time or 0
            if dt > 0 and dt <= 0.5 and state.slab == parameters.Slab_length_metres then
                local travelled = (speed + state.speed) * 0.5 * 0.514444 * dt
                local distance = (state.distance or 0) + travelled
                local crossed = distance >= parameters.Slab_length_metres
                state.distance = distance % parameters.Slab_length_metres
                if crossed and now >= (state.next_pulse or 0) and travelled > 0 then
                    -- Never replay missed boundaries. Keep a gap even at high speed/sim rate.
                    local interval = parameters.Slab_length_metres / travelled * (now - state.now)
                    local duration = math.floor(math.min(parameters.Pulse_ms, interval * 0.5))
                    if duration >= 1 then
                        context:vibrate(name, parameters.Power, duration)
                        state.next_pulse = now + duration * 2
                    end
                end
            else
                state.distance = 0
                state.next_pulse = nil
                context:vibrate(name, off())
            end
            state.time, state.now, state.speed, state.slab = time, now, speed, parameters.Slab_length_metres
        end
    end
end
return effect
