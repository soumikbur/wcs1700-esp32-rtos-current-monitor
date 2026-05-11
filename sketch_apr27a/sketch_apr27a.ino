#include <Arduino.h>

#define ADC_PIN            34
#define RMS_WINDOW         400    // Captures stable AC cycles
#define ZERO_THRESHOLD     0.45f  // Threshold for instant-off

volatile float rmsFiltered   = 0.0f;
volatile float rmsRaw        = 0.0f;
volatile float sumSq         = 0.0f;
volatile float sum           = 0.0f;
volatile int   sampleCount   = 0;

QueueHandle_t sampleQueue;

// ─── THE LOOKUP TABLE (LUT) ─────────────────────────────────────────────────
// IMPORTANT: You MUST replace these placeholder values with your own measurements!
// lutRaw    = The internal ESP32 'RAW_ADC' reading (must be in increasing order)
// lutActual = What your highly accurate multimeter said at that exact moment
const int LUT_SIZE = 6;

float lutRaw[LUT_SIZE] =     {0.00,  0.0215, 0.0410, 0.0602,0.1334, 0.2008}; // <--- EDIT THESE
float lutActual[LUT_SIZE] =  {0.00,  0.77,   1.55,   2.31,5.16, 7.66};   // <--- EDIT THESE
// ────────────────────────────────────────────────────────────────────────────

// Linear Interpolation Function
float interpolateCurrent(float rawIn) {
  // If reading is at or below zero-point, return 0
  if (rawIn <= lutRaw[0]) return lutActual[0];

  // If the load is higher than our max calibrated point, extrapolate the line
  if (rawIn >= lutRaw[LUT_SIZE - 1]) {
    float m = (lutActual[LUT_SIZE-1] - lutActual[LUT_SIZE-2]) / (lutRaw[LUT_SIZE-1] - lutRaw[LUT_SIZE-2]);
    return lutActual[LUT_SIZE-2] + m * (rawIn - lutRaw[LUT_SIZE-2]);
  }

  // Find where the raw value falls between two known points and mathematically stretch it
  for (int i = 0; i < LUT_SIZE - 1; i++) {
    if (rawIn >= lutRaw[i] && rawIn <= lutRaw[i + 1]) {
      float rangeRaw = lutRaw[i + 1] - lutRaw[i];
      float rangeActual = lutActual[i + 1] - lutActual[i];
      float fraction = (rawIn - lutRaw[i]) / rangeRaw;
      return lutActual[i] + (fraction * rangeActual);
    }
  }
  return 0.0f;
}

void adcTask(void *parameter) {
  while (1) {
    int raw = analogRead(ADC_PIN);
    xQueueSend(sampleQueue, &raw, 0);
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void processingTask(void *parameter) {
  int raw;
  while (1) {
    if (xQueueReceive(sampleQueue, &raw, pdMS_TO_TICKS(100))) {
      // Convert raw ADC directly to voltage
      float v = (float)raw * (3.3f / 4095.0f);
      sum   += v;
      sumSq += v * v;
      sampleCount++;

      if (sampleCount >= RMS_WINDOW) {
        // DYNAMIC AC COUPLING (Variance = Mean of Squares - Square of the Mean)
        float meanV    = sum / sampleCount;
        float meanSqV  = sumSq / sampleCount;
        float variance = meanSqV - (meanV * meanV);

        if (variance < 0.0f) variance = 0.0f;

        float adcRMS = sqrt(variance);
        rmsRaw       = adcRMS; // Expose this variable so you can build your LUT!

        // Pass the raw variance into our new Lookup Table function
        float rmsCurrent = interpolateCurrent(adcRMS);

        // Instant Off Logic
        if (rmsCurrent < ZERO_THRESHOLD) {
            rmsFiltered = 0.0f;
        } else {
            // Heavy EMA filter to flatten out the math while running
            rmsFiltered = 0.85f * rmsFiltered + 0.15f * rmsCurrent;
        }

        sum         = 0.0f;
        sumSq       = 0.0f;
        sampleCount = 0;
      }
    }
  }
}

void serialTask(void *parameter) {
  while (1) {
    // Printing BOTH the Raw internal number (for your LUT mapping) and the Final Current
    Serial.print("RAW_ADC:");
    Serial.print(rmsRaw, 4);
    Serial.print(" , Current_A:");
    Serial.println(rmsFiltered, 2);
    
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  analogReadResolution(12);
  analogSetAttenuation((adc_attenuation_t)3);

  sampleQueue = xQueueCreate(256, sizeof(int));
  if (sampleQueue == NULL) {
    while (1);
  }

  xTaskCreatePinnedToCore(adcTask,        "ADC",  4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(processingTask, "PROC", 8192, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(serialTask,     "SER",  4096, NULL, 1, NULL, 0);
}

void loop() {
  // Empty loop: FreeRTOS tasks handle everything
  delay(10);
}