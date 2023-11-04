#include <Arduino.h>

// Định nghĩa các trig và echo
const int TRIG_PIN = 8;
const int ECHO_PIN = 7;

void setup() {
  Serial.begin(9600);
}

// Định nghĩa hàm để đo khoảng cách đến một vật thể
int measure_distance() {
  // Gửi xung đến chân kích hoạt
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Chờ xung hồi âm
  unsigned long start_time = micros();
  while (!digitalRead(ECHO_PIN)) {
  }
  unsigned long end_time = micros();

  // Tính toán khoảng cách
  return pulseIn(ECHO_PIN, HIGH) * 0.0343 / 2;
}

// Định nghĩa hàm để lấy nhiều lần đo và lấy trung bình của chúng
int take_average_measurement(int num_samples) {
  int distances[num_samples];
  for (int i = 0; i < num_samples; i++) {
    distances[i] = measure_distance();
  }

  int average = 0;
  for (int i = 0; i < num_samples; i++) {
    average += distances[i];
  }

  return average / num_samples;
}

// Chỉnh lại cảm biến bằng một khoảng cách đã biết
void calibrate_sensor(int known_distance) {
  int measured_distance = take_average_measurement(10);

  float calibration_factor = known_distance / measured_distance;

  Serial.print("Calibration factor: ");
  Serial.println(calibration_factor);
}

// Định nghĩa hàm bù nhiệt độ
int temperature_compensate(int distance, float temperature) {
  float speed_of_sound = 331.3 + 0.6 * temperature * (1 + 0.00367 * temperature);

  int compensated_distance = distance * 331.3 / speed_of_sound;

  return compensated_distance;
}

// Bắt đầu vòng lặp đo
void loop() {
  // Chỉnh lại cảm biến
  calibrate_sensor(30);

  // Bắt đầu vòng lặp đo
  while (true) {
    // Lấy một lần đo trung bình
    int measured_distance = take_average_measurement(10);

    // Bù nhiệt độ cho phép đo
    float temperature = 25;
    int compensated_distance = temperature_compensate(measured_distance, temperature);

    // In phép đo đã bù
    Serial.print("Distance: ");
    Serial.println(compensated_distance);

    // Chờ 1 giây
    delay(1000);
  }
}
