// Servo sweep on GPIO 13 using the ESP32 LEDC peripheral (no library needed, core 3.x API).
const int SERVO_PIN   = 13;
const int PWM_FREQ    = 50;     // standard 20 ms servo frame
const int PWM_RES     = 16;     // 65535 steps per frame
const int MIN_US      = 500;    // pulse width at 0 deg
const int MAX_US      = 2400;   // pulse width at 180 deg
const int STEP_DEG    = 2;
const int STEP_MS     = 15;

void writeAngle(int deg) {
  int us = map(deg, 0, 180, MIN_US, MAX_US);
  uint32_t duty = (uint32_t)us * 65535UL / 20000UL;
  ledcWrite(SERVO_PIN, duty);
}

void setup() {
  Serial.begin(115200);
  ledcAttach(SERVO_PIN, PWM_FREQ, PWM_RES);
  writeAngle(90);
  delay(500);
  Serial.println("Servo sweep on GPIO13");
}

void loop() {
  for (int a = 0; a <= 180; a += STEP_DEG) { writeAngle(a); delay(STEP_MS); }
  for (int a = 180; a >= 0; a -= STEP_DEG) { writeAngle(a); delay(STEP_MS); }
}
