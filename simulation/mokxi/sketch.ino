/*
 * Smart Irrigation System - STM32F103C8 Blue Pill (Arduino STM32 core)
 * Ban mo phong cho Wokwi / Mokxi. Gan chan giong schematic/PCB cua nhom.
 *
 *  PA0  : cam bien do am (bien tro trong mo phong)   - ADC1_IN0
 *  PA4  : relay IN (kich muc THAP: LOW = bom ON)
 *  PA2/PA3 : USART2 TX/RX, 115200 baud
 *  PB0 RS, PB1 E, PB10..PB13 D4..D7 : LCD 16x2 (4-bit)
 *  PB5 MODE, PB6 UP, PB7 DOWN, PB8 PUMP : nut nhan (pull-up noi, nhan = LOW)
 *
 * 5 "task" chay theo chu ky bang millis() (khong chan), cung chu ky voi
 * thiet ke FreeRTOS: SensorRead 1000ms, ButtonProcess 20ms, Control 100ms,
 * Display 200ms, UART_Tx 1000ms.
 */
#include <LiquidCrystal.h>

// ---------------- Chan ----------------
const uint32_t PIN_SOIL  = PA0;
const uint32_t PIN_RELAY = PA4;
const uint32_t PIN_BTN[4] = {PB5, PB6, PB7, PB8};
enum { BTN_MODE = 0, BTN_UP, BTN_DOWN, BTN_PUMP };

LiquidCrystal lcd(PB0, PB1, PB10, PB11, PB12, PB13);
HardwareSerial uart2(PA3, PA2);   // RX, TX (USART2)

// ---------------- Hieu chuan ADC ----------------
// Mo phong: van bien tro len = am hon. Cam bien dien dung thuc te thuong
// cho dien ap CAO khi KHO -> doi lai ADC_DRY/ADC_WET khi hieu chuan mach that.
const int ADC_DRY = 0;     // gia tri ADC khi dat kho  -> 0 %
const int ADC_WET = 4095;  // gia tri ADC khi dat uot  -> 100 %

// ---------------- Du lieu dung chung ----------------
enum Mode { MODE_AUTO, MODE_MANUAL };
struct SystemData {
  int  soil;       // %
  int  setpoint;   // %
  Mode mode;
  bool pumpOn;
} sys = {0, 60, MODE_AUTO, false};

// ---------------- Relay ----------------
void pumpSet(bool on) {
  sys.pumpOn = on;
  digitalWrite(PIN_RELAY, on ? LOW : HIGH);   // active-low
}

// ---------------- Cam bien ----------------
int soilReadPercent() {
  long sum = 0;
  for (int i = 0; i < 8; i++) sum += analogRead(PIN_SOIL);
  int raw = sum / 8;
  long pct = (long)(raw - ADC_DRY) * 100 / (ADC_WET - ADC_DRY);
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  return (int)pct;
}

// ---------------- Nut nhan (chong doi 3 lan quet) ----------------
uint8_t btnStable[4] = {HIGH, HIGH, HIGH, HIGH};
uint8_t btnCount[4]  = {0, 0, 0, 0};

// tra ve true dung 1 lan khi nut vua duoc nhan (HIGH -> LOW on dinh)
bool buttonPressed(int i) {
  uint8_t now = digitalRead(PIN_BTN[i]);
  if (now != btnStable[i]) {
    if (++btnCount[i] >= 3) {
      btnStable[i] = now;
      btnCount[i] = 0;
      return now == LOW;
    }
  } else {
    btnCount[i] = 0;
  }
  return false;
}

// ---------------- Cac "task" ----------------
void taskSensorRead() { sys.soil = soilReadPercent(); }

void taskButtonProcess() {
  if (buttonPressed(BTN_MODE)) {
    sys.mode = (sys.mode == MODE_AUTO) ? MODE_MANUAL : MODE_AUTO;
    pumpSet(false);                              // doi che do -> tat bom
  }
  if (buttonPressed(BTN_UP)   && sys.setpoint < 100) sys.setpoint++;
  if (buttonPressed(BTN_DOWN) && sys.setpoint > 0)   sys.setpoint--;
  if (buttonPressed(BTN_PUMP) && sys.mode == MODE_MANUAL) pumpSet(!sys.pumpOn);
}

void taskControl() {
  if (sys.mode == MODE_AUTO) {
    if (sys.soil < sys.setpoint)       pumpSet(true);
    else                               pumpSet(false);
  }
}

void taskDisplay() {
  char l1[17], l2[17];
  snprintf(l1, sizeof(l1), "SOIL%3d%% SET%3d%%", sys.soil, sys.setpoint);
  snprintf(l2, sizeof(l2), "%-4s  PUMP:%-3s ", sys.mode == MODE_AUTO ? "AUTO" : "MAN",
           sys.pumpOn ? "ON" : "OFF");
  lcd.setCursor(0, 0); lcd.print(l1);
  lcd.setCursor(0, 1); lcd.print(l2);
}

void taskUartTx() {
  char buf[64];
  snprintf(buf, sizeof(buf), "SOIL: %d%% | SET: %d%% | MODE: %s | PUMP: %s",
           sys.soil, sys.setpoint, sys.mode == MODE_AUTO ? "AUTO" : "MANUAL",
           sys.pumpOn ? "ON" : "OFF");
  uart2.println(buf);
}

// ---------------- Bo lap lich don gian ----------------
struct Task { void (*fn)(); uint32_t period; uint32_t last; };
Task tasks[] = {
  {taskSensorRead,    1000, 0},
  {taskButtonProcess,   20, 0},
  {taskControl,        100, 0},
  {taskDisplay,        200, 0},
  {taskUartTx,        1000, 0},
};

void setup() {
  pinMode(PIN_RELAY, OUTPUT);
  pumpSet(false);                                // bom tat khi khoi dong
  for (int i = 0; i < 4; i++) pinMode(PIN_BTN[i], INPUT_PULLUP);
  analogReadResolution(12);
  uart2.begin(115200);
  lcd.begin(16, 2);
  lcd.print("Smart Irrigation");
  delay(800);
  lcd.clear();
  taskSensorRead();
}

void loop() {
  uint32_t now = millis();
  for (auto &t : tasks) {
    if (now - t.last >= t.period) {
      t.last = now;
      t.fn();
    }
  }
}
