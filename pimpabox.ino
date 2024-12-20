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
#include <ArduinoECCX08.h>

#include <string>
using std::string;
#include <vector>
using std::vector;

constexpr int pin_battery = A1;
constexpr int pin_power = 2;
constexpr int pin_shutdown = 3;
constexpr int pin_SD_CS = 10;

struct State {
  int magic = 42;
  int last_song = 1;
};

State state;

void playFile(const char *filename) {
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

vector<string> file_names;

void readFileNames(void) {
  File root = SD.open("/");
  while (true) {
    File entry = root.openNextFile();
    if (!entry)
      break;
    char *s = entry.name();
    if (isDigit(*s)) {
      file_names.push_back(s);
    }
  }
}

void printFileNames(void) {
  Serial.print(file_names.size());
  Serial.println(" songs:");
  for (int i = 0; i < file_names.size(); i++) {
    Serial.print(i);
    Serial.print(" ");
    Serial.println(file_names[i].c_str());
  }
}

void setup(void) {
  // Lock power switch
  pinMode(pin_power, OUTPUT);
  digitalWrite(pin_power, HIGH);
  // Shutdown amp
  pinMode(pin_shutdown, OUTPUT);
  digitalWrite(pin_shutdown, HIGH);
  // Measure battery voltage
  analogReadResolution(10);
  constexpr float r1 = 68;
  constexpr float r2 = 33;
  const float v_battery =
    (analogRead(pin_battery) / 1024.0) * 3.3 * (r1 + r2) / r2;
  constexpr float v_battery_low = 5.2;
  // Start Serial
  Serial.begin(9600);
  const unsigned long millis0 = millis();
  while (v_battery < v_battery_low
	 && !Serial
	 && millis() - millis0 < 5000)
    yield();
  Serial.println("*** PimpaBox ***");
  Serial.println("***  AP2024. ***");
  // Start SD
  if (!SD.begin(pin_SD_CS)) {
    Serial.println("SD begin error");
    while (true);
  }
  // Read and print out file names
  readFileNames();
  printFileNames();
  // Start AudioZero
  AudioZero.begin(2*44100);
  analogWriteResolution(8);
  // Start IMU
  if (!IMU.begin()) {
    Serial.println("IMU begin error");
    while (true);
  }
  // Shutdown if battery low
  Serial.print("v_battery=");
  Serial.println(v_battery, 3);
  if (v_battery < v_battery_low) {
    char filename[] = "sad.wav";
    playFile(filename);
    digitalWrite(pin_power, LOW);
  }
  // Read state
  State state_temp;
  if (!ECCX08.begin()) {
    Serial.println("ECCX08 begin error");
    while (true);
  }
  if (!ECCX08.locked()) {
    Serial.println("ECCX08 not locked");
    while (true);
  }
  int ret =
    ECCX08.readSlot(8, (byte*)&state_temp, sizeof(state_temp));
  if (ret != 1) {
    Serial.println("ECCX08 read error");
    while (true);
  }
  if (state_temp.magic == state.magic) {
    state = state_temp;
  }
}

void loop(void) {
  // Next song
  int song_number = state.last_song + 1;
  if (song_number >= file_names.size()) {
    song_number = 0;
  }
  state.last_song = song_number;
  // Save song
  int ret = ECCX08.writeSlot(8, (byte*)&state, sizeof(state));
  if (ret != 1) {
    Serial.println("ECCX08 write error");
    while (true);
  }
  // Play song
  playFile(file_names[song_number].c_str());
  digitalWrite(pin_power, LOW);
}

