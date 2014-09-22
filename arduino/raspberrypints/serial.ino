#include <avr/pgmspace.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <Tictocs.h>
#include <Button.h>
#include <nodeconfig.h>
#include <sleep.h>
#include <S_message.h>
#include <printf.h>

void sendPulseCount(uint8_t addr, int pinNum, unsigned int pulseCount) {
  Serial.print("P;");
  Serial.print(addr);
  Serial.print(";");
  Serial.print(pinNum);
  Serial.print(";");
  Serial.println(pulseCount);
  // By default send to the base
  uint16_t to_node = 0;

  char message_type = 'P';
  S_message message;
  message.address = addr;
  message.pin_number = pinNum;
  message.pulseCount = pulseCount;  
  message.lost_packets = min(lost_packets,0xff);

  RF24NetworkHeader header(to_node,message_type);  
  
  printf_P(PSTR("---------------------------------\n\r"));
  printf_P(PSTR("%lu: APP Sending type-%c %s to 0%o...\n\r"),millis(),message_type,message.toString(),to_node);
  bool ok = network.write(header,&message,sizeof(message));
  if (ok)
  {
      //lost_packets = 0;
      printf_P(PSTR("%lu: APP Send ok\n\r"),millis());
  }
  else
  {
      //++lost_packets;
      printf_P(PSTR("%lu: APP Send failed\n\r"),millis());
  }
  // Power down the radio.  Note that the radio will get powered back up
  // on the next write() call.
  //radio.powerDown();

  // Be sure to flush the serial first before sleeping, so everything
  // gets printed properly
  Serial.flush();  
    
}

void sendKickedMsg(uint8_t addr, int pinNum) {
  Serial.print("K;");
  Serial.print(addr);
  Serial.print(";");
  Serial.println(pinNum);
  
    // By default send to the base
  uint16_t to_node = 0;

  char message_type = 'K';
  S_message message;
  message.address = addr;
  message.pin_number = pinNum;
  message.lost_packets = min(lost_packets,0xff);

  RF24NetworkHeader header(to_node,message_type);  
  
  printf_P(PSTR("---------------------------------\n\r"));
  printf_P(PSTR("%lu: APP Sending type-%c %s to 0%o...\n\r"),millis(),message_type,message.toString(),to_node);
  bool ok = network.write(header,&message,sizeof(message));
  if (ok)
  {
      //lost_packets = 0;
      printf_P(PSTR("%lu: APP Send ok\n\r"),millis());
  }
  else
  {
      //++lost_packets;
      printf_P(PSTR("%lu: APP Send failed\n\r"),millis());
  }
  // Power down the radio.  Note that the radio will get powered back up
  // on the next write() call.
  //radio.powerDown();

  // Be sure to flush the serial first before sleeping, so everything
  // gets printed properly
  Serial.flush();  
  
}
