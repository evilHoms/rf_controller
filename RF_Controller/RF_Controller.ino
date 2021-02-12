/* В данном скетче устройство с nrf24l01 модулем представленно в виде передатчика.
 Данные в виде структуры Data передаются на приемник с проверкой ключа.
 Для выбора оптимального канала передачи без шумов, сканируется весь
 диапазон частот и выбирается оптимальный. Для подключения приемника
 к данной частоте, на нем следуют так же проводить сканирование частот,
 и установка того канала, на котором придут данные с предустановленным ключем
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define KEY 1234 // Ключ безопасности, только при ответе с данным ключем будет произведено подключение
//#define SCAN_ENABLED // Если включено, ищется наиболее чистый канал
#define NUMBER_OF_SCANS 3 // Сила сканирования, чем больше, тем выше шанс избежать каналы с шумами, каждая итерация ~10 секунд
#define DEFAULT_CHANNEL 0x60 // Канал, если отключено сканирование
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

#define CE_PIN 9
#define SCN_PIN 10

RF24 radio(CE_PIN, SCN_PIN);
//RF24 radio(9,53); // для Меги

byte address[][6] = {"1Node","2Node","3Node","4Node","5Node","6Node"};  //возможные номера труб

typedef struct {
  bool btn;
  int x;
  int y;
}
Stick;

typedef struct {
  int key;
  bool btn1; // Переменная для хранения передающихся команд
  bool btn2;
  bool btn3;
  bool btn4;
  int pot1;
  Stick stick1;
  Stick stick2;
}
Data;

Data data;

int scanChannels();

class DebugLogger {
  public:
    void printFreeChannel(int freeChannel);
    void printFreeChannel();
    void printControlsState(Data data);
    void printResponse(bool isRadioAvailable, int lastTime, bool response);
};

DebugLogger logger;

void setup(){
  #ifdef IS_DEBUG
    Serial.begin(9600); //открываем порт для связи с ПК
  #endif

  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
  pinMode(BTN3_PIN, INPUT_PULLUP);
  pinMode(BTN4_PIN, INPUT_PULLUP);
  pinMode(STICK1_BTN, INPUT_PULLUP);
  pinMode(STICK2_BTN, INPUT_PULLUP);

  #ifdef RF_ENABLED
    radio.begin(); //активировать модуль
    radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
    radio.setRetries(0,3);     //(время между попыткой достучаться, число попыток)
    radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
    radio.setPayloadSize(32);     //размер пакета, в байтах
  
    radio.openWritingPipe(address[0]);   //мы - труба 0, открываем канал для передачи данных
  
    radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
    radio.setDataRate (RF24_1MBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
    //должна быть одинакова на приёмнике и передатчике!
    //при самой низкой скорости имеем самую высокую чувствительность и дальность!!
    // ВНИМАНИЕ!!! enableAckPayload НЕ РАБОТАЕТ НА СКОРОСТИ 250 kbps!
  
    radio.powerUp(); //начать работу
    radio.stopListening();  //не слушаем радиоэфир, мы передатчик
  #endif
}

void loop(void) {
  static bool isScanning = true;

  #ifndef RF_ENABLED
    isScanning = false;
  #endif

  if (isScanning) {
    int freeChannel = DEFAULT_CHANNEL;
    
    #ifdef SCAN_ENABLED
     freeChannel = scanChannels();
    #endif
    
    if (freeChannel != -1) {
      #ifdef IS_DEBUG
        logger.printFreeChannel(freeChannel);
      #endif
      radio.setChannel(freeChannel);  // Устанавливаем канал
      isScanning = false;
    } else {
      #ifdef IS_DEBUG
        logger.printFreeChannel();
      #endif
      delay(1000);
    }
  } else {
    //  Build object to transmit
    data.key = KEY;
    data.btn1 = !digitalRead(BTN1_PIN);
    data.btn2 = !digitalRead(BTN2_PIN);
    data.btn3 = !digitalRead(BTN3_PIN);
    data.btn4 = !digitalRead(BTN4_PIN);
    data.pot1 = analogRead(POT1_PIN);
    data.stick1.btn = !digitalRead(STICK1_BTN);
    data.stick1.x = analogRead(STICK1_X);
    data.stick1.y = analogRead(STICK1_Y);
    data.stick2.btn = !digitalRead(STICK2_BTN);
    data.stick2.x = analogRead(STICK2_X);
    data.stick2.y = analogRead(STICK2_Y);

    #ifdef IS_DEBUG
      logger.printControlsState(data);
    #endif

    #ifdef RF_ENABLED
      unsigned long last_time = micros();         //запоминаем время отправки
      bool response;                              // Успешно ли отправлены данные
      
      if (radio.write(&data, sizeof(data))) {
        if(!radio.available()) {
          #ifdef IS_DEBUG
            logger.printResponse(false, last_time, false);
          #endif
        } else {
          while(radio.available()) {
            radio.read(&response, 1);
    
            if (response) {
              #ifdef IS_DEBUG
                logger.printResponse(true, last_time, true);
              #endif
            } else {
              #ifdef IS_DEBUG
                logger.printResponse(true, last_time, false);
              #endif
            }
            
          }
        }
      }   
    #endif

    #ifdef IS_DEBUG
      delay(1000);
    #else
      delay(100);
    #endif
  }
  
}

/* ----------------------------------------- */

int scanChannels () {
  static bool isSetUp = false;
  const byte numChannels = 126;
  const byte numberOfScans = NUMBER_OF_SCANS;
  byte values[numChannels] = {0};
  byte resultValues[numChannels] = {0};
  unsigned short scanRepeats = 100;

  #ifdef IS_DEBUG
    if (!isSetUp) {
       Serial.println("Start Scanning for Free Channels...");
    //  radio.startListening();
    //  radio.stopListening();
    //  radio.setAutoAck(0);
      
      // Print out header, high then low digit
      for (int i = 0; i < numChannels; i++) {
        Serial.print(i>>4);
      }
      Serial.println();
      for (int i = 0; i < numChannels; i++) {
        Serial.print(i&0xf, HEX);
      }
      Serial.println();
  
      isSetUp = true;
    }
  #endif

  for (int k = 0; k < numberOfScans; k ++) {
    #ifdef IS_DEBUG
      Serial.print("Scaning");
    #endif
    for (int i = 0; i < scanRepeats; i ++) {
      #ifdef IS_DEBUG
        Serial.print('.');
      #endif
      for (int j = 0; j < numChannels; j ++) {
        radio.setChannel(j);
        radio.startListening();
        delayMicroseconds(128);
        radio.stopListening();
  
        if (radio.testCarrier()){
          ++values[i];
        }
      }
    }
    
    for (int i = 0; i < numChannels; i ++) {
      resultValues[i] += values[i];
      values[i] = 0;
    }
    #ifdef IS_DEBUG
      Serial.println();
    #endif
  }

  byte bestPositionStart = 0;
  byte bestPositionClearLength = 0;
  byte currentPositionStart = 0;
  byte currentPositionClearLength = 0;
  for (int i = 0; i < numChannels; i ++) {
    // Ищим наиболее чистые участки эфира
    if (!bestPositionClearLength && !resultValues[i]) {
      bestPositionStart = i;
      bestPositionClearLength ++;
      currentPositionStart = i;
      currentPositionClearLength ++;
    } else if (!currentPositionClearLength && !resultValues[i]) {
      currentPositionStart = i;
      currentPositionClearLength ++;
    } else if (currentPositionClearLength && !resultValues[i]) {
      currentPositionClearLength ++;
      if (i == numChannels - 1) {
        if (currentPositionClearLength > bestPositionClearLength) {
          bestPositionStart = currentPositionStart;
          bestPositionClearLength = currentPositionClearLength;
        }
      }
    } else if (currentPositionClearLength && resultValues[i]) {
      if (currentPositionClearLength > bestPositionClearLength) {
        bestPositionStart = currentPositionStart;
        bestPositionClearLength = currentPositionClearLength;
      }
      currentPositionStart = 0;
      currentPositionClearLength = 0;
    }
    #ifdef IS_DEBUG
      Serial.print(resultValues[i], HEX);
    #endif
  }

  int resultBestStart = 0;
  if (bestPositionClearLength > 5) {
    resultBestStart = bestPositionStart + 2;
  } else if (bestPositionClearLength > 3) {
    resultBestStart = bestPositionStart + 1;
  } else if (bestPositionClearLength) {
    resultBestStart = bestPositionStart;
  } else {
    return -1;
  }
 
  return resultBestStart;
}

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

void DebugLogger::printResponse(bool isRadioAvailable, int lastTime, bool response) {
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
