/* В данном скетче устройство с nrf24l01 модулем представленно в виде передатчика.
 Данные в виде структуры Data передаются на приемник с проверкой ключа.
 Для выбора оптимального канала передачи без шумов, сканируется весь
 диапазон частот и выбирается оптимальный. Для подключения приемника
 к данной частоте, на нем следуют так же проводить сканирование частот,
 и установка того канала, на котором придут данные с предустановленным ключем
 */

#include <SPI.h>
#include "RF24Service.h"
//#include "nRF24L01.h"
//#include "RF24.h"

#define KEY 123 // Ключ безопасности, только при ответе с данным ключем будет произведено подключение
//#define SCAN_ENABLED // Если включено, ищется наиболее чистый канал
#define NUMBER_OF_SCANS 2 // Сила сканирования, чем больше, тем выше шанс избежать каналы с шумами, каждая итерация ~10 секунд
#define DEFAULT_CHANNEL 0x70 // Канал, если отключено сканирование
#define IS_DEBUG // Выводятся сообщения отладки
#define RF_ENABLED // Передатчик работает, false для тестирования кнопок без передачи данных

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

void getDataFromPins();
void destruct();
bool sendDestructedData(byte order);

RF24Service radio(CE_PIN, SCN_PIN);
byte address[][6] = {"1Node","2Node","3Node","4Node","5Node","6Node"};

typedef struct {
  bool btn;
  byte x;
  byte y;
}
Stick;

typedef struct {
  bool btn1;
  bool btn2;
  bool btn3;
  bool btn4;
  byte pot1;
  Stick stick1;
  Stick stick2;
}
Data;
Data data;

byte b[8]; // Byte data to transmit

typedef struct {
  byte order;
  byte value1;
  byte value2;
  byte value3;
}
DestructedData;

DestructedData destructedData;
DestructedData dDataSet[4];

int scanChannels();
void dataToBytes();

class DebugLogger {
  public:
    void printFreeChannel(int freeChannel);
    void printFreeChannel();
    void printControlsState(Data data);
    void printResponse(bool isRadioAvailable, unsigned long lastTime, bool response);
};

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
    radio.init();
    radio.asTransmitter();
    radio.withPayload();

    byte channel = DEFAULT_CHANNEL;
    #ifdef SCAN_ENABLED
     channel = radio.scanChannels(NUMBER_OF_SCANS);
    #endif

    radio.setChannel(channel);
    radio.stopListening();
 
    #ifdef IS_DEBUG
      logger.printFreeChannel(channel);
    #endif
  #endif

  getDataFromPins();  // Get data from pins for first time
  dataToBytes();
}


//void loop(void) {    
//  byte gotByte;
//  Serial.print("Sending... ");//Serial.println(counter);
//
//  char test[8] = {'h','e','l','l','o',' ','w','o'};
//  
//  unsigned long last_time = micros();
//  
//  if (radio.write(&test, 8) ) {
//    if(!radio.available()){
//      Serial.print("Empty, "); Serial.print(" Time: "); Serial.print(micros()-last_time); Serial.println(" microseconds"); Serial.println();
//    }else{      
//      while(radio.available()) {
//        radio.read( &gotByte, 1 );
//        Serial.print("Anser: "); Serial.print(gotByte); Serial.print(" Time: "); Serial.print(micros()-last_time); Serial.println(" microseconds"); Serial.println();                                
//      }
//    }
//    
//  } else {
//    Serial.println("Fail");
//  }    
//  
//  delay(50); 
//  
//}

void loop(void) {
//  #ifdef RF_ENABLED
    unsigned long last_time = micros();         //запоминаем время отправки
    byte response;                              // Успешно ли отправлены данные

//    char test[8] = {'h','e','l','l','o',' ','w','o'};

    if (radio.write(&b, 8)) {
 
//    if (sendDestructedData(0)) {
//      sendDestructedData(1);
//      sendDestructedData(2);
//      sendDestructedData(3);

//      #ifdef IS_DEBUG
//        logger.printControlsState(data);
//      #endif

      if(!radio.available()) {
      Serial.print("Empty: ");
      Serial.println(micros() - last_time);
//        #ifdef IS_DEBUG
//          logger.printResponse(false, last_time, false);
//        #endif
      } else {
        while(radio.available()) {
          radio.read(&response, 1);
  
            Serial.print("Success: ");
            Serial.print(response);
            Serial.print(" Time: ");
            Serial.println(micros() - last_time);
        }
      }
    } else {
      Serial.println("Error");  
    }
//  #endif
    getDataFromPins();  // Get data from pins
    dataToBytes();
//  destruct(); // Build new destructed data for next request
    delay(50);
//  #ifdef IS_DEBUG
//    delay(1000);
//  #else
//    delay(1000);
//  #endif
}

/* ----------------------------------------- */

void DebugLogger::printFreeChannel(int freeChannel) {
  Serial.println();
  Serial.print("Set channel: ");
  Serial.println(freeChannel, HEX);
}

void DebugLogger::printFreeChannel() {
  Serial.println("Error: No free Channels!");
}

void DebugLogger::printControlsState(Data data) {
  Serial.print("btn1: ");
  Serial.println(data.btn1);
  Serial.print("btn2: ");
  Serial.println(data.btn2);
  Serial.print("btn3: ");
  Serial.println(data.btn3);
  Serial.print("btn4: ");
  Serial.println(data.btn4);
  Serial.print("pot1: ");
  Serial.println(data.pot1);
  Serial.print("stick1: ");
  Serial.print(data.stick1.x);
  Serial.print(" ");
  Serial.print(data.stick1.y);
  Serial.print(" ");
  Serial.println(data.stick1.btn);
  Serial.print("stick2: ");
  Serial.print(data.stick2.x);
  Serial.print(" ");
  Serial.print(data.stick2.y);
  Serial.print(" ");
  Serial.println(data.stick2.btn);
}

void DebugLogger::printResponse(bool isRadioAvailable, unsigned long lastTime, bool response) {
  if (!isRadioAvailable) {
    Serial.print("Empty");
    Serial.print(" Time: ");
    Serial.print(micros()-lastTime);
    Serial.println(" microseconds");
    Serial.println();
  } else {
    if (response) {
      Serial.print("Success");
      Serial.print("Response Time: ");
      Serial.print(micros()-lastTime);
      Serial.println(" microseconds");
      Serial.println();
    } else {
      Serial.println("Wrong key");
    }
  }
}

void destruct() {
  for (byte i = 0; i < 4; i++) {
    dDataSet[i].order = i;
    
    switch (i) {
      case 0:
//        dDataSet[i].value1 = data.key;
        dDataSet[i].value2 = data.btn1;
        dDataSet[i].value3 = data.btn2;
        break;
      case 1:
        dDataSet[i].value1 = data.btn3;
        dDataSet[i].value2 = data.btn4;
        dDataSet[i].value3 = data.pot1;
        break;
      case 2:
        dDataSet[i].value1 = data.stick1.btn;
        dDataSet[i].value2 = data.stick1.x;
        dDataSet[i].value3 = data.stick1.y;
        break;
      case 3:
        dDataSet[i].value1 = data.stick2.btn;
        dDataSet[i].value2 = data.stick2.x;
        dDataSet[i].value3 = data.stick2.y;
        break;
      default:
        break;
    }
  }
}

bool sendDestructedData(byte order) {
  DestructedData d = dDataSet[order];
  return radio.write(&d, sizeof(d));
}

void getDataFromPins() {
  //  Build object to transmit
  data.btn1 = !digitalRead(BTN1_PIN);
  data.btn2 = !digitalRead(BTN2_PIN);
  data.btn3 = !digitalRead(BTN3_PIN);
  data.btn4 = !digitalRead(BTN4_PIN);
  data.pot1 = max(1, analogRead(POT1_PIN) >> 2);
  data.stick1.btn = !digitalRead(STICK1_BTN);
  data.stick1.x = max(1, analogRead(STICK1_X) >> 2);
  data.stick1.y = max(1, analogRead(STICK1_Y) >> 2);
  data.stick2.btn = !digitalRead(STICK2_BTN);
  data.stick2.x = max(1, analogRead(STICK2_X) >> 2);
  data.stick2.y = max(1, analogRead(STICK2_Y) >> 2);
}

/*
 * Take data from struct and build array of 8 bytes to transmit
 */
void dataToBytes() {
  b[0] = KEY;

  // 0  - all false;
  // 1  - 0 0 0 0 0 1;
  // 2  - 0 0 0 0 1 0;
  // ...
  // 31 - 0 1 1 1 1 1;
  // 32 - 1 0 0 0 0 0;
  // ...
  // 63 - 1 1 1 1 1 1
  byte btns[] = {1, 2, 4, 8, 16, 32};
  b[1] = 0; // state of all btns;
  if (data.btn1) b[1] += btns[0];
  if (data.btn2) b[1] += btns[1];
  if (data.btn3) b[1] += btns[2];
  if (data.btn4) b[1] += btns[3];
  if (data.stick1.btn) b[1] += btns[4];
  if (data.stick2.btn) b[1] += btns[5];
 
  b[2] = data.pot1; // pot1
  b[3] = data.stick1.x; // stick1 x
  b[4] = data.stick1.y; // stick1 y
  b[5] = data.stick2.x; // stick2 x
  b[6] = data.stick2.y; // stick2 y

  b[7] = 0; // last byte
}
