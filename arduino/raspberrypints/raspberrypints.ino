//Updated to use RF24Network instead of serial
#include <avr/pgmspace.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <Tictocs.h>
#include <Button.h>
#include <TictocTimer.h>
#include <nodeconfig.h>
#include <sleep.h>
#include <S_message.h>
#include <printf.h>


// Pins for radio
const int rf_ce = 8;
const int rf_csn = 9;

RF24 radio(rf_ce,rf_csn);
RF24Network network(radio);

// Our node configuration 
//eeprom_info_t this_node;
const uint8_t this_node = 01;

// Number of packets we've failed to send since we last sent one
// successfully
uint16_t lost_packets = 0;

//This line is the number of flow sensors connected.
const uint8_t numSensors = 4;
//This line initializes an array with the pins connected to the flow sensors
uint8_t pulsePin[] = {7,6,5,4};
//number of milliseconds to wait after pour before sending message
unsigned int pourMsgDelay = 300;

unsigned int pulseCount[numSensors];
unsigned int kickedCount[numSensors];
unsigned long nowTime;
unsigned long lastPourTime = 0;
unsigned long lastPinStateChangeTime[numSensors];
int lastPinState[numSensors];

unsigned long lastSend = 0;

void setup() {
  
  //
  // Print preamble
  //
  
  Serial.begin(57600);
  printf_begin();

  //
  // Bring up the RF network
  //

  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node);
  
  //
  // Request configuration from parent
  //

  RF24NetworkHeader header(network.parent(),'P');
  
  printf_P(PSTR("%lu: APP Sending type-%c to 0%o...\n\r"),millis(),header.type,header.to_node);
  if ( ! network.write(header,NULL,0) )
    printf_P(PSTR("Failed.\r\n"));
 
  //testing
  // Pump the network regularly
  //network.update();
  //Serial.print("7");
  //sendPulseCount(0, 7, 267);
  //delay(1000);

  
  Serial.flush();
  for( int i = 0; i < numSensors; i++ ) {
    pinMode(pulsePin[i], INPUT);
    digitalWrite(pulsePin[i], HIGH);
    kickedCount[i] = 0;
    lastPinState[i] = digitalRead(pulsePin[i]);
  }
}

void loop() {
  nowTime = millis();
  pollPins();
  if ( (nowTime - lastPourTime) > pourMsgDelay && lastPourTime > 0) {
    //only send pour messages after all taps have stopped pulsing for a short period
    //use lastPourTime=0 to ensure this code doesn't get run constantly
    lastPourTime = 0;
    checkPours();
    checkKicks();
  }
}

void pollPins() {
  for ( int i = 0; i < numSensors; i++ ) {
    int pinState = digitalRead(pulsePin[i]);
    if ( pinState != lastPinState[i] ) {
      if ( pinState == HIGH ) {
        //separate high speed pulses to detect kicked kegs
        if( nowTime - lastPinStateChangeTime[i] > 0 ){
          pulseCount[i] ++;
        }
        else{
          kickedCount[i] ++;
        }
        lastPinStateChangeTime[i] = nowTime;
        lastPourTime = nowTime;
      }
      lastPinState[i] = pinState;
    }
  }
}

void checkPours() {
  for( int i = 0; i < numSensors; i++ ) {
    if ( pulseCount[i] > 0 ) {
      if ( pulseCount[i] > 100 ) {
      //filter out tiny bursts
        sendPulseCount(0, pulsePin[i], pulseCount[i]);
      }
      pulseCount[i] = 0;
    }
  }
}

void checkKicks() {
  for( int i = 0; i < numSensors; i++ ) {
    if ( kickedCount[i] > 0 ) {
      if ( kickedCount[i] > 30 ) {
        //if there are enough high speed pulses, send a kicked message
        sendKickedMsg(0, pulsePin[i]);
      }
      //reset the counter if any high speed pulses exist
      kickedCount[i] = 0;
    }
  }
}
