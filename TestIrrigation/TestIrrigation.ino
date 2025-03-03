#define CLK 2
#define DT 3
#define SW 4

int lastCLKState;
int currentCLKState;

void setup(){
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);
  Serial.begin(9600);
  lastCLKState = digitalRead(CLK);
}

void loop(){
  currentCLKState = digitalRead(CLK);

  if (currentCLKState != lastCLKState){
    if(digitalRead(DT) != currentCLKState){
      Serial.println("Rotated CCW");
    }
    else{
      Serial.println("Rotated CW");
    }
    }
    lastCLKState = currentCLKState;
    if (digitalRead(SW) == LOW) {
        Serial.println("Button Pressed");
        delay(200);  // Debounce delay
  }
  
}
