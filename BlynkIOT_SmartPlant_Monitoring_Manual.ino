// Define Template ID, Template Name, and Auth Token
#define BLYNK_TEMPLATE_ID "TMPL3eE4G-qu3"  // Replace with your Blynk Template ID
#define BLYNK_TEMPLATE_NAME "Smart Garden" // Replace with your Blynk Template Name
#define BLYNK_AUTH_TOKEN "t94sVQggTiVX2cV1nLJr_SwZ1Jf_nUgF" // Replace with your Blynk Auth Token

// Include necessary libraries
#include <LiquidCrystal_I2C.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

// Initialize LCD Display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi credentials
char ssid[] = "BHU_ADMIN";     // Replace with your WiFi SSID
char pass[] = "12345677";      // Replace with your WiFi password

// DHT Sensor setup
DHT dht(D4, DHT11); // (DHT sensor pin, sensor type)
BlynkTimer timer;   // Blynk Timer
WidgetLED led(V5);  // Widget LED on Virtual Pin V5

// Define component pins
#define soil A0     // Soil Moisture Sensor
#define PIR D5      // PIR Motion Sensor
#define RELAY_PIN_1 D3 // Relay
#define PUSH_BUTTON_1 D7 // Button
#define VPIN_BUTTON_1 V12

// Global variables
int PIR_ToggleValue = 0;
int relay1State = LOW;
int pushButton1State = HIGH;

void setup() {
  // Serial and LCD setup
  Serial.begin(9600);
  lcd.begin();         // Initialize LCD
  lcd.backlight();    // Turn on backlight
  lcd.setCursor(0, 0);
  lcd.print("  Initializing  ");

  for (int a = 5; a <= 10; a++) {
    lcd.setCursor(a, 1);
    lcd.print(".");
    delay(500);
  }

  lcd.clear();
  lcd.setCursor(11, 1);
  lcd.print("W:OFF");

  // Pin setup
  pinMode(PIR, INPUT);
  pinMode(RELAY_PIN_1, OUTPUT);
  digitalWrite(RELAY_PIN_1, LOW);
  pinMode(PUSH_BUTTON_1, INPUT_PULLUP);

  // Blynk and DHT setup
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  dht.begin();

  // Timer setup
  timer.setInterval(1000L, soilMoistureSensor); // Adjusted interval
  timer.setInterval(1000L, DHT11sensor);
  timer.setInterval(500L, checkPhysicalButton);
}


// Get the DHT11 sensor values
void DHT11sensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(t);
  lcd.setCursor(8, 0);
  lcd.print("H:");
  lcd.print(h);
}

// Get the soil moisture values
void soilMoistureSensor() {
  int value = analogRead(soil);
  value = map(value, 0, 1024, 0, 100);
  value = (value - 100) * -1;

  Blynk.virtualWrite(V3, value);

  lcd.setCursor(0, 1);
  lcd.print("S:");
  lcd.print(value);
  lcd.print(" ");
}

// Handle PIR sensor values
void PIRsensor() {
  bool motionDetected = digitalRead(PIR);
  if (motionDetected) {
    Blynk.logEvent("pirmotion", "WARNING! Motion Detected!");
    led.on();
  } else {
    led.off();
  }
}

// Handle relay state
BLYNK_WRITE(VPIN_BUTTON_1) {
  relay1State = param.asInt();
  digitalWrite(RELAY_PIN_1, relay1State);
}

// Sync button state
BLYNK_CONNECTED() {
  Blynk.syncVirtual(VPIN_BUTTON_1);
}

// Check physical button
void checkPhysicalButton() {
  if (digitalRead(PUSH_BUTTON_1) == LOW) {
    if (pushButton1State != LOW) {
      relay1State = !relay1State;
      digitalWrite(RELAY_PIN_1, relay1State);
      Blynk.virtualWrite(VPIN_BUTTON_1, relay1State);
    }
    pushButton1State = LOW;
  } else {
    pushButton1State = HIGH;
  }
}

void loop() {
  if (PIR_ToggleValue == 1) {
    lcd.setCursor(5, 1);
    lcd.print("M:ON ");
    PIRsensor();
  } else {
    lcd.setCursor(5, 1);
    lcd.print("M:OFF");
    led.off();
  }

  lcd.setCursor(11, 1);
  lcd.print(relay1State == HIGH ? "W:ON " : "W:OFF");

  Blynk.run();
  timer.run();
}
