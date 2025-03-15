#include "config.h"

#define KOTEL D6      // Котёл
#define SVET D5       // Свет (по умолчанию включен)
#define BOILER D7     // Бойлер
#define COND D8       // Кондиционер
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
bool boilerStatus = 0;
bool condStatus = 0;
bool svetStatus = HIGH;  // Свет включен по умолчанию
float temperature = 0;
bool op_mode = 0;



String ktState;
String svetState;
String boilerState;
String condState;
String opState;
String repl;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT); 
    delay(5);
    digitalWrite(LED_BUILTIN, LOW);
    connectWiFi();
    bot.setChatID(CHAT_ID);
    bot.attach(newMsg);

    pinMode(KOTEL, OUTPUT);
    pinMode(SVET, OUTPUT);
    pinMode(BOILER, OUTPUT);
    pinMode(COND, OUTPUT);

    digitalWrite(SVET, HIGH);  // Свет включен по умолчанию
    digitalWrite(KOTEL, LOW);   // Котёл выключен
    digitalWrite(BOILER, LOW);  // Бойлер выключен
    digitalWrite(COND, LOW);    // Кондиционер выключен

    bot.showMenu("Kotel ON \t Kotel OFF \n"
                 "Svet ON \t Svet OFF \n"
                 "Boiler ON \t Boiler OFF \n"
                 "Cond ON \t Cond OFF \n"
                 "auto_2 \t manual \t sost");
    sensor.requestTemp();
}

// message handler
void newMsg(FB_msg& msg) {
    if (msg.text == "Svet ON") {
        digitalWrite(SVET, HIGH);
        svetStatus = HIGH;
        bot.sendMessage("Svet ON", CHAT_ID);
    }
    else if (msg.text == "Svet OFF") {
        digitalWrite(SVET, LOW);
        svetStatus = LOW;
        bot.sendMessage("Svet OFF", CHAT_ID);
    }

    else if (msg.text == "Kotel ON" && op_mode) {
        digitalWrite(KOTEL, HIGH);
        kotelStatus = HIGH;
        bot.sendMessage("Kotel ON", CHAT_ID);
    }
    else if (msg.text == "Kotel OFF" && op_mode) {
        digitalWrite(KOTEL, LOW);
        kotelStatus = LOW;
        bot.sendMessage("Kotel OFF", CHAT_ID);
    }

    else if (msg.text == "Boiler ON") {
        digitalWrite(BOILER, HIGH);
        boilerStatus = HIGH;
        bot.sendMessage("Boiler ON", CHAT_ID);
    }
    else if (msg.text == "Boiler OFF") {
        digitalWrite(BOILER, LOW);
        boilerStatus = LOW;
        bot.sendMessage("Boiler OFF", CHAT_ID);
    }

    else if (msg.text == "Cond ON") {
        digitalWrite(COND, HIGH);
        condStatus = HIGH;
        bot.sendMessage("Cond ON", CHAT_ID);
    }
    else if (msg.text == "Cond OFF") {
        digitalWrite(COND, LOW);
        condStatus = LOW;
        bot.sendMessage("Cond OFF", CHAT_ID);
    }

    else if (msg.text == "auto_2") {
        op_mode = 0;
        bot.sendMessage("auto_2 mode", CHAT_ID);
    }

    else if (msg.text == "manual") {
        op_mode = 1;
        bot.sendMessage("manual mode", CHAT_ID);
    }

    
    else if (msg.text == "sost") {
        sensor.requestTemp();
        delay(20);
        ktState = kotelStatus ? "ON" : "OFF";
        svetState = svetStatus ? "ON" : "OFF";
        boilerState = boilerStatus ? "ON" : "OFF";
        condState = condStatus ? "ON" : "OFF";
        opState = (op_mode == 1 ? "manual" : "auto_2");
        repl = String(sensor.getTemp()) + " °C"
               + '\n' + "Kotel " + ktState
               + '\n' + "Svet " + svetState
               + '\n' + "Boiler " + boilerState
               + '\n' + "Cond " + condState
               + '\n' + opState + " mode";
        bot.sendMessage(repl, CHAT_ID);
  }

    else if (msg.OTA && msg.text == "update") bot.update();


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
            bot.sendMessage("Kotel auto_2 ON", CHAT_ID);
            bot.sendMessage(String(temperature) + " °C", CHAT_ID);
        }
        if (temperature > HIGH_BOUND_TEMPERATURE && kotelStatus && !op_mode) {
            kotelStatus = LOW;
            digitalWrite(KOTEL, LOW);
            bot.sendMessage("Kotel auto_2 OFF", CHAT_ID);
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
