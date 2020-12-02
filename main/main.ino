//www.elegoo.com
//2016.12.08

#include <Wire.h>
#include <DS3231.h>
#include <LiquidCrystal.h>

DS3231 clock;
RTCDateTime dt;

int temp = 69;
int tempRange = 50;
int waterRange = 10;

int yellow = 13;
int green = 12;
int red = 11;
int blue = 10;

int led1 = 9;
int led2 = 8;
int led3 = 7;
int led4 = 6;
int led5 = 5;
int led6 = 4;

int button = A0;
int water = A1;

LiquidCrystal lcd(led6, led5, led4, led3, led2, led1);

void setup() 
{
  Serial.begin(9600);
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(button, INPUT);
  pinMode(water, INPUT);
  // Initialize DS3231
  Serial.println("Initialize DS3231");
  clock.begin();
  Serial.println("Initialized DS3231");
  // Set sketch compiling time
  clock.setDateTime(__DATE__, __TIME__);
  //Set up LED
  LiquidCrystal lcd(7, 8, 9, 10, 11,12);
  lcd.print("Hello, World!");
}

void loop() 
{
  Serial.println("Start");
  disabledState();
  idleState();
  runningState();
}
int checkButton(){
  delay(500);
  int press = analogRead(button);
  //Serial.println(press);
  if(press>1000){
    return 1;
  }
  else{
    return 0;
  }
}
int checkLevel(){
  delay(200);
  int level = analogRead(water);
  //Serial.println(level);
  if(level>waterRange){
    return 1;
  }
  else{
    return 0;
  }
}
void displayTime(){
    dt = clock.getDateTime();
    Serial.print(dt.year);   Serial.print("-");
    Serial.print(dt.month);  Serial.print("-");
    Serial.print(dt.day);    Serial.print(" ");
    Serial.print(dt.hour);   Serial.print(":");
    Serial.print(dt.minute); Serial.print(":");
    Serial.print(dt.second); Serial.println("");
}
void disabledState(){
  if(checkButton() == 1){
    Serial.println("Disabled");
    lcd.print("Disabled");
    displayTime();
    digitalWrite(green, LOW);
    digitalWrite(blue, LOW);
    digitalWrite(yellow, HIGH);
    while(checkButton() == 1); //checks if button is still pressed
    while(checkButton() == 0); //checks if button was unpressed
    while(checkButton() == 1); //checks if button is pressed again
    digitalWrite(yellow, LOW);
    if(temp < tempRange){
      Serial.println("Idle");
      displayTime(); 
    }
    else{
      Serial.println("Running");
      displayTime();
    }
  }
}
void idleState(){
  if(temp < tempRange){
    Serial.println("Idle");
    lcd.print("Idle");
    displayTime();
    digitalWrite(green, HIGH);
    digitalWrite(blue, LOW);
    while(temp < tempRange){
      digitalWrite(green, HIGH);
      if(Serial.available()> 0){
        temp = Serial.read();
        Serial.println(temp);
      }
      disabledState();
      errorState();
      runningState();
    }
  }
}
void errorState(){
  if(checkLevel()==1){
    Serial.println("Error");
    lcd.print("Error");
    displayTime();
    digitalWrite(green, LOW);
    digitalWrite(blue, LOW);
    digitalWrite(red, HIGH);
    while(checkLevel() == 1); //checks if button is still pressed
    digitalWrite(red, LOW);
    if(temp < tempRange){
      Serial.println("Idle");
      displayTime(); 
    }
    else{
      Serial.println("Running");
      displayTime();
    }
  }
}
void runningState(){
  if(temp >= tempRange){
    Serial.println("Running");
    lcd.print("Running");
    displayTime();
    digitalWrite(green, LOW);
    digitalWrite(blue, HIGH);
    while(temp >= tempRange){
      digitalWrite(blue, HIGH);
      if(Serial.available()> 0){
        temp = Serial.read();
      }
      disabledState();
      idleState();
      errorState();
    }
  }
}
