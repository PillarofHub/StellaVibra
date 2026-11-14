local effect = {}

function effect:init(context)
    context:parameter {name="Warning enabled", type="boolean", default=false}
    context:parameter {name="Power", type="haptic_power", default={strength=20, pwm=false, on_ms=40, period_ms=160}}
    context:parameter {name="Input", type="enum", default="True airspeed", choices={"True airspeed", "Mach"}}
    context:parameter {name="Speed threshold mps", type="float", default=250, min=1, max=1500, step=1}
    context:parameter {name="Speed hysteresis mps", type="float", default=5, min=0, max=100, step=1}
    context:parameter {name="Mach threshold", type="float", default=0.85, min=0.1, max=5, step=0.01}
    context:parameter {name="Mach hysteresis", type="float", default=0.03, min=0, max=0.5, step=0.01}
    context:output("Warning", {"Stick:1"})
    self.active = false
end

function effect:update(context, telemetry, parameters)
    local dcs = telemetry.dcs or {}
    local mach = parameters.Input == "Mach"
    local value
    -- Do not fall back to another unit when a selected input is unavailable.
    if mach then value = dcs.LoGetMachNumber else value = dcs.LoGetTrueAirSpeed end
    local threshold = mach and parameters.Mach_threshold or parameters.Speed_threshold_mps
    local margin = mach and parameters.Mach_hysteresis or parameters.Speed_hysteresis_mps
    if margin >= threshold then error("Hysteresis must be below the selected threshold") end
    if not context:input_valid() or not parameters.Warning_enabled
        or dcs.LoGetInAir ~= true or type(value) ~= "number" then
        self.active = false
    elseif context:sample_is_new() then
        if self.active then self.active = value > threshold - margin
        else self.active = value >= threshold end
    end
    if self.active then context:vibrate("Warning", parameters.Power, 100)
    else context:vibrate("Warning", off()) end
end

return effect
