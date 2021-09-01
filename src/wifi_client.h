

#include <WiFi.h>
extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
}
#include <AsyncMqttClient.h>
#include <FastLED.h>
#include "settings.h"

RTC_DATA_ATTR int bootCount = 0;

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1800        /* Time ESP32 will go to sleep (in seconds) */

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

CRGB leds[LED_COUNT];

struct EspNowMessage *message = (EspNowMessage *) malloc(sizeof *message); 

void showStatus(CRGB led1, CRGB led2) {
    FastLED.clear();
    leds[0] = led1;
    leds[1] = led2;
    FastLED.show();
}

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        showStatus(CRGB::Orange, CRGB::Orange);
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        showStatus(CRGB::Red, CRGB::Red);
        Serial.println("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
		    xTimerStart(wifiReconnectTimer, 0);
        break;
    }
}



void onMqttConnect(bool sessionPresent) {
  showStatus(CRGB::Blue, CRGB::Blue);
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe("irsend/1", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);


}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  showStatus(CRGB::Yellow, CRGB::Yellow);
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
  Serial.printf("payload: %s\n", payload);
  Serial.flush();

}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}



void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  FastLED.addLeds<WS2812B, ESP1_LED_PIN, GRB>(leds, LED_COUNT);
  FastLED.setBrightness(255);
  Serial2.begin(BAUDRATE, SERIAL_8N1, GATEWAY_RXD2, GATEWAY_TXD2);
  showStatus(CRGB::White, CRGB::White);
  ++bootCount;

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();
  showStatus(CRGB::Orange, CRGB::White);
}

void loop() {

  // Receiving message. Stop 
  while (Serial2.available()) {
    FastLED.setBrightness(0);
    FastLED.show();
    Serial2.readBytes((char*) message, sizeof(EspNowMessage));
    Serial.println("Message received: " + String(message->counter));
    String payload = "{\"counter\": " + String(message->counter) 
        + ", \"payload\":" + String(message->payload) + "}";
    mqttClient.publish(MQTT_TOPIC, 0, false, payload.c_str());
    FastLED.setBrightness(200);
    FastLED.show();
  }
}