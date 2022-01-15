#include "Arduino.h"

const byte OK_STATUS = 200;
const byte EMPTY_STATUS = 4;
const byte ERROR_STATUS = 5;

class DebugLogger {
  public:
    void printControlsState(byte data[8]);
    void printResponseInfo(unsigned long responseTime);
    void timeOut();
    void begin();
    void disconnect();
  private:
    bool _enabled = false;
};

void DebugLogger::begin() {
  _enabled = true;
}

void DebugLogger::printControlsState(byte data[8]) {
  if (_enabled){
    Serial.println("");
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
}

void DebugLogger::disconnect() {
  if (_enabled) {
    Serial.println("Disconnect...");
  }
}

void DebugLogger::timeOut() {
  if (_enabled) {
    Serial.println(F("Transmission failed or timed out"));
  }
}

void DebugLogger::printResponseInfo(unsigned long responseTime) {
  if (_enabled) {
    Serial.print(F("Success! "));                          // payload was delivered
    Serial.print(F("Time = "));
    Serial.print(responseTime);                // print the timer result
    Serial.print(F(" us."));
  }
}
