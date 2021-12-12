/* В данном скетче с передающей части (ТХ) отправляется значение переменной counter,
 * переменная эта с каждым шагом увеличивается на единицу. Приёмник (RX) принимает
 * сигнал, и отправляет обратно то, что получил, используя функцию radio.writeAckPayload
 * То есть наш приёмник на одно мгновение становится передатчиком! Если наш передатчик (TX)
 * принимает ответный сигнал, он выдаёт то, что принял, и пишет посчитанное вермя между 
 * отправкой и приёмом сигнала в микросекундах. Данный скетч можно использовать для теста
 * модулей на качество связи, а также для понимания работы функции radio.writeAckPayload
 * by AlexGyver 2016
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(10,9); // "создать" модуль на пинах 9 и 10 Для Уно
//RF24 radio(9,53); // для Меги

byte address[][6] = {"1Node","2Node","3Node","4Node","5Node","6Node"};  //возможные номера труб

long int counter;

void setup(){
  Serial.begin(9600); //открываем порт для связи с ПК

  radio.begin(); //активировать модуль
  radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0,15);     //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);     //размер пакета, в байтах

  radio.openWritingPipe(address[0]);   //мы - труба 0, открываем канал для передачи данных
  radio.setChannel(0x70);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_1MBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!
  // ВНИМАНИЕ!!! enableAckPayload НЕ РАБОТАЕТ НА СКОРОСТИ 250 kbps!

  radio.powerUp(); //начать работу
  radio.stopListening();  //не слушаем радиоэфир, мы передатчик
}

void loop(void) {    
  byte gotByte;
  Serial.print("Sending... ");Serial.println(counter);

  char test[8] = {'h','e','l','l','o',' ','w','o'};
  
  unsigned long last_time = micros();         //запоминаем время отправки
  
  if ( radio.write(&test,8) ){                 //отправляем значение counter
    if(!radio.available()){                     //если получаем пустой ответ
      Serial.print("Empty, "); Serial.print(" Time: "); Serial.print(micros()-last_time); Serial.println(" microseconds"); Serial.println();
    }else{      
      while(radio.available() ){                      // если в ответе что-то есть
        radio.read( &gotByte, 1 );                  // читаем
        Serial.print("Anser: "); Serial.print(gotByte); Serial.print(" Time: "); Serial.print(micros()-last_time); Serial.println(" microseconds"); Serial.println();
        counter++;                                  
      }
    }
    
  }else{   Serial.println("Fail"); }    
  
  delay(50); 
  
}

int toBytes() {
  int v = 0;
  int v1 = pow(2, 0);    // key 1 byte
  int v2= pow(2, 1);    // key 1 byte
  int v3= pow(2, 2);    // key 1 byte
  int v4= pow(2, 3);    // key 1 byte
  int v5= pow(2, 4);    // btns 1 byte
  int v6= pow(2, 5);    // btns 1 byte
  int v7= pow(2, 6);    // btns 1 byte
  int v8= pow(2, 7);    // btns 1 byte
  int v9= pow(2, 8);    // pot1 2byte
  int v10= pow(2, 9);    // pot1 2byte
  int v11= pow(2, 10);   // pot1 2byte
  int v12= pow(2, 11);   // pot1 2byte
  int v13= pow(2, 12);   // pot1 2byte
  int v14= pow(2, 13);   // pot1 2byte
  int v15= pow(2, 14);   // pot1 2byte
  int v16= pow(2, 15);   // pot1 2byte
  int v17= pow(2, 16);   // stick1 x 3byte
  int v18= pow(2, 17);   // stick1 x 3byte
  int v19= pow(2, 18);   // stick1 x 3byte
  int v20= pow(2, 19);   // stick1 x 3byte
  int v21= pow(2, 20);   // stick1 x 3byte
  int v22= pow(2, 21);   // stick1 x 3byte
  int v23= pow(2, 22);   // stick1 x 3byte
  int v24= pow(2, 23);   // stick1 x 3byte
  int v25= pow(2, 24);   // stick1 y 4byte
  int v26= pow(2, 25);   // stick1 y 4byte
  int v27= pow(2, 26);   // stick1 y 4byte
  int v28= pow(2, 27);   // stick1 y 4byte
  int v29= pow(2, 28);   // stick1 y 4byte
  int v30= pow(2, 29);   // stick1 y 4byte
  int v31= pow(2, 30);   // stick1 y 4byte
  int v32= pow(2, 31);   // stick1 y 4byte
}

void dataToCharString() {
  // get data from sensors and transform
  
  byte b[8];
  b[0] = 123;
  
  // 0  - all false;
  // 1  - 0 0 0 1;
  // 2  - 0 0 1 0;
  // 3  - 0 0 1 1;
  // 4  - 0 1 0 0;
  // 5  - 0 1 0 1;
  // 6  - 0 1 1 0;
  // 7  - 0 1 1 1;
  // 8  - 1 0 0 0;
  // 9  - 1 0 0 1;
  // 10 - 1 0 1 0;
  // 11 - 1 0 1 1;
  // 12 - 1 1 0 0;
  // 13 - 1 1 0 1;
  // 14 - 1 1 1 0;
  // 15 - 1 1 1 1;
  b[1] = 0; // state of all btns;
  b[2] = 0; // pot1
  b[3] = 0; // stick1 x
  b[4] = 0; // stick1 y
  b[5] = 0; // stick2 x
  b[6] = 0; // stick2 y

  // 0 - all false;
  // 1 - 0 1;
  // 2 - 1 0;
  // 3 - 1 1;
  b[7] = 0; // state of stick btns
  b[8] = 0;
}
