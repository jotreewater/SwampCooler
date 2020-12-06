#include <Servo.h>
#include <dht_nonblocking.h>
#include <Wire.h>
#include <DS3231.h>
#include <LiquidCrystal.h>
#define DHT_SENSOR_TYPE DHT_TYPE_11

//Disable Variables
int disableLEDPower = 22;
int disableOnButton = 23;
int disableOffButton = 24;

//Servo Variables
int ventOpenButton = 25;
int ventCloseButton = 26;
//pin 27 is attached to servo

//Temp and Hum Variables
int DHTSensorPin = 28;
float temperature;
float humidity;

//LED Variables
int idleLEDPower = 29;
int runningLEDPower = 30;
int errorLEDPower = 31;

//Fan Variables
int fanPowerPin = 32;
int fanForwardPin = 33;
int fanBackwardPin = 34;

//Error Variables
int levelSensorPin = 0;
int level = 0;

//Servo declaration
Servo vent;

//DHT declaration
DHT_nonblocking dht_sensor( DHTSensorPin, DHT_SENSOR_TYPE );

//RTC declaration
DS3231 clock;
RTCDateTime dt;

//LCD declaration
LiquidCrystal lcd(48, 49, 50, 51, 52, 53);

void setup() {
  Serial.begin(9600);
  //Disable Pins
  pinMode(disableLEDPower, OUTPUT);
  pinMode(disableOnButton, INPUT_PULLUP);  
  pinMode(disableOffButton, INPUT_PULLUP);
  //Servo Pins
  pinMode(ventOpenButton, INPUT_PULLUP);  
  pinMode(ventCloseButton, INPUT_PULLUP);
  //Idle Pins
  pinMode(idleLEDPower, OUTPUT);
  //Running Pins
  pinMode(runningLEDPower, OUTPUT);
  //Fan Pins
  pinMode(fanPowerPin, OUTPUT);
  //Error Pins
  pinMode(errorLEDPower, OUTPUT);
  //Fan Pins
  pinMode(fanPowerPin,OUTPUT);
  pinMode(fanForwardPin,OUTPUT);
  pinMode(fanBackwardPin,OUTPUT);
  //Servo Initialization
  vent.attach(27);
  vent.write(0);
  //RTC Initialization
  Serial.println("Start");
  Serial.println("Initializing DS3231");
  clock.begin();
  clock.setDateTime(__DATE__, __TIME__); 
  Serial.println("Initialized DS3231");
  //LCD Initialization
  lcd.begin(16, 2);
}

void loop() {
  disableState();
  idleState();
  toggleVent();
  runningState();
  checkTemp();
  //errorState();
}
void disableState(){
  //Checks if disable on button was pressed
  if(digitalRead(disableOnButton) == LOW)
  {
    digitalWrite(fanPowerPin, LOW);
    digitalWrite(idleLEDPower, LOW);
    digitalWrite(runningLEDPower, LOW);
    digitalWrite(errorLEDPower, LOW);
    digitalWrite(disableLEDPower, HIGH);
    //Stalls returning until disable off button is pressed
    while(digitalRead(disableOffButton) == HIGH){
      static unsigned long timerStart = millis();
        //Once every four seconds
      if( millis() - timerStart > 3000ul ){
        timerStart = millis();
        Serial.println("Disable");
        lcd.setCursor(0, 0);
        lcd.print("Disabled        ");
        checkTime();
      }
      toggleVent();
    }
    return;
  }
  if(digitalRead(disableOffButton) == LOW)
  {
    digitalWrite(disableLEDPower, LOW);
    return;
  }
}
void idleState(){
  if(((temperature*(9/5)+32)) < 54){
  static unsigned long timerStart = millis();
    //Once every four seconds
  if( millis() - timerStart > 3000ul ){
    timerStart = millis();
    Serial.println("Idle");
    checkTime();
    lcd.setCursor(0, 0);
    lcd.print("Idle            ");
    lcd.setCursor(0, 0);
    lcd.print("Temp            ");
    lcd.setCursor(5, 0);
    lcd.print((temperature*(9/5)+32));
    lcd.setCursor(7, 0);
    lcd.print("F Hum ");
    lcd.setCursor(13, 0);
    lcd.print(humidity);
    lcd.setCursor(15, 0);
    lcd.print("%");
  }
  digitalWrite(fanPowerPin, LOW);
  digitalWrite(disableLEDPower, LOW);
  digitalWrite(runningLEDPower, LOW);
  digitalWrite(errorLEDPower, LOW);
  digitalWrite(idleLEDPower, HIGH);
  return;
  }
}
void runningState(){
  if(((temperature*(9/5)+32)) >= 54){
    static unsigned long timerStart = millis();
    //Once every four seconds
    if( millis() - timerStart > 3000ul ){
      timerStart = millis();
      Serial.println("Running");
      checkTime();
      lcd.setCursor(0, 0);
      lcd.print("Running         ");
      lcd.setCursor(0, 0);
      lcd.print("Temp            ");
      lcd.setCursor(5, 0);
      lcd.print((temperature*(9/5)+32));
      lcd.setCursor(7, 0);
      lcd.print("F Hum ");
      lcd.setCursor(13, 0);
      lcd.print(humidity);
      lcd.setCursor(15, 0);
      lcd.print("%");
    }
    digitalWrite(disableLEDPower, LOW);
    digitalWrite(idleLEDPower, LOW);
    digitalWrite(errorLEDPower, LOW);
    digitalWrite(runningLEDPower, HIGH);
    digitalWrite(fanPowerPin, HIGH);
    digitalWrite(fanForwardPin, HIGH);
  }
  return;
}
void errorState(){
  level = analogRead(levelSensorPin);
  if(level > 200){
    digitalWrite(fanPowerPin, LOW);
    digitalWrite(idleLEDPower, LOW);
    digitalWrite(runningLEDPower, LOW);
    digitalWrite(errorLEDPower, HIGH);
    while(level > 200){
      level = analogRead(levelSensorPin);
      static unsigned long timerStart = millis();
      //Once every four seconds
      if( millis() - timerStart > 3000ul ){
        timerStart = millis();
        Serial.println("Error");
        checkTime();
        lcd.setCursor(0, 0);
        lcd.print("Error: Water Low");
      }
      toggleVent();
    }
  }
  return;
}
void toggleVent(){
  if(digitalRead(ventOpenButton) == LOW)
  {
    vent.write(180);
    return;
  }
  if(digitalRead(ventCloseButton) == LOW)
  {
    vent.write(0);
    return;
  }
}
static bool tempAvailable(float *temperature, float *humidity){
  static unsigned long timerStart = millis();
  //Once every four seconds
  if( millis() - timerStart > 3000ul ){
    if( dht_sensor.measure(temperature, humidity) == true){
      timerStart = millis();
      return(true);
    }
  }
  return(false);
}
void checkTemp(){
  if(tempAvailable(&temperature, &humidity ) == true){
    Serial.print("T = ");
    Serial.print(((temperature*(9/5)+32)), 1);
    Serial.print(" deg. F, H = ");
    Serial.print(humidity, 1);
    Serial.println( "%" );
  }
}
void checkTime(){
  dt = clock.getDateTime();
  Serial.print(dt.year);   Serial.print("-");
  Serial.print(dt.month);  Serial.print("-");
  Serial.print(dt.day);    Serial.print(" ");
  Serial.print(dt.hour);   Serial.print(":");
  Serial.print(dt.minute); Serial.print(":");
  Serial.print(dt.second); Serial.println("");
}
