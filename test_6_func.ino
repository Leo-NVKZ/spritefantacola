#include <Servo.h>

Servo manipul; // Сервопривод манипулятора
int ang = 50; // Угол установки сервопривода

const int l_sens = 0; // Пин левого сенсора
const int r_sens = 1; // Пин правого сенсора
const int man_sens = 2; // Пин сенсора манипулятора

const int l_motor_speed = 5; // Пин управления скоростью левого мотора
const int r_motor_speed = 6; // Пин управления скоростью правого мотора
const int l_motor_dir = 10; // Пин управления направлением вращения левого мотора
const int r_motor_dir = 11; // Пин управления направлением вращения правого мотора 

int motorspeed = 200; // Переменная скорости вращения моторов
int keep_cup = 0; // Переменная для разрешения на срабатывание манипулятора
int number_cup = 0; // Переменная для подсчета доставленных баночек

int turn_r() 
      { //чуть-чуть поворачиваем вправо
      digitalWrite (l_motor_dir, LOW);
      digitalWrite (r_motor_dir, HIGH);
      analogWrite (l_motor_speed, motorspeed);
      analogWrite (r_motor_speed, motorspeed);
      delay (10);
      analogWrite (l_motor_speed,0);
      analogWrite (r_motor_speed,0);
      }

int turn_l()
      { //чуть-чуть поворачиваем влево
      digitalWrite (l_motor_dir, HIGH);
      digitalWrite (r_motor_dir, LOW);
      analogWrite (l_motor_speed, motorspeed);
      analogWrite (r_motor_speed, motorspeed);
      delay (10);
      analogWrite (l_motor_speed,0);
      analogWrite (r_motor_speed,0);
      }

int forw()
      { //чуть-чуть едем вперед
      digitalWrite (l_motor_dir, LOW);
      digitalWrite (r_motor_dir, LOW);
      analogWrite (l_motor_speed, motorspeed);
      analogWrite (r_motor_speed, motorspeed);
      delay (10);
      analogWrite (l_motor_speed,0);
      analogWrite (r_motor_speed,0);
      }

int turn_around()
      { // Разворачиваемся
      digitalWrite (l_motor_dir, HIGH);
      digitalWrite (r_motor_dir, LOW);
      analogWrite (l_motor_speed, motorspeed);
      analogWrite (r_motor_speed, motorspeed);
      delay (1350);
      analogWrite (l_motor_speed,0);
      analogWrite (r_motor_speed,0); 
      }

int go_back()
      {  //немного отъезжаем назад 
      digitalWrite (l_motor_dir, HIGH);
      digitalWrite (r_motor_dir, HIGH);
      analogWrite (l_motor_speed, motorspeed);
      analogWrite (r_motor_speed, motorspeed);
      delay (500);
      analogWrite (l_motor_speed,0);
      analogWrite (r_motor_speed,0);
      }
      
void setup() {
  

 pinMode (l_motor_speed, OUTPUT); // Задаем пин как выход
 pinMode (r_motor_speed, OUTPUT); // Задаем пин как выход
 pinMode (l_motor_dir, OUTPUT); // Задаем пин как выход
 pinMode (r_motor_dir, OUTPUT); // Задаем пин как выход
 manipul.attach(4); // Назначаем подключение сервомотора на пин 4
}

void loop() 
{
    delay(4000);
    manipul.write(ang);
    delay(4000);
    while (number_cup < 3) {   // выполняем, пока не унесем все баночки
 
          // Если правый датчик на черной линии, а левый нет, то  
   if (analogRead(r_sens) > 600 && analogRead(l_sens) < 600)
     {
      turn_r();
      }
      
        // Если левый датчик на черной линии, а правый нет, то  
   if (analogRead(r_sens) < 600 && analogRead(l_sens) > 600)
     {
      turn_l();
     }

        // Если левый и правый датчик не на черной линии, то  
   if (analogRead(r_sens) < 600 && analogRead(l_sens) < 600)
     { 
      forw();
      } 

        // Если сенсор манипулятора перекрыт и разрешено реагировать на сенсор манипулятора, то  
   if (analogRead(man_sens) > 700 && keep_cup == 0)
     { // Медленно закрываем манипулятор, берем баночку
      do {
         manipul.write(++ang);  
         delay(15);
         }
      while (ang<115);
      ang = 50;
      ++keep_cup; // Ставим запрет реагировать на сенсор манипулятора
      
       // Разворачиваемся
      turn_around();
      } 

        // Если левый и правый датчик на черной линии или над пропастью, то  
   if (analogRead(r_sens) > 600 && analogRead(l_sens) > 600)
     {  
     manipul.write(ang);  //Кладем баночку
     --keep_cup; // Разрешаем реагировать на сенсор манипулятора
     
      //немного отъезжаем назад
      go_back();
      
      //и разворачиваемся
      turn_around();

      ++number_cup; // Считаем количество приесенных баночек
      } 
    }
      analogWrite (l_motor_speed,0); // Отключаем моторы
      analogWrite (r_motor_speed,0); 
      // manipul.detach(4); // Отключаем сервомотор от пина 4
}
