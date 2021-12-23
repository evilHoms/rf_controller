/**
 * Transmitter code with RF24 lib for nrf24l01 radio module
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"


RF24 radio(10,9); // CE, CNE pins

/**
 * Transmitter and reciever have to use same tag.
 * also called as software channel.
 * is used to verify, that data is sent for exact reciever
 */
byte address[][6] = {"1Node","2Node","3Node","4Node","5Node","6Node"};  // pipe tags
byte isReciever = false;

int request = 0;

void setup(){
  Serial.begin(9600); //открываем порт для связи с ПК

  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  while (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    delay(1000);
  }

  radio.setRetries(0, 15);                    // (время между попыткой достучаться, число попыток)
  radio.setPayloadSize(sizeof(request));      // Size of payload

  radio.openWritingPipe(address[0]);   // мы - труба 0, открываем канал для передачи данных

  // Remove this line, or change channel in case if got error during sending request
  radio.setChannel(77);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel(RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate(RF24_1MBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!
  // ВНИМАНИЕ!!! enableAckPayload НЕ РАБОТАЕТ НА СКОРОСТИ 250 kbps!

  //  radio.setAutoAck(0);         // режим подтверждения приёма, 1 вкл 0 выкл, enabled by default
  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал

  /**
   * TODO enableAckPayload and try to add custom acknoledge payload
   */

  radio.powerUp();                        // Set increased power mode for better sensitivity
  radio.stopListening();                  // Set as transmitter

  Serial.println(F("Radio connected as transmitter!!"));
  Serial.print(F("Listening pipe: "));
  Serial.println((char*)address[0]);
  Serial.print(F("On channel: "));
  Serial.println(radio.getChannel());
}

void loop(void) {    
  byte res = 0;
  Serial.print("Sending... ");
  Serial.println(request);

  unsigned long startTime = micros(); 
  bool report = radio.write(&request, sizeof(request));
  unsigned long endTime = micros();

  if (report) {
    Serial.print(F("Transmission successful! "));          // payload was delivered
    Serial.print(F("Time to transmit = "));
    Serial.print(endTime - startTime);                     // print the timer result
    Serial.print(F(" us. Sent: "));
    Serial.println(request);

    if (radio.available()) {                               // handle acknoledge packet
      byte payloadSize = radio.getPayloadSize();
      radio.read(&res, payloadSize);
      Serial.print("Acknoledge packet: ");
      Serial.println(res);

      request = res;
    }
  } else {
    Serial.println(F("Transmission failed or timed out"));
  }

  delay(1000);
}

//int toBytes() {
//  int v = 0;
//  int v1 = pow(2, 0);    // key 1 byte
//  int v2= pow(2, 1);    // key 1 byte
//  int v3= pow(2, 2);    // key 1 byte
//  int v4= pow(2, 3);    // key 1 byte
//  int v5= pow(2, 4);    // btns 1 byte
//  int v6= pow(2, 5);    // btns 1 byte
//  int v7= pow(2, 6);    // btns 1 byte
//  int v8= pow(2, 7);    // btns 1 byte
//  int v9= pow(2, 8);    // pot1 2byte
//  int v10= pow(2, 9);    // pot1 2byte
//  int v11= pow(2, 10);   // pot1 2byte
//  int v12= pow(2, 11);   // pot1 2byte
//  int v13= pow(2, 12);   // pot1 2byte
//  int v14= pow(2, 13);   // pot1 2byte
//  int v15= pow(2, 14);   // pot1 2byte
//  int v16= pow(2, 15);   // pot1 2byte
//  int v17= pow(2, 16);   // stick1 x 3byte
//  int v18= pow(2, 17);   // stick1 x 3byte
//  int v19= pow(2, 18);   // stick1 x 3byte
//  int v20= pow(2, 19);   // stick1 x 3byte
//  int v21= pow(2, 20);   // stick1 x 3byte
//  int v22= pow(2, 21);   // stick1 x 3byte
//  int v23= pow(2, 22);   // stick1 x 3byte
//  int v24= pow(2, 23);   // stick1 x 3byte
//  int v25= pow(2, 24);   // stick1 y 4byte
//  int v26= pow(2, 25);   // stick1 y 4byte
//  int v27= pow(2, 26);   // stick1 y 4byte
//  int v28= pow(2, 27);   // stick1 y 4byte
//  int v29= pow(2, 28);   // stick1 y 4byte
//  int v30= pow(2, 29);   // stick1 y 4byte
//  int v31= pow(2, 30);   // stick1 y 4byte
//  int v32= pow(2, 31);   // stick1 y 4byte
//}
//
//void dataToCharString() {
//  // get data from sensors and transform
//  
//  byte b[8];
//  b[0] = 123;
//  
//  // 0  - all false;
//  // 1  - 0 0 0 1;
//  // 2  - 0 0 1 0;
//  // 3  - 0 0 1 1;
//  // 4  - 0 1 0 0;
//  // 5  - 0 1 0 1;
//  // 6  - 0 1 1 0;
//  // 7  - 0 1 1 1;
//  // 8  - 1 0 0 0;
//  // 9  - 1 0 0 1;
//  // 10 - 1 0 1 0;
//  // 11 - 1 0 1 1;
//  // 12 - 1 1 0 0;
//  // 13 - 1 1 0 1;
//  // 14 - 1 1 1 0;
//  // 15 - 1 1 1 1;
//  b[1] = 0; // state of all btns;
//  b[2] = 0; // pot1
//  b[3] = 0; // stick1 x
//  b[4] = 0; // stick1 y
//  b[5] = 0; // stick2 x
//  b[6] = 0; // stick2 y
//
//  // 0 - all false;
//  // 1 - 0 1;
//  // 2 - 1 0;
//  // 3 - 1 1;
//  b[7] = 0; // state of stick btns
//  b[8] = 0;
//}
