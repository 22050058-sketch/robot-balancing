
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

> Tham số khởi điểm trong code: `Kp = 15.0`, `Ki = 0.5`, `Kd = 5.0` (bạn có thể tinh chỉnh khi chạy thử).

---

## ⚙️ 2. Yêu cầu phần cứng
- Bo mạch: **ESP32** (khuyến nghị) hoặc **Arduino Uno/Nano**.  
- Cảm biến: **MPU6050**.  
- Driver động cơ: **L298N** (đã hướng dẫn chi tiết bên dưới) hoặc **TB6612FNG**.  
- 2 động cơ DC (có encoder càng tốt).  
- Pin 7.4V – 12V (nên dùng LiPo 2S).  
- Dây nối Jumper.  

---

## 💻 3. Cài đặt phần mềm
1. Tải **Arduino IDE**:  
   👉 https://www.arduino.cc/en/software

2. Nếu dùng ESP32 → thêm board:  
   - **File → Preferences**: thêm  
     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```  
   - **Tools → Board Manager**: cài **ESP32**.  

3. Cài thư viện:  
   - `Wire` (mặc định).  
   - `Adafruit MPU6050`.  
   - `Adafruit Unified Sensor`.  

---

## 🔌 4. Kết nối phần cứng

### 4.1 MPU6050 → ESP32
- VCC → 3.3V (ESP32) / 5V (Arduino).  
- GND → GND.  
- SDA → GPIO21 (ESP32) hoặc A4 (Arduino).  
- SCL → GPIO22 (ESP32) hoặc A5 (Arduino).  

### 4.2 L298N → ESP32 (dùng đúng chân như trong code)
**Pin mapping trong `robot.ino`:**

```cpp
// ESP32 pin map cho L298N (theo file robot.ino)
#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 14
#define ENA 32   // PWM channel A (LEDC ch 0)
#define ENB 33   // PWM channel B (LEDC ch 1)

// Cấu hình PWM trong code:
// PWM_FREQ = 20000 Hz, PWM_RES_BITS = 8-bit
```

**Đấu dây phần công suất & logic**  
- **L298N**:
  - **+12V / 12V / VMS** → Cực dương pin động cơ (7–12V).  
  - **GND** → Mass pin động cơ **và** nối **chung GND** với ESP32.  
  - **5V-EN jumper**:  
    - **CẮM jumper** khi muốn dùng **regulator 5V trên L298N** để cấp **logic cho chính L298N**.  
    - **THÁO jumper** nếu bạn cấp 5V ngoài vào chân **5V** của L298N.  
    - *Không khuyến nghị* dùng 5V của L298N để nuôi ESP32 (dễ sụt áp khi tải động cơ).  
  - **OUT1 / OUT2** → Motor A.  
  - **OUT3 / OUT4** → Motor B.  
  - **ENA / ENB** → **PWM** từ ESP32 (để điều khiển tốc độ). **Hãy tháo 2 jumper ENA/ENB** trên L298N nếu muốn dùng PWM.  
  - **IN1, IN2** điều khiển chiều **Motor A**; **IN3, IN4** cho **Motor B**.

- **ESP32**:
  - **ENA → GPIO32** (PWM).  
  - **ENB → GPIO33** (PWM).  
  - **IN1 → GPIO25**, **IN2 → GPIO26**.  
  - **IN3 → GPIO27**, **IN4 → GPIO14**.  
  - **GND ESP32** nối **chung GND** với L298N.  

**Lưu ý quan trọng**  
- Logic 3.3V của ESP32 **tương thích** với L298N (mức HIGH ~3.3V vẫn nhận).  
- Nếu motor quay ngược hướng: đổi **IN1↔IN2** (hoặc **IN3↔IN4**) hoặc hoán đổi dây động cơ.  
- L298N tỏa nhiệt khá nhiều; nếu nóng, cân nhắc **giảm Kp/tốc độ**, hoặc chuyển sang **TB6612FNG** (hiệu suất cao hơn).  

---

## ▶️ 5. Chạy chương trình
1. Mở `robot.ino` bằng Arduino IDE.  
2. **Tools → Board**: chọn *ESP32 Dev Module* (nếu dùng ESP32).  
3. **Tools → Port**: chọn cổng COM đúng.  
4. Upload code.  
5. Đặt robot thẳng → bật nguồn động cơ → robot sẽ tự cân bằng.  

---

## 🔧 6. Hiệu chỉnh
- Nếu robot **nghiêng nhiều** → tăng `Kp` (cẩn thận rung lắc), tinh chỉnh `Kd` để giảm dao động.  
- Nếu robot **bị trôi** lâu về sau → tăng nhẹ `Ki`.  
- Nếu **rung mạnh** → giảm `Kp` hoặc tăng `Kd`.  
- Kiểm tra **deadzone** động cơ nếu bánh “không nhúc nhích” ở PWM thấp.  

---

## 🛠 7. Gỡ lỗi nhanh
- **Không thấy góc/MPU lỗi** → kiểm tra SDA/SCL, địa chỉ I2C, dây VCC 3.3V.  
- **Động cơ không chạy** → kiểm tra jumper ENA/ENB (đã tháo chưa), chân PWM, mass chung.  
- **Nóng, hụt áp** → pin yếu hoặc L298N quá tải; giảm tốc, đổi driver TB6612FNG.  
