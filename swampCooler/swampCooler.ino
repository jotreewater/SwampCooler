#include <Servo.h>
#include <dht_nonblocking.h>
#include <Wire.h>
#include <DS3231.h>
#include <LiquidCrystal.h>
#define DHT_SENSOR_TYPE DHT_TYPE_11

//Port Registers
volatile unsigned char* portA = (unsigned char*) 0x22;      
volatile unsigned char* ddrA  = (unsigned char*) 0x21;      
volatile unsigned char* pinA  = (unsigned char*) 0x20;      
volatile unsigned char* portB = (unsigned char*) 0x25;      
volatile unsigned char* ddrB  = (unsigned char*) 0x24;      
volatile unsigned char* pinB  = (unsigned char*) 0x23;      
volatile unsigned char* portC = (unsigned char*) 0x28;      
volatile unsigned char* ddrC  = (unsigned char*) 0x27;      
volatile unsigned char* pinC  = (unsigned char*) 0x26;      
volatile unsigned char* portL = (unsigned char*) 0x10B;     
volatile unsigned char* ddrL  = (unsigned char*) 0x10A;     
volatile unsigned char* pinL  = (unsigned char*) 0x109;

//Disable Variables
int disableLEDPower = 22; //00000001
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
int tempRange = 59;

//LED Variables
int idleLEDPower = 29; //10000000
int runningLEDPower = 30; //00000010
int errorLEDPower = 31; //00000100

//Fan Pins
int fanPowerPin = 32; //00100000
int fanForwardPin = 33; //00010000
int fanBackwardPin = 34; //00001000

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
  //Port A pins
  *ddrA |= 10000001; //Set 1 and 7 to output
  *portA |= 10000001;
  //Port C pins
  *ddrC |= 11111000; //Set 7654 to output
  *portC |= 11000000;
  *pinC |= 00000100; //ErrorLEDPower high
  //Disable Pins
  pinMode(disableOnButton, INPUT_PULLUP);  
  pinMode(disableOffButton, INPUT_PULLUP);
  //Servo Pins
  pinMode(ventOpenButton, INPUT_PULLUP);  
  pinMode(ventCloseButton, INPUT_PULLUP);
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
  toggleVent();
  disableState();
  errorState();
  idleState();
  runningState();
  checkTemp();
}
void disableState(){
  //Checks if disable on button was pressed
  if(digitalRead(disableOnButton) == LOW)
  {
    *pinA &= 10000000; //IdleLEDPower low
    *pinC &= 10000000; //RunningLEDPower low
    *pinC &= 01000000; //ErrorLEDPower low
    digitalWrite(fanPowerPin, LOW);
    digitalWrite(fanForwardPin, LOW);
    *pinA |= 00000001; //DisableLEDPower high
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
    *pinA &= 00000001; //DisableLEDPower low
    return;
  }
}
void idleState(){
  if(((temperature*(9/5)+32)) < tempRange){
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
  digitalWrite(fanForwardPin, LOW);
  *pinA &= 00000001; //DisableLEDPower low
  *pinC &= 10000000; //RunningLEDPower low
  *pinC &= 00000100; //ErrorLEDPower low
  *pinA |= 10000000; //IdleLEDPower high
  return;
  }
}
void runningState(){
  if(((temperature*(9/5)+32)) >= tempRange){
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
    *pinA &= 00000001; //DisableLEDPower low
    *pinA &= 10000000; //IdleLEDPower low
    *pinC &= 00000100; //ErrorLEDPower low
    *pinC |= 10000000; //RunningLEDPower high
    digitalWrite(fanPowerPin, HIGH);
    digitalWrite(fanForwardPin, HIGH);
  }
  return;
}
void errorState(){
  level = analogRead(levelSensorPin);
  if(level < 200){
    digitalWrite(fanPowerPin, LOW);
    digitalWrite(fanForwardPin, LOW);
    *pinA &= 00000001; //DisableLEDPower low
    *pinA &= 10000000; //IdleLEDPower low
    *pinC &= 10000000; //RunningLEDPower low
    *pinC |= 00000100; //ErrorLEDPower high
    while(level < 200){
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
