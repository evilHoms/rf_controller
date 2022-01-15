/* Sketch for Controller, base on nrf24l01 module.
Data from controls (buttons, potentiometers, sticks)
transmitts to another nrf module.
Data converts in 8 bytes format, 1 byte for buttons,
others for analog values.
*/

#include <SPI.h>
#include "RF24Service.h"
#include "DebugLogger.h"

#define IS_DEBUG                  // To show debug info in Serial
#define RF_ENABLED                // Can be disabled in testing purpuses

#define BTN1_PIN 4
#define BTN2_PIN 5
#define BTN3_PIN 6
#define BTN4_PIN 7
#define POT1_PIN A0

#define STICK1_X A1
#define STICK1_Y A2
#define STICK1_BTN 3
#define STICK2_X A3
#define STICK2_Y A4
#define STICK2_BTN 2

#define CE_PIN 10
#define SCN_PIN 9

RF24Service radio(CE_PIN, SCN_PIN);
byte address[][6] = {"1Node","2Node","3Node","4Node","5Node","6Node"};
const unsigned short CONNECTED_DELAY = 50;
const unsigned short DISCONNECTED_DELAY = 500;

byte convertedData[8]; // Byte data to transmit

void dataFromPinsToBytes();

DebugLogger logger;

void setup(){
  #ifdef IS_DEBUG
    Serial.begin(9600);
  #endif

  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
  pinMode(BTN3_PIN, INPUT_PULLUP);
  pinMode(BTN4_PIN, INPUT_PULLUP);
  pinMode(STICK1_BTN, INPUT_PULLUP);
  pinMode(STICK2_BTN, INPUT_PULLUP);

  #ifdef RF_ENABLED
    /**
     * Set RF24 configurations
     */
    radio.init(RF24_PA_MAX, RF24_1MBPS, sizeof(convertedData));
    radio.asTransmitter();
    logger.begin();

    #ifdef IS_DEBUG
      Serial.println("Waiting for reciever...");
    #endif
  #endif
}

void loop(void) {
  dataFromPinsToBytes();
  logger.printControlsState(convertedData);

  unsigned short delayValue = CONNECTED_DELAY;                // Value of delay, if no acknoledge

  #ifdef RF_ENABLED
  bool res = radio.send(&convertedData, sizeof(convertedData));  // Same as write, but track additional information

  if (res) {
    if (!radio.isConnected()) {
      radio.connect();
    }
    logger.printResponseInfo(radio.lastResponseTime());
  } else {
    if (radio.isConnected()) {
      logger.timeOut();

      if (radio.fails() > 10) {
        radio.disconnect();
        logger.disconnect();
        radio.resetFails();
      }
    } else {
      delayValue = DISCONNECTED_DELAY;                            // Set longer delay value if no acknoledge
    }
  }
  #endif

  #ifndef IS_DEBUG
    delay(delayValue);
  #else
    delay(delayValue * 10);
  #endif
}

/* ----------------------------------------- */

void dataFromPinsToBytes() {
  //  Build object to transmit

  // 0  - all false;
  // 1  - 0 0 0 0 0 1;
  // 2  - 0 0 0 0 1 0;
  // ...
  // 31 - 0 1 1 1 1 1;
  // 32 - 1 0 0 0 0 0;
  // ...
  // 63 - 1 1 1 1 1 1
  byte btns[] = {1, 2, 4, 8, 16, 32, 64, 128};
  convertedData[0] = 0; // state of all btns;
  if (!digitalRead(BTN2_PIN)) convertedData[0] += btns[1];
  if (!digitalRead(BTN1_PIN)) convertedData[0] += btns[0];
  if (!digitalRead(BTN3_PIN)) convertedData[0] += btns[2];
  if (!digitalRead(BTN4_PIN)) convertedData[0] += btns[3];
  if (!digitalRead(STICK1_BTN)) convertedData[0] += btns[4];
  if (!digitalRead(STICK2_BTN)) convertedData[0] += btns[5];
 
  convertedData[1] = max(1, analogRead(POT1_PIN) >> 2); // pot1
  convertedData[2] = max(1, analogRead(STICK1_X) >> 2); // stick1 x
  convertedData[3] = max(1, analogRead(STICK1_Y) >> 2); // stick1 y
  convertedData[4] = max(1, analogRead(STICK2_X) >> 2); // stick2 x
  convertedData[5] = max(1, analogRead(STICK2_Y) >> 2); // stick2 y

  // Can be used for other controls
  convertedData[6] = 0;
  convertedData[7] = 0;
}
