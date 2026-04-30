# AutoIrrigation

An ESP32 that checks if my plants need water and waters them if they do.
Built because I kept forgetting.

## How it works

The ESP32 wakes up every 10 minutes, reads a soil moisture sensor, and runs
the pump if the soil is dry. Then it goes back to sleep.

The watering itself isn't just "run the pump for N seconds" — it pulses water
in short bursts, waits for it to soak in, then re-checks the moisture. If the
soil is wet enough, it stops. Otherwise it pulses again, up to a max of 3
pulses. This avoids the classic overwatering problem where a long single
pulse floods the top of the soil while the roots stay dry.

## Hardware

- ESP32 dev board
- Capacitive soil moisture sensor
- Small submersible pump driven through a MOSFET (or relay)
- Separate 5V supply for the pump (don't try to power it from the ESP32)

Pin assignments are at the top of `main.cpp`.

## Things I learned / decided along the way

**Capacitive sensors corrode if you leave them powered all the time.** So
the sensor's VCC is wired to a GPIO instead of 3V3, and only powered up
during a read. Big lifespan improvement.

**Cheap ADC reads are noisy.** A single reading would sometimes false-trigger
the pump. I average 7 samples with small delays between them and the readings
got way more stable.

**Safety cap on pump runtime.** There's a hard limit (`MAX_PUMP_RUN_SECONDS`)
so if something goes wrong — sensor unplugged, bug in the logic, whatever —
the pump physically can't run forever and flood the plant. Learned this one
the hard way.

**Deep sleep between checks.** Plants don't need to be monitored every
second. Sleeping between cycles means this could run on a battery if I
ever decide to move it outside.

## Calibration

Two things to tune for your own setup:

- `ML_PER_SECOND` — time the pump dispensing 100ml into a measuring cup,
  then divide. Mine ended up around 12.
- `MOISTURE_THRESHOLD` — stick the sensor in dry soil, note the value.
  Stick it in wet soil, note that value. Pick something in between with
  some margin. Higher = drier for capacitive sensors.

## TODO

- [ ] Multiple zones (multiple sensors + valves)
- [ ] Wi-Fi logging so I can see moisture trends
- [ ] Maybe skip watering if it's about to rain
