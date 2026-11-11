local effect = {}
function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=20, pwm=false}}
    context:output("Airframe", {"Stick:1"})
end
function effect:update(context, telemetry, parameters)
    if context:input_valid() and (telemetry["sim/cockpit2/annunciators/stall_warning"] == 1) then
        context:vibrate("Airframe", parameters.Power, 100)
    else context:vibrate("Airframe", off()) end
end
return effect
