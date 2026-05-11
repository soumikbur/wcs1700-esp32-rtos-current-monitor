# ESP32 True RMS Current Monitor (WCS1700)

[cite_start]A highly optimized, dual-core FreeRTOS application for the ESP32 custom-engineered to read a WCS1700 Hall-effect current sensor[cite: 182]. [cite_start]This project precisely monitors dynamic AC loads—from parallel lightbulb circuits to high-draw appliances like hairdryers—while aggressively filtering hardware noise and strictly maintaining a sub-1% error margin[cite: 183].

## 🚀 Key Features & Problem Solving

[cite_start]Measuring AC current with a bare ESP32 and an analog Hall-effect sensor presents severe physical hardware limitations[cite: 184]. [cite_start]This code utilizes advanced signal processing to mathematically correct these hardware flaws[cite: 185]:

* [cite_start]**Multi-Threaded Architecture (FreeRTOS):** The workload is split across the ESP32's two internal cores to ensure mathematical calculations never interrupt the strict sensor reading timing[cite: 185]. [cite_start]Core 1 acts as a high-priority data producer, sampling the ADC exactly every 1 millisecond, while Core 0 handles the heavy floating-point True RMS math in batches[cite: 186].
* [cite_start]**Dynamic AC Coupling (Solving DC Drift):** Heavy electrical loads cause power supplies to sag, shifting the sensor's physical "zero-point" and creating massive false readings (often spiking >3.00A)[cite: 187]. [cite_start]This code abandons static boot-up calibration and instead dynamically isolates the True RMS AC current using a statistical variance formula (Variance = Mean of Squares - Square of the Mean)[cite: 188]. [cite_start]This allows the math to track the center of the AC wave in real-time, making it entirely immune to power supply voltage drops and DC drift[cite: 189].
* [cite_start]**Metrology-Grade LUT (Solving ADC Non-Linearity):** The ESP32's internal Analog-to-Digital Converter is notoriously non-linear, meaning a single multiplication factor cannot perfectly map a curve from 0A to 10A[cite: 190]. [cite_start]To enforce a strict <1% error limit, this code uses a 6-point Lookup Table (LUT) with Linear Interpolation[cite: 191]. [cite_start]By physically mapping specific raw variance numbers to true multimeter readings, the code mathematically "connects the dots" to smooth out the ESP32's hardware flaws across all loads[cite: 192].
* [cite_start]**Advanced Smoothing & Instant-Off Logic:** The ESP32 ADC is naturally noisy[cite: 193]. [cite_start]The code applies a heavy Exponential Moving Average (EMA) filter to absorb this static and stabilize the output[cite: 194]. [cite_start]To prevent the "parachute effect"—where the displayed number slowly floats down through false readings after the load is turned off—an instant-off bypass snaps the output to 0.00A if the raw current drops below the 0.45A zero-threshold[cite: 195].
* [cite_start]**Headless CSV Logging:** Data is output serially in a millis(), Current format, separated by a comma, for easy continuous background logging into a .csv file[cite: 196].

## 🛠️ Hardware Requirements

* [cite_start]ESP32 Microcontroller[cite: 197].
* [cite_start]WCS1700 Hall-effect Current Sensor[cite: 197].
* [cite_start]Various test loads (e.g., parallel lightbulbs, hairdryer)[cite: 197].
* [cite_start]A highly accurate Digital Multimeter (for initial calibration)[cite: 197].

## ⚙️ Calibration (Crucial Step)

[cite_start]The ESP32's ADC curve varies from chip to chip[cite: 197]. [cite_start]You must calibrate the Lookup Table (LUT) for your specific hardware before use[cite: 198].

1.  [cite_start]Upload the code and open the Serial Monitor[cite: 199]. [cite_start]The monitor prints both the internal RAW_ADC value and the Current_A[cite: 200].
2.  [cite_start]Turn off all loads (0 Bulbs)[cite: 201]. [cite_start]Note the RAW_ADC value[cite: 201].
3.  [cite_start]Turn on 1 Bulb[cite: 201]. [cite_start]Read the exact Amperage on your physical Multimeter[cite: 202]. [cite_start]Then, read the RAW_ADC value in the Serial Monitor[cite: 202].
4.  [cite_start]Repeat this process for higher loads (e.g., 2 Bulbs, 3 Bulbs, Hairdryer Low, Hairdryer High) to gather 6 total data points[cite: 203].
5.  [cite_start]Edit the code: Place the RAW_ADC decimals into the lutRaw array (in strictly increasing order) and the corresponding Multimeter values into the lutActual array[cite: 204].

## 📊 Logging Data to CSV

[cite_start]The Arduino IDE Serial Monitor does not support auto-saving[cite: 205]. [cite_start]To log your data continuously for spreadsheet analysis: [cite: 206]

1.  [cite_start]Download a free Terminal Emulator program like CoolTerm[cite: 206].
2.  [cite_start]Close the Arduino IDE Serial Monitor (the COM port can only be used by one program at a time)[cite: 207].
3.  [cite_start]Open CoolTerm, click Options, select your ESP32's COM Port, and set the Baud Rate to 115200[cite: 208].
4.  [cite_start]Navigate to Connection > Capture to Textfile > Start[cite: 209].
5.  [cite_start]Name your file data.csv and click Connect[cite: 210]. [cite_start]CoolTerm will quietly dump everything the ESP32 calculates directly into the CSV file in the background[cite: 211].
