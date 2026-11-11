-- Runway slab spacing model; not a measured pavement impact signal.
local effect = {}
function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=30, pwm=false}}
    context:parameter {name="Slab length metres", type="float", default=5, min=0.5, max=30, step=0.5}
    context:parameter {name="Minimum speed knots", type="float", default=2, min=1, max=100, step=0.5}
    context:parameter {name="Pulse ms", type="int", default=80, min=20, max=1000, step=1}
    context:parameter {name="Minimum period ms", type="int", default=100, min=20, max=1000, step=1}
    context:parameter {name="Maximum period ms", type="int", default=1500, min=100, max=5000, step=1}
    context:output("Airframe")
end
function effect:update(context, telemetry, parameters)
    if parameters.Maximum_period_ms < parameters.Minimum_period_ms then error("Maximum period must be >= minimum") end
    if not context:input_valid() or not telemetry.on_ground or telemetry.ground_speed_knots < parameters.Minimum_speed_knots then
        self.last = nil
        context:vibrate("Airframe", off())
        return
    end
    local period = math.max(parameters.Minimum_period_ms, math.min(parameters.Maximum_period_ms,
        1000 * parameters.Slab_length_metres / (telemetry.ground_speed_knots * 0.514444)))
    if context:sample_is_new() and (not self.last or (telemetry.sim_time_s - self.last) * 1000 >= period) then
        self.last = telemetry.sim_time_s
        context:vibrate("Airframe", parameters.Power, parameters.Pulse_ms)
    end
end
return effect
