/**
   ESPNOW - Basic communication - Slave
   Date: 26th September 2017
   Author: Arvind Ravulavaru <https://github.com/arvindr21>
   Purpose: ESPNow Communication between a Master ESP32 and a Slave ESP32
   Description: This sketch consists of the code for the Slave module.
   Resources: (A bit outdated)
   a. https://espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
   b. http://www.esploradores.com/practica-6-conexion-esp-now/

   << This Device Slave >>

   Flow: Master
   Step 1 : ESPNow Init on Master and set it in STA mode
   Step 2 : Start scanning for Slave ESP32 (we have added a prefix of `slave` to the SSID of slave for an easy setup)
   Step 3 : Once found, add Slave as peer
   Step 4 : Register for send callback
   Step 5 : Start Transmitting data from Master to Slave

   Flow: Slave
   Step 1 : ESPNow Init on Slave
   Step 2 : Update the SSID of Slave with a prefix of `slave`
   Step 3 : Set Slave in AP mode
   Step 4 : Register for receive callback and wait for data
   Step 5 : Once data arrives, print it in the serial monitor

   Note: Master and Slave have been defined to easily understand the setup.
         Based on the ESPNOW API, there is no concept of Master and Slave.
         Any devices can act as master or salve.
*/

#include <esp_now.h>
#include <WiFi.h>
#include <FastLED.h>
#include "settings.h"

EspNowMessage message;

CRGB leds[LED_COUNT];

void sendDebugMessage() {
  message.counter++;
  message.lastCycleDuration++;
  const char* payload = "{\"value\": \"yes\"}";
  strncpy(message.payload, payload, sizeof(message.payload));
  Serial2.write((uint8_t *) &message, sizeof(message));
  Serial.println("Message transmitted");
}

void showStatus(CRGB led1, CRGB led2) {
    FastLED.clear();
    leds[0] = led1;
    leds[1] = led2;
    FastLED.show();
}

// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
    showStatus(CRGB::Green, CRGB::Green);
  } else {
    Serial.println("ESPNow Init Failed");
    showStatus(CRGB::Red, CRGB::Red);
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// config AP SSID
void configDeviceAP() {
  const char *SSID = "Slave_1";
  bool result = WiFi.softAP(SSID, "Slave_1_Password", CHANNEL, 0);

  if (!result) {
    Serial.println("AP Config failed.");
    showStatus(CRGB::Red, CRGB::Red);
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
    showStatus(CRGB::Green, CRGB::Green);
  }

}

// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  FastLED.setBrightness(0);
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  /*String message = "{\"sender\": \"" + String(macStr) + "\", \"message\": " 
    + String((const char*) data) + " }";*/
  Serial.printf("Data size: %d\n", data_len);
  Serial2.write(data, data_len);
  Serial.print("Last Packet Recv from: "); Serial.println(macStr);
  //Serial.print("Last Packet Recv Data: "); Serial.println(message);
  Serial.println("");
  FastLED.setBrightness(200);
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812B, ESP1_LED_PIN, GRB>(leds, LED_COUNT);
  FastLED.setBrightness(255);

  showStatus(CRGB::White, CRGB::White);

  Serial2.begin(BAUDRATE, SERIAL_8N1, GATEWAY_RXD2, GATEWAY_TXD2);
  //Set device in AP mode to begin with
  WiFi.mode(WIFI_STA);
  // configure device AP mode
  configDeviceAP();
  // This is the mac address of the Slave in AP Mode
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info.
  esp_now_register_recv_cb(OnDataRecv);
}



void loop() {
  //sendDebugMessage();
  delay(1000);
  // Chill
}
