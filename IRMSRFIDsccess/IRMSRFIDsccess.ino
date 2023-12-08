#define BLYNK_TEMPLATE_ID "TMPL6fDSJ9Uv8"
#define BLYNK_TEMPLATE_NAME "Assignmentes"
#define BLYNK_AUTH_TOKEN "efM9z-IjoNjlHLTLU0Bt2fYl6gpx5Wwk"

#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "efM9z-IjoNjlHLTLU0Bt2fYl6gpx5Wwk";
char ssid[] = "iPhone11";
char pass[] = "12345678910";

constexpr uint8_t RST_PIN = D3;
constexpr uint8_t SS_PIN = D4;

const int irSensorPin1 = D1; // Pin for the first infrared sensor
const int irSensorPin2 = D0; // Pin for the second infrared sensor
int obstacleCount1 = 0; // Count the number of infrared obstacles 1
int obstacleCount2 = 0; // Count the number of infrared obstacles 2
int totalObstacleCount = 0; // Total number of obstacles from both sensors
int totalObstacleGaugePin = V0; // Change to virtual pin assigned to total Gauge

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
Servo servo;

IRrecv irrecv(irSensorPin1);  // Use a different pin if needed
decode_results results;

int redLedPin = V1;
int yellowLedPin = V2;
int greenLedPin = V3;
int gaugePin = V4;  // Change to the virtual pin assigned to the Gauge widget
bool isRedLedOn = false;
bool isYellowLedOn = false;
bool isGreenLedOn = false;
bool isObstacle1Detected = false; // Turn the flag to check whether there is an infrared obstacle 1 or not
bool isObstacle2Detected = false;


void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  servo.attach(D2);
  irrecv.enableIRIn();
  pinMode(irSensorPin2, INPUT);
  Blynk.begin(auth, ssid, pass);
  Blynk.virtualWrite(totalObstacleGaugePin, 0); // Reset tổng Gauge về 0 khi khởi động
}

void openDoor() {
  servo.write(0);
  delay(2500);
  servo.write(180);
  delay(1000);
}

void toggleRedLed() {
  isRedLedOn = !isRedLedOn;
  if (isRedLedOn) {
    Blynk.virtualWrite(redLedPin, HIGH);
    updateGauge();
    Serial.println("Red LED ON");
  } else {
    Blynk.virtualWrite(redLedPin, LOW);
    updateGauge();
    Serial.println("Red LED OFF");
  }
}

void toggleYellowLed() {
  isYellowLedOn = !isYellowLedOn;
  if (isYellowLedOn) {
    Blynk.virtualWrite(yellowLedPin, HIGH);
    updateGauge();
    Serial.println("Yellow LED ON");
  } else {
    Blynk.virtualWrite(yellowLedPin, LOW);
    updateGauge();
    Serial.println("Yellow LED OFF");
  }
}

void toggleGreenLed() {
  isGreenLedOn = !isGreenLedOn;
  if (isGreenLedOn) {
    Blynk.virtualWrite(greenLedPin, HIGH);
    updateGauge();
    Serial.println("Green LED ON");
  } else {
    Blynk.virtualWrite(greenLedPin, LOW);
    updateGauge();
    Serial.println("Green LED OFF");
  }
}

void updateGauge() {
  int gaugeValue = 0;

  if (isRedLedOn) {
    gaugeValue += 30;
  }
  if (isYellowLedOn) {
    gaugeValue += 30;
  }
  if (isGreenLedOn) {
    gaugeValue += 40;
  }

  if (!isRedLedOn && !isYellowLedOn && !isGreenLedOn) {
    gaugeValue = 0;
  } else if ((isRedLedOn && !isYellowLedOn && !isGreenLedOn) ||
             (!isRedLedOn && isYellowLedOn && !isGreenLedOn) ||
             (!isRedLedOn && !isYellowLedOn && isGreenLedOn)) {
    gaugeValue = 30;
  } else if ((isRedLedOn && isYellowLedOn && !isGreenLedOn) ||
             (isRedLedOn && !isYellowLedOn && isGreenLedOn) ||
             (!isRedLedOn && isYellowLedOn && isGreenLedOn)) {
    gaugeValue = 70;
  }
  
  Blynk.virtualWrite(gaugePin, gaugeValue);
}

void accessGranted(String tag) {
  Serial.println("Access Granted!");
  openDoor();
  if (tag == "2921325248") {
    toggleRedLed();
  } else if (tag == "18920327") {
    toggleYellowLed();
  } else if (tag == "8516312173") {
    toggleGreenLed();
  }
}

void accessDenied() {
  Serial.println("Access Denied!");
  servo.write(0);
  delay(1000);
}
void updateIRGauge() {
  int gaugeIRValue = totalObstacleCount; // Cập nhật giá trị Gauge dựa trên tổng số vật cản
  Blynk.virtualWrite(totalObstacleGaugePin, gaugeIRValue);
}

void handleIRSensor() {
  if (irrecv.decode(&results)) {
    // Add your logic here based on the received IR code
    totalObstacleCount = (obstacleCount1 > 0 || obstacleCount2 > 0) ? 1 : 0;

    updateIRGauge();
    irrecv.resume(); // Receive the next value
  }
}


void handleIRSensor1() {
  int obstacleValue = digitalRead(irSensorPin1);

  if (obstacleValue == HIGH) {
    if (!isObstacle1Detected) {
      obstacleCount1++;
      totalObstacleCount++;
      isObstacle1Detected = true;
      updateIRGauge();
    }
  } else {
    if (isObstacle1Detected) {
      obstacleCount1--;
      totalObstacleCount--;
      isObstacle1Detected = false;
      updateIRGauge();
    }
  }
}

void handleIRSensor2() {
  int obstacleValue = digitalRead(irSensorPin2);

  if (obstacleValue == HIGH) {
    if (!isObstacle2Detected) {
      obstacleCount2++;
      totalObstacleCount++;
      isObstacle2Detected = true;
      updateIRGauge();
    }
  } else {
    if (isObstacle2Detected) {
      obstacleCount2--;
      totalObstacleCount--;
      isObstacle2Detected = false;
      updateIRGauge();
    }
  }
}

void loop() {
  Blynk.run();
  handleIRSensor();
  handleIRSensor1();
  handleIRSensor2();

  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (rfid.PICC_ReadCardSerial()) {
    String tag;
    for (byte i = 0; i < 4; i++) {
      tag += rfid.uid.uidByte[i];
    }
    Serial.println(tag);

    if (tag == "2921325248" || tag == "18920327" || tag == "8516312173") {
      accessGranted(tag);
    } else {
      accessDenied();
    }
    tag = "";
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}
