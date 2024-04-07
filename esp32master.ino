#include <esp_now.h>
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xD8, 0xBC, 0x38, 0xF9, 0x1E, 0x0C};

const int buttonIN = 2;
const int alert_led = 4;
const int okay_led = 5;
const int thermalProbe = 23;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

float thermalRead;
bool recordGPS;

#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES 10  // Number of snowflakes in the animation example

#define LOGO_HEIGHT 16
#define LOGO_WIDTH 16
                                                  
static const u_int8_t emberalertlogo[1024] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x03, 0x1f, 0xfc, 0xc0, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x0e, 0x1f, 0xfc, 0xe0, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x1e, 0x3f, 0xfc, 0x70, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x3c, 0x7f, 0xfe, 0x78, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x7f, 0xfe, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0xf8, 0xff, 0xfe, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x1c, 0x01, 0xf8, 0xff, 0xfe, 0x7e, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x1e, 0x01, 0xf8, 0xff, 0xfc, 0x7f, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x0f, 0x83, 0xf8, 0xff, 0xfc, 0x7f, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x03, 0x83, 0xf8, 0xff, 0xfc, 0x7f, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf8, 0xff, 0xf8, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf8, 0xff, 0xf8, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xfc, 0x7f, 0xf1, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0xc0, 0x07, 0xfe, 0x3f, 0xe3, 0xff, 0xc0, 0x0e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0xf8, 0x07, 0xfe, 0x0f, 0xc7, 0xff, 0xc0, 0x7f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xf8, 0x07, 0xff, 0x83, 0x0f, 0xff, 0xc0, 0x7c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x18, 0x07, 0xff, 0xc0, 0x1f, 0xff, 0xc0, 0x60, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xf8, 0x3f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xf8, 0x7f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0xff, 0xf1, 0xff, 0xff, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0xff, 0xe3, 0xff, 0xff, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x30, 0x63, 0xff, 0xc7, 0xff, 0xff, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x38, 0x71, 0xff, 0xcf, 0xff, 0xfe, 0x38, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x30, 0x70, 0xff, 0x9f, 0xff, 0xfc, 0x38, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x7f, 0x9f, 0xff, 0xf8, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x1f, 0x3f, 0xff, 0xe0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x07, 0x3f, 0xff, 0x81, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x10, 0x7f, 0x87, 0x7f, 0xff, 0x8f, 0xf8, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x38, 0x7f, 0xe7, 0x7f, 0xff, 0x8f, 0xf8, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x30, 0x7f, 0xe7, 0x7f, 0xff, 0x8f, 0xf8, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe3, 0x7f, 0xff, 0x9f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe3, 0x7e, 0xff, 0x9f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe3, 0xfc, 0x7f, 0x1f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x38, 0x1f, 0xe3, 0xfc, 0x3f, 0x1f, 0xe0, 0x78, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0xf8, 0x1f, 0xf1, 0xf8, 0x3e, 0x3f, 0xe0, 0x7e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0xf0, 0x0f, 0xf0, 0xf0, 0x3c, 0x3f, 0xc0, 0x3f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0xc0, 0x07, 0xf8, 0x70, 0x1c, 0x7f, 0xc0, 0x06, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xfc, 0x70, 0x18, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfe, 0x30, 0x11, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x07, 0x81, 0xff, 0x00, 0x03, 0xfe, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0xff, 0x80, 0x07, 0xfc, 0x03, 0xe0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x7f, 0xc0, 0x0f, 0xf8, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x0c, 0x30, 0x1f, 0xe0, 0x1f, 0xe0, 0x10, 0x40, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x0f, 0xe0, 0x1f, 0xc0, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x01, 0xf0, 0x3f, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x38, 0x70, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

bool dataSent = false;

unsigned int readingId = 0;

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  float thermalTemp;
  bool recordGPS;
} struct_message;

// Create a struct_message called myData
struct_message myData;

OneWire oneWire(thermalProbe);
DallasTemperature sensors(&oneWire);

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void IRAM_ATTR isr(){
  if(!dataSent){
    // Send message via ESP-NOW
    myData.thermalTemp = thermalRead;
    myData.recordGPS = true;
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    dataSent = true;
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.drawBitmap(0, 0, emberalertlogo, 128, 64, 1);
  display.display();
  delay(2000);  // Pause for 2 seconds
  display.clearDisplay();

  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);  // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 40);
  display.println(F("Initializing"));
  display.setCursor(90, 40);
  display.println(F("..."));
  display.display();  // Show initial text
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Transmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  pinMode(buttonIN, INPUT);
  pinMode(alert_led, OUTPUT);
  pinMode(okay_led, OUTPUT);
  pinMode(thermalProbe, INPUT);

  attachInterrupt(buttonIN, isr, RISING);

  sensors.begin();

  display.clearDisplay();
  display.setCursor(10, 40);
  display.println(F("INITIALIZED!"));
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setCursor(20, 20);
  display.println(F("Live Temp"));
  display.display();
}
 
void loop() {
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures(); 
  thermalRead = sensors.getTempCByIndex(0);
  //Serial.print("The probe is reading ");
  Serial.println(thermalRead);

  // Set values to send
  myData.thermalTemp = thermalRead;
  myData.recordGPS = false;
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  if(thermalRead > 26.0){
    digitalWrite(alert_led, HIGH);
    digitalWrite(okay_led, LOW);
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 40);
    display.println(F("ALERT!"));
    display.display();
    delay(500);
  } else {
    digitalWrite(alert_led, LOW);
    digitalWrite(okay_led, HIGH);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(20, 15);
  display.println(F("Live Temp"));
  display.drawFastHLine(0, 25, 128, SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(5, 60);
  display.print(thermalRead);
  display.print(" ");
  display.setTextSize(1);
  display.print("C");
  display.display();
  delay(250);

  if(dataSent == true){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(18, 40);
    display.println(F("Data Sent!"));
    display.display();
    delay(500);
    dataSent = false;
  }
}