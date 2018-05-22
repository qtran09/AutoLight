#define BLYNK_PRINT Serial
#include <Wire.h>
#include <ESP8266WiFi.h>
#include<Adafruit_NeoPixel.h>
#include <BlynkSimpleEsp8266.h>
#include <Adafruit_TSL2561_U.h>
 
// Light Switch
#define BUTTON 0

// Lux Sensor I2C Links
#define SCL 2
#define SDA 14

// Light Connection
#define LIGHT_PIN 5
#define RING_LEDS 24

// Execution Delay
#define GLOBAL_DELAY 300
#define LIGHT_DELAY 100

// Blynk Wireless Connection
char auth[] = "00026e8a845d49f3bc59e817c732f3d4";
char ssid[] = "Q";
char pwd[] = "390ntest";

// Environment Properties
long global_timestamp = 0, light_timestamp = 0, blynk_timer = 0;
bool manual_setting = false; 

// Lux Sensor Properties
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
TwoWire lux_wires = TwoWire();
double current_lux = 0;

// NeoPixel Ring Properties
Adafruit_NeoPixel ring = Adafruit_NeoPixel(RING_LEDS, LIGHT_PIN, NEO_GRB + NEO_KHZ800);
int current_config = 0, final_config = 0, transition_step = 0; 

// Initial Lux Sensor Setup
void setup_lux_sensor() {
  lux_wires.begin(SDA, SCL);
  tsl.begin(&lux_wires);
  tsl.enableAutoRange(true);
  Serial.println("Lux sensor setup!");
}

// NeoPixel Ring Setup
void setup_neopixel_ring() {
  ring.begin();
  ring.setBrightness(1);
  for(int i = 0; i < RING_LEDS; i++) ring.setPixelColor(i, ring.Color(255, 130, 40));
  Serial.println("Neopixel ring setup!");
}

// Initial Program Setup
void setup() {
  Serial.begin(9600);

  // Setup Sensors
  setup_lux_sensor();
  setup_neopixel_ring();

  // Setup Blynk
  Blynk.begin(auth, ssid, pwd);
  Serial.println("Blynk and setup done!");
  Blynk.virtualWrite(V0,1);
  Blynk.virtualWrite(V1,1);
  Blynk.virtualWrite(V2,1);
  Blynk.virtualWrite(V4,255,255,255);
}
//
void transition_light() {
  if(current_config < final_config) current_config += 1;
  else if(current_config > final_config) current_config -= 1;
  Serial.print("Brightness updated to: "); Serial.println(current_config);
  ring.setBrightness(current_config);
  ring.show();
}

void retrieve_light_transition_step(double lux) {
  // Define lux -> brightness and color function
  // Find final_config
  // Return step_diff
  // 650 -> 150 increase, 150 -> 0 decrease
  if(lux >= 650) final_config = 1;
  else if(lux > 150) final_config = ((800 - lux) / 650) * 100;
  else {
    lux = lux < 30 ? 30 : lux;
    final_config = (lux / 150) * 100;
  }
  Serial.print("Final Config: "); Serial.println(final_config);
  }

double read_lux_values() {
  sensors_event_t event;
  tsl.getEvent(&event);
  return event.light ? event.light : -1;
}
//Brightness meter
BLYNK_WRITE(V1){
  //Change to manual settings
  manual_setting = true;
  int brightness = param.asInt();
  ring.setBrightness(brightness);
  ring.show();
  //Need to implement a timer
  Serial.print("Brightness lowered from meter: "); Serial.println(brightness);
}
//Power button
BLYNK_WRITE(V0){
  int power_status = param.asInt();
  //Off => Set brightness to 1
  if(power_status == 0){
    manual_setting = true;
    ring.setBrightness(1);
    for(int i = 0; i < RING_LEDS; i++) ring.setPixelColor(i, ring.Color(0,0,0));
    current_config = 1;
    Serial.println("Power off");
    ring.show();
    Blynk.virtualWrite(V0, 0);
  }
  else{
    manual_setting = false;
    blynk_timer = 0;
    Serial.println("Power on");
    for(int i = 0; i < RING_LEDS; i++) ring.setPixelColor(i, ring.Color(255,255,255));
    Blynk.virtualWrite(V0,1);
  }
}
//Reset button
BLYNK_WRITE(V3){
  manual_setting = false;
  blynk_timer = 0;
  Blynk.virtualWrite(V0,1);
  Blynk.virtualWrite(V1,1);
  Blynk.virtualWrite(V2,1);
  Blynk.virtualWrite(V4,255,255,255);
  for(int i = 0; i < RING_LEDS; i++) ring.setPixelColor(i, ring.Color(255,255,255));

  current_config = 1;
}
//Menu items
BLYNK_WRITE(V2){
  int item = param.asInt();
  switch(item){
    case 2: //Party
      for(int i = 0; i < RING_LEDS; i++){
        if(i%2 == 0) ring.setPixelColor(i,ring.Color(255,0,0));
        else ring.setPixelColor(i,ring.Color(0,255,0));
      }
      ring.setBrightness(50);
      manual_setting =true;
      break;
    case 3: //Chill
      for(int i = 0; i < RING_LEDS; i++) ring.setPixelColor(i, ring.Color(0, 145, 78));
      manual_setting =true;
      ring.setBrightness(25);
      break;
    case 4: //Bright
      manual_setting =true;
      for(int i = 0; i < RING_LEDS; i++) ring.setPixelColor(i, ring.Color(255, 255, 255));
      ring.setBrightness(60);
      break;
    case 5: //Dim
      manual_setting =true;
      for(int i = 0; i < RING_LEDS; i++) ring.setPixelColor(i, ring.Color(0, 50, 91));
      ring.setBrightness(20);
      break;
    default:
      manual_setting = false;
      break;
  }
  ring.show();
}
BLYNK_WRITE(V4){
  for(int i = 0; i < RING_LEDS; i++) ring.setPixelColor(i, ring.Color(param[0].asInt(), param[1].asInt(), param[2].asInt()));
  ring.show();
}
void loop() {
    sensors_event_t event;
    tsl.getEvent(&event);
  // 
  //  /* Display the results (light is measured in lux) */
  //  if (event.light)
  //  {
  //    Serial.print(event.light); Serial.println(" lux");
  //  }
  //  else
  //  {
  //    /* If event.light = 0 lux the sensor is probably saturated
  //       and no reliable data could be generated! */
  //    Serial.println("Sensor overload");
  //  }
  //  read_lux_values();
  //  delay(250);
    Blynk.run();
    long current_timestamp = millis();
      if(manual_setting == false){

    if(current_timestamp - global_timestamp >= GLOBAL_DELAY) {
      global_timestamp = current_timestamp;
      current_lux = read_lux_values();
      if(current_lux == -1) return;
      Serial.print("Lux: "); Serial.println(current_lux);
      retrieve_light_transition_step(current_lux);
    }
    else if(current_timestamp - light_timestamp >= LIGHT_DELAY) {
      light_timestamp = current_timestamp;
      transition_light();
    }
  }
}
