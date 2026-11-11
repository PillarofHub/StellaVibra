local effect = {}
function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=20, pwm=false}}
    context:parameter {name="Bank threshold degrees", type="float", default=45, min=5, max=90, step=1}
    context:output("Airframe", {"Stick:1"})
end
function effect:update(context, telemetry, parameters)
    if context:input_valid() and telemetry["sim/flightmodel/failures/onground_any"] == 0 and telemetry["sim/flightmodel/position/phi"]
        and math.abs(telemetry["sim/flightmodel/position/phi"]) > parameters.Bank_threshold_degrees then
        context:vibrate("Airframe", parameters.Power, 100)
    else context:vibrate("Airframe", off()) end
end
return effect
