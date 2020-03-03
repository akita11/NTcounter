// https://trac.switch-science.com/wiki/esp32_tips

#include <M5Stack.h>
#include <WiFi.h>
#include "time.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>

#define DEV_NAME "NTcounter3"
const char* ssid       = "****";
const char* password   = "****";

#define PIN_IR 15
#define PIN_BL 32
#define LOG_FILE "/log.csv"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

void LCDenable(uint8_t f){
  if (f == 1){ M5.Lcd.wakeup(); M5.Lcd.setBrightness(200); }
  else{ M5.Lcd.sleep(); M5.Lcd.setBrightness(0);}
}

uint16_t cnt = 0;
uint16_t y = 0;

uint8_t getBattery()
{
// https://moosoft.jp/images/moosoft/m5stack/BatteryMeter.ino
  uint8_t vat;
  Wire.beginTransmission(0x75);
  Wire.write(0x78);
  Wire.endTransmission(false);
  if (Wire.requestFrom(0x75, 1)) {
    vat = Wire.read() & 0xF0;
    if      (vat == 0xF0) vat = 0;
    else if (vat == 0xE0) vat = 25;
    else if (vat == 0xC0) vat = 50;
    else if (vat == 0x80) vat = 75;
    else if (vat == 0x00) vat = 100;
    else                  vat = 0;
  } else vat = 0;
  return(vat);
}

void InitScreen(){
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("NTcounterRX v2.0, ");
  M5.Lcd.print(DEV_NAME);
  M5.Lcd.setTextColor(WHITE);
}

void ShowData(uint8_t f){
  struct tm tmi;
  if (!getLocalTime(&tmi)) Serial.println("Failed to obtain time");
  //(tmstruct.tm_year)+1900,( tmstruct.tm_mon)+1, tmstruct.tm_mday,tmstruct.tm_hour , tmstruct.tm_min, tmstruct.tm_sec)

  uint16_t year = tmi.tm_year + 1900;
  uint8_t mon = tmi.tm_mon + 1;
  uint8_t d = tmi.tm_mday;
  uint8_t h = tmi.tm_hour;
  uint8_t m = tmi.tm_min;
  uint8_t s = tmi.tm_sec;
  
  y = y + 10;
  if (y >= 230){
    InitScreen();
    y = 10;
  }

  uint8_t bat = getBattery();
  M5.Lcd.setCursor(0, y);
//  Serial.println(&tmi, "%A, %B %d %Y %H:%M:%S");
  Serial.print(year); Serial.print(',');
  Serial.print(mon); Serial.print(',');
  Serial.print(d); Serial.print(',');
  Serial.print(h); Serial.print(',');
  Serial.print(m); Serial.print(',');
  Serial.print(s); Serial.print(',');
  Serial.print(bat); Serial.print(',');
  Serial.println(cnt);
  M5.Lcd.printf("%d,%02d,%02d,%02d,%02d,%02d,%d,%d", year, mon, d, h, m, s, bat, cnt);
  if (f == 1){
    File file = SD.open(LOG_FILE, FILE_APPEND);
    if(!file) M5.Lcd.print("Failed to open file for appending");
    else{
      file.printf("%d,%02d,%02d,%02d,%02d,%02d,%d,%d\n", year, mon, d, h, m, s, bat, cnt);
      file.close();
    }
  }
}

uint8_t fMode = 0; // 0=test, 1=LCD operation
uint8_t fObstacle = 0;
uint8_t cc = 0;

// ~~__~~~~~~~~~~~~~~~~~~~__~~~~
//   <><----------------->
//  0.6ms   46ms

// T=duration from sleep
//  T < 50ms : illuminatated, no obstacle
//  T > 500ms : interrupted, obstacle

void toggle_mode(){
  if (fMode == 0){
    // go to operation mode
    fMode = 1; LCDenable(0);
  }
  else if (fMode == 1){
    // go to test mode
    fMode = 0; LCDenable(1);
    ShowData(0);
  }
  else fMode = 0;
  Serial.print("mode="); Serial.println(fMode);  
}


void setup()
{
  Serial.begin(115200);
  M5.begin();

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(BLUE);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("connecting WiFi...");

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("CONNECTED");

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  InitScreen();
  ShowData(0);

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  pinMode(PIN_IR, INPUT);

  M5.Power.begin();
//  M5.Power.setWakeupButton(BUTTON_A_PIN);
  M5.Power.setWakeupButton(BUTTON_B_PIN);
//  M5.Power.setWakeupButton(BUTTON_C_PIN);
//  M5.Power.setWakeupButton(PIN_IR);

//http://blog.livedoor.jp/sce_info3-craft/archives/9821232.html
  esp_sleep_enable_ext1_wakeup(BIT64(GPIO_NUM_15), ESP_EXT1_WAKEUP_ALL_LOW);

// https://github.com/espressif/esp-idf/blob/master/components/esp32/sleep_modes.c?fbclid=IwAR33dA5MEB7u818hp0jv_voHPnArxMHoIaSFIRCy0vvUpyfiOYb5scb4Wu0
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);

  M5.Power.isResetbyPowerSW();
  M5.Power.isResetbyDeepsleep();
  if (!M5.Power.canControl()) { M5.Lcd.printf("IP5306 is not i2c version\n"); }

  LCDenable(1); fMode = 0;
  if (!SD.begin()) M5.Lcd.printf("Card Mount Failed");
  File file = SD.open(LOG_FILE, FILE_APPEND);
  if(!file) M5.Lcd.print("Failed to open file for appending");
  else{
    file.printf("%s\n", DEV_NAME);
    file.close();
  }
}

uint32_t timeout = 1000000;

uint16_t lp = 0;

void loop()
{
  M5.update();

  uint32_t tts, tte;
  uint16_t ts, te, t;

  if (fMode == 0){
//    if (M5.BtnA.wasPressed()) toggle_mode();
    if (M5.BtnB.wasPressed()) toggle_mode();
//    if (M5.BtnC.wasPressed()) toggle_mode();
    // test mode
    tts = 0;
    while (digitalRead(PIN_IR) == 0 && tts < timeout) tts++; // wait for Ir goes 1
    ts = millis();
    tte = 0;
    while (digitalRead(PIN_IR) == 1 && tte < timeout) tte++; // wait for Ir goes 0
    te = millis();
//  Serial.print(ts); Serial.print(' '); Serial.print(te); Serial.print(' '); Serial.print(tts); Serial.print(' '); Serial.println(tte);
    if (tts < timeout && tte < timeout){
      if (te > ts){
        t = te - ts;
//      Serial.print("t="); Serial.println(t);
        if (t < 100){ // # illuminated
          if (fMode == 0) M5.Lcd.fillCircle(300, 20, 20, GREEN);
          if (fObstacle == 1){ //obstacle -> no obstacle
            fObstacle = 0;
            cnt = cnt + 1;
            Serial.print("cnt="); Serial.println(cnt);
            ShowData(1);
          }
        }
        else{ // obstacle
          Serial.print("obstacle detected");
          fObstacle = 1;
        }
      }
    }
    else{
      M5.Lcd.fillCircle(300, 20, 20, RED);
      fObstacle = 1;
      tts = 0;
      while (tts < 10000) tts += 1;
      lp++; if (lp == 100) {lp = 0; ShowData(0);}
    }
  }
  else{
    // operation mode
    ts = millis();
//    Serial.println("go to sleep");
    M5.Power.lightSleep(0);
    te = millis();
    if (M5.BtnB.wasPressed()){
      toggle_mode();
    }
    else if (te > ts){
      t = te - ts;
//    Serial.print(ts); Serial.print(' '); Serial.print(te); Serial.print(' '); Serial.println(t);
      if (t < 100){ // # illuminated
        lp++;
        // start: 2020/2/6 11:30
//        if (lp == 20){          
        if (lp == 20 * 60 * 5){
          Serial.println("!!!");
          lp = 0;
          ShowData(1);
        }
        if (fObstacle == 1){ //obstacle -> no obstacle
          fObstacle = 0;
          cnt = cnt + 1;
//          Serial.print("cnt="); Serial.println(cnt);
          ShowData(1);
        }
      }
      else{ // obstacle
//        Serial.print("obstacle detected");
        fObstacle = 1;
      }
    }
  }
}

