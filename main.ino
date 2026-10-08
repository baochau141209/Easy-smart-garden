#define BLYNK_TEMPLATE_ID "TMPL2kpkC6g85"
#define BLYNK_TEMPLATE_NAME "Chouchou V 1.1.0Copy"
#define BLYNK_AUTH_TOKEN "ty48BW3EDE-bvSk4A-Qb3mFU2S0z0M7J"

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Preferences.h>
#include <time.h>

#define SOIL_PIN 1

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0,
  U8X8_PIN_NONE
);

const char* ssid = "Chau";
const char* password = "Mk201060";
const char* NZ_TIMEZONE = "NZST-12NZDT,M9.5.0,M4.1.0/3";
unsigned long lastCheckRefresh = 0;

BlynkTimer blynkTimer;
Preferences preferences;

int soilValue = 0;
int soilLimit = 96;
int humdSet = 50;
int pumpSet = 10;
int wateringHour = 7;
int wateringMinute = 0;
bool editingSetLimit = false;
bool editingSetAuto = false;
int setAutoChoose = 0;
bool timeSetSelecting = false;
int timeSetChoose = 1;
bool setHoursEditing = false;
int setHoursField = 0;
int editWateringHour = 7;
int editWateringMinute = 0;
bool pumpEditing = false;
int pumpEditValue = 10;
// Current time strings used by timeset.h.
char currentTimeText[9] = "-- : --";
char currentAmPmText[3] = "--";

#include "logo.h"
#include "welcome.h"
#include "check.h"
#include "setlimit.h"
#include "setauto.h"
#include "scan.h"
#include "timeset.h"
#include "sethours.h"
#include "setpump.h"

#define OLED_SDA 6
#define OLED_SCL 7
#define RELAY_PIN 10
#define BTN_RIGHT 5
#define BTN_LEFT 20
#define BTN_OK 21
#define BTN_UP 4
#define BTN_DOWN 3
#define BUZZER_PIN 2

#define PAGE_CHECK 0
#define PAGE_TIMESET 1
#define PAGE_SETLIMIT 2
#define PAGE_SETAUTO 3
#define PAGE_SCAN 4
#define PAGE_SETHOURS 5
#define PAGE_SETPUMP 6

int currentPage = PAGE_CHECK;
unsigned long lastButtonTime = 0;
const unsigned long RETURN_TIME = 120000UL;

void resetButtonTimer() {
  lastButtonTime = millis();
}

#define SOIL_DRY_VALUE 3000
#define SOIL_WET_VALUE 1200

bool buzzerActive = false;
bool buzzerState = false;
unsigned long buzzerStartTime = 0;
unsigned long lastBuzzerToggle = 0;
const unsigned long BUZZER_TOTAL_TIME = 60000UL;
const unsigned long BUZZER_INTERVAL = 500UL;
bool humidityAlarmTriggered = false;
bool pumpActive = false;
unsigned long pumpStartTime = 0;
unsigned long pumpRunTime = 0;

int soilRawValue = 0;
int lastWaterDate = 0;  
int scheduleCheckDate = 0;
bool wateredToday = false;
bool manualWaterRequest = false;
struct ButtonState {
  uint8_t pin;
  bool lastReading;
  bool stableState;
  unsigned long lastChange;
};

ButtonState btnRightState = { BTN_RIGHT, HIGH, HIGH, 0 };
ButtonState btnLeftState  = { BTN_LEFT,  HIGH, HIGH, 0 };
ButtonState btnOkState    = { BTN_OK,    HIGH, HIGH, 0 };
ButtonState btnUpState    = { BTN_UP,   HIGH, HIGH, 0 };
ButtonState btnDownState  = { BTN_DOWN, HIGH, HIGH, 0 };
const unsigned long BUTTON_DEBOUNCE = 30UL;
bool buttonPressed(
  uint8_t pin,
  bool &lastReading,
  bool &stableState,
  unsigned long &lastChange
) {

  bool reading = digitalRead(pin);
  unsigned long now = millis();

  if (reading != lastReading) {
    lastReading = reading;
    lastChange = now;
  }

  if ((now - lastChange) >= BUTTON_DEBOUNCE &&
      reading != stableState) {

    stableState = reading;

    if (stableState == LOW) {
      return true;
    }
  }

  return false;
}

void loadSettings() {

  preferences.begin("garden", false);
  soilLimit = preferences.getInt("soilLimit", 96);
  humdSet = preferences.getInt("humdSet", 50);
  pumpSet = preferences.getInt("pumpSet", 10);
  wateringHour = preferences.getInt("waterHour", 7);
  wateringMinute = preferences.getInt("waterMin", 0);
  lastWaterDate = preferences.getInt("lastWater", 0);
  soilLimit = constrain(soilLimit, 0, 99);
  humdSet = constrain(humdSet, 0, 99);
  pumpSet = constrain(pumpSet, 0, 99);
  wateringHour = constrain(wateringHour, 0, 23);
  wateringMinute = constrain(wateringMinute, 0, 59);
  editWateringHour = wateringHour;
  editWateringMinute = wateringMinute;
  pumpEditValue = pumpSet;
}

void saveMainSettings() {
  preferences.putInt("soilLimit", soilLimit);
  preferences.putInt("humdSet", humdSet);
  preferences.putInt("pumpSet", pumpSet);
}

void saveWateringSchedule() {
  preferences.putInt("waterHour", wateringHour);
  preferences.putInt("waterMin", wateringMinute);
  preferences.putInt("pumpSet", pumpSet);
}

bool getLocalTimeSafe(struct tm& timeInfo) {
  return getLocalTime(&timeInfo, 50);
}

bool timeIsValid() {
  struct tm timeInfo;
  if (!getLocalTimeSafe(timeInfo)) {
    return false;
  }

  return (timeInfo.tm_year >= (2020 - 1900));
}

int getDateKey(const struct tm& timeInfo) {
  if (timeInfo.tm_year < (2020 - 1900)) {
    return 0;
  }

  return (timeInfo.tm_year + 1900) * 10000 +
         (timeInfo.tm_mon + 1) * 100 +
         timeInfo.tm_mday;
}

void updateCurrentTimeStrings() {
  struct tm timeInfo;

  if (!getLocalTimeSafe(timeInfo) ||
      timeInfo.tm_year < (2020 - 1900)) {

    snprintf(currentTimeText, sizeof(currentTimeText), "-- : --");
    snprintf(currentAmPmText, sizeof(currentAmPmText), "--");
    return;
  }

  snprintf(
    currentTimeText,
    sizeof(currentTimeText),
    "%02d : %02d",
    timeInfo.tm_hour,
    timeInfo.tm_min
  );

  snprintf(
    currentAmPmText,
    sizeof(currentAmPmText),
    "%s",
    (timeInfo.tm_hour < 12) ? "AM" : "PM"
  );
}

String getCurrentDateText() {
  struct tm timeInfo;

  if (!getLocalTimeSafe(timeInfo) ||
      timeInfo.tm_year < (2020 - 1900)) {
    return "NO DATE";
  }

  char buffer[16];
  snprintf(
    buffer,
    sizeof(buffer),
    "%04d-%02d-%02d",
    timeInfo.tm_year + 1900,
    timeInfo.tm_mon + 1,
    timeInfo.tm_mday
  );
  return String(buffer);
}

String getCurrentTimeString() {
  return String(currentTimeText) + " " + String(currentAmPmText);
}

String getScheduleText() {
  char buffer[16];
  snprintf(
    buffer,
    sizeof(buffer),
    "%02d:%02d %s",
    wateringHour,
    wateringMinute,
    (wateringHour < 12) ? "AM" : "PM"
  );
  return String(buffer);
}

void setupNTP() {
  setenv("TZ", NZ_TIMEZONE, 1);
  tzset();
  configTime(
    0,
    0,
    "pool.ntp.org",
    "time.nist.gov"
  );
}

void startWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < 12000UL) {
    delay(100);
  }
}

void maintainConnections() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    return;
  }

  if (!Blynk.connected()) {
    Blynk.connect(1000);
  }
}

int readSoilValue() {
  long total = 0;

  for (int i = 0; i < 10; i++) {
    total += analogRead(SOIL_PIN);
    delay(2);
  }

  soilRawValue = total / 10;

  int value = map(soilRawValue, SOIL_DRY_VALUE, SOIL_WET_VALUE, 0, 99);
  return constrain(value, 0, 99);
}

void startBuzzer() {
  if (buzzerActive) {
    return;
  }

  buzzerActive = true;
  buzzerState = true;
  buzzerStartTime = millis();
  lastBuzzerToggle = millis();
  digitalWrite(BUZZER_PIN, HIGH);
}

void stopBuzzer() {
  buzzerActive = false;
  buzzerState = false;
  digitalWrite(BUZZER_PIN, LOW);
}

void updateBuzzer() {
  if (!buzzerActive) {
    return;
  }

  unsigned long now = millis();

  if (now - buzzerStartTime >= BUZZER_TOTAL_TIME) {
    stopBuzzer();
    return;
  }

  if (now - lastBuzzerToggle >= BUZZER_INTERVAL) {
    lastBuzzerToggle = now;
    buzzerState = !buzzerState;
    digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
  }
}

void updateHumidityAlarm() {
  if (soilValue >= soilLimit) {
    if (!humidityAlarmTriggered) {
      humidityAlarmTriggered = true;
      startBuzzer();
    }
  } 
  else {
    humidityAlarmTriggered = false;
  }
}

bool getTodayDateKey(int& dateKey) {
  struct tm timeInfo;

  if (!getLocalTimeSafe(timeInfo)) {
    return false;
  }

  dateKey = getDateKey(timeInfo);
  return dateKey != 0;
}

void refreshWateredToday() {
  int dateKey;

  if (!getTodayDateKey(dateKey)) {
    return;
  }

  wateredToday = (lastWaterDate == dateKey);
}

void markWateredToday() {
  int dateKey;

  if (!getTodayDateKey(dateKey)) {
    return;
  }

  lastWaterDate = dateKey;
  wateredToday = true;
  preferences.putInt("lastWater", lastWaterDate);
}

bool startPump() {
  if (pumpActive) {
    return false;
  }

  pumpRunTime = (unsigned long)pumpSet * 3000UL;

  if (pumpRunTime == 0) {
    return false;
  }

  pumpStartTime = millis();
  pumpActive = true;
  digitalWrite(RELAY_PIN, HIGH);
  markWateredToday();
  return true;
}

void stopPump() {
  pumpActive = false;
  digitalWrite(RELAY_PIN, LOW);
}

void updatePump() {
  if (!pumpActive) {
    return;
  }

  unsigned long now = millis();

  if (now - pumpStartTime >= pumpRunTime) {
    stopPump();
  }
}

void updateScheduledWatering() {
  if (pumpActive || wateredToday) {
    return;
  }
  struct tm timeInfo;

  if (!getLocalTimeSafe(timeInfo) ||
      timeInfo.tm_year < (2020 - 1900)) {
    return;
  }

  int dateKey = getDateKey(timeInfo);

  if (dateKey == 0) {
    return;
  }

  if (timeInfo.tm_hour != wateringHour ||
      timeInfo.tm_min != wateringMinute) {
    return;
  }

  if (scheduleCheckDate == dateKey) {
    return;
  }

  scheduleCheckDate = dateKey;
  int wateringLimit = humdSet - 1;
  if (wateringLimit < 0) {
    wateringLimit = 0;
  }
  // Dry enough -> water.
  // If not dry enough, do not mark the day as watered.
  // This allows a later Blynk manual watering on the same day.
  if (soilValue < wateringLimit) {
    startPump();
  }
}

void processManualWaterRequest() {
  if (!manualWaterRequest) {
    return;
  }
  manualWaterRequest = false;
  refreshWateredToday();

  if (!timeIsValid()) {
    // Without valid NTP time, the daily lock cannot be guaranteed.
    return;
  }

  if (pumpActive || wateredToday) {
    return;
  }

  startPump();
}

void drawCurrentPage() {
  switch (currentPage) {
    case PAGE_CHECK:
      drawCheck();
      break;

    case PAGE_TIMESET:
      drawTimeset();
      break;

    case PAGE_SETLIMIT:
      drawSetlimit();
      break;

    case PAGE_SETAUTO:
      drawSetauto();
      break;

    case PAGE_SCAN:
      drawScan();
      break;

    case PAGE_SETHOURS:
      drawSethours();
      break;

    case PAGE_SETPUMP:
      drawSetpump();
      break;
  }
}

void resetPageEditingStates() {
  editingSetLimit = false;
  editingSetAuto = false;
  setAutoChoose = 0;
  setHoursEditing = false;
  setHoursField = 0;
  pumpEditing = false;
  manualWaterRequest = false;
}

void goToNormalTimeset() {
  currentPage = PAGE_TIMESET;
  resetPageEditingStates();
  timeSetSelecting = false;
  timeSetChoose = 1;
  updateCurrentTimeStrings();
  drawTimeset();
}
// Used when coming back from SET HOURS / SET PUMP.
// User wants TIMESET to return with frame 1 already selected.
void returnToTimesetSelected() {
  currentPage = PAGE_TIMESET;
  resetPageEditingStates();
  timeSetSelecting = true;
  timeSetChoose = 1;
  updateCurrentTimeStrings();
  drawTimeset();
}

void goToPage(int page) {
  if (currentPage == PAGE_SETLIMIT || currentPage == PAGE_SETAUTO) {
    saveMainSettings();
  }

  currentPage = page;
  resetPageEditingStates();

  if (page == PAGE_TIMESET) {
    timeSetSelecting = false;
    timeSetChoose = 1;
  }

  if (page == PAGE_CHECK) {
    timeSetSelecting = false;
    timeSetChoose = 1;
  }

  drawCurrentPage();
}
// Timeset page
void enterSetHours() {
  currentPage = PAGE_SETHOURS;
  editWateringHour = wateringHour;
  editWateringMinute = wateringMinute;
  setHoursEditing = false;
  setHoursField = 0;
  drawSethours();
}

void enterSetPump() {
  currentPage = PAGE_SETPUMP;
  pumpEditValue = pumpSet;
  pumpEditing = false;
  drawSetpump();
}

void handleRightButton() {
  resetButtonTimer();
  
  if (currentPage == PAGE_TIMESET) {
    saveMainSettings();
    goToPage(PAGE_SETLIMIT);
    return;
  }
  // Sethours
  if (currentPage == PAGE_SETHOURS) {
    returnToTimesetSelected();
    return;
  }
  // Setpump
  if (currentPage == PAGE_SETPUMP) {
    returnToTimesetSelected();
    return;
  }
  
  if (currentPage == PAGE_SETAUTO && editingSetAuto) {
    if (setAutoChoose == 1) {
      setAutoChoose = 2;
      drawSetauto();
    }
    return;
  }
  
  if (currentPage == PAGE_CHECK) {
    goToNormalTimeset();
  }
  else if (currentPage == PAGE_SETLIMIT) {
    goToPage(PAGE_SETAUTO);
  }
  else if (currentPage == PAGE_SETAUTO) {
    goToPage(PAGE_SCAN);
  }
  else if (currentPage == PAGE_SCAN) {
  // end ....
  }
}

void handleLeftButton() {
  resetButtonTimer();
  // TIMESET -> CHECK
  if (currentPage == PAGE_TIMESET) {
    goToPage(PAGE_CHECK);
    return;
  }
  
  if (currentPage == PAGE_SETHOURS) {
    returnToTimesetSelected();
    return;
  }
  
  if (currentPage == PAGE_SETPUMP) {
    returnToTimesetSelected();
    return;
  }
  
  if (currentPage == PAGE_SETAUTO && editingSetAuto) {
    if (setAutoChoose == 2) {
      setAutoChoose = 1;
      drawSetauto();
    }
    else if (setAutoChoose == 1) {
      saveMainSettings();
      editingSetAuto = false;
      setAutoChoose = 0;
      drawSetauto();
    }
    return;
  }

  if (currentPage == PAGE_SCAN) {
    goToPage(PAGE_SETAUTO);
  }
  else if (currentPage == PAGE_SETAUTO) {
    goToPage(PAGE_SETLIMIT);
  }
  else if (currentPage == PAGE_SETLIMIT) {
    goToNormalTimeset();
  }
}

void handleOkButton() {
  resetButtonTimer();
  //  Timeset ....
  if (currentPage == PAGE_TIMESET) {
    if (!timeSetSelecting) {
      timeSetSelecting = true;
      timeSetChoose = 1;
      drawTimeset();
      return;
    }

    if (timeSetChoose == 1) {
      enterSetHours();
      return;
    }

    if (timeSetChoose == 2) {
      enterSetPump();
      return;
    }
  }
  
  else if (currentPage == PAGE_SETHOURS) {
    if (!setHoursEditing) {
      setHoursEditing = true;
      setHoursField = 1;
      drawSethours();
      return;
    }
    
    if (setHoursField == 1) {
      setHoursField = 2;
      drawSethours();
      return;
    }
    
    if (setHoursField == 2) {
      wateringHour = constrain(editWateringHour, 0, 23);
      wateringMinute = constrain(editWateringMinute, 0, 59);
      saveWateringSchedule();
      setHoursEditing = false;
      setHoursField = 0;
      drawSethours();
      return;
    }
  }
  
  else if (currentPage == PAGE_SETPUMP) {
    if (!pumpEditing) {
      pumpEditing = true;
      pumpEditValue = pumpSet;
      drawSetpump();
      return;
    }

    pumpSet = constrain(pumpEditValue, 0, 99);
    saveWateringSchedule();
    pumpEditing = false;
    drawSetpump();
    return;
  }

  else if (currentPage == PAGE_SETLIMIT) {
    editingSetLimit = !editingSetLimit;

    if (!editingSetLimit) {
      saveMainSettings();
    }

    drawSetlimit();
    return;
  }

  //  Setauto page 
  else if (currentPage == PAGE_SETAUTO) {
    if (!editingSetAuto) {
      editingSetAuto = true;
      setAutoChoose = 1;
      drawSetauto();
      return;
    }

    if (setAutoChoose == 1) {
      saveMainSettings();
      editingSetAuto = false;
      setAutoChoose = 0;
      drawSetauto();
      return;
    }

    if (setAutoChoose == 2) {
      saveMainSettings();
      editingSetAuto = false;
      setAutoChoose = 0;
      drawSetauto();
      return;
    }
  }
}

void handleUpButton() {
  resetButtonTimer();

  if (currentPage == PAGE_TIMESET && timeSetSelecting) {
    timeSetChoose = 1;
    drawTimeset();
    return;
  }

  if (currentPage == PAGE_SETHOURS && setHoursEditing) {
    if (setHoursField == 1) {
      editWateringHour++;
      if (editWateringHour > 23) {
        editWateringHour = 0;
      }
    }
    else if (setHoursField == 2) {
      editWateringMinute++;
      if (editWateringMinute > 59) {
        editWateringMinute = 0;
      }
    }
    drawSethours();
    return;
  }

  if (currentPage == PAGE_SETPUMP && pumpEditing) {
    if (pumpEditValue < 99) {
      pumpEditValue++;
    }
    drawSetpump();
    return;
  }
  
  if (currentPage == PAGE_SETLIMIT && editingSetLimit) {
    if (soilLimit < 99) {
      soilLimit++;
    }
    drawSetlimit();
    return;
  }
  
  if (currentPage == PAGE_SETAUTO && editingSetAuto) {
    if (setAutoChoose == 1) {
      if (humdSet < 99) {
        humdSet++;
      }
    }
    else if (setAutoChoose == 2) {
      if (pumpSet < 99) {
        pumpSet++;
      }
    }
    drawSetauto();
  }
}

void handleDownButton() {
  resetButtonTimer();
  
  if (currentPage == PAGE_TIMESET && timeSetSelecting) {
    timeSetChoose = 2;
    drawTimeset();
    return;
  }
  
  if (currentPage == PAGE_SETHOURS && setHoursEditing) {
    if (setHoursField == 1) {
      editWateringHour--;
      if (editWateringHour < 0) {
        editWateringHour = 23;
      }
    }
    else if (setHoursField == 2) {
      editWateringMinute--;
      if (editWateringMinute < 0) {
        editWateringMinute = 59;
      }
    }
    drawSethours();
    return;
  }
  
  if (currentPage == PAGE_SETPUMP && pumpEditing) {
    if (pumpEditValue > 0) {
      pumpEditValue--;
    }
    drawSetpump();
    return;
  }
  
  if (currentPage == PAGE_SETLIMIT && editingSetLimit) {
    if (soilLimit > 0) {
      soilLimit--;
    }
    drawSetlimit();
    return;
  }
  
  if (currentPage == PAGE_SETAUTO && editingSetAuto) {
    if (setAutoChoose == 1) {
      if (humdSet > 0) {
        humdSet--;
      }
    }
    else if (setAutoChoose == 2) {
      if (pumpSet > 0) {
        pumpSet--;
      }
    }

    drawSetauto();
  }
}

void handleButtons() {
  if (buttonPressed(
      btnRightState.pin,
      btnRightState.lastReading,
      btnRightState.stableState,
      btnRightState.lastChange)) {

  handleRightButton();
  }

else if (buttonPressed(
           btnLeftState.pin,
           btnLeftState.lastReading,
           btnLeftState.stableState,
           btnLeftState.lastChange)) {

  handleLeftButton();
  }

else if (buttonPressed(
           btnOkState.pin,
           btnOkState.lastReading,
           btnOkState.stableState,
           btnOkState.lastChange)) {

  handleOkButton();
  }

else if (buttonPressed(
           btnUpState.pin,
           btnUpState.lastReading,
           btnUpState.stableState,
           btnUpState.lastChange)) {

  handleUpButton();
  }

else if (buttonPressed(
           btnDownState.pin,
           btnDownState.lastReading,
           btnDownState.stableState,
           btnDownState.lastChange)) {

  handleDownButton();
  }
}

String getWateringStatus() {
  if (!timeIsValid()) {
    return "NO TIME SYNC";
  }

  if (pumpActive) {
    return "PUMPING";
  }

  if (wateredToday) {
    return "WATERED TODAY";
  }

  return "NOT WATERED";
}

void sendBlynkData() {
  if (!Blynk.connected()) {
    return;
  }

  Blynk.virtualWrite(V0, soilValue);
  Blynk.virtualWrite(V1, getCurrentTimeString());
  Blynk.virtualWrite(V2, getCurrentDateText());
  Blynk.virtualWrite(V3, getWateringStatus());
  Blynk.virtualWrite(V5, pumpActive ? 1 : 0);
  Blynk.virtualWrite(V6, getScheduleText());
  Blynk.virtualWrite(V7, pumpSet);
  Blynk.virtualWrite(V8, wateredToday ? 1 : 0);
}

BLYNK_CONNECTED() {
  sendBlynkData();
}
BLYNK_WRITE(V4) {
  if (param.asInt() == 1) {
    manualWaterRequest = true;
    Blynk.virtualWrite(V4, 0);
  }
}

void setup() {
  Serial.begin(115200);
  loadSettings();
  Wire.begin(OLED_SDA, OLED_SCL);
  u8g2.begin();
  pinMode(SOIL_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);

  drawLogo();
  delay(2500);
  drawWelcome();
  delay(2000);
  startWiFi();
  setupNTP();

  Blynk.config(BLYNK_AUTH_TOKEN);
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.connect(1000);
  }
  // Timed Blynk network updates
  // Blynk docs recommend using BlynkTimer with virtualWrite
  blynkTimer.setInterval(2000L, sendBlynkData);
  blynkTimer.setInterval(10000L, maintainConnections);
  blynkTimer.setInterval(1000L, updateCurrentTimeStrings);
  blynkTimer.setInterval(1000L, refreshWateredToday);
  currentPage = PAGE_CHECK;
  resetPageEditingStates();
  timeSetSelecting = false;
  timeSetChoose = 1;
  resetButtonTimer();
  drawCheck();
}

void loop() {

  Blynk.run();
  blynkTimer.run();
  soilValue = readSoilValue();
  updateHumidityAlarm();
  updateBuzzer();
  processManualWaterRequest();
  updateScheduledWatering();
  updatePump();

  static unsigned long lastTimeScreenRefresh = 0;
  if (currentPage == PAGE_TIMESET &&
      millis() - lastTimeScreenRefresh >= 1000UL) {
    lastTimeScreenRefresh = millis();
    updateCurrentTimeStrings();
    drawTimeset();
  }
  if (currentPage == PAGE_CHECK &&
    millis() - lastCheckRefresh >= 5000UL) {
  lastCheckRefresh = millis();
  drawCheck();
}
  handleButtons();

  if (millis() - lastButtonTime >= RETURN_TIME) {
    if (currentPage != PAGE_CHECK) {
      saveMainSettings();
      goToPage(PAGE_CHECK);
    }
    resetButtonTimer();
  }
}
