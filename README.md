
# 🚀 Hướng dẫn chạy chương trình Robot Tự Cân Bằng

## 📌 1. Yêu cầu phần cứng
- Bo mạch: **ESP32** hoặc **Arduino Uno/Nano** (tùy code bạn viết cho loại nào).  
- Cảm biến: **MPU6050** (cảm biến gia tốc + con quay hồi chuyển).  
- Driver động cơ: **L298N** hoặc **TB6612FNG**.  
- 2 động cơ DC có encoder (khuyến nghị).  
- Pin cấp nguồn (Li-ion/LiPo hoặc pin AA, 7.4V – 12V).  
- Dây nối Jumper.  

## 📌 2. Cài đặt phần mềm
1. Tải và cài đặt **Arduino IDE**:  
   👉 [Arduino IDE Download](https://www.arduino.cc/en/software)  

2. Cài đặt **board ESP32** (nếu dùng ESP32):  
   - Vào **File → Preferences**.  
   - Thêm URL sau vào mục *Additional Boards Manager URLs*:  
     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```  
   - Sau đó vào **Tools → Board → Board Manager**, tìm **ESP32**, bấm **Install**.  

3. Cài đặt thư viện cần thiết:  
   - Vào **Sketch → Include Library → Manage Libraries**.  
   - Tìm và cài:  
     - `Wire` (có sẵn trong Arduino IDE).  
     - `Adafruit MPU6050`.  
     - `Adafruit Unified Sensor`.  

   > ⚠️ Nếu dùng ESP32, cần chọn đúng cổng COM và board trong **Tools → Board**.  

## 📌 3. Nạp chương trình
1. Mở file `robot.ino` bằng Arduino IDE.  
2. Chọn đúng **Board** (ESP32 Dev Module hoặc Arduino Uno).  
3. Chọn đúng **Port** (COMx).  
4. Bấm **Upload** (Ctrl + U).  
5. Chờ chương trình được nạp vào robot.  

## 📌 4. Kết nối phần cứng
- **MPU6050** → ESP32/Arduino:  
  - VCC → 3.3V (ESP32) hoặc 5V (Arduino).  
  - GND → GND.  
  - SDA → GPIO21 (ESP32) hoặc A4 (Arduino).  
  - SCL → GPIO22 (ESP32) hoặc A5 (Arduino).  

- **Driver động cơ** (L298N/TB6612FNG) → ESP32/Arduino:  
  - IN1, IN2, IN3, IN4 → các chân điều khiển trong code (`#define`).  
  - ENA, ENB → PWM output.  
  - Motor A/B → 2 động cơ DC.  
  - VCC (7V–12V) → Pin.  
  - GND → chung với ESP32/Arduino.  

## 📌 5. Hiệu chỉnh
- Khi khởi động, robot sẽ **lấy giá trị cảm biến ban đầu làm mốc cân bằng**.  
- Đặt robot trên mặt phẳng trước khi bật nguồn.  
- Nếu robot bị ngả nhiều:  
  - Chỉnh lại **hệ số PID trong code** (`Kp`, `Ki`, `Kd`).  
  - Kiểm tra lại hướng đấu dây động cơ.  

## 📌 6. Chạy thử
- Sau khi upload, đặt robot đứng thẳng.  
- Bật nguồn động cơ.  
- Robot sẽ cố gắng tự cân bằng.  

## 📌 7. Gỡ lỗi
- Nếu cảm biến không nhận: kiểm tra SDA/SCL và điện áp cấp nguồn.  
- Nếu động cơ không chạy: kiểm tra chân PWM và dây nối với driver.  
- Nếu robot bị ngả liên tục: cần **tinh chỉnh PID** trong code.  
