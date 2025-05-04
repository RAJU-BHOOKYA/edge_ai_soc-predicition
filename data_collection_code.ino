#include <DHT.h>

// DHT11 configuration
#define DHTPIN 2         // Pin where the DHT11 is connected
#define DHTTYPE DHT11    // DHT11 sensor type
DHT dht(DHTPIN, DHTTYPE);

// Voltage sensing
#define VOLTAGE_PIN A0   // Analog pin for voltage measurement
const float referenceVoltage = 3.3; // Updated for 3.3V systems

float power = 0.0;
float Ah = 0.0;
float Wh = 0.0;

void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  // Read temperature from DHT11
  float temperature = dht.readTemperature(); // Celsius

  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT11 sensor!");
  } else {
    Serial.print("Temperature (°C): ");
    Serial.print(temperature);
  }
  Serial.print("  ");
  // Read analog voltage
  int analogValue = analogRead(VOLTAGE_PIN);
  float voltage = analogValue * (referenceVoltage / 1023.0);
  voltage = voltage / 0.5;

  float current = voltage / 10;

  power = voltage * current;

    // Integrate current and power over time to get Ah and Wh
    // Since we're sampling every second, 1 second = 1/3600 hours
  Ah += current * (0.5 / 3600.0); // Amp-seconds to Ah
  Wh += power * (0.5 / 3600.0);   // Watt-seconds to Wh

  Serial.print("Analog Voltage (V): ");
  Serial.print(voltage);
  Serial.print("  ");
  Serial.print("Current");
  Serial.print(current);
  Serial.print("  ");
  Serial.print("Ampere-hours (Ah): ");
  Serial.print(Ah,6);
  Serial.print("  ");
  Serial.print("Watt-hours (Wh): ");
  Serial.println(Wh,6);

  delay(500); // Wait 2 seconds
}
