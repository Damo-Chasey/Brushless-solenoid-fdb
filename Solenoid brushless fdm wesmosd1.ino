#include <Servo.h>

//#include <TimeLib.h>
//#include <Wire.h>  
#include "SSD1306Wire.h"

SSD1306Wire display(0x3c, SDA, SCL);

//time_t timeStamp = now();
Servo ESC;
Servo ESC2;
int editButton = 0;
int power = 50;
int power2 = 50;
int revTrigger = 0;
int trigger = 0;
int fireMode = 0;
int curSpeed = 0;
int spinMulti = 18;
int activeTime = 25;
int resetTime = 75;
int escOffset = 0; 
int count = 0;

const int triggerPin = 14;
const int triggerPin2 = 12;
const int escPin = 15;
const int escPin2 = 13;
const int solenoidPin = 16;
const int editPin = 0; //To ground to activite

void setup() {
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  //Serial.println("Testing solenoid...");
  
  Serial.begin(9600);
  pinMode(editPin, INPUT);
  
  pinMode(solenoidPin, OUTPUT);
  digitalWrite(solenoidPin, HIGH);
  
  pinMode(triggerPin, INPUT);
  pinMode(triggerPin2, INPUT);
  ESC.write(10);
  ESC.attach(escPin);
  ESC2.write(10);
  ESC2.attach(escPin2);
  pinMode(LED_BUILTIN, OUTPUT);
}

void updateTriggers(){
  revTrigger = digitalRead(triggerPin2);
  trigger = digitalRead(triggerPin);
}

void updatepower(){
  power2 = map(power,0,180,escOffset,180);
}

void editing(){
  updateTriggers();
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  
  if(revTrigger == HIGH){
    count += 1;
    if(count == 6){
      count =0;
    }
    while(revTrigger == HIGH){
      revTrigger = digitalRead(triggerPin2);
      delay(50);
    }
  }
  if(trigger == HIGH){
    switch(count){
      case 0:
        power += 5;
        if(power > 180){
          power = 60;
        }
      break;
      case 1:
        activeTime += 1;
        if(activeTime > 50){
          activeTime = 10;
        }
      break;
      case 2:
        resetTime += 5;
        if(resetTime > 200){
          resetTime = 25;
        }
      break;
      case 3:
        fireMode += 1;
        if(fireMode > 2){
          fireMode = 0;
        }
        while(trigger == HIGH){
          trigger = digitalRead(triggerPin);
          delay(50);
        }
      break;
      case 4:
        spinMulti += 1;
        if(spinMulti > 50){
          spinMulti = 10;
        }
      break;
      case 5:
        escOffset += 5;
        if(escOffset > 100){
          escOffset = 0;
        }
      break;
    }
  }

  
  /*
  if(revTrigger == HIGH){
    power += 5;
    if(power > 80){
      power = 0;
    }
  }

  if(trigger == HIGH){
    fireMode += 1;
    if(fireMode > 2){
      fireMode = 0;
    }
  }
  */
  showStats();
  delay(150);
}

void showStats(){
  display.drawString(0, 0, "Power:");
  display.drawString(64, 0, String(power));

  display.drawString(0, 11, "Punch:");
  display.drawString(64, 11, String(activeTime));

  display.drawString(0, 22, "SPacing:");
  display.drawString(64, 22, String(resetTime));
    
  Serial.println("power set too: ");
  Serial.println(power);
  
  switch(fireMode){
    case 0:
      display.drawString(0, 33, "Single");
      break;
    case 1:
      display.drawString(0, 33, "burst");
      break;
    case 2: 
      display.drawString(0, 33, "Auto");
      break;
  }

  display.drawString(0, 44, "Heat:");
  display.drawString(64, 44, String(spinMulti));

  display.drawString(0, 55, "escOff");
  display.drawString(64, 55, String(escOffset));

  display.drawString(90, count * 11, "<");
  
  display.display();
}

boolean revUp(){
  if(curSpeed >= spinMulti){
    return true;
  }

  updateTriggers();
  ESC.write(power);
  ESC2.write(power2);
  
  while((revTrigger == HIGH || trigger == HIGH) && (curSpeed < spinMulti)){
    delay(50);
    updateTriggers();
    curSpeed += 2;
  }
  if(curSpeed >= spinMulti){
    return true;
  }
  else{
    return false;
  }
}

boolean revDown(){
  updateTriggers();
  ESC.write(0);
  ESC2.write(0);

  while((revTrigger == LOW && trigger == LOW) && (curSpeed > 0)){
    delay(50);
    updateTriggers();
    curSpeed -= 1;
  }
  if(curSpeed == 0){
    return true;
  }
  else{
    return false;
  }
}

boolean fire(){
  boolean revving = revUp();
  if(revving == true){
    digitalWrite(solenoidPin, LOW);
    Serial.println("Fire ON");

    delay(activeTime);
    
    digitalWrite(solenoidPin, HIGH);
    Serial.println("Fire OFF");

    delay(resetTime);
    return true;
  }
  else{
    return false;
  }
}

void autofire(){
  trigger = digitalRead(triggerPin);
  while(trigger == HIGH){
    fire();
    trigger = digitalRead(triggerPin);
  }
}

void burstfire(){
  trigger = digitalRead(triggerPin);
  for(int i = 0 ; i < 3 ; ++i){
    if(trigger == HIGH){
      fire();
      trigger = digitalRead(triggerPin);
    }
  }
}



void loop() {
  // put your main code here, to run repeatedly:
  editButton = digitalRead(editPin);
  updateTriggers();
  
  if(editButton == LOW){
    Serial.println("entering edit mode");
    while(editButton == LOW){
      editButton = digitalRead(editPin);
      delay(50);
    }
    while(editButton == HIGH){
      editing();
      editButton = digitalRead(editPin);
      
    }
    while(editButton == LOW){
      editButton = digitalRead(editPin);
      delay(50);
    }
    Serial.println("Leaving edit mode");
    display.clear();
    display.display();
  }
  
  if(revTrigger == HIGH){
    revUp();
  }
  else{
    revDown();
  }
  
  if(trigger == HIGH){
    switch(fireMode){
      case(0):
        fire();
      break;
      case(1):
        burstfire();
      break;
      case(2):
        autofire();
      break;
    }
    trigger = digitalRead(triggerPin);
    while(trigger == HIGH){
      delay(25);
      trigger = digitalRead(triggerPin);
    }
  }else{
    digitalWrite(solenoidPin, HIGH);
    //Serial.println("Fire OFF");
  }

  delay(50);

}
