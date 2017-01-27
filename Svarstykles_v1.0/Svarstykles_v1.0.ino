/******************* Automatic weight scales ***************************
created by Paulius Plus 
project started 12 Sep 2016
Last-modified 15 Sep 2016
For Arduino Nano Atmega328
/************************************************************************/

// include the library code:
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
#include <PinChangeInt.h>
#include <EEPROM.h>
#include "HX711.h"

//#define calibration_factor -11050.0 //This value is obtained using the SparkFun_HX711_Calibration sketch

#define DOUT  15
#define CLK  14

HX711 scale(DOUT, CLK);

#define OPEN HIGH
#define CLOSE LOW


// Data wire is plugged into port 10 on the Arduino
#define ONE_WIRE_BUS 16
#define RESOLUTION 11
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature termometras(&oneWire);
DeviceAddress tempDeviceAddress;

unsigned long lastTempRequest = 0;
int  delayInMillis = 0;
float temperature = 0.0;




// constants won't change. Used here to set a pin number :
const int RedLed =  13;      // the number of the LED pin
const int GreenLed =  18;      // the number of the LED pin
//const int LcdLed =  16;      // the number of the LCD screen light pin
const int Beeper =  17;      // the number of the Beeper pin
const int Solenoid =  19;      // the number of the solenoid pin

const int btnDOWN =  12;      // the number of the LED pin
const int btnUP =  11;      // the number of the LED pin
const int btnSTART =  10;      // the number of the Beeper pin
const int btnMODE =  9;      // the number of the LCD screen light pin
const int btnENTER =  8;      // the number of the solenoid pin

const long delayTime = 300;           // interval at which to blink (milliseconds) *******************************************************

volatile int keyNumber = 0;

// Variables will change :
int RledState = LOW;             // RledState used to set the R LED
int beepState = LOW;             // beepState used to set the Beeper
int lcdState = LOW;             // lcdState used to set the Lcd
int GledState = LOW;             // GledState used to set the G LED
int SolState = LOW;             // SolState used to set the Solenoid
int runningState = 0;


int calibration_factor = 0;
int tempCalibration_factor = 0;
int maxWeight = 0;  //set max weight
int tempMaxWeight = 0;
int minTemp = 0;  // minimal temperature degree celsius
int tempMinTemp = 0;
float weight = 0;
int tempCheck = 0;

unsigned long currentSolenoidMillis = 0;
unsigned long previousSolenoidMillis =0;

#define TEMPDIF 3  //hysteresis temperature degree celsius +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


 // initialize the library with the numbers of the interface pins
 LiquidCrystal lcd(2, 5, 6, 3, 7, 4);
 //LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

void btnDOWNfunc() {
  keyNumber = 1;
}
void btnUPfunc() {
  keyNumber = 2;
}
void btnSTARTfunc() {
  keyNumber = 3;
}
void btnMODEfunc() {
  keyNumber = 4;
}
void btnENTERfunc() {
  keyNumber = 5;
}
//++++++++++++++++++++++++ SETUP +++++++++++++++++++++++++++++++++

void setup() {
  
  lcd.clear();     // clear the screen
 //  Serial.begin(9600);
  
 // set the digital pin as output:
  pinMode(RedLed, OUTPUT);
  pinMode(GreenLed, OUTPUT);
  pinMode(Beeper, OUTPUT);
  //pinMode(LcdLed, OUTPUT);
  pinMode(Solenoid, OUTPUT);
  
 // set the digital pin as input:
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  
  attachPinChangeInterrupt(btnDOWN, btnDOWNfunc, FALLING);
  attachPinChangeInterrupt(btnUP, btnUPfunc, FALLING);
  attachPinChangeInterrupt(btnSTART, btnSTARTfunc, FALLING);
  attachPinChangeInterrupt(btnMODE, btnMODEfunc, FALLING);
  attachPinChangeInterrupt(btnENTER, btnENTERfunc, FALLING);
  
  int eeAddress = 0;   //EEPROM Location we want the data to be put.
  EEPROM.get(eeAddress, maxWeight);
  
  //eeAddress += sizeof(int);            //Move address to the next byte after int
  eeAddress = 32;   //EEPROM Location we want the data to be put.
  EEPROM.get(eeAddress, minTemp);
  
  //eeAddress += sizeof(int);            //Move address to the next byte after int
  eeAddress = 64;   //EEPROM Location we want the data to be put.
  EEPROM.get(eeAddress, calibration_factor);
  
  //calibration_factor = -25000;
  //maxWeight = 4900;
  //minTemp = 37;
  
  tempMaxWeight = maxWeight;
  tempCalibration_factor = calibration_factor;
  tempMinTemp = minTemp;
  
  
    termometras.begin();
    termometras.getAddress(tempDeviceAddress, 0);
    termometras.setResolution(tempDeviceAddress, RESOLUTION);
    termometras.setWaitForConversion(false);
    termometras.requestTemperatures();
    delayInMillis = 750 / (1 << (12 - RESOLUTION)); 
    lastTempRequest = millis();
       
  
  //delay(500);
  lcd.begin(21, 2);  // set up the LCD's number of columns and rows: 
  lcd.setCursor(20, 0);
  lcd.print("Automatic ");  // Print a message to the LCD.
  //delay(500);
  lcd.setCursor(25, 1);
  lcd.print("scales v1.0 ");
   for (int positionCounter = 0; positionCounter < 18; positionCounter++) {
    // scroll one position left:
    lcd.scrollDisplayLeft();
    // wait a bit:
    delay(160);
  } 
  delay(1000);  
  lcd.clear();     // clear the screen 
  lcd.setCursor(2, 1);
  lcd.print(" Calibration");
    
    scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
    scale.tare();	//Assuming there is no weight on the scale at start up, reset the scale to 0
    
  delay(500);
  lcd.print(".");
  delay(800);
  lcd.print(".");
  delay(800);
  lcd.print(".");
  delay(800);
 
  
  if (digitalRead(btnMODE) == LOW)                  // switch is pressed - pullup keeps pin high normally
   { 
     tempMaxWeight = maxWeight;
     tempCalibration_factor = calibration_factor;
     
      lcd.clear();     // clear the screen 
      lcd.setCursor(6, 0);
      lcd.print("Settings");
      delay(1000);        // delay to debounce switch 
      while(digitalRead(btnMODE) == LOW){};         // wait for button release
      delay(1000);        // delay to debounce switch
      lcd.clear();     // clear the screen
      
      
     keyNumber = 0;
     while(keyNumber != 4){
      lcd.setCursor(4, 0);
      lcd.print("Max weight");
      lcd.setCursor(7, 1);
      lcd.print(tempMaxWeight);
      lcd.print(" g");
      
      switch (keyNumber) {
    case 1:
      delay(60);
      tempMaxWeight = tempMaxWeight - 10;
      lcd.setCursor(7, 1);
      lcd.print("               ");
      lcd.setCursor(7, 1);   
      lcd.print(tempMaxWeight);
      lcd.print(" g");
      keyNumber = 0;
      break;
    case 2:
      delay(60);
      tempMaxWeight = tempMaxWeight + 10;
      lcd.setCursor(7, 1);
      lcd.print("              ");
      lcd.setCursor(7, 1);
      lcd.print(tempMaxWeight);
      lcd.print(" g");
      keyNumber = 0;
      break;
    case 3:
      eeAddress = 0;
      EEPROM.put(eeAddress, tempMaxWeight);
      //eeAddress += sizeof(int);            //Move address to the next byte after int
      maxWeight = tempMaxWeight;
      keyNumber = 0;
      lcd.setCursor(2, 1);
      lcd.print("   Saved...  ");
      delay(500);
      lcd.clear();
      break;
    case 5:
                   delay(300);
                   lcd.clear(); // clear the screen   
                   keyNumber = 0;      
                   while(keyNumber != 4){
                     lcd.setCursor(2, 0);
                     lcd.print("Min temperature");
                     lcd.setCursor(7, 1);
                     lcd.print(tempMinTemp);
                     lcd.print((char)223);
                     lcd.print("C");
                     delay(500);
                     
                    switch (keyNumber) {
                      case 1:
                        delay(60);
                        tempMinTemp = tempMinTemp - 1;
                        lcd.setCursor(5, 1);
                        lcd.print("               ");
                        lcd.setCursor(7, 1);
                        lcd.print(tempMinTemp);
                        keyNumber = 0;
                        break;
                      case 2:
                        delay(60);
                        tempMinTemp = tempMinTemp + 1;
                        lcd.setCursor(5, 1);
                        lcd.print("               ");              
                        lcd.setCursor(7, 1);
                        lcd.print(tempMinTemp);
                        keyNumber = 0;
                        break;
                      case 3:
                        eeAddress = 32;   //EEPROM Location we want the data to be put.
                        EEPROM.put(eeAddress, tempMinTemp);
                        //eeAddress += sizeof(int);            //Move address to the next byte after int
                        minTemp = tempMinTemp;
                        keyNumber = 0;
                        lcd.setCursor(2, 1);
                        lcd.print("    Saved...  ");
                        delay(500);
                        lcd.clear();
                        break;
            	      case 5:
                        	   delay(300);
                                   lcd.clear(); // clear the screen
                        	   keyNumber = 0;     
                        	     while(keyNumber != 4){
                        	        lcd.setCursor(0, 0);
                        	        lcd.print("COEFF");
                        	        lcd.setCursor(7, 0);
                        	        lcd.print(tempCalibration_factor);
                               		printSvorisKg(scale.get_units(3));
                                        delay(60);
                             	    switch (keyNumber) {
                                      case 1:
                                        delay(60);
                                        tempCalibration_factor = tempCalibration_factor - 100;
                                        scale.set_scale(tempCalibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
                                        lcd.setCursor(5, 0);
                                        lcd.print("               ");
                                        lcd.setCursor(7, 0);
                                        lcd.print(tempCalibration_factor);
                                        keyNumber = 0;
                                        break;
                                      case 2:
                                        delay(60);
                                        tempCalibration_factor = tempCalibration_factor + 100;
                                        scale.set_scale(tempCalibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
                                        lcd.setCursor(5, 0);
                                        lcd.print("               ");              
                                        lcd.setCursor(7, 0);
                                        lcd.print(tempCalibration_factor);
                                        keyNumber = 0;
                                        break;
                                      case 3:
                                        eeAddress = 64;   //EEPROM Location we want the data to be put.
                                        EEPROM.put(eeAddress, tempCalibration_factor);
                                        //eeAddress += sizeof(int);            //Move address to the next byte after int
                                        calibration_factor = tempCalibration_factor;
                                        keyNumber = 0;
                                        lcd.setCursor(2, 1);
                                        lcd.print("    Saved...       ");
                                        delay(500);
                                        lcd.clear();
                                        break;
                                  }   //swith     
                            }  //while  
                  }   //swith     
              }  //while     
              break;
           } //swith
   } //while
   
      lcd.clear(); // clear the screen
      
  } //if
  
 
    lcd.clear(); // clear the screen
  
    keyNumber = 0;    //reset keyboard 
  
}//end of setup



//********************** MAIN LOOP **********************************//
void loop() {
  
    weight = scale.get_units(1);
    printSvorisKg(weight);
    //lcd.setCursor(0, 0);
   // lcd.print(scale.get_units(3), 3); //scale.get_units() returns a float
  //  lcd.print("Svoris");
    lcd.setCursor(4, 0);
    lcd.print("   START  ");
    lcd.setCursor(1, 1);
    tempCheck = 1;
    
    blinkGreenLedShort(3000);
    
    tempRead();    //read and print temperature data
    if(keyNumber == 3){
        beep(3);
                 // lcd.noDisplay();
                 // delay(100);
                  //lcd.display();
                  //delay(300);
        runningState = 1;
        keyNumber = 0;
        delay(100);
        
             while(weight*1000 < maxWeight){
               
               //lcd.setCursor(16, 0);
               //lcd.print(maxWeight);
               
                if(runningState == 0){
                  printSvorisKg(weight);
                  digitalWrite(GreenLed, LOW);
                  //keyNumber = 0;
                  digitalWrite(Solenoid, CLOSE);
                  lcd.setCursor(6, 0);
                  lcd.print(" Pause    ");
                  delay(200);
                  blinkRedLed(delayTime);

                  tempRead();    //read and print temperature data
                  
                  if(keyNumber == 3){
                      beep(3);
                      runningState = 1;
                      keyNumber = 0;
                    }
                  
                  }
                else{
                  weight = scale.get_units(1);
                  printSvorisKg(weight);
                  digitalWrite(Solenoid, OPEN);
                  delay(200);
                  blinkGreenLed(delayTime);
                  digitalWrite(RedLed, LOW);
                  
                    if(tempCheck != 1){                  
                      lcd.setCursor(6, 0);
                      lcd.print("Running    ");
                    }
                  
                        tempRead();    //read and print temperature data
                  
                    if(keyNumber == 3){
                        beep(2);
                        runningState = 0;
                        keyNumber = 0;
                      }

                 
                 
                 
                 
                
  
                      if(tempCheck == 1){
                            //lcd.setCursor(0, 0);
                            //lcd.print("TC1 ");
                            //lcd.print(minTemp);
                            tempRead();    //read and print temperature data
                            
                          if((int)temperature > minTemp){
                            //tempRead();    //read and print temperature data
                            lcd.setCursor(6, 0);
                            lcd.print("Running    ");
                            }
                          else{
                              
                              lcd.setCursor(0, 0);
                              lcd.print("                      ");
                              previousSolenoidMillis = millis();
                              //lcd.setCursor(6, 0);
                              int x = 17;  //i turi but daugiau uz 5
                              
                           while(((int)temperature < minTemp + TEMPDIF)&&(keyNumber != 1)){
                                digitalWrite(Solenoid, CLOSE);
                                tempRead();    //read and print temperature data
                                //lcd.setCursor(6, 0);
                                //lcd.print("Kaista");
                                    if(x < 5){
                                      lcd.setCursor((x+12), 0);
                                      lcd.print(".");
                                      x++;
                                      //lcd.setCursor((x+6), 0);
                                      delay(800);
                                    }
                                    else
                                    {
                                      lcd.setCursor(5, 0);
                                      lcd.print("Heating       ");
                                      x = 0;
                                      delay(800);
                                      //lcd.setCursor(9, 0);
                                    }
                                    
                                    
                                    currentSolenoidMillis = millis();
                                    if(currentSolenoidMillis - previousSolenoidMillis >= 20000) //laukiam 20 sekundziu kol kaista
                                     { 
                                        previousSolenoidMillis = currentSolenoidMillis;   
                                        digitalWrite(Solenoid, HIGH);
                                        delay(1500); 
                                        digitalWrite(Solenoid, HIGH);
                                      }
                                    
                                    
                                    
                                digitalWrite(GreenLed, LOW);
                                blinkRedLed(delayTime);
                              }  //while
                              
                              lcd.clear();     // clear the screen
                              
                              if(keyNumber == 1){
                                
                                //lcd.clear();     // clear the screen
                                beep(1);
                                digitalWrite(RedLed, HIGH);
                                lcd.setCursor(7, 0);
                                lcd.print("STOP?        ");
                                lcd.setCursor(0, 1);
                                lcd.print("Yes              No");
                                delay(400);
                                keyNumber = 0;
                                  while(keyNumber == 0);
                                  digitalWrite(RedLed, LOW);
                                  if(keyNumber == 1){
                                    
                                   lcd.clear();     // clear the screen
                                   tempCheck = 0;
                                   break;                                
                                  }
                              }
                            }  //else
                         }//end of if 
                      
                      }  //end of else
                         
                }  //end of while 

      digitalWrite(Solenoid, CLOSE);
      //delay(200); 
      runningState = 0; 
      lcd.setCursor(4, 0);
      lcd.print("  FINISHED   ");
      beep(6);
      while(keyNumber != 3){
        digitalWrite(GreenLed, HIGH);
        digitalWrite(RedLed, LOW);
      }
      digitalWrite(GreenLed, LOW);
      lcd.clear();
      beep(1);
      delay(200);
      keyNumber = 0;
      
      
        
     } //end of if

    
   //lcd.print(scale.get_units(), 3); //scale.get_units() returns a float
 //  blinkRedLed();
  // lcd.print(checkKeyboard());

}//end of loop





//--------------------FUNCTIONS----------------------------//
void printSvorisKg(float svoris){

lcd.setCursor(11, 1);
  if ((svoris < -100)||(svoris > 100)) {
    lcd.print("    OL  "); 
  }
  else if (((svoris > -10)&&(svoris < 0))||((svoris >= 10)&&(svoris < 100))){
    lcd.print(" "); 
    lcd.print(svoris, 2); //scale.get_units() returns a float
  }
  else if ((svoris >= 0)&&(svoris < 100)) {
    lcd.print("  ");
    lcd.print(svoris, 2); //scale.get_units() returns a float 
  }
  else {
    lcd.print(svoris, 2); //scale.get_units() returns a float  
  }
  
  lcd.setCursor(17, 1);
  lcd.print(" Kg");
}

void blinkGreenLed(long interval){
  // check to see if it's time to blink the LED; that is, if the
  // difference between the current time and last time you blinked
  // the LED is bigger than the interval at which you want to
  // blink the LED.
  unsigned long currentMillis = millis();
  
   if(currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED 
    previousMillis = currentMillis;   

    // if the LED is off turn it on and vice-versa:
    if (GledState == LOW)
      GledState = HIGH;
    else
      GledState = LOW;

    // set the LED with the ledState of the variable:
    digitalWrite(GreenLed, GledState);
  }
}



void blinkGreenLedShort(long interval){
  unsigned long currentMillis = millis();
   if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;   
    if (GledState == LOW){
      digitalWrite(GreenLed, HIGH);
      delay(2);
      digitalWrite(GreenLed, LOW);
    }
    else
    {
      digitalWrite(GreenLed, LOW);
    }
  }
}

void blinkRedLed(long interval){
  // check to see if it's time to blink the LED; that is, if the
  // difference between the current time and last time you blinked
  // the LED is bigger than the interval at which you want to
  // blink the LED.
  unsigned long currentMillis = millis();
  
   if(currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED 
    previousMillis = currentMillis;   

    // if the LED is off turn it on and vice-versa:
    if (RledState == LOW)
      RledState = HIGH;
    else
      RledState = LOW;

    // set the LED with the ledState of the variable:
    digitalWrite(RedLed, RledState);
  }
}


/*
char checkKeyboard(){
 checkKeyboard();
 if (digitalRead(btnENTER) == LOW)
  {  // switch is pressed - pullup keeps pin high normally
    delay(100); 
lcd.setCursor(0, 1);    // delay to debounce switch
    lcd.print("    enter     ");
    return 1;
  }
if (digitalRead(btnDOWN) == LOW)
  {  // switch is pressed - pullup keeps pin high normally
    delay(100); 
lcd.setCursor(0, 1);    // delay to debounce switch
    lcd.print("    down      ");
    return 2;
  }
if (digitalRead(btnSTART) == LOW)
  {  // switch is pressed - pullup keeps pin high normally
    delay(100); 
lcd.setCursor(0, 1);    // delay to debounce switch
    lcd.print("    start     ");
    return 3;
  }
}

*/


void tempRead(){
  
  if (millis() - lastTempRequest >= delayInMillis) // waited long enough??
    {
      temperature = termometras.getTempCByIndex(0);
     // Serial.println("temperatura");
     // Serial.println(temperature);   
        if (temperature > -126.00) 
         {
           //Serial.println("CHECK01");
           lcd.setCursor(0, 1);
           lcd.print(temperature, 1);
           lcd.print((char)223);
           lcd.print("C");
           tempCheck = 1;
         }
        else
         {
           //Serial.println("Error");
           lcd.setCursor(0, 1);
           lcd.print(" ---   ");
           //lcd.setCursor(0, 1);
           //lcd.print(temperature, 1);
           tempCheck = 0;  
         }   
      termometras.requestTemperatures(); 
      delayInMillis = 750 / (1 << (12 - RESOLUTION));
      lastTempRequest = millis();
    } // end of if
}


void beep(int times){
  for(int i=0; i < times; i++){
    digitalWrite(Beeper, HIGH);
    delay(60);
    digitalWrite(Beeper, LOW);
    delay(60);            
  }
}
