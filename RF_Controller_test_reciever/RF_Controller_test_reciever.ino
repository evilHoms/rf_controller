#include "RF24Service.h"

RF24Service radio(10, 9);
const byte OK_STATUS = 200;
const byte EMPTY_STATUS = 4;
const byte ERROR_STATUS = 5;
byte request[8];
unsigned long maxTimeout = 10000000;

void setup(void) {
  Serial.begin(9600);

  /**
   * Set RF24 configurations
   */
  radio.init(RF24_PA_MAX, RF24_1MBPS, sizeof(request));
  radio.asReciever();
}

void loop(void) {
  byte pipeId;                          // openReadingPipe id, needed if few reading pipes opened
  while (radio.available(&pipeId)) {
    byte payloadSize = radio.getPayloadSize();
    radio.read(&request, payloadSize);
  
    Serial.print("Recieved:");
    for (byte i = 0; i < sizeof(request); i++) {
      Serial.print(" ");
      Serial.print(request[i]);
    }
    Serial.println("");
  }
}
