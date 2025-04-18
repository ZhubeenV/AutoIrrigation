#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>


#define MOISTURE_PIN 25
#define PUMP_EN_PIN 26

#define LOW_MOISTURE 3000

static int MoistureValue = 0;
static int response = 10; // Minutes

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(Moisture1Pwr, OUTPUT);
  pinMode(PUMP_EN_PIN, OUTPUT)
  Serial.begin(9600);
  Serial.println("Hello from the setup"); // Should print 5
}

void loop() {
  delay(3000);  // delay for some time to allow webserver to come up, etc.
  read_moisture1();
  //update_webserver();
  Serial.println(MoistureValue);
  if (MoistureValue > LOW_MOISTURE) // if the moisture value is higher (less moist)
    water_plants()

  delay(3000);  
  // Put ESP in deep sleep for <response> amount of time
}

// This is the moisture reading function, if I add multiple plants, this can be a method, and I could use a class for each  
void read_moisture1() {
  digitalWrite(LED_BUILTIN, HIGH);  // optional, but shows that the system is doing something. 
  digitalWrite(Moisture1Pwr, HIGH);
  delay(100);
  MoistureValue = analogRead(MOISTURE_PIN);
  digitalWrite(MoisturePwr, LOW);
  digitalWrite(LED_BUILTIN, LOW);
}

void water_plants(int seconds) {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(PUMP_EN_PIN, HIGH);
  delay(seconds * 1000);
  digitalWrite(PUMP_EN_PIN, LOW);
  digitalWrite(LED_BUILTIN, LOW);
}

// The pump can run from 3-5V, when I gave it 5V it drew roughly 100mA.
// It emptied a glass of water in less than 10 seconds. So it is pretty fast.
// Test steps:
// 1. Connect relay to the pump, give the pump longer leads (be sure to heat shrink). connect relay to esp and to 5v power on a power supply.
// 2. Test water flow rate. time how long it takes to empty a specified amount of water.
// 3. Use the flow rate to determine how long to run the pump for. 
//    Maybe create a function to enter the mL and the fucntion can automatically determine how much water to dispense.
// 4. Hook up moisture sensor and collect tests withe the values. 
//    The mosture sensor should create a feedback loop, so the pump should know when to stop watering...for the most part
