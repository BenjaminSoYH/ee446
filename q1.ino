#include <PDM.h>
#include <Arduino_APDS9960.h>
#include <Arduino_BMI270_BMM150.h>


short sampleBuffer[256];
volatile int samplesRead = 0;

void onPDMdata() {
  int bytesAvailable = PDM.available();
  PDM.read(sampleBuffer, bytesAvailable);
  samplesRead = bytesAvailable / 2;
}


String sit_classifier(int sound, float brightness,float acceleration, int distance) {
  String output = "";
  if (sound > 500) {
    output += "NOISY_";
  } else{
    output += "QUIET_";
  }

  if (brightness > 30) {
    output += "BRIGHT_";
  } else {
    output += "DARK_";
  }
  if (acceleration > 3) {
    output += "MOVING_";
  }
  else {
    output += "STEADY_";
  }
    if (distance > 2) {
    output += "FAR";
  }
  else {
    output += "NEAR";
  }
  return output;
}


void setup() {
  Serial.begin(115200);
  delay(1500);

  PDM.onReceive(onPDMdata);

  if (!PDM.begin(1, 16000)) {
    Serial.println("Failed to start PDM microphone.");
    while (1);
  }

    if (!APDS.begin()) {
    Serial.println("Failed to initialize APDS9960 sensor.");
    while (1);
  }

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU.");
    while (1);
  }
    if (!APDS.begin()) {
    Serial.println("Failed to initialize APDS9960 sensor.");
    while (1);
  }

}

void loop() {
  int level = 0;
  if (samplesRead) {
    long sum = 0;
    for (int i = 0; i < samplesRead; i++) {
      sum += abs(sampleBuffer[i]);
    }
    level = sum / samplesRead;
    // Serial.print("Sound: ");
    // Serial.println(level);
    samplesRead = 0;
  }


  int r, g, b, c;
  if (APDS.colorAvailable()) {
    APDS.readColor(r, g, b, c);

  }

  float x, y, z;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);

  }
  int proximity = 0;
  if (APDS.proximityAvailable()) {
    proximity = APDS.readProximity();

  }
  float avg_speed = sqrt(x*x + y*y + z*z);
  // Serial.println(avg_speed);
  // Serial.println(level);
  String situation = sit_classifier(level,c,avg_speed,proximity);
  Serial.println(situation);


   int sound_flag = (level > 500) ? 1 : 0;
  int dark_flag = (c <= 30) ? 1 : 0;
  int moving_flag = (avg_speed > 3) ? 1 : 0;
  int near_flag = (proximity <= 2) ? 1 : 0;
  Serial.print("raw,mic=");
  Serial.print(level);
  Serial.print(",clear=");
  Serial.print(c);
  Serial.print(",motion=");
  Serial.print(avg_speed);
  Serial.print(",prox=");
  Serial.println(proximity);

  Serial.print("flags,sound=");
  Serial.print(sound_flag);
  Serial.print(",dark=");
  Serial.print(dark_flag);
  Serial.print(",moving=");
  Serial.print(moving_flag);
  Serial.print(",near=");
  Serial.println(near_flag);


  delay(500);
}