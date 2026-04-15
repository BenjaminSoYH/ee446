github
#include <Arduino_HS300x.h>
#include <Arduino_APDS9960.h>
#include <Arduino_BMI270_BMM150.h>

float baseHumidity = 0.0;
float baseTemperature = 0.0;
float baseMagStrength = 0.0;
int baseRed = 0, baseGreen = 0, baseBlue = 0, baseClear = 0;

float currentHumidity = 0.0;
float currentTemperature = 0.0;
float currentMagStrength = 0.0;
int currentRed = 0, currentGreen = 0, currentBlue = 0, currentClear = 0;

const float HUMIDITY_THRESHOLD = 3.0;
const float TEMPERATURE_THRESHOLD = 1.0;
const float MAGNETIC_THRESHOLD = 8.0;
const int COLOR_THRESHOLD = 40;

float readMagStrength() {
  float mx, my, mz;

  if (IMU.magneticFieldAvailable()) {
    IMU.readMagneticField(mx, my, mz);
    return sqrt(mx * mx + my * my + mz * mz);
  }

  return currentMagStrength;
}

void updateColorReading() {
  if (APDS.colorAvailable()) {
    APDS.readColor(currentRed, currentGreen, currentBlue, currentClear);
  }
}

void collectBaseline() {
  float humidityTotal = 0.0;
  float temperatureTotal = 0.0;
  float magneticTotal = 0.0;
  long redTotal = 0, greenTotal = 0, blueTotal = 0, clearTotal = 0;

  const int sampleCount = 30;

  for (int i = 0; i < sampleCount; i++) {
    delay(150);

    humidityTotal += HS300x.readHumidity();
    temperatureTotal += HS300x.readTemperature();
    magneticTotal += readMagStrength();

    updateColorReading();

    redTotal += currentRed;
    greenTotal += currentGreen;
    blueTotal += currentBlue;
    clearTotal += currentClear;
  }

  baseHumidity = humidityTotal / sampleCount;
  baseTemperature = temperatureTotal / sampleCount;
  baseMagStrength = magneticTotal / sampleCount;

  baseRed = redTotal / sampleCount;
  baseGreen = greenTotal / sampleCount;
  baseBlue = blueTotal / sampleCount;
  baseClear = clearTotal / sampleCount;
}

String classifyEnvironmentEvent() {
  int humiditySpike = (currentHumidity > baseHumidity + HUMIDITY_THRESHOLD) ? 1 : 0;
  int temperatureSpike = (currentTemperature > baseTemperature + TEMPERATURE_THRESHOLD) ? 1 : 0;
  int magneticChange = (abs(currentMagStrength - baseMagStrength) > MAGNETIC_THRESHOLD) ? 1 : 0;

  int totalColorDifference =
      abs(currentRed - baseRed) +
      abs(currentGreen - baseGreen) +
      abs(currentBlue - baseBlue) +
      abs(currentClear - baseClear);

  int lightingChange = (totalColorDifference > COLOR_THRESHOLD) ? 1 : 0;

  Serial.print("flags,humid_jump=");
  Serial.print(humiditySpike);
  Serial.print(",temp_rise=");
  Serial.print(temperatureSpike);
  Serial.print(",mag_shift=");
  Serial.print(magneticChange);
  Serial.print(",light_or_color_change=");
  Serial.println(lightingChange);

  if (humiditySpike && temperatureSpike) {
    return "BREATH_OR_WARM_AIR_EVENT";
  } else if (magneticChange) {
    return "MAGNETIC_DISTURBANCE_EVENT";
  } else if (lightingChange) {
    return "LIGHT_OR_COLOR_CHANGE_EVENT";
  } else {
    return "BASELINE_NORMAL";
  }
}

void readAllSensors() {
  currentHumidity = HS300x.readHumidity();
  currentTemperature = HS300x.readTemperature();
  currentMagStrength = readMagStrength();
  updateColorReading();
}

void printSensorData() {
  Serial.print("raw,rh=");
  Serial.print(currentHumidity, 2);
  Serial.print(",temp=");
  Serial.print(currentTemperature, 2);
  Serial.print(",mag=");
  Serial.print(currentMagStrength, 2);
  Serial.print(",r=");
  Serial.print(currentRed);
  Serial.print(",g=");
  Serial.print(currentGreen);
  Serial.print(",b=");
  Serial.print(currentBlue);
  Serial.print(",clear=");
  Serial.println(currentClear);
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  if (!HS300x.begin()) {
    Serial.println("HS300x failed");
    while (1);
  }

  if (!APDS.begin()) {
    Serial.println("APDS failed");
    while (1);
  }

  if (!IMU.begin()) {
    Serial.println("IMU failed");
    while (1);
  }

  collectBaseline();
}

void loop() {
  readAllSensors();
  printSensorData();

  String detectedEvent = classifyEnvironmentEvent();

  Serial.print("event,");
  Serial.println(detectedEvent);

  delay(500);
}