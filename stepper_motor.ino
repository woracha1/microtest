#include <Stepper.h>
#include <WiFi.h>
#include <MQTT.h>
#include <Bounce2.h>

#define LDR 36

//-----------------------------------in1, in3, in2, in4
Stepper myStepper(stepsPerRevolution, 18, 21, 19, 22);
const int stepsPerRevolution = 2048;

int LDR_Value = 0;
long currentStep = 0;  
bool isLockMode = true;

Bounce switchMode = Bounce();

const char ssid[] = "@JumboPlusIoT";
const char pass[] = "ballafu123";

const char mqtt_broker[] = "test.mosquitto.org";
const char mqtt_topic[] = "wora/command";
const char mqtt_client_id[] = "arduino_group_1";
int MQTT_PORT = 1883;


WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect(mqtt_client_id)) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe(mqtt_topic);
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  if (payload == "reset") {
    isLockMode = true;
    myStepper.step(-currentStep);  // หมุนกลับไปที่ 0°
    currentStep = 0;
  }
  if (payload == "changeMode") {
    isLockMode = !isLockMode;
  }
}

void setup() {

  currentStep = 0;

  Serial.begin(9600);
  WiFi.begin(ssid, pass);

  analogReadResolution(10);
  myStepper.setSpeed(15);
  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
  // by Arduino. You need to set the IP address directly.
  client.begin(mqtt_broker, MQTT_PORT, net);
  client.onMessage(messageReceived);

  pinMode(LDR, INPUT);

  switchMode.interval(25);

  connect();
}

void loop() {
  client.loop();
  delay(10);
  if (!client.connected()) connect();

  
  if (!isLockMode) {
    LDR_Value = analogRead(LDR);
    if (LDR_Value > 300 && LDR_Value < 800) {
      int moveStep = (LDR_Value <= 550) ? -10 : 10;
      myStepper.step(moveStep);
      currentStep += moveStep;

      // จัดการค่า currentStep ให้อยู่ในระยะ 1 รอบเสมอ
      if (currentStep >= stepsPerRevolution) currentStep -= stepsPerRevolution;
      if (currentStep < 0) currentStep += stepsPerRevolution;
    }
  }

  // ส่งข้อมูลขึ้น Cloud ทุก 1 วินาที
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    int angle = (currentStep * 360) / stepsPerRevolution;
    
    client.publish("wora/LDR", String(LDR_Value));
    client.publish("wora/angle", String(angle));
    client.publish("wora/mode", isLockMode ? "Lock Position" : "Unlock Position");
  }
}
