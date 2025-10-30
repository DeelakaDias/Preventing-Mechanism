#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

#define SSID "Mr.Wifi"
#define password "LabanaBawe#@12"

// #define SSID "Dyzenn"
// #define password "Kokila75"

// LIS3DHTR I2C address
#define LIS3DHTR_ADDR 0x19  

// LIS3DHTR registers
#define LIS3DHTR_WHO_AM_I 0x0F
#define LIS3DHTR_CTRL_REG1 0x20
#define LIS3DHTR_CTRL_REG4 0x23
#define LIS3DHTR_OUT_X_L 0x28

// Global scale factor (g per LSB)
float SCALE = 0.000061f; // default for ±2g (61 µg/LSB)

// --- Function to set range (2, 4, 8, or 16g) ---
void setRange(uint8_t range_g)
{
  uint8_t reg_value = 0x00;

  switch (range_g)
  {
  case 2:
    reg_value = 0x00; // ±2g
    SCALE = 0.000061f;
    break;
  case 4:
    reg_value = 0x10; // ±4g
    SCALE = 0.000122f;
    break;
  case 8:
    reg_value = 0x20; // ±8g
    SCALE = 0.000244f;
    break;
  case 16:
    reg_value = 0x30; // ±16g
    SCALE = 0.000732f;
    break;
  default:
    Serial.println("Invalid range! Defaulting to ±2g.");
    reg_value = 0x00;
    SCALE = 0.000061f;
    break;
  }

  // Write range setting to CTRL_REG4
  Wire.beginTransmission(LIS3DHTR_ADDR);
  Wire.write(LIS3DHTR_CTRL_REG4);
  Wire.write(reg_value);
  Wire.endTransmission();

  Serial.print("Range set to ±");
  Serial.print(range_g);
  Serial.println("g");
}

void setup()
{
  WiFi.begin(SSID, password);
  Serial.begin(115200);
  Wire.begin(); // default SDA=21, SCL=22 for ESP32-S3, change if needed
  pinMode(38, OUTPUT); // Pin to indicate measurement start
  delay(1000);

  // Initialize LIS3DHTR
  Wire.beginTransmission(LIS3DHTR_ADDR);
  Wire.write(LIS3DHTR_CTRL_REG1);
  Wire.write(0x57); // 100Hz, all axes enabled
  Wire.endTransmission();

  // Set range (you can change this: 2, 4, 8, or 16)
  setRange(2);

  Serial.println("LIS3DHTR Initialized!");

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("✅ Connected to WiFi!");
  digitalWrite(38, HIGH);
  delay(100);
  digitalWrite(38, LOW);
  delay(100);
  digitalWrite(38, HIGH);
  delay(100);
  digitalWrite(38, LOW);
  delay(100);
  digitalWrite(38, HIGH);
  delay(100);
  digitalWrite(38, LOW);

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // Prints the device’s IP

}

void loop()
{
  int16_t x, y, z;

  Wire.beginTransmission(LIS3DHTR_ADDR);
  Wire.write(LIS3DHTR_OUT_X_L | 0x80); // auto-increment
  Wire.endTransmission(false);

  Wire.requestFrom(LIS3DHTR_ADDR, 6);
  if (Wire.available() == 6)
  {
    x = Wire.read() | (Wire.read() << 8);
    y = Wire.read() | (Wire.read() << 8);
    z = Wire.read() | (Wire.read() << 8);
  }
  // Convert to g
  float x_g = x * SCALE;
  float y_g = y * SCALE;
  float z_g = z * SCALE;
  float vibration = fabs(sqrt(x_g * x_g + y_g * y_g + z_g * z_g) - 1.0);

  // Print
  char buffer[256];
  sprintf(buffer, "X: %+07.3f g  Y: %+07.3f g  Z: %+07.3f g | Vibration: %07.3f g", x_g, y_g, z_g, vibration);
  if (vibration > 0.1f)
  {
    Serial.println("High vibration detected!");
    digitalWrite(38, HIGH); // Set pin 38 high if vibration > 1g
  }
  else
  {
    digitalWrite(38, LOW); // Set pin 38 low if vibration <= 1g
  }

  Serial.println(buffer);
  delay(50);
}