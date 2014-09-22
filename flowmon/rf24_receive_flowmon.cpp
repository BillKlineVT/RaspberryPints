/*
 Initial Release Sept 21 2014 - billklinevt
 */

/**
 * Adapted from example code of TMRh20 RF24Network Library
 *
 * 
 * Listens for messages from the transmitter (arduino with flow meters in this case) and calls PHP to update pulseCounts.
 */



#include <cstdlib>
#include <iostream>
#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <ctime>
#include <stdio.h>
#include <time.h>
#include <string>
#include <iostream>

/**
 * g++ -L/usr/lib main.cc -I/usr/include -o main -lrrd
 **/
using namespace std;

// CE Pin, CSN Pin, SPI Speed

// Setup for GPIO 22 CE and CE0 CSN with SPI Speed @ 8Mhz
RF24 radio(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);  

RF24Network network(radio);

// Address of our node in Octal format
const uint16_t this_node = 00;

// Address of the other node in Octal format (01,021, etc)
const uint16_t other_node = 01;

const unsigned long interval = 2000; //ms  // How often to send 'hello world to the other unit

unsigned long last_sent;             // When did we last send?
unsigned long packets_sent;          // How many have we sent already


struct payload_t {                  // Structure of our payload
  uint16_t address;
  uint16_t pin_number;
  uint16_t pulseCount;
  uint8_t lost_packets;
};

int main(int argc, char** argv) 
{
	// Refer to RF24.h or nRF24L01 DS for settings

	radio.begin();
	
	delay(5);
	network.begin(/*channel*/ 90, /*node address*/ this_node);
	radio.printDetails();
	
	string path = "/var/www/includes/pours.php";

	while(1)
	{

		  network.update();
  		  while ( network.available() ) {     // Is there anything ready for us?
    			
		 	RF24NetworkHeader header;        // If so, grab it and print it out
   			 payload_t payload;
  			 network.read(header,&payload,sizeof(payload));
			
			//printf("Received payload address %1u pin %lu pulseCount %1u \n",payload.address,payload.pin_number,payload.pulseCount);
			if (header.type == 'P')
			{
			  	// Pour message received
				printf("Pour msg received: addr: %1u pinNum: %1u pulseCount: %1u",payload.address,payload.pin_number,payload.pulseCount);
				char pin_num[2];
				char pulse_count[20];
				sprintf(pin_num,"%u", payload.pin_number);
				sprintf(pulse_count,"%u", payload.pulseCount);
				string str_command = "php " + path + " " + pin_num + " " + pulse_count;
				char command[str_command.size()+1];
				strcpy(command,str_command.c_str());
				system(command);
			}
			else if (header.type == 'K')
			{
			  	// Kicked message received
				printf("Kicked msg received: addr: %1u pinNum: %1u",payload.address,payload.pin_number);
			}



  }		  
		 sleep(2);
		 //fclose(pFile);
	}

	return 0;
}

