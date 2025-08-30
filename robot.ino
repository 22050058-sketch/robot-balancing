// --- Thư viện ---
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// --- PINOUT L298N ---
#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 14
#define ENA 32   // PWM A
#define ENB 33   // PWM B

// Kênh PWM cho ESP32
#define ENA_CH 0
#define ENB_CH 1

// --- Biến và hằng số ---
#define LED_PIN 2 // Chân LED tích hợp trên ESP32

#define PID_TASK_PRIORITY 3
#define MOTOR_TASK_PRIORITY 2
#define MPU_TASK_PRIORITY 4
#define DISPLAY_TASK_PRIORITY 1

#define PID_TASK_DELAY_MS 10
#define MOTOR_TASK_DELAY_MS 10
#define MPU_TASK_DELAY_MS 5
#define DISPLAY_TASK_DELAY_MS 50

double Kp = 15.0;
double Ki = 0.5;
double Kd = 5.0;
double setpoint = 0.0;

// Giới hạn và tham số hệ thống
#define SAFE_ANGLE_DEG 35
#define INTEGRAL_LIMIT 200.0
#define MOTOR_DEADZONE 20
#define PWM_FREQ 20000
#define PWM_RES_BITS 8

// Khai báo handle cho các Queue
QueueHandle_t angle_queue;
QueueHandle_t pid_output_queue;

// Khai báo đối tượng MPU6050
Adafruit_MPU6050 mpu;

// --- Khai báo các hàm của FreeRTOS Task ---
void mpuTask(void *parameter);
void pidTask(void *parameter);
void motorTask(void *parameter);
void displayTask(void *parameter);
void setMotor(int power);

// Tuỳ chọn kiểm tra motor khi khởi động (0: tắt, 1: bật)
#define MOTOR_TEST 0

// Cờ trạng thái để hiển thị/debug
volatile bool g_outOfSafeAngle = false;
volatile int g_lastMotorPower = 0;

void setup() {
  Serial.begin(115200);

  // Khai báo chân LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Bật LED để báo hiệu setup đang chạy

  // Khởi tạo MPU6050
  Wire.begin();
  if (!mpu.begin()) {
    Serial.println("Lỗi: Không tìm thấy MPU6050!");
    // Nhấp nháy LED để báo lỗi
    while (1) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Cấu hình chân L298N
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Cấu hình PWM: dùng analogWrite để tương thích rộng (ESP32 tự ánh xạ LEDC)
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

 
  // Khởi tạo Queue
  angle_queue = xQueueCreate(1, sizeof(float));
  pid_output_queue = xQueueCreate(1, sizeof(int));

  if (angle_queue == NULL || pid_output_queue == NULL) {
    Serial.println("Lỗi: Không tạo được Queue!");
    while(1);
  }

  // Tạo các Task
  xTaskCreatePinnedToCore(mpuTask, "MPU Task", 2048, NULL, MPU_TASK_PRIORITY, NULL, 1);
  xTaskCreatePinnedToCore(pidTask, "PID Task", 4096, NULL, PID_TASK_PRIORITY, NULL, 1);
  xTaskCreatePinnedToCore(motorTask, "Motor Task", 2048, NULL, MOTOR_TASK_PRIORITY, NULL, 0);
  xTaskCreatePinnedToCore(displayTask, "Display Task", 2048, NULL, DISPLAY_TASK_PRIORITY, NULL, 0);

  // Tắt LED sau khi setup hoàn thành và các task được tạo
  digitalWrite(LED_PIN, LOW);
}

void loop() {

}

// --- Các hàm FreeRTOS Task ---

// Task đọc dữ liệu MPU6050
void mpuTask(void *parameter) {
  float angle = 0.0;
  long lastTime = micros();
  sensors_event_t a, g, temp;
  // Hiệu chỉnh nhanh: lấy bias gyro và offset góc từ accel trong thời gian ngắn
  const int calibSamples = 200;
  float gyroBiasY = 0.0f;
  float accelAngleOffset = 0.0f;
  for (int i = 0; i < calibSamples; i++) {
    mpu.getEvent(&a, &g, &temp);
    gyroBiasY += g.gyro.y;
    // Góc từ accel (pitch) dùng atan2(x, z)
    float accelAngle = atan2f(a.acceleration.x, a.acceleration.z) * 57.2957795f;
    accelAngleOffset += accelAngle;
    vTaskDelay(pdMS_TO_TICKS(5));
  }
  gyroBiasY /= calibSamples;
  accelAngleOffset /= calibSamples;
  
  for (;;) {
    long currentTime = micros();
    float dt = (currentTime - lastTime) / 1000000.0;
    lastTime = currentTime;
    
    mpu.getEvent(&a, &g, &temp);

    // Tính toán góc từ Gyro và Accel (sử dụng Complementary filter)
    float gyroY = g.gyro.y - gyroBiasY;
    float accelAngle = atan2f(a.acceleration.x, a.acceleration.z) * 57.2957795f - accelAngleOffset;
    angle = 0.98f * (angle + gyroY * dt) + 0.02f * accelAngle;

    // Gửi góc nghiêng vào Queue cho task PID
    xQueueOverwrite(angle_queue, &angle);

    vTaskDelay(pdMS_TO_TICKS(MPU_TASK_DELAY_MS));
  }
}

// Task tính toán PID
void pidTask(void *parameter) {
  float angle = 0.0;
  double error, integral = 0, lastError = 0;
  long lastTime = millis();
  
  for (;;) {
    if (xQueueReceive(angle_queue, &angle, portMAX_DELAY) == pdPASS) {
      long currentTime = millis();
      float dt = (currentTime - lastTime) / 1000.0;
      lastTime = currentTime;

      // Ngắt an toàn: nếu góc vượt quá giới hạn, tắt motor và reset PID
      if (fabs(angle) > SAFE_ANGLE_DEG) {
        g_outOfSafeAngle = true;
        int zero = 0;
        xQueueOverwrite(pid_output_queue, &zero);
        integral = 0;
        lastError = 0;
        vTaskDelay(pdMS_TO_TICKS(PID_TASK_DELAY_MS));
        continue;
      }
      g_outOfSafeAngle = false;

      error = setpoint - angle;
      // Tích phân có giới hạn (anti-windup)
      integral += error * dt;
      if (integral > INTEGRAL_LIMIT) integral = INTEGRAL_LIMIT;
      else if (integral < -INTEGRAL_LIMIT) integral = -INTEGRAL_LIMIT;
      double derivative = (error - lastError) / dt;
      int output = Kp * error + Ki * integral + Kd * derivative;
      lastError = error;

      int motorPower = constrain(output, -255, 255);
      // Deadzone để tránh rung nhẹ
      if (motorPower > -MOTOR_DEADZONE && motorPower < MOTOR_DEADZONE) {
        motorPower = 0;
      }
      g_lastMotorPower = motorPower;
      xQueueOverwrite(pid_output_queue, &motorPower);
    }
    vTaskDelay(pdMS_TO_TICKS(PID_TASK_DELAY_MS));
  }
}

// Task điều khiển motor
void motorTask(void *parameter) {
  int motorPower = 0;
  
  for (;;) {
    if (xQueueReceive(pid_output_queue, &motorPower, portMAX_DELAY) == pdPASS) {
      setMotor(motorPower);
    }
    vTaskDelay(pdMS_TO_TICKS(MOTOR_TASK_DELAY_MS));
  }
}

// Task hiển thị dữ liệu (ví dụ: qua Serial)
void displayTask(void *parameter) {
  float angle;
  int motorPower;
  
  for (;;) {
    if (xQueuePeek(angle_queue, &angle, 0) == pdPASS &&
        xQueuePeek(pid_output_queue, &motorPower, 0) == pdPASS) {
      Serial.print("Angle: ");
      Serial.print(angle);
      Serial.print(" | PID Output: ");
      Serial.println(motorPower);
    }
    // LED báo trạng thái: nhanh nếu vượt SAFE, chậm nếu bình thường và power=0, sáng nếu đang drive
    if (g_outOfSafeAngle) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    } else if (g_lastMotorPower == 0) {
      static uint32_t t = 0;
      if (millis() - t > 500) { t = millis(); digitalWrite(LED_PIN, !digitalRead(LED_PIN)); }
    } else {
      digitalWrite(LED_PIN, HIGH);
    }
    vTaskDelay(pdMS_TO_TICKS(DISPLAY_TASK_DELAY_MS));
  }
}

// Hàm điều khiển motor
void setMotor(int power) {
  if (power > 0) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, power);
    analogWrite(ENB, power);
  } else if (power < 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, abs(power));
    analogWrite(ENB, abs(power));
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
  }
}