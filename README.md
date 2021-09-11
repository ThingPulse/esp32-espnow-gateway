# ThingPulse ESPGateway

The ThingPulse ESPGateway is a versatile device that comes with two ESP32-WROVER-IB modules.
The two ESP32s are connected by two lines used for inter-ESP32 communication over Serial
or other protocols. Each of the ESP32 can also control two WS2812B RGB Leds at the front 
of the device. 

The ESPGateway is available for purchase at [ThingPulse ESPGateway](https://thingpulse.com/product/espgateway/)

![Gateway](/resources/GatewayWithAntenna.jpg)

## The Project

This project shows how the ESPGateway can be used with ESPNow protocol.
One of the two ESP32 modules listens for ESPNow messages. When it receives
a message it will send it over the serial line to the other ESP32 module.
This module is connected over WiFi to a MQTT broker. When receiving
a message over the serial line it will forward the message to MQTT.

The project also contains a ESPNow node. The ESPNow node is usually a device 
running from battery power. It will send short messages over ESPNow to the gateway and go back to sleep.

![Architecture](/resources/Architecture.png)

In this blog post we describe how this can extend the battery life by a factor of 6 or more:
[ESPNow Blog Post](https://thingpulse.com/esp32-ultra-long-battery-life-with-espnow/)

## Building the project

This project contains actually code for three different ESP32 modules. Control which firmware
shall be flashed by selecting the right target in platformio.ini:

```
;default_envs = esp32-wifi-client
;default_envs = esp32-now-node
default_envs = esp32-now-gateway
````
for instance will build and flash the ESP32 module listening for the ESPNow messages.

## Pins

These are the occupied pins relevant for programming:

```
#define BUS_RXD     14
#define BUS_TXD     15
#define LED_PIN     32
```

![Labelled](/resources/GatewayInside.png)

## Photos

![Labelled](/resources/GatewayBackWithLabels.jpg)
![Back](/resources/GatewayBack.jpg)
![Perspective](/resources/GatewayPerspective.jpg)
![Top](/resources/GatewayTop.jpg)