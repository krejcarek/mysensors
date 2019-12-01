/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Henrik EKblad
 * 
 * DESCRIPTION
 * This sketch provides an example how to implement a humidity/temperature
 * sensor using DHT11/DHT-22 
 * http://www.mysensors.org/build/humidity
 */


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

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define HUMIDITY_SENSOR_DIGITAL_PIN 3
unsigned long SLEEP_TIME = 30000; // Sleep time between reads (in milliseconds)

float lastTemp;
float lastHum;
boolean metric = true; 
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);

#define DHTPIN 3     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

void setup()  
{ 
  dht.begin(); 
}

void presentation()  
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Humidity", "1.0");
  
  present(CHILD_ID_HUM, S_HUM, "HumiditySensor", false);
  present(CHILD_ID_TEMP, S_TEMP, "TemperatureSensor", false);

  Serial.print("Fetched node ID:");
  Serial.println(getNodeId());

}

void loop()      
{  
  delay(2000);

  float temperature = dht.readTemperature();
  if (isnan(temperature)) {
      Serial.println("Failed reading temperature from DHT");
  } else if (temperature != lastTemp) {
    lastTemp = temperature;
   
    send(msgTemp.set(temperature, 1));
    Serial.print("T: ");
    Serial.println(temperature);
  }
  
  float humidity = dht.readHumidity();
  if (isnan(humidity)) {
      Serial.println("Failed reading humidity from DHT");
  } else if (humidity != lastHum) {
      lastHum = humidity;
      send(msgHum.set(humidity, 1));
      Serial.print("H: ");
      Serial.println(humidity);
  }

  sleep(SLEEP_TIME); //sleep a bit
}


