## Oil temperature gauge

Using a thermistor to measure the oil temperature in a motorcycle and display on a small 128 x 32 OLED, compact prototype using through-hole components for a one-off meter application.

<img src=./docs/images/temp_gauge_schematic.png alt= “” width=60% height=60%>

Modern oils use a number of additives such as detergents, viscosity modifiers, corrosion inhibitors, friction inhibitors which degrade if the oil gets too hot. Old air-cooled motorcycle engines use oil circulation to move heat away from the cylinder head, a hot day riding with a pillion in slow traffic can overheat the oil and engine. 
An engine may also overheat from sticking ignition advance and air leaks into the carburettor manifolds creating an overly lean air/fuel mixture.  Building a gauge to monitor the oil temperature can give advance warning of overheating trouble.

<img src=./docs/images/meter_z650.png alt= “” width=60% height=60%>

### Thermistor 
The temperature sensor is a 1/8" NPT threaded thermistor commonly marketed as a universal automotive temperature sensor for oil or water. In the Kawasaki kz650/z650 motorcycle, there is an oil galley at the rear of the sump (oil pan) and capped with a threaded plug. The earlier kz650/z650 models use a galley plug with an external hexagonal head, allowing a 1/8" Female NPT - 3/8" BSPT Male Thread Coupler Brass adapter to be used with the thermistor. Use PTFE tape with BSPT threads. 

<img src=./docs/images/sensor.png alt= “zip-tied to kz650 handlebars” width=60% height=60%>

Later kz650 models used a 1.25mm pitch 17mm metric threaded plug, identifiable by a recessed allen head, which will require a specialist fitting or tapping an existing plug for a thermistor.
As the threads are insulated in PTFE tape, a 2-wire thermistor is required. The type used here is a widely available negative thermal coefficient or NTC thermistor specified for the range of temperatures encountered in vehicle applications, the exact model type is not critical as the temperature/resistance values are measured and tabulated to create a piecewise linear model that approximates the measured relationship.

<img src=./docs/images/sensor_z650.png alt= “rear sump galley plug” width=60% height=60%>

The resistance of a NTC thermistor decreases with increasing temperature. Using the thermistor as a variable resistance in a voltage divider circuit allows temperature to be measured as voltage according to the formula,

$$V_{temp} = V_{cc}\frac{R_{thermistor}}{(R_{thermistor} + R_{fixed})}$$ 

The characteristics of the thermistor used here was measured in water, where temperature and resistance are simultaneously checked with a thermometer and multimeter as the water cooled from boiling, higher values were obtained with a hairdryer and a multimeter K-type thermocouple. It may be easier to fit a polynomial to values taken at irregular temperature intervals to return estimated values at regular intervals. A spreadsheet (adapted from XXX) calculates the predicted voltage $V_{temp}$, then maps this value to the 16-bit value between the supply voltage and zero. As this build uses cheap components, the value of the fixed resistor and power supply voltage is measured.

### Circuit details
The ATTiny85 can measure voltage with a 10-bit ADC relative to the supply voltage, this value is then oversampled to return a 16 bit integer. The piecewise linear approximation is stored in a flash look-up table to save space, the second modification to reduce memory size is to remove all unused symbols from the stock Adafruit library, font16x32.h to create font16x32temp.h.

The input voltage is dropped from the nominal 12 volt supply to 5 volt using an LM2937-5 low dropout 500mA regulator. This device is touted as resistant to overcurrent, reverse polarity and noisy supply voltage. A ceramic 10uF capacitor is used for output stability and a 100uF electrolytic capacitor is added to smooth irregular device power demand. An 18 volt transient voltage suppressor is added to the input for good luck. A grounded 22uF capacitor is added to the voltage sense input for noise decoupling. The wire from the display unit to the sensor is relatively long so avoid running it parallel to noisy HT ignition or digital tachometer signals. Shielded microphone cable can be used as an additional measure. A generic 0.91" I2C OLED 128x32 display is used, driven by the [Adafruit TinyWireM I2C library](https://github.com/adafruit/TinyWireM). 
Components were mounted on perfboard with the same dimensions as the display, wire loops were soldered to the ends to allow mounting with zip-ties, and the completed unit was encased in translucent heatshrink and the ends plugged with hot glue. 

The meter display would randomly freeze in use. This was remedied with an inline 12v DC-DC converter with a large output capacitor and passive filter network. This also cured glitching of the aftermarket tachometer. Use a ceramic multilayer 10uF capacitor with low ESR for LDO stability.

<img src=./docs/images/board1.png alt= “insulate LDO tab” width=60% height=60%>
<img src=./docs/images/board_rear.png alt= “” width=60% height=60%>
