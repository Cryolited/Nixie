 #include <GyverPWM.h>
#include <Wire.h>
#include <Rtc_Pcf8563.h>

#include "DHT.h"
#define DHT_PIN 17 // A3=17
DHT dht(DHT_PIN, DHT11);

/*

  74HC595 shift register
    +---------+
  1 -|Q1    VCC|- 16
  2 -|Q2     Q0|- 15
  3 -|Q3     DS|- 14
  4 -|Q4     OE|- 13
  5 -|Q5  ST_CP|- 12
  6 -|Q6  SH_CP|- 11
  7 -|Q7     MR|- 10
  8 -|GND   Q7'|- 9
   +---------+

*/
byte bitsToSend = 255;
byte bitsToSend1 = 255;
byte bitsToSend2 = 255;

// Знак градус и процент
byte termPin = 12;

// разделительные точки
byte dotsPin = 13;

// запятые
byte commas = 16; // A2=16

// регулятор яркости
int brightAdj = 15; // A1=15

//Пин подключен к ST_CP входу 74HC595
int latchPin = 8;
//Пин подключен к SH_CP входу 74HC595
int clockPin = 7;
//Пин подключен к DS входу 74HC595
int dataPin = 6;

// WEEK LAMP
//Пин подключен к ST_CP входу 74HC595
int latchWeek = 5;
//Пин подключен к SH_CP входу 74HC595
int clockWeek = 4;
//Пин подключен к DS входу 74HC595
int dataWeek = 2;
int nweek = 0;
byte testweek[9] = {         // байты, который будут последовательно циклически выводиться в регистре
  0b00000000,
  0b00000001,
  0b00000010,
  0b00000100,
  0b00001000,
  0b00010000,
  0b00100000,
  0b01000000,
  0b10000000,
};
//init the real time clock
Rtc_Pcf8563 rtc;

void setup() {
  //RTC
  //clear out the registers
  rtc.initClock();
  //set a time to start with.
  //day, weekday, month, century(1=1900, 0=2000), year(0-99)
  rtc.setDate(14, 6, 3, 1, 10);
  //hr, min, sec
  rtc.setTime(1, 15, 0);

  //устанавливаем режим OUTPUT
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
    //устанавливаем режим OUTPUT
  pinMode(latchWeek, OUTPUT);
  pinMode(dataWeek, OUTPUT);
  pinMode(clockWeek, OUTPUT);
  // PWM
  pinMode(3, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  PWM_prescaler(3, 1); // Timer2-  D3, D11 на 62кГц
  //TCCR2B = 0;
  //TCCR2B = TCCR2B & 0b11111000 | 0x01;
  PWM_mode(3, 0); // FastPWM
  //TCCR2A |= _BV(WGM20) | _BV(WGM21);
  PWM_prescaler(9, 1); // Timer1-  D9, D10 на 31(62)кГц
  PWM_prescaler(10, 1); // Timer1-  D9, D10 на 31(62)кГц
  PWM_prescaler(11, 1); // Timer1-  D9, D10 на 31(62)кГц
  PWM_mode(9, 0); // Phase-correct
  PWM_mode(10, 0); // Phase-correct
  PWM_mode(11, 0); // Phase-correct

  Serial.begin(9600);
  Serial.println("reset");
  //bitSet(TCCR2A, COM2B1);
  //OCR2B = 239;
  PWM_set(3, 239);
  //analogWrite(3, 239); // 239 = 167V
  dht.begin();
}

void setColorLED(int a1, int a2, int a3)
{
  PWM_set(9, a1);
  PWM_set(10, a2);
  PWM_set(11, a3);
}

void getButtValue()
{
  int sensorValue = analogRead(A0);
  Serial.println(sensorValue);
}

void getHighVoltage()
{
  int highVoltage = analogRead(A1);
  Serial.print("High voltage= ");
  Serial.println(highVoltage);
}
byte testVal = 0;
void loop() {
  if (Serial.available() > 0) {
    // Символы от '0' до '9'
    // представлены в ASCII таблице значения от 48 до 57.
    //int bitToSet = Serial.parseInt();
    int bitToSet = 1;
    bitToSet = (bitToSet % 10 << 4) + bitToSet / 10;
    int bitToSet1 = bitToSet;
    int bitToSet2 = bitToSet;
    int PWM = 1;
    // Записываем HIGH в позицию соответствующую bitToSet
    //registerWrite(bitToSet, bitToSet1, bitToSet2, HIGH);
    //bitSet(TCCR2A, COM2B1);
    //OCR2B = PWM;
    PWM_set(3, 239);
    int a1 = Serial.parseInt();
    int a2 = Serial.parseInt();
    int a3 = Serial.parseInt();
    setColorLED(a1, a2, a3);
  }
  rtc.getDateTime();
  int bitToSet = rtc.getSecond();
  bitToSet = (bitToSet % 10 << 4) + bitToSet / 10;
  int bitToSet1 = rtc.getMinute();
  bitToSet1 = (bitToSet1 % 10 << 4) + bitToSet1 / 10;
  int bitToSet2 = rtc.getHour();
  bitToSet2 = (bitToSet2 % 10 << 4) + bitToSet2 / 10;
  registerWrite(bitToSet1, bitToSet2, bitToSet, HIGH);

  registerWrite2(testweek[++nweek % 9], HIGH);

  //getButtValue();
  //getHighVoltage();

  digitalWrite(termPin, testVal);
  digitalWrite(dotsPin, testVal);
  digitalWrite(commas, testVal);
  testVal = !testVal;

  byte temp = dht.readTemperature();
  byte hum = dht.readHumidity();
  Serial.print("temp= ");
  Serial.println(temp);
  Serial.print("hum= ");
  Serial.println(hum);
  delay(1000);
  //analogWrite(9, 20);
  //analogWrite(10, 120);
  //analogWrite(11, 240);
}

// Этот метот записывает байт в регистр
void registerWrite(int whichPin, int whichPin1, int whichPin2, int whichState) {
  // инициализируем и обнуляем байт
  //byte bitsToSend = 0;

  //Отключаем вывод на регистре
  digitalWrite(latchPin, LOW);

  // устанавливаем HIGH в соответствующем бите
  //bitWrite(bitsToSend, bitsToSend++, whichState);

  // проталкиваем байт в регистр
  shiftOut(dataPin, clockPin, MSBFIRST, whichPin);
  shiftOut(dataPin, clockPin, MSBFIRST, whichPin1);
  shiftOut(dataPin, clockPin, MSBFIRST, whichPin2);

  // "защелкиваем" регистр, чтобы байт появился на его выходах
  digitalWrite(latchPin, HIGH);
}

// Этот метот записывает байт в регистр
void registerWrite2(int whichPin, int whichState) {
  // инициализируем и обнуляем байт
  //byte bitsToSend = 0;

  //Отключаем вывод на регистре
  digitalWrite(latchWeek, LOW);

  // устанавливаем HIGH в соответствующем бите
  //bitWrite(bitsToSend, bitsToSend++, whichState);

  // проталкиваем байт в регистр
  shiftOut(dataWeek, clockWeek, MSBFIRST, whichPin);

  // "защелкиваем" регистр, чтобы байт появился на его выходах
  digitalWrite(latchWeek, HIGH);
}
