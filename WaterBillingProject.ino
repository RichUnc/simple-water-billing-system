// ================================================
// SMART WATER BILLING SYSTEM – FINAL VERSION WITH KEYPAD INPUT
// RFID + YF-S201 + LCD + Relay + Keypad
// ================================================

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// ---------------- PINS ----------------
#define FLOW_PIN   2
#define RELAY_PIN  7
#define SS_PIN     10
#define RST_PIN    9

// ---------------- MODULES ----------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------------- KEYPAD ----------------
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {3, 4, 5};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ---------------- FLOW SENSOR ----------------
volatile unsigned long pulseCount = 0;
const float calibrationFactor = 450.0;

// ---------------- USERS ----------------
// Joan
const String JOAN_UID = "230835AA";
const String JOAN_NAME = "JOAN WAMBURA";
const float JOAN_CREDITS = 10.0;

// Prof. Nuhu
const String NUHU_UID = "236D7F11";
const String NUHU_NAME = "Prof. NUHU";
const float NUHU_CREDITS = 20.0;

// ---------------- SYSTEM ----------------
bool sessionActive = false;
bool waterFlowing = false;

float credits = 0;
float inputLitres = 0;

// Double press timing
unsigned long lastHash = 0;
unsigned long lastStar = 0;
const unsigned long dblTime = 700;

// ================================================
// FLOW INTERRUPT
// ================================================
void pulseCounter() {
  pulseCount++;
}

// ================================================
// SETUP
// ================================================
void setup() {
  Serial.begin(9600);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);   // Relay OFF

  pinMode(FLOW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), pulseCounter, FALLING);

  SPI.begin();
  mfrc522.PCD_Init();

  lcd.init();
  lcd.backlight();

  showIdle();

  Serial.println("System Ready");
}

// ================================================
// LOOP
// ================================================
void loop() {
  if (!sessionActive) {
    waitForCard();
  }
}

// ================================================
// WAIT FOR RFID
// ================================================
void waitForCard() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  String uid = getUID();
  Serial.print("UID: ");
  Serial.println(uid);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  // --- Check Joan ---
  if (uid == JOAN_UID) {
    credits = JOAN_CREDITS;
    sessionActive = true;
    welcomeUser(JOAN_NAME, credits);
    enterLitres();
  }

  // --- Check Prof. Nuhu ---
  else if (uid == NUHU_UID) {
    credits = NUHU_CREDITS;
    sessionActive = true;
    welcomeUser(NUHU_NAME, credits);
    enterLitres();
  }

  // --- Unknown card ---
  else {
    lcd.clear();
    lcd.print("Access Denied");
    lcd.setCursor(0,1);
    lcd.print("Unknown Card");
    delay(2000);
    showIdle();
  }
}

// ================================================
// GET UID
// ================================================
String getUID() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

// ================================================
// IDLE SCREEN
// ================================================
void showIdle() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Water Billing");
  lcd.setCursor(0,1);
  lcd.print("Scan Card...");
}

// ================================================
// WELCOME
// ================================================
void welcomeUser(String name, float credit) {
  lcd.clear();
  lcd.print("Welcome");
  lcd.setCursor(0,1);
  lcd.print(name);
  delay(1500);

  lcd.clear();
  lcd.print("Credits: ");
  lcd.print(credit,1);
  lcd.print("L");
  delay(1500);
}

// ================================================
// ENTER LITRES
// ================================================
void enterLitres() {
  inputLitres = 0;
  lcd.clear();
  lcd.print("Enter Litres:");
  lcd.setCursor(0,1);
  lcd.print("0L");

  while (sessionActive) {
    char key = keypad.getKey();
    if (!key) continue;

    // -------- NUMBERS --------
    if (key >= '0' && key <= '9') {
      inputLitres = inputLitres * 10 + (key - '0');
      lcd.setCursor(0,1);
      lcd.print(inputLitres);
      lcd.print("L   ");
    }

    // -------- CONFIRM --------
    else if (key == '#') {
      unsigned long now = millis();

      // Double press = reset credits
      if (now - lastHash < dblTime) {
        // Reset credits depending on current user
        if (credits == JOAN_CREDITS) credits = JOAN_CREDITS;
        else credits = NUHU_CREDITS;

        lcd.clear();
        lcd.print("Credits Reset");
        delay(1500);

        enterLitres();
        return;
      }
      lastHash = now;

      if (credits <= 0) {
        endSession();
        return;
      }

      if (inputLitres <= 0) continue;

      if (inputLitres > credits) {
        lcd.clear();
        lcd.print("Low Credits");
        delay(1500);
        enterLitres();
        return;
      }

      // Start water
      dispenseWater(inputLitres);
      return;
    }

    // -------- STAR --------
    else if (key == '*') {
      unsigned long now = millis();

      // Emergency stop
      if (now - lastStar < dblTime) {
        stopWater();
        lcd.clear();
        lcd.print("EMERGENCY STOP");
        delay(1500);
        endSession();
        return;
      }

      lastStar = now;

      // Clear input
      inputLitres = 0;
      lcd.setCursor(0,1);
      lcd.print("0L   ");
    }
  }
}

// ================================================
// DISPENSE WATER
// ================================================
void dispenseWater(float target) {
  pulseCount = 0;
  digitalWrite(RELAY_PIN, HIGH);
  delay(300); // let flow stabilize
  waterFlowing = true;
  pulseCount = 0; // reset again after stabilization

  lcd.clear();
  lcd.print("Dispensing...");

  float dispensed = 0;
  unsigned long startTime = millis();

  while (dispensed < target && waterFlowing) {
    if (millis() - startTime > 180000) break; // Safety timeout 3 min

    noInterrupts();
    unsigned long pulses = pulseCount;
    interrupts();

    dispensed = pulses / calibrationFactor;

    lcd.setCursor(0,1);
    lcd.print(dispensed,1);
    lcd.print("/");
    lcd.print(target);
    lcd.print("L   ");

    // Emergency check
    char key = keypad.getKey();
    if (key == '*') {
      unsigned long now = millis();
      if (now - lastStar < dblTime) {
        stopWater();
        endSession();
        return;
      }
      lastStar = now;
    }
  }

  stopWater();

  credits -= target;

  lcd.clear();
  lcd.print("Done");
  lcd.setCursor(0,1);
  lcd.print("Left: ");
  lcd.print(credits,1);
  lcd.print("L");
  delay(2000);

  if (credits <= 0) endSession();
  else enterLitres();
}

// ================================================
// STOP WATER
// ================================================
void stopWater() {
  digitalWrite(RELAY_PIN, LOW);
  waterFlowing = false;
}

// ================================================
// END SESSION
// ================================================
void endSession() {
  stopWater();
  sessionActive = false;

  lcd.clear();
  lcd.print("Session Ended");
  delay(1500);

  showIdle();
}

