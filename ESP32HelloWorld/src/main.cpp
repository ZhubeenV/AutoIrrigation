#include <Arduino.h>
#include <esp_sleep.h>

// Pin definitions
#define MOISTURE_PIN 25          // ADC-capable GPIO for moisture sensor signal
#define MOISTURE_PWR_PIN 27       // GPIO to power moisture sensor (or tie to 3V3 if not switching)
#define PUMP_EN_PIN 26            // GPIO to control pump relay/MOSFET

// Configuration constants
#define MOISTURE_THRESHOLD 3000   // Dry soil threshold (higher = drier, adjust based on sensor)
#define RELAY_ACTIVE_LOW false    // Set to true if relay board is active-low
#define ML_PER_SECOND 12          // Calibration: ml of water per second (tune after flow tests)
#define MAX_PUMP_RUN_SECONDS 60   // Safety limit: max continuous pump run time
#define MAX_WATERING_PULSES 3     // Max pulses in feedback loop
#define PULSE_DURATION_SECONDS 2  // Duration of each watering pulse
#define SOAK_WAIT_SECONDS 15      // Wait time between pulses for water to soak
#define MOISTURE_SAMPLES 7        // Number of samples to average for noise reduction

static int MoistureValue = 0;
static int response = 10; // Sleep duration in minutes

// Helper function to control pump relay/MOSFET
void pump_set(bool on) {
  bool pinState = on;
  if (RELAY_ACTIVE_LOW) {
    pinState = !pinState;
  }
  digitalWrite(PUMP_EN_PIN, pinState);
}

// Read moisture sensor with averaging
int read_moisture() {
  // Power on sensor
  digitalWrite(MOISTURE_PWR_PIN, HIGH);
  delay(100); // Warm-up time
  
  // Take multiple samples and average
  long sum = 0;
  for (int i = 0; i < MOISTURE_SAMPLES; i++) {
    sum += analogRead(MOISTURE_PIN);
    delay(30); // Small delay between samples
  }
  
  // Power off sensor
  digitalWrite(MOISTURE_PWR_PIN, LOW);
  
  return sum / MOISTURE_SAMPLES;
}

// Water plants for specified seconds (with safety limit)
void water_for_seconds(int seconds) {
  if (seconds <= 0) return;
  
  // Apply safety limit
  if (seconds > MAX_PUMP_RUN_SECONDS) {
    Serial.print("WARNING: Requested ");
    Serial.print(seconds);
    Serial.print("s exceeds max ");
    Serial.print(MAX_PUMP_RUN_SECONDS);
    Serial.println("s. Limiting.");
    seconds = MAX_PUMP_RUN_SECONDS;
  }
  
  Serial.print("Pump ON for ");
  Serial.print(seconds);
  Serial.println(" seconds");
  
  digitalWrite(LED_BUILTIN, HIGH);
  pump_set(true);
  delay(seconds * 1000);
  pump_set(false);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.println("Pump OFF");
}

// Water plants for specified milliliters
void water_for_ml(int ml) {
  if (ml <= 0) return;
  
  int seconds = (ml + ML_PER_SECOND - 1) / ML_PER_SECOND; // Round up
  Serial.print("Watering ");
  Serial.print(ml);
  Serial.print(" ml (approx ");
  Serial.print(seconds);
  Serial.println(" seconds)");
  
  water_for_seconds(seconds);
}

// Put ESP32 into deep sleep for specified minutes
void go_to_sleep_minutes(int minutes) {
  unsigned long sleep_us = (unsigned long)minutes * 60 * 1000000;
  
  Serial.print("Going to deep sleep for ");
  Serial.print(minutes);
  Serial.println(" minutes");
  Serial.flush();
  
  esp_sleep_enable_timer_wakeup(sleep_us);
  esp_deep_sleep_start();
}

void setup() {
  // Initialize Serial
  Serial.begin(115200);
  delay(1000); // Give serial time to initialize
  
  // Configure pins
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MOISTURE_PWR_PIN, OUTPUT);
  pinMode(PUMP_EN_PIN, OUTPUT);
  
  // Ensure pump is OFF on boot (safe default)
  pump_set(false);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(MOISTURE_PWR_PIN, LOW);
  
  // Print wake reason
  esp_sleep_wakeup_cause_t wake_reason = esp_sleep_get_wakeup_cause();
  Serial.println("\n=== ESP32 Plant Watering Controller ===");
  switch(wake_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wake reason: External signal (RTC_IO)");
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Wake reason: External signal (RTC_CNTL)");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Wake reason: Timer");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("Wake reason: Touchpad");
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println("Wake reason: ULP program");
      break;
    default:
      Serial.println("Wake reason: Power-on reset");
      break;
  }
  
  // Print current configuration
  Serial.print("Moisture threshold: ");
  Serial.println(MOISTURE_THRESHOLD);
  Serial.print("Sleep duration: ");
  Serial.print(response);
  Serial.println(" minutes");
  Serial.print("Max pump run time: ");
  Serial.print(MAX_PUMP_RUN_SECONDS);
  Serial.println(" seconds");
  Serial.print("Calibration: ");
  Serial.print(ML_PER_SECOND);
  Serial.println(" ml/second");
  Serial.println("---");
}

void loop() {
  // Read moisture
  Serial.print("Reading moisture... ");
  MoistureValue = read_moisture();
  Serial.print(MoistureValue);
  Serial.print(" (threshold: ");
  Serial.print(MOISTURE_THRESHOLD);
  Serial.print(") - ");
  
  if (MoistureValue > MOISTURE_THRESHOLD) {
    Serial.println("DRY - Starting watering cycle");
    
    // Feedback loop: pulse watering with re-reading
    bool moisture_ok = false;
    for (int pulse = 1; pulse <= MAX_WATERING_PULSES; pulse++) {
      Serial.print("Pulse ");
      Serial.print(pulse);
      Serial.print("/");
      Serial.print(MAX_WATERING_PULSES);
      Serial.println(": Watering...");
      
      // Water for pulse duration
      water_for_seconds(PULSE_DURATION_SECONDS);
      
      // Wait for water to soak
      Serial.print("Waiting ");
      Serial.print(SOAK_WAIT_SECONDS);
      Serial.println(" seconds for water to soak...");
      delay(SOAK_WAIT_SECONDS * 1000);
      
      // Re-read moisture
      Serial.print("Re-reading moisture... ");
      MoistureValue = read_moisture();
      Serial.print(MoistureValue);
      Serial.print(" (threshold: ");
      Serial.print(MOISTURE_THRESHOLD);
      Serial.print(") - ");
      
      if (MoistureValue <= MOISTURE_THRESHOLD) {
        Serial.println("OK - Moisture sufficient, stopping");
        moisture_ok = true;
        break;
      } else {
        Serial.println("Still dry");
      }
    }
    
    if (!moisture_ok) {
      Serial.print("Stopped after ");
      Serial.print(MAX_WATERING_PULSES);
      Serial.println(" pulses (max reached)");
    }
  } else {
    Serial.println("OK - No watering needed");
  }
  
  // Go to deep sleep
  go_to_sleep_minutes(response);
  
  // This should never be reached due to deep sleep, but just in case:
  delay(1000);
}

/*
 * WIRING AND CALIBRATION NOTES
 * =============================
 * 
 * Moisture Sensor:
 *   - Signal wire to MOISTURE_PIN (GPIO 25, ADC-capable)
 *   - VCC to MOISTURE_PWR_PIN (GPIO 27) for power switching, OR tie directly to 3V3 if not switching
 *   - GND to GND
 *   - Note: Capacitive sensors benefit from power switching to reduce corrosion
 * 
 * Pump Control:
 *   - PUMP_EN_PIN (GPIO 26) drives relay or MOSFET
 *   - If using relay module: check if active-low (set RELAY_ACTIVE_LOW = true)
 *   - If using MOSFET: ensure proper gate voltage and flyback diode for inductive loads
 *   - Pump power should come from external supply (USB power supply, battery, etc.)
 *     DO NOT power pump directly from ESP32 GPIO pins
 * 
 * Calibration:
 *   1. Test flow rate: Time how long pump takes to dispense known volume (e.g., 100ml)
 *   2. Calculate ML_PER_SECOND = volume_ml / time_seconds
 *   3. Adjust MOISTURE_THRESHOLD based on sensor readings:
 *      - Higher values = drier soil threshold
 *      - Test with dry and wet soil to find appropriate value
 *   4. Tune PULSE_DURATION_SECONDS and SOAK_WAIT_SECONDS based on soil type
 * 
 * Power:
 *   - ESP32 can be powered via USB
 *   - Pump should have separate power supply (5V recommended based on comments)
 *   - Consider deep sleep current draw for battery operation
 */
