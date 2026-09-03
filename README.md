**31 aug 2026.**

1. Check the soil moisture sensor in n minutes times, if the
moisture is more than const limit - turn on the relay of water pomp
in N seconds and check: is the moisture normal? if yes, turn off
relay, else turn on relay and run the cicle again while time isn't
over.

```Arduino
uint32_t soil_moisture_sens_tmr;
if (millis() - soil_moisture_sens_tmr >= period) {
    soil_moisture_sens_tmr = millis();

    if (soil_moisture_value > moisture_limit) {
        if (millis() - pomp_work_tmr >= pomp_work_perid) {
            pomp_work_tmr = millis();
            digitalWrite(RELAY_POMP_PIN, HIGH);
        }
    else digitalWrite(RELAY_POMP_PIN, HIGH);
}
```