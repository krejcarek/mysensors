
//#define MY_DEBUG_SKETCH 
//#define MY_DEBUG //!< Enable debug prints to serial monitor
//#define MY_DEBUG_VERBOSE_SIGNING //!< Enable signing related debug prints to serial monitor

// SETTINGS FOR MY_SIGNING_SOFT

#define MY_SIGNING_SOFT //!< Software signing
#define MY_SIGNING_REQUEST_SIGNATURES
#define MY_SIGNING_SOFT_RANDOMSEED_PIN 7 //!< Unconnected analog pin for random seed

//MySensors declaration -------------------------------------------------------

#define MY_RADIO_NRF24 //!< NRF24L01 radio driver

#define MY_RF24_PA_LEVEL RF24_PA_HIGH
#define MY_REPEATER_FEATURE

#define MY_NODE_ID 51
#define MY_PARENT_NODE_ID 0
#define MY_PARENT_NODE_IS_STATIC

#include <SPI.h>
#include <MySensors.h>
#include <DHT.h>

//unsigned long SLEEP_TIME = 10000; // Sleep time between reads (in milliseconds)
//unsigned long lastRefreshTime = 0; // Use this to implement a non-blocking delay function

// ********************* SHUTTER/RELAY DEFINES **********************************************

#define NUM_SHUTTERS  4
#define NUMBER_OF_RELAYS 8 // Total number of attached relays
int relayPins[] = {14, 2, 3, 4, 5, 6, 7, 8};

#define RELAY_ON 0  // GPIO value to write to turn on attached relay
#define RELAY_OFF 1 // GPIO value to write to turn off attached relay

#define SHUTTER_UP -1
#define SHUTTER_STOP 0
#define SHUTTER_DOWN 1

#define ENDPOSITION 10

// ***************************************************************

void before() {
  for (int sensor = 0; sensor < NUMBER_OF_RELAYS; sensor++) {
    // Then set relay pins in output mode
    pinMode(relayPins[sensor], OUTPUT);
    digitalWrite(relayPins[sensor], RELAY_OFF);
  }

  loadLastSM();
}


void setup() {
}


void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Okoshaz_Redony", "1.0");

  for (int shutter = 0; shutter < NUM_SHUTTERS; shutter++) {
    // Register all sensors to gw (they will be created as child devices)
    present(shutter, S_COVER);
  }
}

void loop() {
   shutterUpdateSM(); 
}


void receive(const MyMessage &message) {
  
  if (message.isAck()) {}
  else {
    // Message received : Open shutters
    if (message.type == V_UP) {
      #ifdef MY_DEBUG_SKETCH 
      Serial.println(F("CMD: Up")); 
      #endif 
      setPosition(message.sensor,(0-ENDPOSITION));
    }  
       
    // Message received : Close shutters
    if (message.type == V_DOWN) {
      #ifdef MY_DEBUG_SKETCH 
      Serial.println(F("CMD: Down")); 
      #endif 
      setPosition(message.sensor,(100+ENDPOSITION));
    }
  
    // Message received : Stop shutters motor
    if (message.type == V_STOP) {
       #ifdef MY_DEBUG_SKETCH 
       Serial.println(F("CMD: Stop")); 
       #endif 
       ShutterStop(message.sensor); 
     }     
    
    // Message received : Set position of Rollershutter 
    if (message.type == V_PERCENTAGE) {
       if (message.getByte() >= 100) setPosition(message.sensor,100+ENDPOSITION);
       else{
       if (message.getByte() <= 0) setPosition(message.sensor,0-ENDPOSITION); else setPosition(message.sensor,message.getByte());
       }
       #ifdef MY_DEBUG_SKETCH 
          Serial.print(F("CMD: Set position to ")); 
          // Write some debug info               
          Serial.print(message.getByte());Serial.println(F("%")); 
       #endif     
     } 

    if (message.type == V_VAR1) {
      ShutterTimerSetUp(message.sensor,message.getULong()); 
    }
    
    if (message.type == V_VAR2) {
      ShutterTimerSetDown(message.sensor,message.getULong()); 
    } 
 
  }
     

}
