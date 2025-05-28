#include <ArduinoJson.h>
#include <DHT.h>

#define IR_SENSOR_PIN 2
#define TRIG_PIN 3
#define ECHO_PIN 4
#define DHT_PIN 5
#define RED_PIN 6
#define GREEN_PIN 7
#define BLUE_PIN 8
#define SOUND_LED_PIN 9
#define MQ135_PIN A0
#define MIC_PIN A1

#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

long duration;
int distance;
int irState = 0;
float temperature, humidity;
int airQuality;
int soundValue = 0;
int soundThreshold = 600;

void setRGB(int red, int green, int blue) {
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}

int getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;
  return distance;
}

void controlLED(int dist) {
  if (dist < 10) {
    for (int i = 0; i < 10; i++) {
      digitalWrite(SOUND_LED_PIN, HIGH);
      delay(200);
      digitalWrite(SOUND_LED_PIN, LOW);
      delay(200);
    }
  } else {
    digitalWrite(SOUND_LED_PIN, LOW);
  }
}

void checkSound() {
  soundValue = analogRead(MIC_PIN);
  if (soundValue > soundThreshold) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(SOUND_LED_PIN, HIGH);
      delay(200);
      digitalWrite(SOUND_LED_PIN, LOW);
      delay(200);
    }
  }
}

void updateRGBBasedOnAQI(int airQuality) {
  if (airQuality > 400) {
    setRGB(255, 0, 0);
  } else if (airQuality >= 200 && airQuality <= 400) {
    setRGB(0, 255, 0);
  } else {
    setRGB(0, 0, 255);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(SOUND_LED_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  dht.begin();
}

void loop() {
  StaticJsonDocument<512> doc;


  irState = digitalRead(IR_SENSOR_PIN);
  if(irState==0)irState=1;
  else irState=0;
  doc["Motion Detected"] = (irState == HIGH);
  distance = getDistance();
  doc["Distance"] = distance;
  controlLED(distance);
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  doc["Temperature"] = temperature;
  doc["Humidity"] = humidity;
  airQuality = analogRead(MQ135_PIN);
  doc["Air Quality"] = airQuality;
  updateRGBBasedOnAQI(airQuality);
  checkSound();
  doc["Sound Level"] = soundValue;
  serializeJson(doc, Serial);
  Serial.println();
  delay(5000);
}
