//www.elegoo.com
//2016.12.08

#include <Wire.h>
#include <DS3231.h>
#include <LiquidCrystal.h>
#include <dht_nonblocking.h>
#define DHT_SENSOR_TYPE DHT_TYPE_11

static const int DHT_SENSOR_PIN = 3;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );

DS3231 clock;
RTCDateTime dt;

int temp = 69;
int tempRange = 50;
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

int button = A0;
int water = A1;

float temperature;
float humidity;

int debug = 0;

//                 BS    E     D4    D5    D6    D7
LiquidCrystal lcd(lcd6, lcd5, lcd4, lcd3, lcd2, lcd1);

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
  lcd.begin(16, 2);
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
  static unsigned long measurement_timestamp = millis( );
  int press = analogRead(button);
  //wait 1 second
  if( millis( ) - measurement_timestamp > 1000ul ){
    measurement_timestamp = millis( );
    //Serial.println(press);
    if(press>1000){
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
    lcd.setCursor(0, 0);
    lcd.print("Disabled        ");
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
  if(temp < tempRange){
    Serial.println("Idle");
    lcd.setCursor(0, 0);
    lcd.print("Idle            ");
    displayTime();
    digitalWrite(green, HIGH);
    digitalWrite(blue, LOW);
    while(temp < tempRange){
      digitalWrite(green, HIGH);
      if(Serial.available()> 0){
        temp = Serial.read();
        Serial.println(temp);
      }
      if( measure_environment( &temperature, &humidity ) == true ){
        Serial.print( "T = " );
        Serial.print( temperature, 1 );
        Serial.print( " deg. C, H = " );
        Serial.print( humidity, 1 );
        Serial.println( "%" );
        lcd.setCursor(0, 0);
        lcd.print("Temp=           ");
        lcd.setCursor(5, 0);
        lcd.print(temperature);
        lcd.setCursor(7, 0);
        lcd.print("C       ");
        lcd.setCursor(9, 0);
        lcd.print("Hum=  ");
        lcd.setCursor(13, 0);
        lcd.print(humidity);
        lcd.setCursor(15, 0);
        lcd.print("%");
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
    lcd.setCursor(0, 0);
    lcd.print("Error: Water Low");
    displayTime();
    digitalWrite(green, LOW);
    digitalWrite(blue, LOW);
    digitalWrite(red, HIGH);
    while(checkLevel() == 1); //checks if water level is still too low
    digitalWrite(red, LOW);
    if(temp < tempRange){
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
  if(temp >= tempRange){
    Serial.println("Running");
    lcd.setCursor(0, 0);
    lcd.print("Running         ");
    displayTime();
    digitalWrite(green, LOW);
    digitalWrite(blue, HIGH);
    while(temp >= tempRange){
      digitalWrite(blue, HIGH);
      if(Serial.available()> 0){
        temp = Serial.read();
      }
      if( measure_environment( &temperature, &humidity ) == true ){
        Serial.print( "T = " );
        Serial.print( temperature, 1 );
        Serial.print( " deg. C, H = " );
        Serial.print( humidity, 1 );
        Serial.println( "%" );
        lcd.setCursor(0, 0);
        lcd.print("Temp=           ");
        lcd.setCursor(5, 0);
        lcd.print(temperature);
        lcd.setCursor(7, 0);
        lcd.print("C       ");
        lcd.setCursor(9, 0);
        lcd.print("Hum=  ");
        lcd.setCursor(13, 0);
        lcd.print(humidity);
        lcd.setCursor(15, 0);
        lcd.print("%");
      }
      disabledState();
      idleState();
      errorState();
    }
  }
}
static bool measure_environment( float *temperature, float *humidity )
{
  static unsigned long measurement_timestamp = millis( );
  /* Checks for new reading every 3 seconds. */
  if( millis( ) - measurement_timestamp > 3000ul ){
    if( dht_sensor.measure( temperature, humidity ) == true )
    {
      measurement_timestamp = millis( );
      return( true );
    }
  }
  return( false );
}
