
#define MY_DEBUG //!< Enable debug prints to serial monitor
#define MY_DEBUG_VERBOSE_SIGNING //!< Enable signing related debug prints to serial monitor
#define MY_NODE_LOCK_FEATURE //!< Enable lockdown of node if suspicious activity is detected

#define MY_RADIO_NRF24 //!< NRF24L01 radio driver
#define MY_SIGNING_SOFT //!< Software signing

//#define MY_SIGNING_NODE_WHITELISTING {{.nodeId = GATEWAY_ADDRESS,.serial = {0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01}}}
#define MY_SIGNING_REQUEST_SIGNATURES

// SETTINGS FOR MY_SIGNING_SOFT
#define MY_SIGNING_SOFT_RANDOMSEED_PIN 7 //!< Unconnected analog pin for random seed

#include <SPI.h>
#include <MySensors.h>
#include <DHT.h>

//MySensors declaration -------------------------------------------------------
#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define CHILD_ID_MOT 2

MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgMot(CHILD_ID_MOT, V_TRIPPED);
unsigned long SLEEP_TIME = 10000; // Sleep time between reads (in milliseconds)
unsigned long lastRefreshTime = 0; // Use this to implement a non-blocking delay function


//DHT declaration -------------------------------------------------------------
float lastTemp;
float lastHum;
boolean metric = true;

#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT21   // DHT 11 22 21 AM2301
DHT dht(DHTPIN, DHTTYPE);

//Motion sensor declaration ---------------------------------------------------
#define DIGITAL_INPUT_SENSOR 4   // The digital input you attached your motion sensor.  (Only 2 and 3 generates interrupt!)
boolean tripped = false; 

void setup()
{
  dht.begin();
  pinMode(DIGITAL_INPUT_SENSOR, INPUT);
}


void presentation()  
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Humidity", "1.0");
  
  present(CHILD_ID_HUM, S_HUM, "HumiditySensor", false);
  present(CHILD_ID_TEMP, S_TEMP, "TemperatureSensor", false);
  present(CHILD_ID_MOT, S_MOTION, "Motion Sensor", false);

  Serial.print("Fetched node ID:");
  Serial.println(getNodeId());
}

void loop()
{
  
  if (digitalRead(DIGITAL_INPUT_SENSOR) == HIGH) {
    tripped=true;
  } 

  boolean needRefresh = (millis() - lastRefreshTime) > SLEEP_TIME;
  if (needRefresh)
  {
    lastRefreshTime = millis();
    float temperature = dht.readTemperature();
    if (isnan(temperature)) {
            Serial.println("Failed reading temperature from DHT");
    } else 
    //if (temperature > lastTemp + 0.2 || temperature < lastTemp - 0.2) 
    {
      lastTemp = temperature;

      send(msgTemp.set(temperature, 1));
            Serial.print(lastRefreshTime);
            Serial.print(" T: ");
            Serial.println(temperature);
    }

    float humidity = dht.readHumidity();
    if (isnan(humidity)) {
            Serial.println("Failed reading humidity from DHT");
    } else 
    //if (humidity > lastHum + 0.2 || humidity < lastHum - 0.2) 
    {
      lastHum = humidity;
      send(msgHum.set(humidity, 1));
            Serial.print("H: ");
            Serial.println(humidity);
    }

  
    send(msgMot.set(tripped?"1":"0"));  // Send tripped value to gw 
    Serial.print("Motion: ");
    Serial.println(tripped);
    tripped = false;
    
  }


  
}


void incomingMessage(const MyMessage &message) {
}
