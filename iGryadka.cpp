#include "iGryadka.h"


//Вывод в консоль информации о влажности и освещенности
void iGryadka::print_info(int (*_sensor_data)[2]){
  if(millis() - _start_time_info > 3600){
    _start_time_info = millis();
//    Serial.print("Humid value = ");Serial.println(_sensor_data[iter][0]);
//    Serial.print("Illumination value = ");Serial.println(_sensor_data[iter][1]);
//    Serial.println();
    // График освещенности 
//   Serial.print(_sensor_data[11][1]);
//   Serial.println();
    //Вывести массив
   // Serial.print("iter = ");Serial.println(iter);
    for(int i = 0; i < 24; i++){
      for (int j = 0; j < 2; j++){
          Serial.print(_sensor_data[i][j]);Serial.print("  ");
        }
      Serial.println();  
      }
    Serial.println(time_diff);
    Serial.println("-------------------------------------------------------");  
  }
}

//Включение, считывание данных и отключение датчика влажности; Считывания данных с датчика освещенности
void iGryadka::sensor_read(unsigned char HUMID_POWER, unsigned char HUMID, unsigned char PIN_PHOTO_SENSOR, int (*_sensor_data)[2]){
  // Чтение с датчика влажности
  for(int i=0; i<sizeof(_humid_time_hour)/sizeof(byte); i++){
    if(current_hour == _humid_time_hour[i]){
        if(current_minute == _humid_time_minutes[i] && flag_h != 1){
          digitalWrite(HUMID_POWER, HIGH); 
          flag_h = 1;
          _time_humid = millis(); 
          }
      }
    }
 
  if(millis() - _time_humid > 800 && flag_h == 1){
      flag_h = 0;
      _value_humid = analogRead(HUMID);
      digitalWrite(HUMID_POWER, LOW);
      flag_info = 1;    
    }
        
   // Чтение с датчика света
  for(int i=0; i<sizeof(_solar_time_hour)/sizeof(byte); i++){
    if(current_hour == _solar_time_hour[i]){
        if(current_minute == _solar_time_minutes[i] && flag_s != 1){  
            flag_s = 1;
            _time_solar = millis(); 
        }
      }
    } 
  if(millis() - _time_solar > 65000/time_speed && flag_s == 1){
      flag_s = 0;
      _value_solar = analogRead(PIN_PHOTO_SENSOR);
      flag_info = 1;    
    }
    
  //Запись данных в массив
  if(flag_info){
    _sensor_data[iter][0] = _value_humid;
    _sensor_data[iter][1] = _value_solar;
    flag_info = 0;

  }
}

//Счетчик часов (0-23)
byte iGryadka::hours_count(){
  _time_delta_hour = millis()-_time_hour;
  if(_time_delta_hour >= 3600000 / time_speed){
      _time_hour = millis();
      if(current_hour >= 23){
          current_hour = 0;
          //Синхронизация с минутами
          current_minute = 0;
          _time_minutes = millis();
          if(iter < 23){
            Serial.print("DEBUG1__iter = ");Serial.println(iter);
            iter = iter + 1;
          }else{
            Serial.print("DEBUG2__iter = ");Serial.println(iter);
            iter = 0;  
            }
          Serial.print("hour = ");Serial.println(current_hour);
          return current_hour;
        }else{
          current_hour += 1;
          //Синхронизация с минутами
          current_minute = 0;
          _time_minutes = millis();
          if(iter < 23){
            Serial.print("DEBUG1__iter = ");Serial.println(iter);
            iter = iter + 1;
          }else{
            Serial.print("DEBUG2__iter = ");Serial.println(iter);
            iter = 0;  
            }
          Serial.print("hour = ");Serial.println(current_hour);  
          return current_hour;
          }
      }
  }
  
//Счетчик минут
byte iGryadka::minutes_count(){
  _time_delta_minutes = millis()-_time_minutes;
  if(_time_delta_minutes >= 60000 / time_speed){
      _time_minutes = millis();
      if(current_minute >= 60){
          current_minute = 0;
          //Serial.println(current_minute);
          return current_minute;
        }else{
          current_minute += 1;
          //Serial.println(current_minute);  
          return current_minute;
          }
      }
  }

void iGryadka::watering(byte _water_time_hour, byte _water_time_minute, byte _modePOMP, byte *_flagD, int (*_sensor_data)[2]){
  if(_water_time_hour == current_hour){
    if(_water_time_minute == current_minute){
      int limit = 1010 - _modePOMP * 100;
      if(_sensor_data[iter][0] > limit && *_flagD != 3 ){
        *_flagD = 3;
        flag_waterred = false;
        _time_waterring = millis();
        //Serial.println("START");
      }
     }
  }
   if(millis() - _time_waterring > _interval_time_waterring && !flag_waterred){
     flag_waterred = true;
     *_flagD = 0;
     Serial.println("STOP");
    }    
   
 }

void iGryadka::lightning(byte _light_time_hour, byte _modeLED, byte *_flagD, int (*_sensor_data)[2]){

  if(_light_time_hour + _modeLED >= 24 && current_hour>=0 && current_hour<_light_time_hour){
    time_diff = _light_time_hour + _modeLED - 24;
  } 
  else if(_light_time_hour + _modeLED >= 24 ){
      time_diff = _light_time_hour + _modeLED;
  }else{
    time_diff = _light_time_hour + _modeLED;
    }
  //Включение
  if(_sensor_data[iter][1]<350){
    if(current_hour < time_diff && current_hour >= _light_time_hour && *_flagD==0){
        *_flagD = 1;
        flag_light = false;
        Serial.println("START_L");
    }
    if(_light_time_hour + _modeLED > 24 && current_hour<_light_time_hour){
        if(current_hour < time_diff && *_flagD==0){
        *_flagD = 1;
        flag_light = false;
        Serial.println("START_L");    
      }
    }
  }
  //Выключение
  if(current_hour >= time_diff && !flag_light){
     flag_light = true;
     *_flagD = 0;
     Serial.println("STOPPPP");
    }     
 }

unsigned char iGryadka::smooth_switch(unsigned long _start_time, unsigned char _start_power, unsigned char _target_power){
    unsigned char _current_power;
    if (_target_power - _start_power >= 0){
      if(millis()-_start_time < 2500){
        return _target_power * (millis()-_start_time)/2500;         
      }
        return _target_power; 
      }
      
    if (_target_power - _start_power < 0){
      if(millis()-_start_time < 2500){
        return _target_power + _start_power * (1 - float(millis()-_start_time)/2500);         
      }
        return _target_power; 
      }      
}
