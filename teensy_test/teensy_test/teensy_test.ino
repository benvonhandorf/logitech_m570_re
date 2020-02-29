#include <SPI.h>

#include "initialization.h"
#include "poll.h"

SPISettings spiSettings = SPISettings(500000, MSBFIRST, SPI_MODE0);

int ssPin = 10;

byte buffer[107]; //Send buffer for buffer transfers

void printBuffer(byte *bytes, int length) {
  for(int i = 0; i < length; i++) {
    Serial.print(bytes[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void transferBuffer(byte *source, int length) {
  Serial.print("Transmitting ");
  Serial.print(length, DEC);
  Serial.println(" bytes");

  memcpy(buffer, source, length);

	SPI.transfer(buffer, length);
}

void validateResponse(byte *expected, int length) {
  printBuffer(expected, length);
  
  printBuffer(buffer, length);
  
	for(int i = 0; i < length; i++) {
		if(expected[i] != 0xFF
		    && buffer[i] != expected[i]) {
			Serial.print(i);
			Serial.print( " expected ");
			Serial.print(expected[i], HEX);
			Serial.print(" got ");
			Serial.print(buffer[i], HEX);
			Serial.println();
		}
	}
}

void setup() {
  Serial.begin(9600);

  delay(1000); //Allow teensy to reconfigure for a serial output stream

  Serial.println("Configuring pins");

  pinMode(ssPin, OUTPUT);
  digitalWrite(ssPin, 1);

  SPI.begin();

  Serial.println("Beginning initialization phase 1");

  SPI.beginTransaction(spiSettings);

  digitalWrite(ssPin, 0);

  transferBuffer(initialize_cmd_1, sizeof(initialize_cmd_1));

  validateResponse(initialize_response_1, sizeof(initialize_response_1));

  digitalWrite(ssPin, 1);

  SPI.beginTransaction(spiSettings);

  digitalWrite(ssPin, 0);

  transferBuffer(initialize_cmd_2, sizeof(initialize_cmd_2));

  validateResponse(initialize_response_2, sizeof(initialize_response_2));

  digitalWrite(ssPin, 1);

  SPI.endTransaction();
}

void loop() {

  delay(1000);

  SPI.beginTransaction(spiSettings);

  digitalWrite(ssPin, 0);

  transferBuffer(poll_cmd_1, sizeof(poll_cmd_1));

  validateResponse(poll_response_1, sizeof(poll_response_1));

  digitalWrite(ssPin, 1);

  SPI.endTransaction();

  SPI.beginTransaction(spiSettings);

  digitalWrite(ssPin, 0);

  transferBuffer(poll_cmd_2, sizeof(poll_cmd_2));

  digitalWrite(ssPin, 1);

  printBuffer(buffer, sizeof(poll_cmd_2));

  SPI.endTransaction();

}
