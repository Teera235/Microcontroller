#define BLYNK_TEMPLATE_ID "xxxxxxxxxxx"
#define BLYNK_TEMPLATE_NAME "xxxxxxxxxxx"
#define BLYNK_AUTH_TOKEN "xxxxxxxxxxx"

#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <PZEM004Tv30.h>
#include <WiFiClient.h>
#include <TridentTD_LineNotify.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> 


char ssid[] = "xxxxxxxxxxx";
char pass[] = "xxxxxxxxxxx";

char LINE_TOKEN[] = "xxxxxxxxxxx";

#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif

#if !defined(PZEM_SERIAL)
#define PZEM_SERIAL Serial1
#endif

PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);

const int RELAY_PIN_1 = 2;
const int LED_PIN = 13;

bool relayState1 = LOW;
bool ledState = LOW;

unsigned long lastNotificationTime = 0;
const unsigned long notificationInterval = 60000*60; 

LiquidCrystal_I2C lcd(0x27, 16, 4);

void relayOn(int relayNumber) {
  switch (relayNumber) {
    case 1:
      digitalWrite(RELAY_PIN_1, HIGH);
      relayState1 = HIGH;
      printRelayState(relayNumber, true);
      break;
  }
}

void relayOff(int relayNumber) {
  switch (relayNumber) {
    case 1:
      digitalWrite(RELAY_PIN_1, LOW);
      relayState1 = LOW;
      printRelayState(relayNumber, false);
      break;
  }
}

void ledOn() {
  digitalWrite(LED_PIN, HIGH);
  ledState = HIGH;
}

void ledOff() {
  digitalWrite(LED_PIN, LOW);
  ledState = LOW;
}

void printRelayState(int relayNumber, bool state) {
  switch (relayNumber) {
    case 1:
      if (state) {
        Serial.println("[INFO] เปิดรีเลย์ 1");
      } else {
        Serial.println("[INFO] ปิดรีเลย์ 1");
      }
      break;
  }
}

void sendToLineNotify(String message) {
  LINE.notify(message);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println(LINE.getVersion());
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  LINE.setToken(LINE_TOKEN);
  LINE.notify("ระบบวัดไฟฟ้าเริ่่ม!!!");
  delay(2000);
  
  lcd.init();                   
  lcd.backlight();               
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(2000);
}

BLYNK_WRITE(V0) {
  int pinValue = param.asInt();

  if (pinValue == 0) {
    if (!relayState1) {
      relayOn(1);
      ledOn();
    }
  } else {
    if (relayState1) {
      relayOff(1);
      ledOff();
    }
  }
}

void loop() {
  Blynk.run();
  Serial.print("Custom Address:");
  Serial.println(pzem.readAddress(), HEX);

  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();
  float frequency = pzem.frequency();
  float pf = pzem.pf();
  float electricityCostPerUnit = 4.54;
  float energyCost = (energy / 1000.0) * electricityCostPerUnit;

  if (isnan(voltage) || isnan(current) || isnan(power) || isnan(energy) || isnan(frequency) || isnan(pf)) {
    Serial.println("Error reading sensor values");
  } else {
    if (millis() - lastNotificationTime >= notificationInterval) {
      String notificationMessage =
        "Voltage: " + String(voltage) + " V\n"
        + "Current: " + String(current) + " A\n"
        + "Power: " + String(power) + " W\n"
        + "Energy: " + String(energyCost / 1000.0) + " kWh (" + String(energyCost) + " บาท)\n";

      sendToLineNotify(notificationMessage);
      lastNotificationTime = millis();
    }

    Serial.print("Voltage: ");      Serial.print(voltage);      Serial.println("V");
    Serial.print("Current: ");      Serial.print(current);      Serial.println("A");
    Serial.print("Power: ");        Serial.print(power);        Serial.println("W");
    Serial.print("Energy: ");       Serial.print(energy / 1000.0);     Serial.println("kWh");
    Serial.print("Frequency: ");    Serial.print(frequency, 1); Serial.println("Hz");
    Serial.print("PF: ");           Serial.println(pf);
    Serial.print("ค่าไฟ: ");      Serial.print(energyCost);      Serial.println("บาท");

    Blynk.virtualWrite(V1, voltage);
    Blynk.virtualWrite(V2, current);
    Blynk.virtualWrite(V3, power);
    Blynk.virtualWrite(V4, energy / 1000.0);
    Blynk.virtualWrite(V5, frequency);
    Blynk.virtualWrite(V6, pf);
    Blynk.virtualWrite(V8, energyCost);

    // Update LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Voltage: ");
    lcd.print(voltage);
    lcd.print("V");

    lcd.setCursor(0, 1);
    lcd.print("Current: ");
    lcd.print(current);
    lcd.print("A");

    lcd.setCursor(0, 2);
    lcd.print("Power: ");
    lcd.print(power);
    lcd.print("W");

    lcd.setCursor(0, 3);
    lcd.print("Energy: ");
    lcd.print(energy / 1000.0);
    lcd.print("kWh");
  }

  Serial.println();
  delay(500);
}
