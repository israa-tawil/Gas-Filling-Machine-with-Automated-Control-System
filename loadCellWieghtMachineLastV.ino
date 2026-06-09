// libraries 
#include <Keypad.h>
#include <LiquidCrystal.h>
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif
#include <ESP32_Servo.h>

// variables 

Servo myservo;
int pos = 0;


float i=0;

bool goal = false; // goal state variable

const int ROW_NUM = 4; //four rows
const int COLUMN_NUM = 4; //four columns

// keypad buttons
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3','.'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte pin_rows[ROW_NUM] = {13, 12, 14, 27} ; //connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {26, 25, 33, 32}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

String inputString;
long inputInt;

 
// Create An LCD Object. Signals: [ RS, EN, D4, D5, D6, D7 ]
LiquidCrystal My_LCD(19, 21, 18, 17, 16, 15);

// create relay and buzzer
int relay = 2;
int buzzer = 4;

// wieght variables 
float nowWeight = 0; // change cotinusly
float zeroWeight; // the wieght before start
int goalWeight; // the wieght i want  

// load cell hx711
//pins: 
const int HX711_dout = 23; //mcu > HX711 dout pin
const int HX711_sck = 22; //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_calVal_eepromAdress = 0;
unsigned long t = 0;

 static boolean newDataReady = 0;
 const int serialPrintInterval = 500; //increase value to slow down serial print activity



void setup() {

  myservo.attach(5);
  
  Serial.begin(57600); delay(10);
  Serial.println();
  Serial.println("Starting..."); 
  float calibrationValue; // calibration value
  calibrationValue = 777.39; 
#if defined(ESP8266) || defined(ESP32)
  //EEPROM.begin(512); // uncomment this if you use ESP8266 and want to fetch this value from eeprom
#endif
  //EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch this value from eeprom



  // load cell
  LoadCell.begin();
  //LoadCell.setReverseOutput();
  unsigned long stabilizingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration factor (float)
    Serial.println("Startup is complete");
  }
  while (!LoadCell.update());
  Serial.print("Calibration value: ");
  Serial.println(LoadCell.getCalFactor());
  Serial.print("HX711 measured conversion time ms: ");
  Serial.println(LoadCell.getConversionTime());
  Serial.print("HX711 measured sampling rate HZ: ");
  Serial.println(LoadCell.getSPS());
  Serial.print("HX711 measured settlingtime ms: ");
  Serial.println(LoadCell.getSettlingTime());
  Serial.println("Note that the settling time may increase significantly if you use delay() in your sketch!");
  if (LoadCell.getSPS() < 7) {
    Serial.println("!!Sampling rate is lower than specification, check MCU>HX711 wiring and pin designations");
  }
  else if (LoadCell.getSPS() > 100) {
    Serial.println("!!Sampling rate is higher than specification, check MCU>HX711 wiring and pin designations");
  }



  inputString.reserve(7); // maximum number of digit for a number is 7, change if needed

  pinMode(relay,OUTPUT);
  pinMode(buzzer,OUTPUT);

  // Initialize The LCD. Parameters: [ Columns, Rows ]
  My_LCD.begin(20, 2);
  // Clears The LCD Display
  My_LCD.clear();
 
  // Display The First Message In Home Position (0, 0)
  My_LCD.setCursor(0,0);
  My_LCD.print("Enter the weight: ");

  
        // zoroWieght
        // check for new data/start next conversion:
        if (LoadCell.update()) newDataReady = true;
      
        // get smoothed value from the dataset:
        if (newDataReady) {
          if (millis() > t + serialPrintInterval) {
             zeroWeight =LoadCell.getData();
             Serial.println("zeroWeight =");
             Serial.println(zeroWeight);
             }}
             
       newDataReady = false;
  }


void loop() {
  char key = keypad.getKey();

  if (key) {
    //My_LCD.print(key);
    Serial.print(key);
    if ((key >= '0' && key <= '9')||key == '.') {     // only act on numeric keys
      inputString += key;

         My_LCD.clear();
         My_LCD.setCursor(0,0);
         My_LCD.print("Goal Weight= ");
         My_LCD.setCursor(14,0);
         My_LCD.print(inputString);
      
      // append new character to input string
    } else if (key == '#') { // the # here == start button after enter the weight
      if (inputString.length() > 0) {
        inputInt = inputString.toInt(); // YOU GOT AN INTEGER NUMBER
        goalWeight =  inputInt;
        inputString = "";               // clear input
        // DO YOUR WORK HERE

        

        

        // while we dont get the goal

        while(!goal ){

 
  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
       i =LoadCell.getData();
       
       if (goalWeight+zeroWeight >= i)
        {
      Serial.print("Load_cell output val: ");
      Serial.println(i-zeroWeight);
      My_LCD.setCursor(0,1);
      My_LCD.print(i-zeroWeight);
      digitalWrite(relay,HIGH);
      
      for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15 ms for the servo to reach the position
      }
  
  }
      

      if(goalWeight+zeroWeight <= i)
          {
    
             for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
             myservo.write(pos);              // tell servo to go to position in variable 'pos'
             delay(15);                       // waits 15 ms for the servo to reach the position
             }

  
             digitalWrite(relay,LOW);
             My_LCD.clear();
             My_LCD.setCursor(0,0);
             My_LCD.print("END...");
             My_LCD.setCursor(0,2);
             My_LCD.print("Goal Weight= ");
             My_LCD.setCursor(14,2);
             My_LCD.print(i-zeroWeight);
             digitalWrite(buzzer,HIGH);
             delay(200);
             digitalWrite(buzzer,LOW);
             delay(200);
             digitalWrite(buzzer,HIGH);
             delay(200);
             digitalWrite(buzzer,LOW);

             goal = true;
           }       
      newDataReady = 0;
      t = millis();
    }
  }

  // receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

  // check if last tare operation is complete:
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }
        }
  
         
         
      }
    } else if (key == '*') {
      inputString = ""; //we use the * here to clear input and zero the wight
         My_LCD.clear();
         My_LCD.setCursor(0,0);
         My_LCD.print("zeroWeight= ");
         My_LCD.setCursor(0,2);
         My_LCD.print(zeroWeight);
       
        // zoroWieght
        // check for new data/start next conversion:
        if (LoadCell.update()) newDataReady = true;
      
        // get smoothed value from the dataset:
        if (newDataReady) {
          if (millis() > t + serialPrintInterval) {
             zeroWeight =LoadCell.getData();
             Serial.println("zeroWeight =");
             Serial.println(zeroWeight);
             }}
             
       newDataReady = false;
       goal = false;
   
       
  }
}
 
      
    }
  
