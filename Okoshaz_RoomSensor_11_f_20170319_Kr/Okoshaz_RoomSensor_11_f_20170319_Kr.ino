
//#define MY_DEBUG_SKETCH
//#define MY_DEBUG //!< Enable debug prints to serial monitor

#define MY_RADIO_NRF24 //!< NRF24L01 radio driver

#define MY_RF24_PA_LEVEL RF24_PA_HIGH
#define MY_REPEATER_FEATURE

#define MY_NODE_ID 11
//#define MY_PARENT_NODE_ID 0
//#define MY_PARENT_NODE_IS_STATIC

#include <SPI.h>
#include <MySensors.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>


//MySensors declaration -------------------------------------------------------
#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define CHILD_ID_MOT 2
#define CHILD_ID_LIGHT 3


MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgMot(CHILD_ID_MOT, V_TRIPPED);
MyMessage msgLight(CHILD_ID_LIGHT, V_LIGHT_LEVEL);

unsigned long SLEEP_TIME = 10000; // Sleep time between reads (in milliseconds)
unsigned long lastRefreshTime = 0; // Use this to implement a non-blocking delay function
unsigned long DHT_INT = 1800000; // Sleep time between reads (in milliseconds)
//unsigned long DHT_INT = 30000; // Sleep time between reads (in milliseconds)
unsigned long lastRefreshTime_DHT = 0; // Use this to implement a non-blocking delay function


//DHT declaration -------------------------------------------------------------
float lastTemp = 0;
float lastHum = 0;
int lastLight = 0;
boolean lastTripped = false;
boolean metric = true;

#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT21   // DHT 11 22 21 AM2301
DHT_Unified dht(DHTPIN, DHTTYPE);

//Motion sensor declaration ---------------------------------------------------
#define DIGITAL_INPUT_SENSOR 4   // The digital input you attached your motion sensor.  (Only 2 and 3 generates interrupt!)
boolean tripped = false;

//Light sensor declaration ---------------------------------------------------
#define LIGHT_INPUT_SENSOR A2   // The digital input you attached your motion sensor.  (Only 2 and 3 generates interrupt!)


void before()
{
  dht.begin();
  pinMode(DIGITAL_INPUT_SENSOR, INPUT);
  pinMode(LIGHT_INPUT_SENSOR, INPUT);
}


void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("RoomSensor", "1.0");

  present(CHILD_ID_HUM, S_HUM, "Humidity Sensor", false);
  present(CHILD_ID_TEMP, S_TEMP, "Temperature Sensor", false);
  present(CHILD_ID_MOT, S_MOTION, "Motion Sensor", false);
  present(CHILD_ID_LIGHT, S_LIGHT_LEVEL, "Light Sensor", false);

#ifdef MY_DEBUG_SKETCH
  Serial.print("Fetched node ID:");
  Serial.println(getNodeId());
#endif
}

void loop()
{

  boolean tripped = digitalRead(DIGITAL_INPUT_SENSOR);
  if (tripped != lastTripped) {
    lastTripped = tripped;
    send(msgMot.set(tripped)); // Send tripped value to gw
#ifdef MY_DEBUG_SKETCH
    Serial.print("Motion: ");
    Serial.println(tripped);
#endif
  }


  if ((millis() - lastRefreshTime) > SLEEP_TIME) {
    lastRefreshTime = millis();
    sensors_event_t event_T;
    sensors_event_t event_H;

    int light = analogRead(LIGHT_INPUT_SENSOR);
    dht.temperature().getEvent(&event_T);
    dht.humidity().getEvent(&event_H);
    if (isnan(event_T.temperature) || isnan(event_H.relative_humidity)) {
#ifdef MY_DEBUG_SKETCH
      Serial.println("Failed reading temperature or humidity from DHT");
#endif
    } else if (light > lastLight + 50 || light < lastLight - 50 || event_T.temperature > lastTemp + 0.3 || event_T.temperature < lastTemp - 0.3 || event_H.relative_humidity > lastHum + 3 || event_H.relative_humidity < lastHum - 3 || (millis() - lastRefreshTime_DHT) > DHT_INT)
    {
      lastRefreshTime_DHT = millis();
      lastTemp = event_T.temperature;
      lastHum = event_H.relative_humidity;
      lastLight = light;

      send(msgTemp.set(event_T.temperature, 1));
      send(msgHum.set(event_H.relative_humidity, 0));
      send(msgLight.set(light));

#ifdef MY_DEBUG_SKETCH
      Serial.print(lastRefreshTime);
      Serial.print(": T: ");
      Serial.print(event_T.temperature);
      Serial.print("  H: ");
      Serial.print(event_H.relative_humidity);
      Serial.print("  Light: ");
      Serial.println(light);
#endif

    }
  }



}

