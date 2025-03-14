#include <Wire.h>
#include <LCD_I2C.h>
#include <OneButton.h>

LCD_I2C lcd(0x27, 16, 2);

int THERMISTOR_PIN = A0;
int JOYSTICK_X = A1;
int JOYSTICK_Y = A2;
int LED_PIN = 8;
int BTN_PIN = 2;

int Vo;
float R1 = 10000;
float logR2, R2, tempC;
float c1 = 1.129148e-03, c2 = 2.34125e-04, c3 = 8.76741e-08;

unsigned long currentTime = 0;
bool screenToggle = false;
float temperature = 0.0;
int vitesse = 0;
char direction = 'N';
unsigned long lastSerialTime = 0;
unsigned long lastButtonTime = 0;
const unsigned long serialInterval = 100;
const unsigned long button = 300;

void debut() {
  byte customChar[8] = {
    0b01110,
    0b00100,
    0b00100,
    0b00100,
    0b00000,
    0b01110,
    0b00010,
    0b01110
  };

  lcd.createChar(0, customChar);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SOUNOU");
  lcd.setCursor(14, 1);
  lcd.write(byte(0));
  lcd.setCursor(14, 1);
  lcd.print("72");

  delay(3000);
  lcd.clear();
}

float lireTemperature() {
  Vo = analogRead(THERMISTOR_PIN);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  tempC = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  return tempC;
}

void lireJoystick() {
  int joyY = analogRead(JOYSTICK_Y);
  int joyX = analogRead(JOYSTICK_X);

  vitesse = 0;
  if (joyY > 512) {
    vitesse = map(joyY, 512, 1023, 0, 120);
  } else {
    vitesse = map(joyY, 512, 0, 0, -25);
  }
  //direction
  if (joyX < 480) {
    direction = 'G';
  } else if (joyX > 544) {
    direction = 'D';
  } else {
    direction = 'N';
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  lcd.backlight();
  Serial.begin(115200);

  debut();
}

void loop() {
  currentTime = millis();
  temperature = lireTemperature();
  lireJoystick();

  if (temperature > 25.0) {
    digitalWrite(LED_PIN, HIGH);
  } else if (temperature < 24.0) {
    digitalWrite(LED_PIN, L
    OW);
  }

  if (digitalRead(BTN_PIN) == LOW && millis() - lastButtonTime > button) {
    screenToggle = !screenToggle;
    lastButtonTime = millis();
  }

  lcd.clear();
  if (!screenToggle) {
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("AC: ");
    lcd.print(digitalRead(LED_PIN) ? "ON " : "OFF");
  } else {
    lcd.setCursor(0, 0);
    lcd.print((vitesse >= 0) ? "Avance: " : "Recule: ");
    lcd.print(abs(vitesse));
    lcd.print(" km/h");
    lcd.setCursor(0, 1);
    lcd.print("Dir: ");
    lcd.print(direction);
  }

  if (millis() - lastSerialTime > serialInterval) {
    Serial.print("etd:6225272,x:");
    Serial.print(analogRead(JOYSTICK_X));
    Serial.print(",y:");
    Serial.print(analogRead(JOYSTICK_Y));
    Serial.print(",sys:");
    Serial.println(digitalRead(LED_PIN));
    lastSerialTime = millis();
  }
}
