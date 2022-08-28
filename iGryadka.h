#include "Arduino.h"


class iGryadka{
  public:
    //Конструктор класса
    iGryadka(byte init_time_hour, byte init_time_minute){
      current_hour = init_time_hour;//Время в часах(0-23)
      current_minute = init_time_minute;//Время (минуты)
    }
    void print_info(int (*)[2]);
    unsigned char smooth_switch(unsigned long _start_time, unsigned char _start_power, unsigned char _target_power);
    void sensor_read(unsigned char HUMID_POWER, unsigned char HUMID, unsigned char PIN_PHOTO_SENSOR, int(*)[2]);
    void watering(byte _water_time, byte _water_minute, byte _modePOMP, byte *_flagD, int (*)[2]);
    void lightning(byte _light_time_hour, byte _modeLED, byte *_flagD, int (*)[2]);
    byte hours_count();
    byte minutes_count();
    
  private:
    int _value_humid;//Текущее значение влажности
    int _value_solar;
    // 86400000 мс в сутках
    byte _init_time; //время запуска !!!!!!!!
    unsigned int time_speed = 1; //Ускорение времени(для отладки)
    byte current_hour; //Время в часах(0-23)
    byte current_minute; //Время (минуты)
    unsigned long _start_time_info = 0; // Таймер для вывода информации 
    unsigned long _interval_humid = 3600000 / time_speed; // Интервал для работы датчика влажности (частота обновления данных)!!!!!!!!!
    byte _humid_time_hour[2] = {12, 19}; // Время работы датчика влажности hourse
    byte _humid_time_minutes [2] = {10, 35}; // Время работы датчика влажности minutes
    unsigned long _interval_solar = 3600000 / time_speed; // Интервал для работы датчика света (частота обновления данных)!!!!!!!!!
    unsigned long _interval_time_waterring = 10600; // Интервал для полива !!!!!!!!!!!
    byte _solar_time_hour [24] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23}; // Время работы датчика влажности
    byte _solar_time_minutes [24] = {5,5,5,5,5,5, 5,5,5,5,5,5, 5,5,5,5,5,5, 5,5,5,5,5,5}; 
    
    bool flag_h = 0; //Для таймера датчика влажности
    bool flag_s = 0; //Для таймера датчика света
    bool flag_info = 0; // Флаг для записи в массив 
    bool flag_waterred = true; // Флаг для полива
    bool flag_light = true; // Флаг для освещения
    byte time_diff = 0; //Для подсчета времени прошедшего с включения
    unsigned int iter = 0; // Итератор для массива
    
    unsigned long _time_humid = 0; // Таймер для вывода информаци
    unsigned long _time_solar = 0; // Таймер для вывода информаци
    unsigned long _time_hour = 0; // Таймер для часов
    unsigned long _time_delta_hour = 0; // Разница для таймера времени 
    unsigned long _time_minutes = 0; // Таймер для минут
    unsigned long _time_delta_minutes = 0; // Разница для таймера времени (минуты)
    unsigned long _time_waterring = 0; // Таймер для полива
    
  };
