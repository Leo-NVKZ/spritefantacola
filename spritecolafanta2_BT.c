/*

  Konstantin Zakharov <zakharov.k.l@yandex.ru>

  Возможный вариант решения задачи spritecolafanta #2 (см. youtube.com/watch?v=TzMQ6M7-H_s)

  Сперва мотивировка: пара слов о literate programming.

  Логику действий робота (т.е. код для arduino) можно описывать, используя значения датчиков. 
  Однако, довольно интересно (и в некотором смысле красиво) описывать логику в антропоморфной 
  терминологии ("брось", "едь", "не тупи"). Конечно, это не попытка придать хайтековский глянец 
  примитивной робоплатформе (на журналистский лад), просто это довольно удачный в педагогическом 
  плане ход, который, в случае с задачей spritecolafanta #2, позволяет: 
  а) познакомить с синтаксисом - с массивом и перечислением, с представлением через них абстракций
  б) продемонстрировать красоту (я настаиваю, именно красоту) частного случая конечного автомата, 
  который какой-то иностранный геймдевелопер красочно озаглавил behaviour tree (набор переходов 
  между <<заданиями>>)
  ...и все это, разумеется, молча, просто показывая довольно простой код, без традиционных 
  вступлений вроде <<Марлезонский балет, второй акт>>. Этот подход не нужно обобщать на остальные 
  задачи, т.к. логика на датчиках часто лаконичнее и вернее любых красивостей и математических 
  моделей, а здесь - я нагородил все это вовсе не по необходимости, а смеху ради.

  Решение.

  Ездить по линии и преодолевать плюс-образные перекрестки - мы умеем благодаря низким требованиям 
  к скорости передвижения и заботливо подобранным неочевидным геометрическим параметрам робота и 
  толщины линии. Оставшаяся проблема - как узнать, куда ставить банку, уже тоже решена дизайнером
  трассы - это место оформлено, как перекресток. Представляя разные ситуации, в которых роботу 
  придется побывать, можно перейти к списку элементарных действий, которые нужно запрограммировать.
  Далее останется расположить их в нужном порядке. Всего есть 6 способов притащить банки к точке
  старта, если, конечно, не ездить по линиям просто так и не выдумывать ничего особенного, например,
  не бросать банки по дороге.

*/

#include <Servo.h>

// Элементарные действия
enum Task {
    FOLLOW_THE_LINE,            // Ехать по линии до перекрестка, пропасти или банки
    CROSSROAD_TURN_RIGHT,       // Повернуть направо
    CROSSROAD_TURN_LEFT,        // Поворот налево
    CROSSROAD_MOVE_FORWARD,     // Проезд прямо
    GRAB_JAR_AND_TURN_AROUND,   // Схватить баночку и развернуться
    DROP_JAR_AND_TURN_AROUND,   // Бросить баночку и развернуться
    TURN_OFF                    // Отключиться
};

// Массив с шагами алгоритма (куски, ответственные за каждую банку - разделены пустыми строками)
const Task task[] = {

    FOLLOW_THE_LINE,
    CROSSROAD_TURN_RIGHT,
    FOLLOW_THE_LINE,
    GRAB_JAR_AND_TURN_AROUND,
    FOLLOW_THE_LINE,
    CROSSROAD_TURN_LEFT,
    FOLLOW_THE_LINE,
    DROP_JAR_AND_TURN_AROUND,

    FOLLOW_THE_LINE,
    CROSSROAD_MOVE_FORWARD,
    FOLLOW_THE_LINE,
    GRAB_JAR_AND_TURN_AROUND,
    FOLLOW_THE_LINE,
    CROSSROAD_MOVE_FORWARD,
    FOLLOW_THE_LINE,
    DROP_JAR_AND_TURN_AROUND,

    FOLLOW_THE_LINE,
    CROSSROAD_TURN_RIGHT,
    FOLLOW_THE_LINE,
    GRAB_JAR_AND_TURN_AROUND,
    FOLLOW_THE_LINE,
    CROSSROAD_TURN_LEFT,
    FOLLOW_THE_LINE,
    DROP_JAR_AND_TURN_AROUND,

    TURN_OFF
};

const int tasks_count = sizeof(task) / sizeof(task[0]); // Количество шагов алгоритма

// Расстановка пинов по функциям
const int PIN_LEFT_SENSOR = 0;
const int PIN_RIGHT_SENSOR = 1;
const int PIN_MANIPULATOR_SENSOR = 2;
const int PIN_MANIPULATOR_ANGLE = 4;
const int PIN_LEFT_MOTORSPEED = 5;
const int PIN_RIGHT_MOTORSPEED = 6;
const int PIN_LEFT_MOTORDIR = 10;
const int PIN_RIGHT_MOTORDIR = 11;

// Некоторые константы
const int VALUE_MOVETICK = 10;
const int VALUE_MOTORSPEED = 200;
const int VALUE_MANIPULATOR_OPENED = 50;
const int VALUE_MANIPULATOR_CLOSED = 135;

// Функции управления моторами. Задают нужные направления вращения и скорости моторов 
// на указанное время.
int turn_right(int duration);
int turn_left(int duration);
int go_forward(int duration);
int go_back(int duration);

Servo manipulator; // Сервопривод манипулятора

void setup()
{
    // Назначаем пины моторов на выход
    pinMode(PIN_LEFT_MOTORSPEED, OUTPUT);
    pinMode(PIN_RIGHT_MOTORSPEED, OUTPUT);
    pinMode(PIN_LEFT_MOTORDIR, OUTPUT);
    pinMode(PIN_RIGHT_MOTORDIR, OUTPUT);
    manipulator.attach(PIN_MANIPULATOR_ANGLE); // Назначаем подключение сервомотора
    manipulator.write(VALUE_MANIPULATOR_OPENED); // Установка манипулятора в открытое положение
}

void turn_off() 
{
    // Отключаем моторы
    analogWrite(PIN_LEFT_MOTORSPEED, 0);
    analogWrite(PIN_RIGHT_MOTORSPEED, 0); 
    manipulator.detach(); // Отключаем сервомотор от пина 4, TODO (20.07.2016): на серве 
                          // остается напряжение
}

void loop() 
{
    static int id = 0; // Номер задания
    
    // Если алгоритм выполнен
    if (id == tasks_count)
    {
        delay(1000);
        return;
    }
    
    // Выполняем текущее задание
    switch (task[id++]) 
    {
        case FOLLOW_THE_LINE:
        {
            // Цикл езды по линии
            while (1)
            {
                // Если найдена баночка
                if (analogRead(PIN_MANIPULATOR_SENSOR) > 700)
                {
                    break;
                }
                else
                // Если найден перекресток или пропасть
                if (analogRead(PIN_RIGHT_SENSOR) > 600 && analogRead(PIN_LEFT_SENSOR) > 600)
                {
                    break;
                }
                else
                // Если правый датчик на черной линии, а левый нет, то  
                if (analogRead(PIN_RIGHT_SENSOR) > 600 && analogRead(PIN_LEFT_SENSOR) < 600)
                {
                    turn_right(VALUE_MOVETICK);
                }
                else
                // Если левый датчик на черной линии, а правый нет, то  
                if (analogRead(PIN_RIGHT_SENSOR) < 600 && analogRead(PIN_LEFT_SENSOR) > 600)
                {
                    turn_left(VALUE_MOVETICK);
                }
                else
                // Если левый и правый датчик не на черной линии, то  
                if (analogRead(PIN_RIGHT_SENSOR) < 600 && analogRead(PIN_LEFT_SENSOR) < 600)
                { 
                    go_forward(VALUE_MOVETICK);
                }
            }
            break;
        }
        case CROSSROAD_TURN_LEFT:
        {
            go_forward(700);
            turn_left(700);
            break;
        }
        case CROSSROAD_MOVE_FORWARD:
        {
            go_forward(700);
            break;
        }
        case CROSSROAD_TURN_RIGHT:
        {
            go_forward(700);
            turn_right(700);
            break;
        }
        case GRAB_JAR_AND_TURN_AROUND:
        {
            // Медленно закрываем манипулятор, берем баночку
            int value = VALUE_MANIPULATOR_OPENED;
            do
            {
                manipulator.write(++value);  
                delay(15);
            }
            while (value != VALUE_MANIPULATOR_CLOSED);
            turn_left(1400); // Разворачиваемся
            break;
        }
        case DROP_JAR_AND_TURN_AROUND:
        {
            manipulator.write(VALUE_MANIPULATOR_OPENED); // Бросаем баночку
            go_back(500); // Немного отъезжаем назад
            turn_left(1400); // Разворачиваемся
            break;
        }
        case TURN_OFF:
        {
            turn_off();
        }
    }
}

int turn_right(int duration) 
{
    digitalWrite(PIN_LEFT_MOTORDIR, LOW); 
    digitalWrite(PIN_RIGHT_MOTORDIR, HIGH);
    analogWrite(PIN_LEFT_MOTORSPEED, VALUE_MOTORSPEED);
    analogWrite(PIN_RIGHT_MOTORSPEED, VALUE_MOTORSPEED);
    delay(duration); 
    analogWrite(PIN_LEFT_MOTORSPEED, 0);
    analogWrite(PIN_RIGHT_MOTORSPEED, 0);
}

int turn_left(int duration)
{
    digitalWrite(PIN_LEFT_MOTORDIR, HIGH);
    digitalWrite(PIN_RIGHT_MOTORDIR, LOW);
    analogWrite(PIN_LEFT_MOTORSPEED, VALUE_MOTORSPEED);
    analogWrite(PIN_RIGHT_MOTORSPEED, VALUE_MOTORSPEED);
    delay(duration);
    analogWrite(PIN_LEFT_MOTORSPEED, 0);
    analogWrite(PIN_RIGHT_MOTORSPEED, 0);
}

int go_forward(int duration)
{
    digitalWrite(PIN_LEFT_MOTORDIR, LOW);
    digitalWrite(PIN_RIGHT_MOTORDIR, LOW);
    analogWrite(PIN_LEFT_MOTORSPEED, VALUE_MOTORSPEED);
    analogWrite(PIN_RIGHT_MOTORSPEED, VALUE_MOTORSPEED);
    delay(duration);
    analogWrite(PIN_LEFT_MOTORSPEED, 0);
    analogWrite(PIN_RIGHT_MOTORSPEED, 0);
}

int go_back(int duration)
{
    digitalWrite(PIN_LEFT_MOTORDIR, HIGH);
    digitalWrite(PIN_RIGHT_MOTORDIR, HIGH);
    analogWrite(PIN_LEFT_MOTORSPEED, VALUE_MOTORSPEED);
    analogWrite(PIN_RIGHT_MOTORSPEED, VALUE_MOTORSPEED);
    delay(duration);
    analogWrite(PIN_LEFT_MOTORSPEED, 0);
    analogWrite(PIN_RIGHT_MOTORSPEED, 0);
}
