#include <esp_now.h>
#include <WiFi.h>

#define ENA 13  
#define IN1 12 
#define IN2 14
#define IN3 27
#define IN4 26
#define ENB 25 

typedef struct struct_message {
  int x; 
  int y; 
} struct_message;

struct_message myData;

void driveMotor(int motorPin1, int motorPin2, int enablePin, int speed) {
  speed = constrain(speed, -255, 255);

  if (speed > 0) {
    // Move Forward
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
    analogWrite(enablePin, speed);
  } else if (speed < 0) {
    // Move Backward
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
    analogWrite(enablePin, abs(speed)); 
  } else {
    // Stop
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    analogWrite(enablePin, 0);
  }
}


void OnDataRecv(const esp_now_recv_info_t * info, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  
  int throttle = myData.y; 
  int steering = myData.x; 

  // Differential Drive Mixing Algorithm
  int leftMotorSpeed = throttle + steering;
  int rightMotorSpeed = throttle - steering;

  leftMotorSpeed = constrain(leftMotorSpeed, -255, 255);
  rightMotorSpeed = constrain(rightMotorSpeed, -255, 255);

  if (abs(leftMotorSpeed) < 40) leftMotorSpeed = 0;
  if (abs(rightMotorSpeed) < 40) rightMotorSpeed = 0;

  driveMotor(IN3, IN4, ENB, leftMotorSpeed);    
  driveMotor(IN1, IN2, ENA, rightMotorSpeed);   
}

void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);

  driveMotor(IN1, IN2, ENA, 0);
  driveMotor(IN3, IN4, ENB, 0);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("Car Ready");
}

void loop() {
  delay(100); 
}