
# 🤖 Robot Tự Cân Bằng

## 📘 1. Lý thuyết

### 1.1 Nguyên lý hoạt động
Robot tự cân bằng là một hệ thống điều khiển **ngược con lắc** (inverted pendulum).  
- Robot có 2 bánh xe làm điểm tựa.  
- Khi robot bị nghiêng, cảm biến sẽ đo được góc nghiêng.  
- Bộ điều khiển PID sẽ tính toán và điều chỉnh tốc độ động cơ để kéo robot về trạng thái cân bằng.  

### 1.2 Cảm biến MPU6050
- MPU6050 là cảm biến gồm **gia tốc kế (accelerometer)** và **con quay hồi chuyển (gyroscope)**.  
- Gia tốc kế đo độ nghiêng theo phương trọng lực.  
- Con quay đo tốc độ quay theo các trục.  
- Kết hợp 2 giá trị này (qua bộ lọc Complementary/Kalman) → tính được góc nghiêng chính xác.  

### 1.3 Bộ điều khiển PID
- **P (Proportional)**: tác động theo độ nghiêng hiện tại.  
- **I (Integral)**: bù trừ sai số tích lũy theo thời gian.  
- **D (Derivative)**: phản ứng nhanh theo tốc độ thay đổi góc.  
- Kết hợp 3 tham số **Kp, Ki, Kd** giúp robot giữ thăng bằng ổn định.  

---

## ⚙️ 2. Yêu cầu phần cứng
- Bo mạch: **ESP32** hoặc **Arduino Uno/Nano**.  
- Cảm biến: **MPU6050**.  
- Driver động cơ: **L298N** hoặc **TB6612FNG**.  
- 2 động cơ DC (có encoder càng tốt).  
- Pin 7.4V – 12V.  
- Dây nối Jumper.  

---

## 💻 3. Cài đặt phần mềm
1. Tải **Arduino IDE**:  
   👉 [Arduino IDE Download](https://www.arduino.cc/en/software)  

2. Nếu dùng ESP32 → thêm board:  
   - Mở **File → Preferences**, thêm:  
     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```  
   - Vào **Tools → Board Manager**, cài **ESP32**.  

3. Cài thư viện:  
   - `Wire` (mặc định).  
   - `Adafruit MPU6050`.  
   - `Adafruit Unified Sensor`.  

---

## 🔌 4. Kết nối phần cứng
- **MPU6050 → ESP32/Arduino**:  
  - VCC → 3.3V (ESP32) hoặc 5V (Arduino).  
  - GND → GND.  
  - SDA → GPIO21 (ESP32) hoặc A4 (Arduino).  
  - SCL → GPIO22 (ESP32) hoặc A5 (Arduino).  

- **Driver động cơ**:  
  - IN1, IN2, IN3, IN4 → chân điều khiển trong code.  
  - ENA, ENB → chân PWM.  
  - Motor A/B → động cơ DC.  
  - VCC (7V–12V) → pin.  
  - GND → chung với ESP32/Arduino.  

---

## ▶️ 5. Chạy chương trình
1. Mở file `robot.ino` bằng Arduino IDE.  
2. Chọn **Board** (ESP32 Dev Module hoặc Arduino Uno).  
3. Chọn đúng **Port** (COMx).  
4. Upload code.  
5. Đặt robot thẳng đứng → bật nguồn → robot sẽ tự cân bằng.  

---

## 🔧 6. Hiệu chỉnh
- Nếu robot nghiêng nhiều → chỉnh hệ số PID (`Kp`, `Ki`, `Kd`).  
- Nếu robot chạy ngược → đảo dây động cơ hoặc đổi chiều trong code.  
- Nếu rung lắc mạnh → giảm `Kp`, tăng `D`.  

---

## 🛠 7. Gỡ lỗi
- Cảm biến không nhận → kiểm tra SDA/SCL và điện áp.  
- Động cơ không chạy → kiểm tra PWM và driver.  
- Robot ngả liên tục → cần **tinh chỉnh PID**.  
