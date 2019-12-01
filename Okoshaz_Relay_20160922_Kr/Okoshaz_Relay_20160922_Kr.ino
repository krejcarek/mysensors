
#define MY_DEBUG //!< Enable debug prints to serial monitor
//#define MY_DEBUG_VERBOSE_SIGNING //!< Enable signing related debug prints to serial monitor
//#define MY_NODE_LOCK_FEATURE //!< Enable lockdown of node if suspicious activity is detected

#define MY_RADIO_NRF24 //!< NRF24L01 radio driver
#define MY_SIGNING_SOFT //!< Software signing

//#define MY_SIGNING_NODE_WHITELISTING {{.nodeId = GATEWAY_ADDRESS,.serial = {0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01}}}
#define MY_SIGNING_REQUEST_SIGNATURES

// SETTINGS FOR MY_SIGNING_SOFT
#define MY_SIGNING_SOFT_RANDOMSEED_PIN 7 //!< Unconnected analog pin for random seed

#define MY_RF24_PA_LEVEL RF24_PA_HIGH
#define MY_REPEATER_FEATURE

#define MY_NODE_ID 12
#define MY_PARENT_NODE_ID 0
#define MY_PARENT_NODE_IS_STATIC

#include <SPI.h>
#include <MySensors.h>
#include <DHT.h>

//MySensors declaration -------------------------------------------------------

unsigned long SLEEP_TIME = 10000; // Sleep time between reads (in milliseconds)
unsigned long lastRefreshTime = 0; // Use this to implement a non-blocking delay function

//Relay declaration -------------------------------------------------------
#define RELAY_1  3  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define NUMBER_OF_RELAYS 2 // Total number of attached relays
#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay

void before() { 
  for (int sensor=1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS;sensor++, pin++) {
    // Then set relay pins in output mode
    pinMode(pin, OUTPUT);   
    // Set relay to last known state (using eeprom storage) 
    digitalWrite(pin, loadState(sensor)?RELAY_OFF:RELAY_ON);
  }
}


void setup(){
}


void presentation()  
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Relay", "1.0");

  for (int sensor=1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS;sensor++, pin++) {
    // Register all sensors to gw (they will be created as child devices)
    present(sensor, S_LIGHT);
  }
}

void loop(){
}


void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  //if (message.type==V_LIGHT) {
     // Change relay state
     digitalWrite(message.sensor-1+RELAY_1, message.getBool()?RELAY_OFF:RELAY_ON);
     // Store state in eeprom
     saveState(message.sensor, message.getBool());
     // Write some debug info
     Serial.print("Incoming change for sensor:");
     Serial.print(message.sensor);
     Serial.print(", New status: ");
     Serial.println(message.getBool());
   //} 

}
