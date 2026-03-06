#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

uint8_t broadcastAddress[] = {0x00, 0x4B, 0x12, 0x2C, 0x9E, 0x88}; 

typedef struct struct_message {
  int x; 
  int y; 
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

const int MPU_addr = 0x68;

void setup() {
  Serial.begin(115200);
  
  // Initialize MPU6050
  Wire.begin(21, 22); 
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); 
  Wire.write(0);    
  if (Wire.endTransmission(true) != 0) {
    Serial.println("MPU Connection Failed");
    while(1);
  }

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register Peer (The Robot)
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  
  int numBytes = Wire.requestFrom(MPU_addr, 4, true);
  
  if (numBytes == 4) {
    int16_t AcX = Wire.read() << 8 | Wire.read();
    int16_t AcY = Wire.read() << 8 | Wire.read();

    int y_val = map(AcY, -17000, 17000, -255, 255); 
    int x_val = map(AcX, -17000, 17000, -255, 255);

    y_val = constrain(y_val, -255, 255);
    x_val = constrain(x_val, -255, 255);

    if (abs(y_val) < 35) y_val = 0;
    if (abs(x_val) < 35) x_val = 0;

    myData.x = x_val;
    myData.y = y_val;

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  }
  delay(50); 
}