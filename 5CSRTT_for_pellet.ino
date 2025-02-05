#include <Time.h>
#include <Stepper.h> 
#include <Time.h>
#include <stdlib.h>

const int stepsPerRevolution = 600; //Setup stepper degree for pellet delivery 
Stepper stepper(stepsPerRevolution, 14, 15, 16, 17); //stepper pins from A0-A3
int noseLed1 = 2, noseLed2 = 3, noseLed3 = 4, noseLed4 = 5, noseLed5 = 6, MagazineLed = A4, HouseLED = 12;  //Pins for LEDs
int noseSensor1 = 7, noseSensor2 = 8, noseSensor3 = 9, noseSensor4 = 10, noseSensor5 = 11, MagazineSensor = 13;   //Pins for infrared sensors
int CorrectResponse, Omission, IncorrectResponse, PrematureResponse, RandLedNum, LedNum, SensorNum, SensorPin;    //Parameters
int x, y, z, ITICount, LHCount, StimulusCount;
boolean TrialStart = false;
boolean CR;
unsigned long ITITime;
unsigned long LHTime;
unsigned long StimulusTime;
unsigned long StimulusTime0;
unsigned long ITITime0;
unsigned long LHTime0;

//Time parameters used for training, can be modified according to training needs
int StimulusDuration =16000;       //millisecond as unit
int LHDuration = 4000;             //millisecond as unit
int ITIDuration =5000;             //millisecond as unit
int TODuration = 5000;             //millisecond as unit
int TrialNum;                      //Total trial numbers


void setup() 
{
     stepper.setSpeed(50); // set the speed of the motor to 30 RPMs
    Serial.begin(9600); 
    TrialStart = false;
    pinMode(noseSensor1, INPUT);
    pinMode(noseSensor2, INPUT);
    pinMode(noseSensor3, INPUT);
    pinMode(noseSensor4, INPUT);
    pinMode(noseSensor5, INPUT);
    pinMode(MagazineSensor, INPUT);
    pinMode(MagazineLed, OUTPUT);
    pinMode(noseLed1, OUTPUT);
    pinMode(noseLed2, OUTPUT);
    pinMode(noseLed3, OUTPUT);
    pinMode(noseLed4, OUTPUT);
    pinMode(noseLed5, OUTPUT);
    pinMode(HouseLED, OUTPUT);
}

void loop() 
{ 
 char ch = Serial.read();  
  if (ch == 's') //Enter 's' on the keyboard to start the program
  { 
start:    Serial.print("Total trail numbers:");
          Serial.println(TrialNum);
          Serial.println("start");
          Serial.print("Premature reponse:");
          Serial.println(PrematureResponse);
          Serial.print("Correct response:");
          Serial.println(CorrectResponse);
          Serial.print("Incorrect response:");
          Serial.println(IncorrectResponse);
          Serial.print("Omission:");
          Serial.println(Omission);
          TrialNum ++;
          delay(100);
          
          ITITime0 = millis();
          ITITime = millis();
          x = 0;
          y = 0;
          z = 0;

    // Premature behvior detection   
    while((ITITime-ITITime0) < ITIDuration)
        {          
            digitalWrite(HouseLED,HIGH);
            if (digitalRead(noseSensor1) == 1 || digitalRead(noseSensor2) == 1 || digitalRead(noseSensor3) == 1 || digitalRead(noseSensor4) == 1 || digitalRead(noseSensor5) == 1) 
                {          
                  digitalWrite(HouseLED,LOW);
                  PrematureResponse = PrematureResponse + 1;
                  Serial.println("Premature reponse");
                  delay(5000);       // Time out for punishment
                  goto start;          
                }       
            delay(100);        //sensor detection rate
            ITITime = millis();
            ITICount = int(((ITITime-ITITime0)+1000)/1000);
            if (ITICount != x && ITICount < (ITIDuration/1000 + 1))
            {           
                Serial.print("ITI time:");          
                Serial.println(ITICount);
            }
            x = ITICount;
        }
   
    // If the mouse does not nosepoke any holes during inter trial interval, skip to the light stimulus step
    Serial.println("Light stimulus");
    RandLedNum = random(2,7);     //random LED light and sensor
    LedNum = RandLedNum;          //Led number
    SensorNum = RandLedNum + 5;   //Sensor number
    Serial.print("LedNum:");
    Serial.println(LedNum-1);

    //Nosepoking hole light stimulation 
    StimulusTime0 = millis();
    StimulusTime = millis();
    while((StimulusTime - StimulusTime0) < StimulusDuration)    
      { 
        digitalWrite(LedNum, HIGH);

        if (digitalRead(SensorNum) == 1)
        {
        
             CR = true;
             stepper.step(600);                //Reward a pellet
             Serial.println("CorrectResponse");
             CorrectResponse = CorrectResponse + 1; 
             digitalWrite(MagazineLed, HIGH);
             delay(100);
             stepper.step(-600); 
             do                                //make sure whether magazineSensor works
                { 
                    delay(100);
                    if (digitalRead(MagazineSensor) == 1)
                        {
                            CR = false;
                            digitalWrite(MagazineLed, LOW);       
                            delay(1000);
                            digitalWrite(LedNum, LOW);       
                            goto start;
                        }                
                 }while(CR = true);
        }

        //If the mouse do the incorrect nosepoking behavior, punish the mouse with house LED off.           
        else if ((digitalRead(noseSensor1) == 1 || digitalRead(noseSensor2) == 1 || digitalRead(noseSensor3) == 1 || digitalRead(noseSensor4) == 1 || digitalRead(noseSensor5) == 1) != digitalRead(SensorNum))
          {    
              IncorrectResponse = IncorrectResponse + 1;
              Serial.println("IncorrectResponse");
              digitalWrite(LedNum, LOW);
              digitalWrite(HouseLED,LOW);  //Precribed punish timefor 5s
              delay(TODuration);  
              goto start;       
          }
          delay(100); //sensor detection rate

          //Calculate the stimulus time before the nosepoking behavior
          StimulusTime = millis();
          StimulusCount = int(((StimulusTime-StimulusTime0)+1000)/1000); 
          if (StimulusCount != y && StimulusCount < ((StimulusDuration/1000) + 1))
            { 
                Serial.print("Stimulus time:");
                Serial.println(StimulusCount);
            }
          y = StimulusCount;
      }
      digitalWrite(LedNum, LOW);   //Turn off the stimulus light

     
      
  //Limited hold time for mouse to make response 
  LHTime0 = millis();
  LHTime = millis();
  while((LHTime - LHTime0) < LHDuration)
      {
          //If mouse make correct response, reward the mouse with pellet
          if (digitalRead(SensorNum) == 1)
              {
                  CR = true;
                  stepper.step(600);                //Reward a pellet
                  Serial.println("CorrectResponse");
                  CorrectResponse = CorrectResponse + 1; //Correct response number
                  digitalWrite(MagazineLed, HIGH);
                  delay(100);
                  stepper.step(-600); 
                  do                                //make sure whether magazineSensor works
                      { 
                       delay(100);
                       if (digitalRead(MagazineSensor) == 1)
                          {
                              CR = false;
                              digitalWrite(MagazineLed, LOW);       
                              delay(1000);
                              goto start;
                          }
                       }while(CR = true);
               }

            //If mouse make incorrect response, reward the mouse with pellet
            else if ((digitalRead(noseSensor1) == 1 || digitalRead(noseSensor2) == 1 || digitalRead(noseSensor3) == 1 || digitalRead(noseSensor4) == 1 || digitalRead(noseSensor5) == 1) != digitalRead(SensorNum))
                {    
                    IncorrectResponse = IncorrectResponse + 1;
                    Serial.println("IncorrectResponse");
                    digitalWrite(LedNum, LOW);
                    digitalWrite(HouseLED,LOW);  //Time out punish mice for 5s
                    delay(TODuration);  
                    goto start;       
                 }       
            delay(100); //sensor detection rate
            LHTime=millis();
            LHCount = int(((LHTime-LHTime0)+1000)/1000);
            if (LHCount != z && LHCount < ((LHDuration/1000) + 1))
                { 
                    Serial.print("Limited Hold time:");
                    Serial.println(LHCount);
                }
            z = LHCount;
      }
  //If the mouse fail to nose poke any hole during the LH result in a time out 
    Omission = Omission + 1;
    digitalWrite(LedNum, LOW);
    Serial.println("Omission"); 
    digitalWrite(HouseLED,LOW);  //Time out punish mice for 5s
    delay(TODuration); 
    goto start;
    
  }
}
