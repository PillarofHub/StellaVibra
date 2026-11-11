-- WT has no standard starter flag: use a new RPM rise from a stopped engine.
local effect = {}
function effect:init(context)
    context:parameter {name="Power",type="haptic_power",default={strength=20,pwm=false}}
    context:parameter {name="Pulse ms",type="int",default=1000,min=20,max=5000,step=20}
    context:output("Engine 1", {"Throttle:1"})
end
function effect:update(context,telemetry,parameters)
    local rpm = (telemetry.state or {})["RPM 1"]
    if context:sample_is_new() and rpm then
        if self.previous and self.previous <= 1 and rpm > 1 then self.deadline=context:now()+parameters.Pulse_ms end
        self.previous=rpm
    end
    local remaining=self.deadline and self.deadline-context:now() or 0
    if context:input_valid() and rpm and remaining > 0 then context:vibrate("Engine 1",parameters.Power,remaining)
    else context:vibrate("Engine 1",off()) end
end
return effect
