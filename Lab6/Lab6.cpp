#include <WiFi.h>
#include <SPI.h>
#include <DHT.h>


#define DHTPIN 23
#define DHTTYPE DHT11


TaskHandle_t ReadData;
TaskHandle_t FormatData;


const int ledPin = 5;
const char ssid[] = "maszt_sygnalowy";


float temperature, humidity;
WiFiServer server(21);
char httpMSG[256];
char header[] = "HTTP/1.1 200 OK\nContent-type:text/html\nConnection: close\n";


DHT dht(DHTPIN, DHTTYPE);
;
void fUpdateData(void* params);
void fFormatData(void* params);


void setup() {
  Serial.begin(115200);
  Serial.printf("\nConnecting to WiFi...\n");
  dht.begin();
  // Connect to WiFi
  IPAddress ip(192, 168, 1, 22);
  WiFi.config(ip);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid);
  Serial.printf("\nConnecting to WiFi...\n");


  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(100);
  }


  Serial.printf("Connection successful\n");
  Serial.print("Connected to WiFi. My address: ");
  Serial.println(WiFi.localIP());


  server.begin();
  Serial.print(httpMSG);
  // Create tasks
  xTaskCreatePinnedToCore(fUpdateData, "Data Update", 2048, NULL, 1, &ReadData, 0);
  xTaskCreatePinnedToCore(fFormatData, "String Update", 2048, NULL, 1, &FormatData, 1);
}


void loop() {  // Listen for incoming clients WiFiClient client = server.available();
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client");
      // An HTTP request ends with a blank line
      bool currentLineIsBlank = true;


    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.println(httpMSG);


        client.print(httpMSG);
      }
    }
    delay(15);
    client.stop();
    Serial.println("Client disconnected");
  }
}


void fUpdateData(void* params) {
  while (true) {
    float prevHumidity = humidity;
    float prevTemperature = temperature;
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      temperature = prevTemperature;
      humidity = prevHumidity;
    }                                       // Random humidity between 0.0 and 100.0
    vTaskDelay(2000 / portTICK_PERIOD_MS);  // Delay for 1 second
  }
}


void fFormatData(void* params) {
  while (true) {
    // Format the data into the HTTP message
    sprintf(httpMSG, "%s<!DOCTYPE html>\n<html>\n<head>\n<title>ESP32 Web Server</title>\n</head>\n<body>\nTemperature today is: %.1f °C, and the humidity in the air is: %.1f%%</body>\n</html>\n", header, temperature, humidity);
    //Serial.print(httpMSG);
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay for 1 second
  }
}



