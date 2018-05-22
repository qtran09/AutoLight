# AutoLight
IoT project that automates LED light intensity, as well as additional functionality

## How it Works
Using our ESP8266 microcontroller, an Adafruit TSL2561 Lux Sensor, and an LED ring, the light intensity can be changed automatically based on the values the Lux sensor is able to read. The brightness of the LED is tuned to reasonable readings from the lux sensor, where very high readings(indicative of a very bright environemnt) will dim the LED ring, and lower readings(indicative of a dark environment) will increase the intensity. 

## Blynk
Using a Blynk app, we are able to manually control the settings of the LED. A connection is established between the microcontroller and the app, where users are able to power on and off the LEDs, control light intensity manually, or even change the colors emitted. There are additionally a few presets users can choose from that affect color and brightness.
![Blynk App](https://imgur.com/dA7fY5r)
