-- Native deployment percentage movement; no inferred ground contact.
local effect = {}
function effect:init(context)
    context:parameter {name="Power",type="haptic_power",default={strength=20,pwm=false}}
    context:parameter {name="Movement margin ms",type="int",default=200,min=50,max=2000,step=10}
    context:output("Airframe", {"Stick:1"})
end
function effect:update(context,telemetry,parameters)
    local value=(telemetry.state or {})["flaps, %"]
    if context:sample_is_new() and value then
        value=math.floor(value+0.5)
        if self.previous and value~=self.previous then self.deadline=context:now()+parameters.Movement_margin_ms end
        self.previous=value
    end
    local remaining=self.deadline and self.deadline-context:now() or 0
    if context:input_valid() and remaining > 0 then context:vibrate("Airframe",parameters.Power,remaining)
    else context:vibrate("Airframe",off()) end
end
return effect
