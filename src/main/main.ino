#include "config.h"

#define KOTEL D6
#define RELAY2 D8
#define TEMP_SENSOR D3
#define INTERVAL 300000
#define LOW_BOUND_TEMPERATURE 1
#define HIGH_BOUND_TEMPERATURE 3


#include <FastBot.h>
FastBot bot(BOT_TOKEN);
#include <microDS18B20.h>
MicroDS18B20<TEMP_SENSOR> sensor;

unsigned long previousMillis = 0;
bool kotelStatus = 0;
float temperature = 0;
bool op_mode = 0;

void setup() {
    connectWiFi();
    bot.setChatID(CHAT_ID);
    bot.attach(newMsg);
    pinMode(KOTEL, OUTPUT);
    pinMode(RELAY2, OUTPUT);
    digitalWrite(RELAY2, HIGH);
    bot.showMenu("Kotel ON \t Svet ON \n Kotel OFF \t Svet OFF \n auto \t manual \n Temp");
    sensor.requestTemp();
    delay(20);
}

// message handler
void newMsg(FB_msg& msg) {
  if (msg.text == "Kotel ON" && op_mode) {
      digitalWrite(KOTEL, HIGH);
      kotelStatus = HIGH;
      bot.sendMessage("Kotel ON", CHAT_ID);
  }
  if (msg.text == "Kotel OFF" && op_mode) {
      digitalWrite(KOTEL, LOW);
      kotelStatus = LOW;
      bot.sendMessage("Kotel OFF", CHAT_ID);
  }
  if (msg.text == "Svet OFF") {
      digitalWrite(RELAY2, LOW);
      bot.sendMessage("Svet OFF", CHAT_ID);
  }
  if (msg.text == "Svet ON") {
      digitalWrite(RELAY2, HIGH);
      bot.sendMessage("Svet ON", CHAT_ID);
  }

  if (msg.text == "auto") {
      op_mode = 0;
      bot.sendMessage("auto mode", CHAT_ID);
  }

  if (msg.text == "manual") {
      op_mode = 1;
      bot.sendMessage("manual mode", CHAT_ID);
  }
  
  if (msg.text == "Temp") {
      sensor.requestTemp();
      delay(20);
      String ktState = kotelStatus ? "ON" : "OFF";
      String repl = String(sensor.getTemp()) + " °C" + '\n' + "Kotel " + ktState;
      bot.sendMessage(repl, CHAT_ID);
  }
}

void loop() {
    bot.tick();
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis > INTERVAL) {
        previousMillis = currentMillis;  
        sensor.requestTemp();
        delay(20);
        temperature = sensor.getTemp();
    
        if (temperature < LOW_BOUND_TEMPERATURE && !kotelStatus && !op_mode) {
            kotelStatus = HIGH;
            digitalWrite(KOTEL, HIGH);
            bot.sendMessage("Kotel auto ON", CHAT_ID);
            bot.sendMessage(String(temperature) + " °C", CHAT_ID);
        }
        if (temperature > HIGH_BOUND_TEMPERATURE && kotelStatus && !op_mode) {
            kotelStatus = LOW;
            digitalWrite(KOTEL, LOW);
            bot.sendMessage("Kotel auto OFF", CHAT_ID);
            bot.sendMessage(String(temperature) + " °C", CHAT_ID);
        }
    }
    
       
}

void connectWiFi() {
    delay(2000);
    Serial.begin(115200);
    Serial.println();
    
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      if (millis() > 15000) ESP.restart();
    }
    Serial.println("Connected");
}
