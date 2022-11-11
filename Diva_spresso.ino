#include "Wire.h"
//#include <LiquidCrystal.h>


// Arduino version compatibility Pre-Compiler Directives
/*#if defined(ARDUINO) && ARDUINO >= 100   // Arduino v1.0 and newer
  #define I2C_WRITE Wire.write 
  #define I2C_READ Wire.read
#else                                   // Arduino Prior to v1.0 
  #define I2C_WRITE Wire.send 
  #define I2C_READ Wire.receive
#endif
*/

// Global Variables
int command = 0;       // This is the command char, in ascii form, sent from the serial port     
int i,now;
long previousMillis = 0;        // will store last time Temp was updated
const int chipSelect = 8;
boolean fillTimeout = false;
boolean pullingShot = false;
long int shotStart, shotFinish;
const int BREW_BUTTON = 17;
const int WATER_LEVEL_SENSOR = 6;
const int AUTOFILL_EV = 5;
const int PUMP = 4;
const int COFFEE_EV = 3;
// Connect via i2c, default address #0 (A0-A2 not jumpered)
//LiquidCrystal lcd(0);liquidcrystal


//-----------------------------------------------------------------------------------------//
void setup() {
  Wire.begin();
  Serial.begin(9600); 
  
//  lcd.setBacklight(LOW);
  // set up the LCD's number of rows and columns: 
//  lcd.begin(16, 2);
  // Print a message to the LCD.
//  lcd.setCursor(0,0);
//  lcd.print("Idle");
  // pins used as inputs with pullup
  pinMode(WATER_LEVEL_SENSOR, INPUT);
  digitalWrite(WATER_LEVEL_SENSOR, HIGH);
  pinMode(BREW_BUTTON, INPUT);
  digitalWrite(BREW_BUTTON, HIGH);

  // pins used as outputs
  // LOW = relay energized
  pinMode(AUTOFILL_EV, OUTPUT);
  digitalWrite(AUTOFILL_EV, HIGH);
  pinMode(PUMP, OUTPUT);
  digitalWrite(PUMP, HIGH);
  pinMode(COFFEE_EV, OUTPUT);
  digitalWrite(COFFEE_EV, HIGH);

  // required for SD card library?
   pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
    pinMode(chipSelect, OUTPUT);
  digitalWrite(chipSelect, HIGH);
   
  // see if the card is present and can be initialized:
 /* if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized."); */
}
//-----------------------------------------------------------------------------------------//
void loop() {
  
check_water_level();
check_brew_button();

Serial.println("In loop");
if(!digitalRead(BREW_BUTTON) && pullingShot)
{
  write_time((millis()-shotStart)/1000);

  Serial.println((millis()-shotStart)/1000);
}

  delay(1000); 
}
//--------------------------------------------------------------
void check_brew_button()
{
  if(!digitalRead(BREW_BUTTON) && !pullingShot)
  {
    delay(20); // debounce
    if(!digitalRead(BREW_BUTTON) && !pullingShot)
    {
      //turn on brew and pump solenoid
      pullingShot = true;
      shotStart = millis();
      digitalWrite(COFFEE_EV, LOW);
      //preinfusion at water mains pressure 8 seconds
//      lcd.setCursor(0,0);
//      lcd.print("Pre-Infuse");

      delay(1000);
      Serial.println((millis()-shotStart)/1000);
      write_time((millis()-shotStart)/1000);
      delay(1000);
      Serial.println((millis()-shotStart)/1000);
      write_time((millis()-shotStart)/1000);
      delay(1000);
      Serial.println((millis()-shotStart)/1000);
      write_time((millis()-shotStart)/1000);
      delay(1000);
      Serial.println((millis()-shotStart)/1000);
      write_time((millis()-shotStart)/1000);
      delay(1000);
      Serial.println((millis()-shotStart)/1000);
      write_time((millis()-shotStart)/1000);
      delay(1000);
      Serial.println((millis()-shotStart)/1000);
      write_time((millis()-shotStart)/1000);
      delay(1000);
      Serial.println((millis()-shotStart)/1000);
      write_time((millis()-shotStart)/1000);
      delay(1000);
      Serial.println((millis()-shotStart)/1000);
      write_time((millis()-shotStart)/1000);

 
      digitalWrite(PUMP, LOW);
      Serial.println("Brew On");
//      lcd.setCursor(0,0);
//      lcd.clear();
//      lcd.print("Brew");

    }
    return;
  }
  if(digitalRead(BREW_BUTTON) && pullingShot)
  {
    delay(20); // debounce
    if(digitalRead(BREW_BUTTON) && pullingShot)
    {
      //turn off brew and pump solenoid
      pullingShot = false;
      shotFinish = millis();
      digitalWrite(COFFEE_EV, HIGH);
      delay(100);
      digitalWrite(PUMP, HIGH);
      Serial.println("Brew Off");
     return; 
    }
  }
}
//-----------------------------------------------------------------------
// water level sense
//-----------------------------------------------------------------------
// If the pin is configured as an INPUT, writing a HIGH value with digitalWrite() will enable an internal 20K pullup resistor (see the tutorial on digital pins). 
//  Writing LOW will disable the pullup.
//  using pin D6
//
//--------------------------------------------------------------
void check_water_level()
{

if(digitalRead(WATER_LEVEL_SENSOR) && !pullingShot)  // HIGH means probe (pullup) not grounded so water level is low - don't fill if in the middle of a shot
{
   autoFill();
}
}

void write_time(int time)
{
//  lcd.setCursor(0,1);
//  lcd.print("  ");
  if(time<10)
  {
//    lcd.setCursor(1,1);
  }
  else
  {
//    lcd.setCursor(0,1);
  }
//  lcd.print(time);
    
  
}
//---------------------------------------------------------
// LOW = level touching |
// HIGH = not touching  | 
//---------------------------------------------------------
// WATER_LEVEL_SENSOR   |  fillTimeout  | Action
//---------------------------------------------------------
//                LOW   |  LOW          | don't fill
//                LOW   |  HIGH         | don't fill
//               HIGH   |  LOW          | fill
//               HIGH   |  HIGH         | don't fill
//----------------------------------------------------------                
void autoFill()
{
  long lwait;
  lwait = millis()+20000; // wait up to 20 seconds to fill
  
  // debug
  //Serial.println("in autoFill");
  //Serial.println((digitalRead(WATER_LEVEL_SENSOR) && !fillTimeout));
  
  while((digitalRead(WATER_LEVEL_SENSOR)) && !fillTimeout)
  { 
     digitalWrite(AUTOFILL_EV, LOW);
     delay(250); // stagger relays on to minimize current spikes
     digitalWrite(PUMP, LOW);

     if((long)(millis()-lwait)>=0)
     {
       fillTimeout = true;
     }
  
    if(fillTimeout){
       digitalWrite(AUTOFILL_EV, HIGH);
       delay(250);
       digitalWrite(PUMP, HIGH);
       Serial.println("fill timeout!!!!!!!!!!!!!!!!");
       return;
    }
    if (!digitalRead(WATER_LEVEL_SENSOR))
    {
       // fill for four seconds past just touching probe
       delay(4000);
       // turn off
       digitalWrite(AUTOFILL_EV, HIGH);
       delay(250);
       digitalWrite(PUMP, HIGH);
       return;
    }
  }
}
//*****************************************************The End***********************
