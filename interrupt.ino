#define LED1 18
#define LED2 22
#define BUTTON 23

void IRAM_ATTR IO_INT_ISR(){
  digitalWrite(LED2, !digitalRead(LED2));
}

void setup() {
  // put your setup code here, to run once:
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUTTON, INPUT);
  attachInterrupt(BUTTON, IO_INT_ISR, RISING);
}

void loop() {
  digitalWrite(LED1, 1);
  delay(1000);
  digitalWrite(LED1, 0);
  delay(1000);
}
