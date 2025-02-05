#include <Time.h>
#include <Stepper.h> 
#include <Time.h>
#include <stdlib.h>

const int stepsPerRevolution = 60; //stpper degree setup
Stepper stepper(100, 34, 35, 36, 37); //stepper magazine
Stepper stepperNum1(100, 14, 15, 16, 17); //stepper hole1
Stepper stepperNum2(100, 18, 19, 20, 21); //stepper hole2
Stepper stepperNum3(100, 22, 23, 24, 25); //stepper hole3
Stepper stepperNum4(100, 26, 27, 28, 29); //stepper hole4
Stepper stepperNum5(100, 30, 31, 32, 33); //stepper hole5

int noseLed1 = 2,noseLed2 = 3, noseLed3 = 4, noseLed4 = 5, noseLed5 = 6, MagazineLed = A4, HouseLED = 12;
int noseSensor1 = 7, noseSensor2 = 8, noseSensor3 = 9, noseSensor4 = 10, noseSensor5 = 11, MagazineSensor = 13;
int CorrectResponse, Omission, IncorrectResponse, PrematureResponse, RandLedNum, LedNum, SensorNum, TrialNum, SensorPin;

boolean TrialStart = false, stepper_state, CR;
unsigned long StimulusTimeStart;
unsigned long StimulusTimeEnd;
unsigned long ITITimeStart, ITITimeEnd, LHTimeStart, LHTimeEnd;
unsigned long LatencyToResponseTime, LatencyToResponseTimeStart, LatencyToResponseTimeEnd,LatencyToResponseTimeTotal;
unsigned long LatencyToRewardTime, LatencyToRewardTimeStart, LatencyToRewardTimeEnd, LatencyToRewardTimeTotal;
unsigned long MagazineTime = 600000, MagazineTimeTotal, MagazineTimeStart, MagazineTimeEnd;

//Time parameters used for training, can be modified according to training needs
int StimulusDuration = 30000;
int LHDuration = 5000;
int ITIDuration = 7000;
int TODuration = 5000;
int PumpSteps = 30;
int MagazineWaterSteps = 250; 



void setup()
{
    stepper.setSpeed(550); // set the speed of the motor
    stepperNum1.setSpeed(200);
    stepperNum2.setSpeed(200);
    stepperNum3.setSpeed(200);
    stepperNum4.setSpeed(200);
    stepperNum5.setSpeed(200);
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
        Serial.print("Total trail numbers:");
        Serial.println(TrialNum);
        Serial.print("Stimulus time and ITI time:");
        Serial.print(StimulusDuration/1000);
        Serial.print("s");
        Serial.print(" ");
        Serial.print(ITIDuration/1000);
        Serial.println("s");
        Serial.println("start");
        Serial.print("Premature response number and Premature(%):");
        Serial.print(PrematureResponse);
        Serial.print(" ");
        Serial.print(100 * PrematureResponse / (PrematureResponse + Omission + CorrectResponse + IncorrectResponse));
        Serial.println("%");
        Serial.print("Correct response number and accuracy(%):");
        Serial.print(CorrectResponse);
        Serial.print(" ");
        Serial.print(100 * CorrectResponse / (CorrectResponse + IncorrectResponse));
        Serial.println("%");
        Serial.print("Incorrect response:");
        Serial.println(IncorrectResponse);
        Serial.print("Omission and Ommision(%):");
        Serial.print(Omission);
        Serial.print(" ");
        Serial.print(100 * Omission / (Omission + CorrectResponse + IncorrectResponse));
        Serial.println("%");
        Serial.print("Latency to resonse (s)");
        Serial.print(LatencyToResponseTimeTotal/CorrectResponse);
        Serial.println("s");
        Serial.print("Latency to reward (s)");
        Serial.print(LatencyToRewardTimeTotal/CorrectResponse);
        Serial.println("s");

        //Trial number count, ITItime count.
        TrialNum ++;
        ITITimeStart = millis();
        ITITimeEnd = millis();

        //ITI precess
        while((ITITimeEnd-ITITimeStart) < ITIDuration)
        { 
            reset();
            digitalWrite(HouseLED,HIGH);

            //detect the premature process
            PrematureBehavior();
            ITITimeEnd = millis();
        }
    delay(10); //sensor detection rate
    Serial.println("Light stimulus");
    RandLedNum = random(2,7); //random LED light and sensor
    LedNum = RandLedNum; //Led number
    SensorNum = RandLedNum + 5;
    Serial.print("LedNum:");
    Serial.println(LedNum-1);

    StimulusTimeStart = millis();
    StimulusTimeEnd = millis();
    LatencyToResponseTimeStart = millis(); //Latency setup time

    WaterDelivery(); //Delivery sugar water to the lighted hole

    //Stimulus process
    while((StimulusTimeEnd - StimulusTimeStart) < StimulusDuration) //Stimulus duration
    {
        digitalWrite(LedNum, HIGH);
        char ch = Serial.read();
        
        //reset
        reset();

        //Nose poke detection
        NosePokeDetection();
        
        //Stimulus over, turn off the stimulus light
        digitalWrite(LedNum, LOW);
        LHTimeStart = millis();
        LHTimeEnd = millis();

    }
    
    // LH progress
    while((LHTimeEnd - LHTimeStart)< LHDuration)
    {
        reset();

        //Nose poke detection
        NosePokeDetection();
        delay(10); //sensor detection rate
        LHTimeEnd = millis();
    }
    Omission = Omission + 1;
    digitalWrite(LedNum, LOW);
    Serial.println("");
    Serial.println("**Omission**");
    Serial.println("");
    digitalWrite(HouseLED, LOW); //Time out punish mice for 5s

    WaterWithdraw();

    delay(TODuration);
    
    //Magazine LED duration count
    MagazineTimeStart = millis();

    //Magazine poke detection
    MagazinePoke();
    }
}


void reset()
{
    char ch = Serial.read();
    if (ch == 'r') //Enter 'r' on the keyboard to reset the program
    { 
        TrialNum = 0;
        CorrectResponse = 0;
        PrematureResponse = 0;
        IncorrectResponse = 0;
        Omission = 0;
        LatencyToResponseTimeTotal = 0;
        LatencyToRewardTimeTotal = 0;
        digitalWrite(noseLed1, LOW);
        digitalWrite(noseLed2, LOW);
        digitalWrite(noseLed3, LOW);
        digitalWrite(noseLed4, LOW);
        digitalWrite(noseLed5, LOW);
        Serial.println("");
        Serial.println("****************************");
        Serial.println("Reset");
        Serial.println("****************************");
        Serial.println("");
        loop();
     } 
}

void PrematureBehavior()
{
    if (digitalRead(noseSensor1) == 1 || digitalRead(noseSensor2) == 1 || digitalRead(noseSensor3) == 1 || digitalRead(noseSensor4) == 1 || digitalRead(noseSensor5) == 1)
    { 
        digitalWrite(HouseLED, LOW);
        PrematureResponse = PrematureResponse + 1;
        Serial.println("");
        Serial.println("**Premature response**");
        Serial.println("");

        // Time out for 5s punishment
        delay(TODuration);
        digitalWrite(MagazineLed, HIGH);

        //Magazine LED duration count
        MagazineTimeStart = millis();

        //Magazine poke behavior detection
        MagazinePoke();
     }    
}

 void MagazinePoke()
 {
    do
    {
        MagazineTimeEnd = millis();
        MagazineTimeTotal = MagazineTimeEnd - MagazineTimeStart;
        if (MagazineTimeTotal >= MagazineTime)
        {
            MagazineTimeStart = 0;
            MagazineTimeEnd = 0;
            CR = false;
            digitalWrite(MagazineLed, LOW);
            stepper.step(-PumpSteps); //Magazine water withdraw
            loop();
        }
        else if (digitalRead(MagazineSensor) == 1)
        {
            CR = false;
            digitalWrite(MagazineLed, LOW);
            loop();
        }
        delay(100);
     }while(CR = true); 
 }

void WaterDelivery()
{
    //Pumping sugar water in hole
    if (LedNum == 2)
    {
        stepperNum1.step(PumpSteps);
    }
    else if (LedNum == 3)
    {
        stepperNum2.step(PumpSteps);
    }
    else if (LedNum == 4)
    {
        stepperNum3.step(PumpSteps);
    }
    else if (LedNum == 5)
    {
        stepperNum4.step(PumpSteps);
    }
    else if (LedNum == 6)
    {
        stepperNum5.step(PumpSteps);
    }
 }

void WaterWithdraw()
{
     //Pumping sugar water
     if (LedNum == 2)
     {
         stepperNum1.step(-PumpSteps);
     }
     else if (LedNum == 3)
     {
         stepperNum2.step(-PumpSteps);
     }
     else if (LedNum == 4)
     {
         stepperNum3.step(-PumpSteps);
     }
     else if (LedNum == 5)
     {
         stepperNum4.step(-PumpSteps);
     }
     else if (LedNum == 6)
     {
         stepperNum5.step(-PumpSteps);
     }
}

void NosePokeDetection()
{
    //Correct response
    if (digitalRead(SensorNum) == 1)
    {
        LatencyToResponseTimeEnd = millis();
        LatencyToResponseTime = LatencyToResponseTimeEnd - LatencyToResponseTimeStart;
        LatencyToResponseTimeTotal = LatencyToResponseTimeTotal + LatencyToResponseTime/1000;

        CR = true;
        stepper_state = true;
        LatencyToRewardTimeStart = millis();
        stepper.step(MagazineWaterSteps); //Reward sugar water in magazine
        Serial.println("");
        Serial.println("**CorrectResponse**");
        Serial.println("");
        CorrectResponse = CorrectResponse + 1; //Correct response number
        digitalWrite(MagazineLed, HIGH);
        digitalWrite(LedNum, LOW);
        delay(10);
        stepper_state = false;
        MagazinePoke();             
    }

     //Incorrect response
     else if (digitalRead(noseSensor1) == 1 || digitalRead(noseSensor2) == 1 || digitalRead(noseSensor3) == 1 || digitalRead(noseSensor4) == 1 || digitalRead(noseSensor5) == 1)
     {
        IncorrectResponse = IncorrectResponse + 1;
        Serial.println("");
        Serial.println("**IncorrectResponse**");
        Serial.println(""); 
        digitalWrite(LedNum, LOW);
        digitalWrite(HouseLED, LOW); 

        //Withdrawing sugar water
        WaterWithdraw();

        delay(TODuration);   //Time out punish mouse for 5s
        digitalWrite(HouseLED, HIGH); 
        loop(); 
     }
}
