/**
  Kevin Manan's Program for Alarm Clock
**/




/**
	Libraries
**/
#include <Arduino.h>
#include "pitches.h"
#include "Time.h"  //manipulator
#include "DS1307RTC.h"  //for rtc
#include <Wire.h> //I2C 



/**
  Below is the PinMap
**/
//define alarm system
#define speaker 13
#define stopButton 12 //pin for the switch.

//define hour LED
#define HMSB 2
#define H3 A3 //watch out for pin.
#define H2 0//not sure if pin exists.
#define H1 1
#define H0 4

//define MSB LED. uses BCD. pinMap  
#define MSB2 A0
#define MSB1 A1
#define MSB0 A2

//define LSB LED. uses BCD. pinMap
#define LSB3 5
#define LSB2 6
#define LSB1 7
#define LSB0 8

#define setAmPm 3 //temporary

//unused pins,




/**
  Below is for global definition. 
**/
int melody[]= {
   NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};
int noteDuration[]= {
  4, 8, 8, 4, 4, 4, 4, 4
};

tmElements_t tm; //time struct

//below is used to determine the #'s into BCD. 
int MSBH; //MSB of the hour x-:--
int H; //LSB of the hour -x:--
int MSBminute; //MSB of min --:x-
int LSBminute; //LSB of min --:-x

//am or pm for output
bool AMPM; //Determines whether it is am or pm 

//alarm switch for turning off alarm. 
bool alarmSwitch;

//raw data. Need to assign this to the RTC for actual time, then converted to each digit, then to BCD. MAY NOT NEED. MIGHT BE CIRCUMVENTED.
int currentHour; //current hour. raw data. 12= 12. **IN USE** 
int currentMinute; //current minute.raw data. *NOT* in use. 




/**
	Below are declared functions.
**/
void playAlarm(); //plays alarm
void setAlarmoff(); //for interrupt
void callOUTPUTS(); //to call all the outputs, this is for displaying the time on the 8-segment. ****This function also calls the write from rtc.*****
void setOUTPUTHMSB(); //sets x-:--
void setOUTPUTH();  //sets -x:--
void setOUTPUTMSBm(); //sets --:x-
void setOUTPUTLSBm(); //sets --:-x
void determineNUM(int*, int*, int*, int*); // for actually setting the # to BCD. Used at specific output functions. prior to setting off output.
void determineNUM(int*, int*, int*, int*, int*); //overloaded for H & MSBm. purpose is the same as above^ keeps output function clean. 
void outputAMPM(); //sets the AM/PM feature. IF AM then 0. PM = 1. 




/**
	SETUP, arduino basic setup
**/
void setup() {

  //Below sets pin
  
  //speaker pins
  pinMode(speaker, OUTPUT);
  pinMode(stopButton, INPUT);
  
  //SEndING THE TIME TO 8 SEGMENT.
  pinMode(HMSB, OUTPUT);
  pinMode(H3, OUTPUT);
  pinMode(H2, OUTPUT);
  pinMode(H1, OUTPUT);
  pinMode(H0, OUTPUT);
  pinMode(MSB2, OUTPUT);
  pinMode(MSB1, OUTPUT);
  pinMode(MSB0, OUTPUT);
  pinMode(LSB3, OUTPUT);
  pinMode(LSB2, OUTPUT);
  pinMode(LSB1, OUTPUT);
  pinMode(LSB0, OUTPUT);
  pinMode(setAmPm, OUTPUT);

  /* This is for user changing time, feature will be taken off for now. More modern to setup bluetooth/internet that does timing.
  //switches for changing time
  pinMode(MSBS, INPUT);
  pinMode(LSBS, INPUT);
  pinMode(HS, INPUT);
  */


  //interrupts
  attachInterrupt(digitalPinToInterrupt(stopButton), setAlarmOff, CHANGE); //switch to turn of alarm. 

  /* For user setup time. dont need.
  attachedInterrupt(digitalPinToInterrupt(MSBS), setMSBm, CHANGE);
  attachedInterrupt(digitalPinToInterrupt(LSBS), setLSBm, CHANGE);
  attachedInterrupt(digitalPinToInterrupt(HS), setHour, CHANGE);
  */
}

/**
	LOOP MAIN FUNCTION
**/
void loop() {

  playAlarm(); //function that does a lot of the work. checks for alarm.
 
  //retrigger alarm. if it's no longer 0. 
  if(alarmSwitch ==1){
    if(LSBminute != 0){
    	alarmSwitch= 0;
  	}
  }
  callOUTPUTS();//makes sure the LEDs are changing. **also gets the read from the rtc. 
}




/**
  function definition
**/

//alarm
void playAlarm(){
    int duration=0;
    int pauseBetweenNotes= 1.3;
    int thisNote=0;//iterator for the while loop of alarm. 

    while(H==8 && LSBminute == 0 && alarmSwitch == 0 && AMPM==0){ //alarm set @ 8AM. Alarm only plays for a minute OR until pressed. No snooze implemented YET. 

    	//Plays the alarm. 
		duration = 1000/noteDuration[thisNote];
		pauseBetweenNotes= 1.3* duration;
		tone(speaker, melody[thisNote], duration);
		delay(pauseBetweenNotes);
		noTone(speaker);
		++ thisNote;
		if(thisNote == 9){
			thisNote = 0; 
			delay(150); //just to have some pause when the melody ends. 
		}
		if(RTC.write(tm)){
			currentHour= tm.Hour;
			currentMinute= tm.Minute;
		}
		callOUTPUTS();
    }
}


//this function is the interrupt to change the alarmSwitch from a 0 -> 1. turning off the alarm. 
void setAlarmOff(){ 
  alarmSwitch = 1; 
}


//For decoding the output in the 8 segments
void callOUTPUTS(){
	if(RTC.write(tm)){
		setOUTPUTH();
		setOUTPUTHMSB();
		setOUTPUTLSBm();
		outputAMPM();
	}else{
		printf("there was a problem reading the time");
	}
}
void setOUTPUTHMSB(){
   digitalWrite(HMSB, MSBH);
}
void setOUTPUTH(){
	int hr3, hr2, hr1, hr0;
  	currentHour= hourFormat12();//sets currentHour to current Hour of TM with 12 hour format. 
    if(currentHour > 9){
      H=currentHour - 10; //so if it's 10=0, 11=1, 12= 2
      MSBH =1;
    }if(currentHour <10){
      H= currentHour;
      MSBH=0;
    }else{
      printf("THere was an error setting up the hourFormat");
    }
    determineNUM(&hr3, &hr2, &hr1, &hr0, H);
    digitalWrite(H3, hr3);
    digitalWrite(H2, hr2);
    digitalWrite(H1, hr1);
    digitalWrite(H0, hr0);
}
void setOUTPUTMSBm(int MSBminute){
  int m2, m1, m0;
  MSBminute = tm.Minute / 10; //so if it's 50, it'll display a 5. 
  determineNUM(&m2, &m1, &m0, MSBminute);
  digitalWrite(MSB2, m2);
  digitalWrite(MSB1, m1);
  digitalWrite(MSB0, m0);
}
void setOUTPUTLSBm(){
  int LSBminute=0;
  int l3, l2, l1, l0;
  LSBminute = tm.Minute %10; //gets the remainder number by 10 ie lsb.
  determineNUM(&l3, &l2, &l1, &l0, LSBminute);
  digitalWrite(LSB3, l3);
  digitalWrite(LSB2, l2);
  digitalWrite(LSB1, l1);
  digitalWrite(LSB0, l0);
}

//these are functions that are used to implement the outputs
void determineNUM(int *x, int *y, int *z, int numInput){
  //determines the x.
  if(numInput > 4 || numInput< 8){
    *x= 1;
  }else{
    *x=0;
  }
  //determines y.
  if(numInput ==2 || numInput == 3 || numInput == 6 || numInput == 7 || numInput == 10){
    *y=1;
  }else{
    *y=0;
  }
  //detmines z.
  if(numInput %2){ //if it has a remainder after 0 it is odd.
    *z= 1; 
  }else{
    *z=0; 
  }
}
void determineNUM(int *w, int *x, int *y, int *z, int numInput){
  //determines the W.
  if(numInput <8){
    *w=0;
  }else{
    *w=1;
  }
  //detmines x
  if(numInput > 4 || numInput< 8){
    *x= 1;
  }else{
    *x=0;
  }
  //determines y.
  if(numInput ==2 || numInput == 3 || numInput == 6 || numInput == 7 || numInput == 10){
    *y=1;
  }else{
    *y=0;
  }
  //detmines z.
  if(numInput %2){ //if it has a remainder after 0 it is odd.
    *z= 1; 
  }else{
    *z=0; 
  }
}

//sets output of the ampm
void outputAMPM(){
	if(RTC.write(tm)){
  		AMPM = isPM(); //if it's AM then it'll return false or 0;
  		digitalWrite(setAmPm, AMPM);
  	}else{
  		printf("There was an error reading the time");
  	}
}






















