local effect = {}
function effect:init(context)
    context:parameter {name="Power", type="haptic_power", default={strength=20, pwm=false}}
    context:parameter {name="Propeller thrust", type="boolean", default=false}
    context:parameter {name="Full scale pounds", type="float", default=10000, min=1, max=200000, step=100}
    context:parameter {name="Dead zone pounds", type="float", default=100, min=0, max=20000, step=10}
    for i = 1, 4 do context:output("Engine " .. i) end
end
function effect:update(context, telemetry, parameters)
    if parameters.Full_scale_pounds <= parameters.Dead_zone_pounds then error("Full scale must exceed dead zone") end
    for i = 1, 4 do
        local field = parameters.Propeller_thrust and "prop_thrust_" or "turb_eng_jet_thrust_"
        local thrust = telemetry[field .. i]
        local exists = telemetry.number_of_engines and i <= telemetry.number_of_engines
        if context:input_valid() and exists and telemetry["general_eng_combustion_" .. i] and thrust then
            local fraction = math.max(0, math.min(1, (math.abs(thrust) - parameters.Dead_zone_pounds)
                / (parameters.Full_scale_pounds - parameters.Dead_zone_pounds)))
            local power = parameters.Power
            local strength = math.floor(power.strength * fraction + 0.5)
            local specification = power.pwm and pwm {strength=strength, on_ms=power.on_ms, period_ms=power.period_ms} or constant(strength)
            context:vibrate("Engine " .. i, specification, 150)
        else context:vibrate("Engine " .. i, off()) end
    end
end
return effect
