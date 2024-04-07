#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <ESPAsyncWebSrv.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <TAMC_GT911.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include "html.h"

//Define pins and variables for touchscreen
#define TOUCH_SDA  33
#define TOUCH_SCL  32
#define TOUCH_INT 21
#define TOUCH_RST 25
#define TOUCH_WIDTH  320
#define TOUCH_HEIGHT 480

#define TFT_EMBERRED 780116;

#define DELAY 1000
#define WIDTH  320
#define HEIGHT 128

TAMC_GT911 tp = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, TOUCH_WIDTH, TOUCH_HEIGHT);

// Define display size  
#define TFT_WIDTH 320
#define TFT_HEIGHT 480

// Define color constants
#define BLACK  0x000000
#define WHITE  0xFFFFFF
#define ember_red 0x5800
#define AA_FONT_LARGE "NotoSansBold12"
unsigned long targetTime = 0;
byte red = 31;
byte green = 0;
byte blue = 0;
byte state = 0;
unsigned int colour = red << 11;

int previousTemp = 0;
int currentPage = 0; // Current page being displayed (0, 1, or 2)
bool movepageright = false, movepageleft = false, reloadpage = false;

// Define page indicator position
#define PAGE_INDICATOR_X 10
#define PAGE_INDICATOR_Y 10

TFT_eSPI tft = TFT_eSPI(); // Create an instance of the TFT display
TFT_eSprite spr = TFT_eSprite(&tft);  // Declare Sprite object "spr" with pointer to "tft" object
TFT_eSprite data = TFT_eSprite(&tft); // Declare Sprite object "data" with pointer to "tft" object

//ESP32 Wifi access
const char *ssid = "EmberAlert";
const char *password = "12345";

//GPS Module Pins and setup
static const int RXPin = 1, TXPin = 3;
static const uint32_t GPSBaud = 9600;
static const uint32_t UTC_offset = -8;
String coordinates;

//GPS init
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

//Website init
JsonDocument doc;

AsyncWebServer server(80);
AsyncEventSource events("/events");

String header;

//ESPnow message structure
typedef struct struct_message {
  float thermalTemp;
  bool recordGPS;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  char macStr[18];
  //Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  //Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));

  doc["temp"] = myData.thermalTemp;
  doc["recordData"] = String(myData.recordGPS);
  doc["lat"] = String(gps.location.lat(), 6);
  doc["lng"] = String(gps.location.lng(), 6);
  doc["coords"] = String(gps.location.lat(), 6) + String(gps.location.lng(), 6);
  char buffer[256];
  serializeJson(doc, buffer);
  events.send(buffer, "new_readings", millis());
}

// Function to draw temperature data for page 1 
void displayTemperaturePage1() {
  previousTemp = myData.thermalTemp;
  spr.setTextDatum(MC_DATUM);
  data.setTextDatum(MC_DATUM);
  spr.drawString("Page 1 - LIVE TEMP", WIDTH / 2, HEIGHT / 2, 4);
  data.print(myData.thermalTemp, 6);
  spr.pushSprite(tft.width() / 2 - WIDTH / 2, HEIGHT - (tft.height() / 2), TFT_BLACK);
  data.pushSprite(tft.width() / 2 - WIDTH / 2, HEIGHT / 2, TFT_BLACK);
}

//Function to display GPS data for page 2
void displayTemperaturePage2() {
  spr.setTextDatum(MC_DATUM);
  spr.fillSprite(ember_red);
  spr.drawString("Page 2 - LIVE GPS", WIDTH / 2, HEIGHT / 2, 4);
  spr.setCursor(WIDTH / 2 - 10, (HEIGHT / 2) + 20);
  spr.print(gps.location.lat(), 6);
  spr.setCursor(WIDTH / 2 - 10, (HEIGHT / 2) + 40);
  spr.print(gps.location.lng(), 6);
  spr.pushSprite(tft.width() / 2 - WIDTH / 2, tft.height() / 2 - HEIGHT / 2, TFT_BLACK);
}

//Function to display recorded data
void displayTemperaturePage3() {
  spr.setTextDatum(MC_DATUM);
  spr.fillSprite(ember_red);
  spr.pushSprite(tft.width() / 2 - WIDTH / 2, tft.height() / 2 - HEIGHT / 2, TFT_BLACK);
  spr.fillSprite(ember_red);
  spr.drawString("Page 3 - RECORDED DATA", WIDTH / 2, HEIGHT / 2, 4);
  spr.drawString("3 °C", WIDTH / 2, (HEIGHT / 2) + 20, 4);
  spr.pushSprite(tft.width() / 2 - WIDTH / 2, tft.height() / 2 - HEIGHT / 2, TFT_BLACK);
}

// Function to draw a page indicator (3 dots representing 3 pages)
void drawPageIndicator(int currentPage) {
  tft.fillCircle(PAGE_INDICATOR_X, PAGE_INDICATOR_Y, 5, (currentPage == 0) ? WHITE : BLACK);
  tft.fillCircle(PAGE_INDICATOR_X + 15, PAGE_INDICATOR_Y, 5, (currentPage == 1) ? WHITE : BLACK);
  tft.fillCircle(PAGE_INDICATOR_X + 30, PAGE_INDICATOR_Y, 5, (currentPage == 2) ? WHITE : BLACK);
}

// Functions to check for left and right arrow press events (replace with your touch library code)
bool checkLeftArrowPress() {
  tp.read();
  if (tp.isTouched){
    for (int i=0; i<tp.touches; i++){
      if(tp.points[i].y < 250){
        movepageleft = true;
      } else {
        movepageleft = false;
      }
    }
  }
  if(!tp.isTouched){
    return movepageleft;
  } else {
    return false;
  }
}

bool checkRightArrowPress() {
  tp.read();
  if (tp.isTouched){
    for (int i=0; i<tp.touches; i++){
      if(tp.points[i].y > 250){
        movepageright = true;
      } else {
        movepageright = false;
      }
    }
  }
  if(!tp.isTouched){
    return movepageright;
  } else {
    return false;
  }
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);
  ss.begin(GPSBaud);

  //touchscreen and sprite init
  tp.begin();
  tp.setRotation(ROTATION_NORMAL);
  tft.init(); // Initialize the TFT display
  tft.setRotation(1); // Set rotation (adjust if needed)
  spr.setSwapBytes(true);
  data.setSwapBytes(false);
  tft.setSwapBytes(true);
  tft.fillScreen(ember_red); 
  
  targetTime = millis() + 100;
  spr.createSprite(WIDTH, HEIGHT);
  data.createSprite(WIDTH, HEIGHT);
  delay(DELAY);

  spr.drawRect(0, 0, WIDTH, HEIGHT, TFT_BLACK);
  data.drawRect(0, 0, WIDTH, HEIGHT, TFT_BLACK);

  uint32_t updateTime = 0;

  // Draw initial page indicator dots
  drawPageIndicator(currentPage);

  //Serial.print("Setting AP (Access Point)…");
  WiFi.mode(WIFI_STA);
  WiFi.softAP(ssid);

  IPAddress IP = WiFi.softAPIP();

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    delay(100);
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  events.onConnect([](AsyncEventSourceClient *client) {
    if (client->lastId()) {
    }
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin();
}

void loop() {
  //Draw current page on display
  drawPageIndicator(currentPage);
  if (currentPage == 0) {
    displayTemperaturePage1();
  } else if (currentPage == 1) {
    displayTemperaturePage2();
  } else if (currentPage == 2) {
    displayTemperaturePage3();
  }

  // Check for touch events (replace with your touch library code)
  delay(20);
  if (checkLeftArrowPress()) {
    movepageleft = false;
    currentPage++;
    if (currentPage > 2) {
      currentPage = 0; 
    }
  } 
  delay(20);
  if (checkRightArrowPress()) {
    movepageright = false;
    currentPage--;
    if (currentPage < 0) {
      currentPage = 2; 
    }
  }

  //Send webapp live data from ESPnow 
  static unsigned long lastEventTime = millis();
  static const unsigned long EVENT_INTERVAL_MS = 5000;
  if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
    events.send("ping", NULL, millis());
    lastEventTime = millis();
  }

  //Update GPS data
  while (ss.available() > 0){
    gps.encode(ss.read());
    //coordinates = std::to_string(gps.location.lat(), 6) + std::to_string(gps.location.lng(), 6);
  }
}