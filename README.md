# ESP32 True RMS Current Monitor (WCS1700)

A dual-core FreeRTOS-based ESP32 application designed for accurate AC current measurement using the WCS1700 Hall-effect current sensor.

The system monitors dynamic AC loads such as lightbulbs and high-power appliances while minimizing ADC noise and maintaining high measurement accuracy.

---

## Features

* Dual-core FreeRTOS architecture
* Real-time AC current monitoring
* True RMS current calculation
* Dynamic AC coupling for DC drift correction
* Lookup Table (LUT) based ADC linearization
* Exponential Moving Average (EMA) filtering
* Instant-off current detection
* CSV serial logging support

---

## FreeRTOS Architecture

### Core 1

* High-priority ADC sampling task
* Reads sensor data every 1ms

### Core 0

* Performs RMS calculations
* Applies filtering and calibration logic

This separation ensures stable timing and efficient processing.

---

## Signal Processing Techniques

### Dynamic AC Coupling

Removes DC offset drift caused by power supply fluctuations using:

Variance = Mean of Squares - Square of Mean

This allows stable RMS current extraction even under varying loads.

---

### LUT-Based Calibration

The ESP32 ADC is non-linear.

A 6-point Lookup Table with linear interpolation is used to map raw ADC values to accurate multimeter readings.

---

### EMA Filtering

An Exponential Moving Average filter stabilizes noisy ADC readings.

---

### Instant-Off Logic

If measured current drops below the threshold (~0.45A), output immediately resets to 0.00A to eliminate false decay readings.

---

## Hardware Used

* ESP32
* WCS1700 Hall-effect current sensor
* AC test loads
* Digital multimeter

---

## Calibration Procedure

1. Upload code to ESP32
2. Open Serial Monitor
3. Record RAW ADC value at no load
4. Apply known loads
5. Compare readings with multimeter
6. Update:

   * `lutRaw[]`
   * `lutActual[]`

---

## CSV Logging

The ESP32 outputs serial data in CSV format:

millis,current

Example:

1250,0.76
2250,1.54
3250,2.30

---

## Tools Used

* Arduino IDE / ESP-IDF
* FreeRTOS
* CoolTerm (for CSV logging)

---

## Author

Soumik Ghosh
