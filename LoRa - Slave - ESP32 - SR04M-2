#include <DHT.h>
#include <DHT_U.h>
#include <SoftwareSerial.h>

#define DHTPIN 5
#define DHTTYPE 11
#define SLAVE_ID 2  // ID của thiết bị Slave

DHT dht(DHTPIN, DHTTYPE);

SoftwareSerial lora(13, 15);  // RX, TX

struct lora_request_t {
  uint8_t id;
  char message[32];
};

struct lora_response_t {
  uint8_t id;
  float temperature;
  float humidity;
};

lora_request_t lora_request;
lora_response_t lora_response;

void setup() {
  Serial.begin(9600);
  lora.begin(9600);
  dht.begin();
}

void loop() {
  // Đọc nhiệt độ và độ ẩm từ cảm biến DHT
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Kiểm tra nếu DHT không đọc được giá trị hợp lệ
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    delay(5000);
    return; // Thoát khỏi vòng lặp và thử lại sau 5 giây
  }

  // Gửi dữ liệu nhiệt độ và độ ẩm về LoRa Master nếu có yêu cầu
  if (lora.available()) {
    lora.readBytes((uint8_t*)&lora_request, sizeof(lora_request_t));

    // Kiểm tra xem yêu cầu có dành cho Slave này không
    if (lora_request.id == SLAVE_ID) {
      Serial.print("Request received from Master: ");
      Serial.println(lora_request.message);

      // Chuẩn bị dữ liệu phản hồi
      lora_response.id = SLAVE_ID;
      lora_response.temperature = temperature;
      lora_response.humidity = humidity;

      // Gửi phản hồi tới LoRa Master
      lora.write((uint8_t*)&lora_response, sizeof(lora_response_t));
    }
  }
}
