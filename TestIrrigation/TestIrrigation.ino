#define CLK 2
#define DT 3
#define SW 4
#define PWM_PIN 9

int lastCLKState;
int currentCLKState;
bool fanEn = false;
int fanPWM = 0;
char buffer[50];

void setup(){
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);
  pinMode(PWM_PIN, OUTPUT); // Set PWM pin as output
  Serial.begin(9600);
  lastCLKState = digitalRead(CLK);
}

void loop(){
  currentCLKState = digitalRead(CLK);

  if (currentCLKState != lastCLKState){
    if(digitalRead(DT) != currentCLKState){
      
      if (fanPWM > 0){
        fanPWM -= 20;
      } else{
        fanPWM = 0;
      }
      sprintf(buffer, "Rotated CCW, %d\n", fanPWM);
      Serial.print(buffer);
    }
    else{
//      Serial.printf("Rotated CW");
      if (fanPWM < 255){
        fanPWM += 20;
      } else {
        fanPWM = 255;
      }
      sprintf(buffer, "Rotated CW, %d\n", fanPWM);
      Serial.print(buffer);
    }
    }
    lastCLKState = currentCLKState;
    if (digitalRead(SW) == LOW) {
        sprintf(buffer, "Fan enable status: %b\n", fanEn);
        Serial.println(buffer);
        fanEn != fanEn;
        delay(200);  // Debounce delay
  }
  if(fanEn){
    analogWrite(PWM_PIN, fanPWM);
  }
  else
    analogWrite(PWM_PIN, 0);
  
}
