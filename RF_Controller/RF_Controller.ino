/* В данно скетче устройство с nrf24l01 модулем представленно в виде передатчика.
 Данные в виде структуры Data передаются на приемник с проверкой ключа.
 Для выбора оптимального канала передачи без шумов, сканируется весь
 диапазон частот и выбирается оптимальный. Для подключения приемника
 к данно частоте, на нем следуют так же проводить сканирование частот,
 и установка того канала, на котором придут данные с предустановленным ключем
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define KEY 1234
#define SCAN_ENABLED // Если включено, ищется наиболее чистый канал
#define NUMBER_OF_SCANS 3 // Сила сканирования, чем больше, тем выше шанс избежать каналы с шумами, каждая итерация ~10 секунд
#define DEFAULT_CHANNEL 0x60 // Канал, если отключено сканирование

#define BTN1_PIN 4
#define BTN2_PIN 5
#define BTN3_PIN 6

RF24 radio(10,9); // "создать" модуль на пинах 9 и 10 Для Уно
//RF24 radio(9,53); // для Меги

byte address[][6] = {"1Node","2Node","3Node","4Node","5Node","6Node"};  //возможные номера труб

typedef struct {
  int key;
  bool btn1; // Переменная для хранения передающихся команд
  bool btn2;
  bool btn3;
}
Data;

Data data;

int scanChannels();

void setup(){
  Serial.begin(9600); //открываем порт для связи с ПК

  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
  pinMode(BTN3_PIN, INPUT_PULLUP);

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
}

void loop(void) {
  static bool isScanning = true;

  if (isScanning) {
    int freeChannel = DEFAULT_CHANNEL;
    
    #ifdef SCAN_ENABLED
     freeChannel = scanChannels();
    #endif
    
    if (freeChannel != -1) {
      Serial.println();
      Serial.print("Set channel: ");
      Serial.println(freeChannel, HEX);
      radio.setChannel(freeChannel);  // Устанавливаем канал
      isScanning = false;
    } else {
      Serial.println("Error: No free Channels!");
      delay(1000);
    }
  } else {
    bool isBtn1Pressed = !digitalRead(BTN1_PIN);
    bool isBtn2Pressed = !digitalRead(BTN2_PIN);
    bool isBtn3Pressed = !digitalRead(BTN3_PIN);
  
  //  Build object to transmit
    data.key = KEY;
    data.btn1 = isBtn1Pressed;
    data.btn2 = isBtn2Pressed;
    data.btn3 = isBtn3Pressed;
    
    unsigned long last_time = micros();         //запоминаем время отправки
    bool response;                              // Успешно ли отправлены данные
    
    if (radio.write(&data, sizeof(data))) {
      if(!radio.available()) {
        Serial.print("Empty");
        Serial.print(" Time: ");
        Serial.print(micros()-last_time);
        Serial.println(" microseconds");
        Serial.println();
      } else {
        while(radio.available()) {
          radio.read(&response, 1);
  
          if (response) {
            Serial.print("Success");
            Serial.print("Response Time: ");
            Serial.print(micros()-last_time);
            Serial.println(" microseconds");
            Serial.println();
          } else {
            Serial.println("Wrong Key!");
          }
          
        }
      }
    }   
    
    delay(100);  
  }
  
}

int scanChannels () {
  static bool isSetUp = false;
  const byte numChannels = 126;
  const byte numberOfScans = NUMBER_OF_SCANS;
  byte values[numChannels] = {0};
  byte resultValues[numChannels] = {0};
  unsigned short scanRepeats = 100;

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

  for (int k = 0; k < numberOfScans; k ++) {
    Serial.print("Scaning");
    for (int i = 0; i < scanRepeats; i ++) {
      Serial.print('.');
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
    Serial.println();
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
    Serial.print(resultValues[i], HEX);
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
