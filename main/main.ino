//www.elegoo.com
//2016.12.08

#include <Wire.h>
#include <DS3231.h>
#include <LiquidCrystal.h>
#include <dht_nonblocking.h>
#include <Servo.h>
#define DHT_SENSOR_TYPE DHT_TYPE_11

static const int DHT_SENSOR_PIN = 3;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );

Servo myservo;

DS3231 clock;
RTCDateTime dt;

int tempRange = 21;
int waterRange = 10;

int yellow = 13;
int green = 12;
int red = 11;
int blue = 10;

int lcd1 = 9;
int lcd2 = 8;
int lcd3 = 7;
int lcd4 = 6;
int lcd5 = 5;
int lcd6 = 4;

int disableButton = A0;
int water = A1;

float temperature;
float humidity;

int ventButton = A2;
int ventState = 0;

int fan = 22;
int fanState = 0;


//                 BS    E     D4    D5    D6    D7
LiquidCrystal lcd(lcd6, lcd5, lcd4, lcd3, lcd2, lcd1);

void setup() 
{
  Serial.begin(9600);
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(fan, OUTPUT);
  pinMode(disableButton, INPUT);
  pinMode(water, INPUT);
  // Initialize DS3231
  Serial.println("Initialize DS3231");
  clock.begin();
  Serial.println("Initialized DS3231");
  // Set sketch compiling time
  clock.setDateTime(__DATE__, __TIME__);
  //Set up LED
  LiquidCrystal lcd(7, 8, 9, 10, 11,12);
  lcd.begin(16, 2);
  lcd.print("Hello, World!");
  myservo.attach(2);
  myservo.write(90);
}

void loop() 
{
  Serial.println("Start");
  disabledState();
  idleState();
  runningState();
  toggleVent();
}
int checkDisableButton(){
  static unsigned long timerStart = millis( );
  int disable = analogRead(disableButton);
  //wait 1 second
  if( millis( ) - timerStart > 1000ul ){
    timerStart = millis( );
    //Serial.println(press);
    if(disable>1000){
      return 1;
    }
    else{
      return 0;
    }
  }
}
void toggleFan(){
  if((fanState == 0) && (temperature >= tempRange)){
    Serial.println("Fan High");
    digitalWrite(fan , HIGH);
    fanState = 1;
    return;
  }
  if(checkLevel()==1){
    Serial.println("Fan Low");
    digitalWrite(fan , LOW);
    fanState = 0;
    return;
  }
  if(checkDisableButton()==1){
    Serial.println("Fan Low");
    digitalWrite(fan , LOW);
    fanState = 0;
    return;
  }
  if((temperature < tempRange)){
    Serial.println("Fan Low");
    digitalWrite(fan , LOW);
    fanState = 0;
    return;
  }
}
int checkVentButton(){
  static unsigned long timerStart = millis( );
  int vent = analogRead(ventButton);
  //wait 1 second
  if( millis( ) - timerStart > 500ul ){
    timerStart = millis( );
    //Serial.println(press);
    if(vent>700){
      return 1;
    }
    else{
      return 0;
    }
  }
}
int checkLevel(){
  delay(10);
  int level = analogRead(water);
  //Serial.println(level);
  if(level>waterRange){
    return 1;
  }
  else{
    return 0;
  }
}
bool checkTemp( float *temperature, float *humidity )
{
  static unsigned long timerStart = millis( );
  /* Checks for new reading every 3 seconds. */
  if( millis( ) - timerStart > 3000ul ){
    if( dht_sensor.measure( temperature, humidity ) == true )
    {
      timerStart = millis( );
      return( true );
    }
  }
  return( false );
}
void toggleVent(){
  if((checkVentButton() && !(checkVentButton())) == 1){
    Serial.println("Toggled Vent");
    displayTime();
    if(ventState == 1){
      Serial.println("Vent Down");
      myservo.write(90);
      static unsigned long timerStart = millis( );
      //wait 3 seconds
      if( millis( ) - timerStart > 5000ul ){
        timerStart = millis( );
      }
      ventState = 0;
      return;
    }
    if(ventState == 0){
      Serial.println("Vent Up");
      myservo.write(180);
      static unsigned long timerStart = millis( );
      //wait 3 seconds
      if( millis( ) - timerStart > 5000ul ){
        timerStart = millis( );
      }
      ventState = 1;
      return;
    }
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
  if(checkDisableButton() == 1){
    toggleFan();
    Serial.println("Disabled");
    lcd.setCursor(0, 0);
    lcd.print("Disabled        ");
    displayTime();
    digitalWrite(green, LOW);
    digitalWrite(blue, LOW);
    digitalWrite(yellow, HIGH);
    while(checkDisableButton() == 1){//checks if button is still pressed
    }
    while(checkDisableButton() == 0){//checks if button was unpressed
      toggleVent(); 
    }
    while(checkDisableButton() == 1){ //checks if button is pressed again
      toggleVent(); 
    }
    digitalWrite(yellow, LOW);
    if(temperature < tempRange){
      Serial.println("Idle");
      displayTime();
      lcd.setCursor(0, 0);
      lcd.print("Idle            ");
    }
    else{
      Serial.println("Running");
      displayTime();
      lcd.setCursor(0, 0);
      lcd.print("Running         ");
    }
  }
}
void idleState(){
  if(temperature < tempRange){
    toggleFan();
    Serial.println("Idle");
    lcd.setCursor(0, 0);
    lcd.print("Idle            ");
    displayTime();
    digitalWrite(green, HIGH);
    digitalWrite(blue, LOW);
    while(temperature < tempRange){
      digitalWrite(green, HIGH);
      if( checkTemp( &temperature, &humidity ) == true ){
        Serial.print( "T = " );
        Serial.print( temperature, 1 );
        Serial.print( " deg. C, H = " );
        Serial.print( humidity, 1 );
        Serial.println( "%" );
        lcd.setCursor(0, 0);
        lcd.print("Temp=           ");
        lcd.setCursor(5, 0);
        lcd.print((temperature * 9/5)+32);
        lcd.setCursor(7, 0);
        lcd.print("F       ");
        lcd.setCursor(9, 0);
        lcd.print("Hum=  ");
        lcd.setCursor(13, 0);
        lcd.print(humidity);
        lcd.setCursor(15, 0);
        lcd.print("%");
      }
      toggleVent();
      disabledState();
      errorState();
      runningState();
    }
  }
}
void errorState(){
  if(checkLevel()==1){
    toggleFan();
    static unsigned long timerStart = millis( );
    if( millis( ) - timerStart > 5000ul ){
        timerStart = millis( );
      }
    Serial.println("Error");
    lcd.setCursor(0, 0);
    lcd.print("Error: Water Low");
    displayTime();
    digitalWrite(green, LOW);
    digitalWrite(blue, LOW);
    digitalWrite(red, HIGH);
    while(checkLevel() == 1){//checks if water level is still too low
      toggleVent();
    }
    digitalWrite(red, LOW);
    if(temperature < tempRange){
      Serial.println("Idle");
      lcd.setCursor(0, 0);
      lcd.print("Idle            ");
      displayTime(); 
    }
    else{
      Serial.println("Running");
      lcd.setCursor(0, 0);
      lcd.print("Running         ");
      displayTime();
    }
  }
}
void runningState(){
  if(temperature >= tempRange){
    Serial.println("Running");
    lcd.setCursor(0, 0);
    lcd.print("Running         ");
    displayTime();
    digitalWrite(green, LOW);
    digitalWrite(blue, HIGH);
    while(temperature >= tempRange){
      digitalWrite(blue, HIGH);
      if( checkTemp( &temperature, &humidity ) == true ){
        Serial.print( "T = " );
        Serial.print( temperature, 1 );
        Serial.print( " deg. C, H = " );
        Serial.print( humidity, 1 );
        Serial.println( "%" );
        lcd.setCursor(0, 0);
        lcd.print("Temp=           ");
        lcd.setCursor(5, 0);
        lcd.print((temperature * 9/5)+32);
        lcd.setCursor(7, 0);
        lcd.print("F       ");
        lcd.setCursor(9, 0);
        lcd.print("Hum=  ");
        lcd.setCursor(13, 0);
        lcd.print(humidity);
        lcd.setCursor(15, 0);
        lcd.print("%");
      }
      toggleFan();
      toggleVent();
      disabledState();
      idleState();
      errorState();
    }
  }
}
