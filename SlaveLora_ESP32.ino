#include <DHT.h>
#include <DHT_U.h>
#include <SoftwareSerial.h>
#define SLAVE_ID 2

#define DHTPIN 5
#define DHTTYPE 11

SoftwareSerial lora(13, 15);  // RX, TX

int trigPin = 11;  // Trigger
int echoPin = 12;  // Echo

struct lora_request_t {
  uint8_t id;
  char message[32];
};

struct lora_response_t {
  uint8_t id;
  float temperature;
  float humidity;
  int cm;
};

lora_request_t lora_request;
lora_response_t lora_response;

void setup() {
  Serial.begin(9600);
  lora.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

int measure_distance() {
  // Gửi xung đến chân kích hoạt
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  long duration = pulseIn(echoPin, HIGH);

  // Chuyển đổi thời gian thành khoảng cách
  float distance = duration / 2 / 29.1;

  return distance;
}
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

void loop() {
  int distance = measure_distance();
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  calibrate_sensor(25);  // Khoảng cách đã biết trước

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    delay(5000);
    return;  // Thoát khỏi vòng lặp và thử lại sau 5 giây
  }

  if (distance != 0) {
    // Lấy một lần đo trung bình
    int measured_distance = take_average_measurement(10);

    // Bù nhiệt độ cho phép đo
    float temperature = 25;
    int compensated_distance = temperature_compensate(measured_distance, temperature);

    // In phép đo đã bù
    Serial.print("Distance: ");
    Serial.println(compensated_distance);

    // Chờ 1 giây
    delay(2000);
  } else {
    int compensated_distance = 0;
  }


  // Kiểm tra nếu DHT không đọc được giá trị hợp lệ
  if (isnan(temperature) || isnan(humidity)) {
    int tries = 0;
    while (tries < 3) {
      temperature = dht.readTemperature();
      humidity = dht.readHumidity();

      if (!isnan(temperature) && !isnan(humidity)) {
        break;
      }

      tries++;
      delay(5000);
    }

    if (tries == 3) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;  // Thoát khỏi vòng lặp
    }
  }

  bool has_responded = false;

  if (lora.available()) {
    lora.readBytes((uint8_t*)&lora_request, sizeof(lora_request_t));

    // Kiểm tra xem yêu cầu có dành cho Slave này không
    if (lora_request.id == SLAVE_ID) {
      if (!has_responded) {
        Serial.print("Request received from Master: ");
        Serial.println(lora_request.message);

        // Chuẩn bị dữ liệu phản hồi
        lora_response.id = SLAVE_ID;
        lora_response.cm = compensated_distance;
        lora_response.temperature = temperature;
        lora_response.humidity = humidity;

        // Gửi phản hồi tới LoRa Master
        lora.write((uint8_t*)&lora_response, sizeof(lora_response_t));

        has_responded = true;
      }
    }
  }

  // Đặt cờ has_responded về false sau khi gửi phản hồi
  has_responded = false;
}
