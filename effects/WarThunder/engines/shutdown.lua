-- Power fall to zero while RPM remains positive indicates spin-down in Aurora captures.
local effect = {}
function effect:init(context)
    context:parameter {name="Power",type="haptic_power",default={strength=20,pwm=false}}
    context:parameter {name="Fade ms",type="int",default=3000,min=100,max=10000,step=100}
    context:output("Engine 1", {"Throttle:1"})
end
function effect:update(context,telemetry,parameters)
    local s=telemetry.state or {}
    local power,rpm=s["power 1, hp"],s["RPM 1"]
    if context:sample_is_new() and power and rpm then
        if self.previous and self.previous > 0 and power <= 0 and rpm > 1 then self.started=context:now() end
        if power > 0 or rpm <= 1 then self.started=nil end
        self.previous=power
    end
    local fraction=self.started and math.max(0,1-(context:now()-self.started)/parameters.Fade_ms) or 0
    if context:input_valid() and power and rpm and fraction > 0 then
        local p = {}
        for key, value in pairs(parameters.Power) do p[key] = value end
        local strength=math.floor(p.strength*fraction+0.5)
        p.strength = strength
        context:vibrate("Engine 1", p, 100)
    else context:vibrate("Engine 1",off()) end
end
return effect
