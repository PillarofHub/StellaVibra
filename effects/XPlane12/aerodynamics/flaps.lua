local effect = {}
function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=20, pwm=false}}
    context:parameter {name="Movement margin ms", type="int", default=200, min=50, max=2000, step=1}
    context:parameter {name="Slat tolerance percent", type="float", default=3, min=0, max=20, step=1}
    context:output("Airframe", {"Stick:1"})
    self.previous = {}
end
function effect:update(context, telemetry, parameters)
    local flap = telemetry["sim/flightmodel2/controls/flap1_deploy_ratio"]
    local slat = telemetry["sim/flightmodel2/controls/slat1_deploy_ratio"]
    local values = {flap and flap * 100, nil, slat and slat * 100, nil}
    if context:sample_is_new() then
        for i = 1, 4 do
            local value = values[i]
            if value then
                value = math.floor(value + 0.5)
                local threshold = i > 2 and parameters.Slat_tolerance_percent or 0
                if self.previous[i] and math.abs(value - self.previous[i]) > threshold then
                    self.deadline = context:now() + parameters.Movement_margin_ms
                    self.previous[i] = value
                elseif not self.previous[i] then self.previous[i] = value end
            else self.previous[i] = nil end
        end
    end
    if context:input_valid() and self.deadline and self.deadline > context:now() then
        context:vibrate("Airframe", parameters.Power, self.deadline - context:now())
    else context:vibrate("Airframe", off()) end
end
return effect
