#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// === Khai báo chân ===
#define TRIG_GATE 7
#define ECHO_GATE 6
#define SERVO_PIN 9
#define BUZZER_PIN 8
#define RELAY_PIN 5
#define MQ2_PIN A0

#define TRIG_SLOT1 10
#define ECHO_SLOT1 11
#define TRIG_SLOT2 12
#define ECHO_SLOT2 13

// === Biến toàn cục ===
Servo myServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C LCD 16x2

bool fireDetected = false;
long distanceGate, distance1, distance2;

// === Hàm đo khoảng cách bằng HC-SR04 ===
long readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return duration / 58.2; // cm
}

void setup() {
  Serial.begin(115200);

  lcd.begin(16, 2);
  lcd.print("Automatic Parking");
  delay(3000);

  myServo.attach(SERVO_PIN);
  myServo.write(0); // Cửa đóng ban đầu

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Hệ thống hoạt động

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Buzzer tắt

  pinMode(TRIG_GATE, OUTPUT);
  pinMode(ECHO_GATE, INPUT);

  pinMode(TRIG_SLOT1, OUTPUT);
  pinMode(ECHO_SLOT1, INPUT);
  pinMode(TRIG_SLOT2, OUTPUT);
  pinMode(ECHO_SLOT2, INPUT);
}

void loop() {
  // Đọc MQ2
  int mq2Value = analogRead(MQ2_PIN);

  // === Phát hiện cháy ===
  if (mq2Value > 400) {
    if (!fireDetected) {
      fireDetected = true;
      digitalWrite(RELAY_PIN, LOW);  // Ngắt hệ thống
      digitalWrite(BUZZER_PIN, HIGH); // Cảnh báo
      myServo.write(90);  // Mở cửa khẩn cấp
      lcd.clear();
      lcd.print("CHAY! Mo cua!!");
      tone(BUZZER_PIN, 1000); // Phát ra âm thanh cảnh báo với tần số 000 Hz
    }
    return;  // Dừng các chức năng khác
  } else {
    if (fireDetected) {
      fireDetected = false;
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(BUZZER_PIN, LOW); 
      noTone(BUZZER_PIN); // Tắt còi buzzer nếu không có cảnh báo
    }
  }
       
    

  // === Đọc khoảng cách xe trước cổng ===
  distanceGate = readDistance(TRIG_GATE, ECHO_GATE);

  lcd.clear();
  if (distanceGate < 60) {
    myServo.write(90);
    lcd.print("Cua da mo!");
   
  } else {
    myServo.write(0);
    lcd.print("Cua da dong!");
   
  }

  // === Đọc trạng thái ô đỗ xe ===
  distance1 = readDistance(TRIG_SLOT1, ECHO_SLOT1);
  distance2 = readDistance(TRIG_SLOT2, ECHO_SLOT2);

  bool slot1Occupied = distance1 < 20;
  bool slot2Occupied = distance2 < 20;

  lcd.setCursor(0, 1);
  lcd.print("O1:");
  lcd.print(slot1Occupied ? "FULL " : "NO ");
  lcd.print("O2:");
  lcd.print(slot2Occupied ? "FULL " : "NO ");

  // Debug Serial
  Serial.print("MQ2: "); Serial.print(mq2Value);
  Serial.print(" | Gate: "); Serial.print(distanceGate);
  Serial.print(" | O1: "); Serial.print(slot1Occupied);
  Serial.print(" | O2: "); Serial.println(slot2Occupied);

  delay(500);
}
