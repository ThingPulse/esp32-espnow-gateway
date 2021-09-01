#include <esp_now.h>
#include <WiFi.h>
#include "settings.h"
#include "driver/adc.h"
#include <esp_wifi.h>
#include <esp_bt.h>

#define TOUCH_INT GPIO_NUM_39
#define TOUCH_SDA 33
#define TOUCH_SCL 22

#define EPD_CS 4  // D8
#define EPD_RST 14 // D4
#define EPD_DC 12   // D1
#define EPD_BUSY 27 // D2
#define EPD_DIN 23
#define EPD_EN 15
#define SDA 33
#define SCL 22
#define IMU_INT1 GPIO_NUM_35
#define IMU_INT2 GPIO_NUM_34

// Variable to store if sending data was successful
String success;

RTC_DATA_ATTR uint16_t counter = 0;
RTC_DATA_ATTR uint16_t lastCycleDuration = 0;
RTC_DATA_ATTR uint16_t errorCount = 0;
EspNowMessage message;


// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
    errorCount = 0;
  }
  else{
    success = "Delivery Fail :(";
    errorCount++;
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  
  message.counter = counter;
  message.lastCycleDuration = lastCycleDuration;
  message.errorCount = errorCount;
  const char* payload = "{\"value\": \"yes\"}";
  strncpy(message.payload, payload, sizeof(message.payload));
  counter++;
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, gatewayAddress, 6);
  peerInfo.channel = CHANNEL;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(gatewayAddress, (uint8_t *) &message, sizeof(message));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  pinMode(EPD_CS, INPUT_PULLDOWN);
  pinMode(EPD_RST, INPUT_PULLDOWN);
  pinMode(EPD_DC, INPUT_PULLDOWN);
  pinMode(EPD_BUSY, INPUT_PULLDOWN);
  pinMode(EPD_DIN, INPUT_PULLDOWN);
  pinMode(EPD_EN, INPUT_PULLDOWN);
  pinMode(IMU_INT1, INPUT_PULLDOWN);
  pinMode(IMU_INT2, INPUT_PULLDOWN);
  pinMode(TOUCH_INT, INPUT_PULLDOWN);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();

  adc_power_off();
  esp_wifi_stop();
  esp_bt_controller_disable();


  lastCycleDuration = millis();
  Serial.printf("Time before sleep: %d\n", lastCycleDuration);

  esp_sleep_enable_timer_wakeup(1000 * 1000 * 5);
  //esp_sleep_enable_ext0_wakeup(IMU_INT1,1); 
  //rtc_gpio_pulldown_en(GPIO_NUM_35);


  esp_deep_sleep_start();

}
 
void loop() {



 
  delay(3000);
}
