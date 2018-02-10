
#include <Arduino.h>
#include <EEPROM.h>
#include <math.h>



// ********************* EEPROM DEFINES **********************************************
#define EEPROM_OFFSET_SKETCH  512                   //  Address start where we store our data (before is mysensors stuff)
#define EEPROM_POSITION_OFFSET  5 //  Address of shutter last position
#define EEPROM_TRAVELUP_OFFSET  1  //  Address of travel time for Up. 16bit value
#define EEPROM_TRAVELDOWN_OFFSET  3 //  Address of travel time for Down. 16bit value


// ********************* CALIBRATION DEFINES *****************************************
#define START_POSITION            0                     // Shutter start position in % if no parameter in memory

#define TRAVELTIME_UP_DEFAULT             10000                 // in ms, time measured between 0-100% for Up. it's initialization value, these will be overriden
#define TRAVELTIME_DOWN_DEFAULT           5000                 // in ms, time measured between 0-100% for Down


// ********************* State Machine DEFINES *****************************************

typedef struct {
  uint32_t  upTimeRef               = TRAVELTIME_UP_DEFAULT;
  uint32_t  downTimeRef             = TRAVELTIME_DOWN_DEFAULT;
  int8_t    positionOld             = START_POSITION;
  int8_t    positionRequested       = 0;
  uint32_t  stateEnter              = 0;
  uint32_t  travelTime              = 0;
  boolean   moving                  = false;

} ShutterSM;


ShutterSM Shutter[NUM_SHUTTERS];

void ShutterRelay(uint8_t shutter, int shutterDir){
  switch(shutterDir){
    case SHUTTER_UP:{
      digitalWrite(relayPins[shutter*2+1], RELAY_OFF); 
      digitalWrite(relayPins[shutter*2], RELAY_ON);   
    }      
    break;
    
    case SHUTTER_DOWN:{
      digitalWrite(relayPins[shutter*2+1], RELAY_ON);
      digitalWrite(relayPins[shutter*2], RELAY_ON);
    }
    break;
    
    case SHUTTER_STOP:{
      digitalWrite(relayPins[shutter*2], RELAY_OFF);
      digitalWrite(relayPins[shutter*2+1], RELAY_OFF);
    }
    break;
  }
}

void ShutterStop(uint8_t shutter)
{
  if (Shutter[shutter].moving) {
    uint32_t travelled = (millis() - Shutter[shutter].stateEnter);
  #ifdef MY_DEBUG_SKETCH 
       Serial.print(F("Stopping... Elapsed time: "));Serial.println(travelled); 
  #endif 
    if(Shutter[shutter].positionOld>Shutter[shutter].positionRequested) { //shutter going up
      int posDelta = round((float)travelled/(float)(Shutter[shutter].upTimeRef)*100.0);
  #ifdef MY_DEBUG_SKETCH 
       Serial.print(F("And travelled: "));Serial.println(posDelta); 
  #endif 
      Shutter[shutter].positionOld=constrain(Shutter[shutter].positionOld-posDelta,0,100);
    }
    else {                                                                //shutter going down
      int posDelta = round((float)travelled/(float)(Shutter[shutter].downTimeRef)*100.0);  

  #ifdef MY_DEBUG_SKETCH 
       Serial.print(F("And travelled: "));Serial.println(posDelta); 
  #endif
      Shutter[shutter].positionOld=constrain(Shutter[shutter].positionOld+posDelta,0,100);
    }
  }
  ShutterRelay(shutter,SHUTTER_STOP);
  Shutter[shutter].moving=false;
  saveSMState(shutter);
  
  MyMessage msgShutterPosition(shutter,V_PERCENTAGE); // Message for % shutter 
  send(msgShutterPosition.set(Shutter[shutter].positionOld));
  
 #ifdef MY_DEBUG_SKETCH 
       Serial.print(F("Shutter Stop. Position at: "));Serial.println(Shutter[shutter].positionOld); 
  #endif 
}

void setPosition(uint8_t shutter, int shutterPos) {

  if (Shutter[shutter].moving) ShutterStop(shutter);
  int posDelta=shutterPos-Shutter[shutter].positionOld;
  #ifdef MY_DEBUG_SKETCH 
       Serial.print(F("Position Delta: "));Serial.println(posDelta); 
  #endif 
  if (posDelta!=0){
    Shutter[shutter].stateEnter=millis();
    Shutter[shutter].positionRequested=shutterPos;
    if (posDelta>0){                                                        //shutter going down
      Shutter[shutter].travelTime=round((float)(Shutter[shutter].downTimeRef*posDelta)/100.0);
      ShutterRelay(shutter,SHUTTER_DOWN);
    }
    else{                                                                   //shutter going up
      Shutter[shutter].travelTime=round((float)(Shutter[shutter].upTimeRef*(-posDelta))/100.0);
      ShutterRelay(shutter,SHUTTER_UP);
    }
    


    #ifdef MY_DEBUG_SKETCH 
       Serial.print(F("Heading towards: "));Serial.println(Shutter[shutter].positionRequested); 
    #endif 
    Shutter[shutter].moving=true;
  }
}





void shutterUpdateSM() {
  for (int s = 0; s < NUM_SHUTTERS; s++) {
    if (Shutter[s].moving) {
      if (millis()>=Shutter[s].travelTime+Shutter[s].stateEnter) ShutterStop(s);
    }
  }
}


void writeEeprom2Byte(uint16_t pos, uint16_t value) {
  
    EEPROM.write(pos, ((uint16_t)value >> 8 ));
    pos++;
    EEPROM.write(pos, (value & 0xff));
    
}

uint16_t readEeprom2Byte(uint16_t pos) {
  
    uint16_t hiByte;
    uint16_t loByte;

    hiByte = EEPROM.read(pos) << 8;
    pos++;
    loByte = EEPROM.read(pos);
    return (hiByte | loByte);
    
}

void ShutterTimerSetUp(uint8_t shutter, uint32_t travelTime) {
  int offset = EEPROM_OFFSET_SKETCH + EEPROM_POSITION_OFFSET * shutter;
  writeEeprom2Byte(offset + EEPROM_TRAVELUP_OFFSET, travelTime);
  Shutter[shutter].upTimeRef=travelTime;
}

void ShutterTimerSetDown(uint8_t shutter, uint32_t travelTime) {
  int offset = EEPROM_OFFSET_SKETCH + EEPROM_POSITION_OFFSET * shutter;
  writeEeprom2Byte(offset + EEPROM_TRAVELDOWN_OFFSET, travelTime);
  Shutter[shutter].downTimeRef=travelTime;
}

void saveSMState(uint8_t shutter) {
  int offset = EEPROM_OFFSET_SKETCH + (EEPROM_POSITION_OFFSET * shutter);
  EEPROM.write(offset, Shutter[shutter].positionOld);

}

void loadLastSM() {
  for (int s = 0; s < NUM_SHUTTERS; s++) {
    int offset = EEPROM_OFFSET_SKETCH + EEPROM_POSITION_OFFSET * s;
    Shutter[s].upTimeRef = readEeprom2Byte(offset + EEPROM_TRAVELUP_OFFSET);
    Shutter[s].downTimeRef = readEeprom2Byte(offset + EEPROM_TRAVELDOWN_OFFSET);
    Shutter[s].positionOld = EEPROM.read(offset);
  }

}


