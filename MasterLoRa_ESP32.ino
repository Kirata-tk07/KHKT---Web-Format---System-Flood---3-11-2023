#include <ThingSpeak.h>
#include <WiFi.h>
#include <HardwareSerial.h>  // Vì đây là esp32 (tức không thể dùng thư viện SoftwareSerial) nên tôi đổi sang sài <HardwareSerial.h>
#include <Wire.h>

#define CHANNEL_ID 2290721
#define CHANNEL_API "C1ZG9P1H95UC6QYY"
// Định nghĩa số lượng Slaves
#define NUMBER_OF_SLAVES 2

const char* ssid = "HOCSINH";
const char* password = "";

HardwareSerial lora(2);  // RX, TX

WiFiClient client;

unsigned long previousMillis = 0;
const long interval = 20000;

struct lora_request_t {
  uint8_t id;
  char message[32];
};

struct lora_response_t {
  uint8_t id;
  float temperature;  // Thêm biến temperature để lưu trữ nhiệt độ
  int cm;
};

// Lưu trữ dữ liệu từ lora_request_t vào mảng lora_requests[Vị trí của nó trong mảng]
lora_request_t lora_requests[NUMBER_OF_SLAVES];
lora_response_t lora_response;


void setup() {
  Serial.begin(9600);
  lora.begin(9600);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  ThingSpeak.begin(client);  // Move this line to after connecting to WiFi

  for (uint8_t i = 0; i < NUMBER_OF_SLAVES; ++i) {
    lora_requests[i].id = i + 1;
    strcpy(lora_requests[i].message, "Request data from Slave");
  }
}

void loop() {
  unsigned long currentMillis = millis();
  // Gửi yêu cầu tới từng Slave
  for (uint8_t i = 0; i < NUMBER_OF_SLAVES; ++i) {
    lora.write((uint8_t*)&lora_requests[i], sizeof(lora_request_t));  // lora.write((uint8_t*)(lora_requests + i), sizeof(lora_request_t));
    Serial.print("Request sent to Slave ");
    Serial.println(lora_requests[i].id);
    delay(4000);  // Đợi một chút trước khi gửi yêu cầu tiếp theo
  }

  // Đợi nhận phản hồi từ các Slaves
  while (lora.available()) {
    lora_response_t lora_response;  // Khởi tạo biến lora_response với kiểu cấu trúc lora_response_t

    // Đọc dữ liệu từ lora vào biến lora_response
    lora.readBytes((uint8_t*)&lora_response, sizeof(lora_response_t));

    // Hiển thị dữ liệu nhận được lên Serial Monitor
    if (lora_response.id == 1) {
      Serial.print("Response from SlaveID: ");
      Serial.println(lora_response.id);
      Serial.print("Temperature: ");
      Serial.print(lora_response.temperature);
      Serial.println(" °C");
    }
    if (lora_response.id == 2) {
      Serial.print("Response from SlaveID: ");
      Serial.println(lora_response.id);
      Serial.print("Distance: ");
      Serial.print(lora_response.cm);
    }
  }

  ThingSpeak.setField(1, lora_response.temperature);
  ThingSpeak.setField(3, lora_response.cm);



  if (currentMillis - previousMillis >= interval) {

    ThingSpeak.writeFields(CHANNEL_ID, CHANNEL_API);

    previousMillis = currentMillis;
  }

  // Đợi một khoảng thời gian trước khi gửi yêu cầu tiếp theo
}
