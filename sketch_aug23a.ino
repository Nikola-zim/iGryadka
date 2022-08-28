#include "GyverButton.h"
#include "iGryadka.h"

#define POMP1 3
#define POMP2 2
#define LED1P 4
#define LED1N 5
#define HUMID A7
#define HUMID_POWER 8
#define PIN_PHOTO_SENSOR A6

byte day_time_hour = 14; //начальное время в часах 0-24 !!!!!!!!!!!!
byte day_time_minute = 15; // !!!!!!!!!!!!
byte water_time_hour = 19; //Время полива часы !!!!!!!!!!
byte water_time_minute = 50; //Время полива минуты (должны быть после чтения данных с датчиков) !!!!!!!!!!
byte light_time_hour = 21; //Время освещения часы !!!!!!!!!!

uint32_t sec;// просто секунды
iGryadka myGryadka(day_time_hour, day_time_minute); //Экземпляр класса из библиотеки
byte current_hour;
byte current_minute;

int humid_value = 0; //Значение влажности
int photo_value = 0; //Значение освещенности
int sensor_data[24][2]; //Массив для записи данных за "день" 

unsigned char LED_P = 255; // мощьность для LED
unsigned char LED_P_point = 0; // мощьность в момент переключения
unsigned char LED_P_current = 0; // текущая мощьность 
unsigned long start_timeLED; //время для плавного старта
byte modeLED = 0;
int water_counter_start; // Счётчик воды (начало)
int water_counter_current; // Счётчик воды (начало)
int water_tank_volume = 5000; // Время опустошения резервуара воды в millis

unsigned char POMP_P = 255; // мощьность для POMP
unsigned char POMP_P_point = 0; // мощьность в момент переключения
unsigned char POMP_P_current = 0; // текущая мощьность 
unsigned long start_timePOMP; //время для плавного старта
byte modePOMP =0;
byte flagD = 0;
byte flagL = 0; //для level

//Для GUI
bool notholdPOMP = 1;
bool notholdLED = 1;
//bool flag_power_save = 0; //для начала отсчёта перед выключением GUI
unsigned long last_timeLED;
unsigned long last_timePOMP;
bool blinkLED = 1;
bool blinkPOMP = 1;

//Назначение кнопок
GButton LEDset(6, HIGH_PULL, NORM_OPEN);
GButton POMPset(7, HIGH_PULL, NORM_OPEN);
GButton LEVELdet(9, HIGH_PULL, NORM_OPEN);


void setup() {
  //Настройки кнопок
  LEDset.setDebounce(80);
  LEDset.setTimeout(700);
  POMPset.setDebounce(80);
  POMPset.setTimeout(700);
  LEVELdet.setDebounce(80);
  LEVELdet.setTimeout(700);
  //Для LED и помпы
  pinMode(POMP1, OUTPUT);
  pinMode(POMP2, OUTPUT);
  pinMode(HUMID_POWER, OUTPUT);
  pinMode(LED1P, OUTPUT);
  pinMode(LED1N, OUTPUT);
  //Для блока лэд
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  //Настройка кнопок
  
  /*//Проверка блока лэд
  digitalWrite(A0, HIGH);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A3, HIGH);
  digitalWrite(A4, HIGH);
  digitalWrite(A5, HIGH);
  */
  // Включение датчика влажности
  pinMode(HUMID_POWER, OUTPUT);
  digitalWrite(HUMID_POWER, HIGH);
  Serial.begin(9600);  
  water_counter_current = water_tank_volume + 1;
}

//НАЧАЛО ЛУПЫ

void loop() {
  uint32_t sec = millis() / 1000ul;      // полное количество секунд
  int timeSecs = (sec % 3600ul) % 60ul;  // секунды
  //Чтение с датчиков
  myGryadka.sensor_read(HUMID_POWER, HUMID, PIN_PHOTO_SENSOR, sensor_data);
  //Определение времени
  current_hour = myGryadka.hours_count();
  current_minute = myGryadka.minutes_count();
  //Полив
  myGryadka.watering(water_time_hour, water_time_minute, modePOMP, &flagD, sensor_data);
  //Освещение
  myGryadka.lightning(light_time_hour, modeLED, &flagD, sensor_data);
  //Обработчик кнопок
  LEDset.tick();
  POMPset.tick();
  LEVELdet.tick();
  //GUI
  gui();
  //Serial.println(flagD);
    
  //Включение лампы
  if (LEDset.isHolded()){
    notholdLED = 0;
    notholdPOMP = 1;
    flagD = 1;
    start_timeLED = millis();
    modeLED -= 1; // при удержании добавляется modeLED, поэтому отнимаем
  }
   //Включение помпы
  if (!flagL && POMPset.isHolded()){
    LED_P_point = LED_P_current;
    start_timePOMP = millis();
    notholdPOMP = 0;//для GUI
    notholdLED = 1;// для GUI
    flagD = 2;
    modePOMP -= 1;//Для предотваращения изменений modePOMP
  } 
    
    //Настройка авто освещения
  if (LEDset.isPress()){
    if(flagD == 1 && flagD != 2){
        start_timeLED = millis();
        LED_P_point = LED_P_current;
        flagD = 0;
        
      } 
    if(notholdLED){
      modeLED += 1;
      if (modeLED > 7) modeLED =0;
    }
    //flag_power_save = 1; //для GUI
    last_timeLED = millis();
    notholdLED = 1; //для GUI
  }
  
  //Настройка режима авто полива
  if (POMPset.isPress()){
    if(flagD == 2 && flagD != 1){
        start_timePOMP = millis();
        POMP_P_point = POMP_P_current;
        flagD = 0;
      } 
    if(notholdPOMP){
      modePOMP += 1;
      if (modePOMP > 7) modePOMP =0;
    }
    //flag_power_save = 1;
    last_timeLED = millis(); 
    notholdPOMP = 1;//для GUI
  }
    
  //Блок с управлением лампы и помпы (поочередное включение)
  if (flagD == 1){
    LED_P_current = myGryadka.smooth_switch(start_timeLED, LED_P_point, LED_P);
    POMP_P_current = myGryadka.smooth_switch(start_timeLED, POMP_P_point, 0);
    digitalWrite(LED1P, 0); 
    analogWrite(LED1N, LED_P_current);
    analogWrite(POMP1,0); 
    digitalWrite(POMP2, 0);   
    }
  
  if (flagD == 2 || flagD == 3){
    if (LED_P_point > 0){
        LED_P_current = myGryadka.smooth_switch(start_timePOMP, LED_P_point, 0);
        POMP_P_current = myGryadka.smooth_switch(start_timePOMP, 0, POMP_P);
      }
      else{
        LED_P_current = 0;
        POMP_P_current = POMP_P;
      }
    digitalWrite(LED1P, 0); 
    analogWrite(LED1N, LED_P_current);
    analogWrite(POMP1,POMP_P_current); 
    digitalWrite(POMP2, 0); 
    }
  if (flagD == 0){
    LED_P_current = myGryadka.smooth_switch(start_timeLED, LED_P_point, 0);
    POMP_P_current = 0;
    digitalWrite(LED1P, 0); 
    analogWrite(LED1N, LED_P_current);
    analogWrite(POMP1,0); 
    digitalWrite(POMP2, 0);   
    }   
  // вывод значений с датчиков в монитор последовательного порта Arduino
  myGryadka.print_info(sensor_data);
} 

//КОНЕЦ ЛУПЫ

void gui(){
  if(notholdLED){
    if(millis() - last_timeLED < 5000){
      switch(modeLED){
          case 0:
            digitalWrite(A0, LOW);
            digitalWrite(A1, LOW);
            digitalWrite(A2, LOW);
            digitalWrite(A3, LOW); 
            break;  
          case 1:
            digitalWrite(A0, LOW);
            digitalWrite(A1, LOW);
            digitalWrite(A2, LOW);
            digitalWrite(A3, HIGH);
            break; 
          case 2:
            digitalWrite(A0, HIGH);
            digitalWrite(A1, LOW);
            digitalWrite(A2, LOW);
            digitalWrite(A3, LOW);
            break; 
          case 3:
            digitalWrite(A0, HIGH);
            digitalWrite(A1, LOW);
            digitalWrite(A2, LOW);
            digitalWrite(A3, HIGH);
            break; 
          case 4:
            digitalWrite(A0, HIGH);
            digitalWrite(A1, HIGH);
            digitalWrite(A2, LOW);
            digitalWrite(A3, LOW);
            break;
          case 5:
            digitalWrite(A0, HIGH);
            digitalWrite(A1, HIGH);
            digitalWrite(A2, LOW);
            digitalWrite(A3, HIGH);
            break;
          case 6:
            digitalWrite(A0, HIGH);
            digitalWrite(A1, HIGH);
            digitalWrite(A2, HIGH);
            digitalWrite(A3, LOW);
            break;
          case 7:
            digitalWrite(A0, HIGH);
            digitalWrite(A1, HIGH);
            digitalWrite(A2, HIGH);
            digitalWrite(A3, HIGH);
            break;
      }    
    }else{
      digitalWrite(A0, LOW);
      digitalWrite(A1, LOW);
      digitalWrite(A2, LOW);
      digitalWrite(A3, LOW);  
    }
  }else{
    if(millis()-last_timeLED > 850){
      last_timeLED = millis();
      //Serial.println(flagD);
      if (blinkLED){
        last_timeLED = millis();
          digitalWrite(A0, HIGH);
          digitalWrite(A1, HIGH);
          digitalWrite(A2, HIGH);
          digitalWrite(A3, LOW);
          blinkLED = 0;
      }else{
          digitalWrite(A0, LOW);
          digitalWrite(A1, LOW);
          digitalWrite(A2, LOW);
          digitalWrite(A3, LOW);
          blinkLED = 1;
       }
           
    }
  }
  if(notholdPOMP){
    if(millis() - last_timeLED < 5000){
       switch(modePOMP){
        case 0:
          digitalWrite(A4, LOW);
          digitalWrite(A5, LOW);
          digitalWrite(11, LOW);
          digitalWrite(12, LOW); 
          break;  
        case 1:
          digitalWrite(A4, LOW);
          digitalWrite(A5, LOW);
          digitalWrite(11, LOW);
          digitalWrite(12, HIGH);
          break; 
        case 2:
          digitalWrite(A4, HIGH);
          digitalWrite(A5, LOW);
          digitalWrite(11, LOW);
          digitalWrite(12, LOW);
          break; 
        case 3:
          digitalWrite(A4, HIGH);
          digitalWrite(A5, LOW);
          digitalWrite(11, LOW);
          digitalWrite(12, HIGH);
          break; 
        case 4:
          digitalWrite(A4, HIGH);
          digitalWrite(A5, HIGH);
          digitalWrite(11, LOW);
          digitalWrite(12, LOW);
          break;
        case 5:
          digitalWrite(A4, HIGH);
          digitalWrite(A5, HIGH);
          digitalWrite(11, LOW);
          digitalWrite(12, HIGH);
          break;
        case 6:
          digitalWrite(A4, HIGH);
          digitalWrite(A5, HIGH);
          digitalWrite(11, HIGH);
          digitalWrite(12, LOW);
          break;
        case 7:
          digitalWrite(A4, HIGH);
          digitalWrite(A5, HIGH);
          digitalWrite(11, HIGH);
          digitalWrite(12, HIGH);
          break;
       }
    }else{
      digitalWrite(A4, LOW);
      digitalWrite(A5, LOW);
      digitalWrite(11, LOW);
      digitalWrite(12, LOW);  
    }    
  }else{
    if(millis()-last_timePOMP > 850){
      last_timePOMP = millis();
      //Serial.println(flagD);
      if (blinkPOMP){
        last_timePOMP = millis();
          digitalWrite(A4, HIGH);
          digitalWrite(A5, HIGH);
          digitalWrite(11, HIGH);
          digitalWrite(12, LOW);
          blinkPOMP = 0;
      }else{
          digitalWrite(A4, LOW);
          digitalWrite(A5, LOW);
          digitalWrite(11, LOW);
          digitalWrite(12, LOW);
          blinkPOMP = 1;
       }      
    } 
  }
}
