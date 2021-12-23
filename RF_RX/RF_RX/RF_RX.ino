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

RF24 radio(10,9); // CE, CNE pins

/**
 * Transmitter and reciever have to use same tag.
 * also called as software channel.
 * is used to verify, that data is sent for exact reciever
 */
byte address[][6] = {"1Node","2Node","3Node","4Node","5Node","6Node"};  // pipe tags
byte isReciever = true;

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

  radio.setRetries(0,15);     // (время между попыткой достучаться, число попыток)
  radio.setPayloadSize(sizeof(request));     // размер пакета, в байтах

  radio.openReadingPipe(1, address[0]);      // хотим слушать трубу 0

  // Remove this line, or change channel in case if got error during sending request
  radio.setChannel(77);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel(RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate(RF24_1MBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!
  // ВНИМАНИЕ!!! enableAckPayload НЕ РАБОТАЕТ НА СКОРОСТИ 250 kbps!

  /**
   * Auto Acknoledge
   * if setAutoAck is enabled, transmitter will automatically get
   * acknoledge packets from reciever.
   * if enableAckPayload called, then reciever will be able to
   * put some custom payload to acknoledge packets
   */
//  radio.setAutoAck(1);         // enable or disable auto acknoledge, enabled by default
  radio.enableAckPayload();    // allow add payload to acknoledge packets
  
  radio.powerUp(); //начать работу
  radio.startListening();  //начинаем слушать эфир, мы приёмный модуль

  Serial.println(F("Radio connected as reciever!!"));
  Serial.print(F("Listening pipe: "));
  Serial.println((char*)address[0]);
  Serial.print(F("On channel: "));
  Serial.println(radio.getChannel());
}

void loop(void) {
    byte pipeNo;        
    while (radio.available(&pipeNo)){    // слушаем эфир со всех труб
      byte payloadSize = radio.getPayloadSize();
      radio.read(&request, payloadSize);         // чиатем входящий сигнал
      
      request += 1;
      if (request > 256) {
        request = 0;
      }
      
      radio.writeAckPayload(pipeNo, &request, payloadSize);  // отправляем обратно то что приняли

      Serial.print("Recieved: ");
      Serial.println(request);
   }
}
