#include "Arduino.h"

#define OK_STATUS 2
#define EMPTY_STATUS 4
#define ERROR_STATUS 5

class DebugLogger {
  public:
    void printControlsState(byte data[8]);
    void printResponseInfo(unsigned long responseTime);
    void printResponseAck(byte ack);
};

void DebugLogger::printControlsState(byte data[8]) {
  Serial.print("btns: ");
  Serial.println(data[0]);
  Serial.print("pot1: ");
  Serial.println(data[1]);
  Serial.print("stick1: ");
  Serial.print(data[2]);
  Serial.print(" ");
  Serial.println(data[3]);
  Serial.print("stick2: ");
  Serial.print(data[4]);
  Serial.print(" ");
  Serial.println(data[5]);
  Serial.print("data[6]: ");
  Serial.println(data[6]);
  Serial.print("data[7]: ");
  Serial.println(data[7]);
}

void DebugLogger::printResponseInfo(unsigned long responseTime) {
    Serial.print(F("Success! "));                          // payload was delivered
    Serial.print(F("Time = "));
    Serial.print(responseTime);                // print the timer result
    Serial.print(F(" us."));
}

void DebugLogger::printResponseAck(byte ack) {
    switch (ack) {
      case OK_STATUS:
        Serial.println("OK: 2");
        break;

      case EMPTY_STATUS:
        Serial.println("EMPTY ACK: 4");

      case ERROR_STATUS:
        Serial.println("ERROR: 5");
      
      default:
        Serial.print("Unknown status got: ");
        Serial.println(ack);
        break;
    }
}
