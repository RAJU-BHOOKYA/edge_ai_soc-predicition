#include <TensorFlowLite.h>
#include "model.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include <DHT.h>

#define DHTPIN 2         // Pin where the DHT11 is connected
#define DHTTYPE DHT11    // DHT11 sensor type
DHT dht(DHTPIN, DHTTYPE);

#define VOLTAGE_PIN A0   // Analog pin for voltage measurement
const float referenceVoltage = 3.3; // Updated for 3.3V systems

const int buttonPin = 3;
int storedValue = 0;
bool buttonState = HIGH;

float power = 0.0;
float Ah = 0.0;
float Wh = 0.0;


#define INPUT_SIZE 2
#define OUTPUT_SIZE 1
#define TENSOR_ARENA_SIZE 2 * 1024

float MIN_VOLT = 0;
float MAX_VOLT = 0; 
float MIN_CURR = 0;
float MAX_CURR = 0;



uint8_t tensor_arena[TENSOR_ARENA_SIZE];

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* model_input = nullptr;
TfLiteTensor* model_output = nullptr;
bool model_initialized = false;

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);
  dht.begin();
  Serial.println("Press button to select chemistry.");
}

void loop() {
  buttonState = digitalRead(buttonPin);

  if (buttonState == LOW) {
    Serial.println("Select Chemistry:");
    Serial.println("1 - Li ion");
    Serial.println("2 - LFP");

    while (!Serial.available()) {
      delay(100);
    }

    String inputStr = Serial.readStringUntil('\n');
    inputStr.trim();

    if (inputStr == "1" || inputStr == "2") {
      storedValue = inputStr.toInt();
      const unsigned char* model_data = nullptr;

      if (storedValue == 1) {
        Serial.println("Li-ion selected.");
        MIN_VOLT = 2.90056;
        MAX_VOLT = 4.19995;                                                                                           
        MIN_CURR = -9.18451;
        MAX_CURR = 5.27935;
        model_data = mlp_soc_model_li_tflite;
      } else {
        Serial.println("LFP selected.");
        MIN_VOLT = 2.50025821;
        MAX_VOLT = 3.53489327;
        MIN_CURR = -0.12502533;
        MAX_CURR = -0.12501293;
        model_data = mlp_soc_model_lfp_tflite;
      }

      model = tflite::GetModel(model_data);
      if (model->version() != TFLITE_SCHEMA_VERSION) {
        Serial.println("Model schema version mismatch!");
        while (1);
      }

      static tflite::AllOpsResolver resolver;
      static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, TENSOR_ARENA_SIZE);
      interpreter = &static_interpreter;

      if (interpreter->AllocateTensors() != kTfLiteOk) {
        Serial.println("Tensor allocation failed!");
        while (1);
      }

      model_input = interpreter->input(0);
      model_output = interpreter->output(0);
      model_initialized = true;
    } else {
      Serial.println("Invalid input. Only enter 1 or 2.");
    }

    // Wait until button is released to prevent re-triggering
    while (digitalRead(buttonPin) == LOW) {
      delay(10);
    }
  }

  // Run inference only if model is ready
  if (model_initialized) {
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

    current = -1 * current;

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


    float normalized_voltage = (voltage - MIN_VOLT) / (MAX_VOLT - MIN_VOLT);
    float normalized_current = (current - MIN_CURR) / (MAX_CURR - MIN_CURR);

    model_input->data.f[0] = normalized_voltage;
    model_input->data.f[1] = normalized_current;
    model_input->data.f[2] = normalized_temperature;
    model_input->data.f[4] = Ah;
    model_input->data.f[5] = Wh;


    if (interpreter->Invoke() != kTfLiteOk) {
      Serial.println("Invoke failed!");
      return;
    }

    float soc = model_output->data.f[0];

      Serial.print("Predicted SoC: ");
      Serial.println(0);  // Output scaled from 0–1 to %

  delay(1000);
}
