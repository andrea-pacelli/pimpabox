//////////////////////////////////////////////////////////////////
//
// PimpaBox
// AP2024.
//
//////////////////////////////////////////////////////////////////

#include <Arduino.h>

#include <SD.h>
#include <SPI.h>
#include <AudioZero.h>
#include <Arduino_LSM6DS3.h>

constexpr int pin_battery = A1;
constexpr int pin_power = 2;
constexpr int pin_shutdown = 3;
constexpr int pin_SD_CS = 10;

void playFile(char *filename) {
  Serial.print("Playing ");
  Serial.println(filename);
  File file = SD.open(filename);
  if (!file) {
    Serial.println("File open error");
    while (true);
  }
  digitalWrite(pin_shutdown, LOW);
  AudioZero.play(file);
  digitalWrite(pin_shutdown, HIGH);
  file.close();
  delay(500);
}

void setup(void) {
  // Lock power switch
  pinMode(pin_power, OUTPUT);
  digitalWrite(pin_power, HIGH);
  // Shutdown amp
  pinMode(pin_shutdown, OUTPUT);
  digitalWrite(pin_shutdown, HIGH);
  // Start Serial
  Serial.begin(9600);
  const unsigned long millis0 = millis();
  while (!Serial && millis() - millis0 < 2000)
    yield();
  Serial.println("*** PimpaBox ***");
  Serial.println("***  AP2024. ***");
  // Start SD
  if (!SD.begin(pin_SD_CS)) {
    Serial.println("SD begin error");
    while (true);
  }
  // Start AudioZero
  AudioZero.begin(2*44100);
  analogWriteResolution(8);
  // Start IMU
  if (!IMU.begin()) {
    Serial.println("IMU begin error");
    while (true);
  }
  // Check battery status
  analogReadResolution(10);
  constexpr float r1 = 68;
  constexpr float r2 = 33;
  const float v_battery =
    (analogRead(pin_battery) / 1024.0) * 3.3 * (r1 + r2) / r2;
  Serial.print("v_battery=");
  Serial.println(v_battery, 3);
  if (v_battery < 5.2) {
    char filename[] = "sad.wav";
    playFile(filename);
    digitalWrite(pin_power, LOW);
  }
  randomSeed(analogRead(pin_battery));
}

void loop(void) {
  float a[3];
  IMU.readAcceleration(a[0], a[1], a[2]);
  Serial.print("a=(");
  Serial.print(a[0]);
  Serial.print(",");
  Serial.print(a[1]);
  Serial.print(",");
  Serial.print(a[2]);
  Serial.println(")");
  int song_number = 0;
  for (int i = 0; i < 3; i++) {
    const int j = 1 + 2 * i;
    if (a[i] > 0.5) {
      song_number = j;
    }
    else if (a[i] < -0.5) {
      song_number = j + 1;
    }
  }
  char filename[10];
  sprintf(filename, "%s%d.wav",
	  (song_number < 10 ? "0" : ""),
	  song_number);
  playFile(filename);
  digitalWrite(pin_power, LOW);
}

