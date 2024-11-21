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

#include <CdSLightSensor.h>

void setup(void) {
  // Start Serial
  Serial.begin(9600);
  const unsigned long millis0 = millis();
  while (!Serial && millis() - millis0 < 2000)
    yield();
  Serial.println("*** PimpaBox ***");
  Serial.println("***  AP2024. ***");
  // Start SD
  if (!SD.begin(4)) {
    Serial.println("SD begin error");
    while (true);
  }
  // Start AudioZero
  AudioZero.begin(2*44100);
  analogWriteResolution(8);
}

void loop(void) {
  int song_number = random(1, 7);
  char filename[10];
  sprintf(filename, "%s%d.wav",
	  (song_number < 10 ? "0" : ""),
	  song_number);
  Serial.print("Playing ");
  Serial.println(filename);
  File file = SD.open(filename);
  if (!file) {
    Serial.println("File open error");
    while (true);
  }
  AudioZero.play(file);
  file.close();
  delay(500);
}

