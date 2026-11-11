local effect = {}
function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=20, pwm=false}}
    context:parameter {name="Bank threshold degrees", type="float", default=45, min=5, max=90, step=1}
    context:output("Airframe", {"Stick:1"})
end
function effect:update(context, telemetry, parameters)
    if context:input_valid() and not telemetry.on_ground and telemetry.plane_bank_degrees
        and math.abs(telemetry.plane_bank_degrees) > parameters.Bank_threshold_degrees then
        context:vibrate("Airframe", parameters.Power, 100)
    else context:vibrate("Airframe", off()) end
end
return effect
