// Oil temp gauge display, squeezed into ATtiny85 memory
// Code license LGPL

#include <TinyWireM.h> // Adafruit's TinyWireM library

// To use the TinyI2C library from https://github.com/technoblogy/tiny-i2c
//#include <TinyI2CMaster.h>

// The blue OLED screen requires a long initialization on power on.
// The code to wait for it to be ready uses 20 bytes of program storage space
// If you are using a white OLED, this can be reclaimed by uncommenting
// the following line (before including Tiny4kOLED.h):
#define TINY4KOLED_QUICK_BEGIN

#include <Tiny4kOLED.h>
#include "font16x32temp.h" // customised font16x32temp 

/************************************************
 * Lookup tables and interpolation to convert
 * Voltage to temperature using a thermistor-fixed resistor divider
 * piecewise linear code lifted from thermistor library written by Benjamin Shaya
 * https://github.com/beshaya/ArduinoThermistor/blob/master/thermistor.cpp
 * 
 * Lookup tables are input voltages scaled to 16 bits (note ATTiny85 10 bit A2D)
 * LUT stored in flash, reducing memory consumption 
 * Requires pgm_read_word_near to obtain values
 ***************************************************************/
 
#define PRECISION 1 //multiplier for C
#define SAMPLES 3 // 2^N samples
#include <avr/pgmspace.h>
#define FLASH(X) pgm_read_word_near(X)

// using 4K7 resistor @5v supply - change these values to suit your own thermistor

const uint16_t LUT[] PROGMEM = { 

	// 4.9v, 9.7-135C, 4670R
	64162, 63763, 63074, 62429, 
	61690, 60961, 60259, 59301, 
	57920, 56238, 54734, 52985, 
	51297, 49377, 47909, 46043, 
	44057, 42541, 40873, 38613, 
	36331, 34305, 31910, 30030, 
	27516, 26298, 26044};


const uint16_t LUT_length = sizeof(LUT)/sizeof(LUT[0]);
const int16_t LUT_offset = 10;
const uint16_t LUT_units = 5;

//Default is 6 (10->16 bits) shift a 10 bit analog value to a 16 bit value
//reduce the shifts applied to inputs for oversampling
//Reduce by 1 for every factor of 2 oversamplings
uint8_t input_shift = 6 - SAMPLES;
const uint16_t SAMPLESpow2 = pow(2, SAMPLES);

//interpolation function
uint16_t interpolate (uint16_t x_lo16, 
                      uint16_t x_hi16, 
                      int32_t y_lo, 
                      int32_t y_hi, 
                      uint16_t x16) {
                        
  //cast to 32 bits unsigned to avoid overflow
  int32_t x_lo = (uint32_t) x_lo16;
  int32_t x_hi = (uint32_t) x_hi16;
  int32_t x = (uint32_t) x16;

  int32_t interpolated = y_lo + ((x - x_lo) * (y_hi - y_lo) / (x_hi - x_lo));
  return (int16_t) ( interpolated );
}

//return degrees C * PRECISION (good to +/- 300C for PRECISION = 100)
int16_t getTemp (uint16_t analog_val, 
                 int16_t offset, 
                 const uint16_t * LUT, 
                 uint16_t LUT_length, 
                 uint16_t units) {
                  
  analog_val = analog_val << input_shift;
  
  // check target is in range  
  if ( analog_val > FLASH(LUT + 0) ) { // too big
      return (offset - 1) * PRECISION;
  }
  
  if ( analog_val < FLASH(LUT + LUT_length-1) ) { // too small
      return (offset + LUT_length * units + 1) * PRECISION;
  }
    
  //binary search to find value
  uint16_t high = LUT_length-1;
  uint16_t low = 0;
  while (high - low > 1) {
    uint16_t mid = (high+low) >> 1;
    uint16_t midval = FLASH(LUT+mid);
    if ( midval == analog_val ) {
        return (mid*units + offset) * PRECISION;
    }
    //y values are in oposite order of x values
    if ( midval > analog_val ) {
      low = mid;
    } else if ( midval < analog_val ) {
      high = mid;
    }
  }

  //interpolate
  int32_t low_temp = (int16_t(low * units + offset)) * PRECISION;
  int32_t high_temp = (int16_t(high*units + offset)) * PRECISION;
  
  return interpolate (FLASH(LUT+low), FLASH(LUT+high), low_temp, high_temp, analog_val);
}

void setup() {
  
  // Send the initialization sequence to the oled. This leaves the display turned off
  oled.begin();

  // Some newer devices do not contain an external current reference.
  // Older devices may also support using the internal curret reference,
  // which provides more consistent brightness across devices.
  // The internal current reference can be configured as either low current, or high current.
  // Using true as the parameter value choses the high current internal current reference,
  // resulting in a brighter display, and a more effective contrast setting.
  //oled.setInternalIref(true);

  oled.setFont(FONT16X32TEMP);

  // Clear the memory before turning on the display
  oled.clear();

  // Turn on the display
  oled.on();

  // Switch the half of RAM that we are writing to, to be the half that is non currently displayed
  oled.switchRenderFrame();
}

void loop() {

  uint16_t analog_val = 0;

  // sample the value 2^SAMPLES times (SAMPLES <= 6), accumulating the value
  for (uint8_t i = 0; i < SAMPLESpow2; i++){
    analog_val += analogRead(2);
  } 
  
  int16_t Celsius = getTemp (analog_val, 
                             LUT_offset, 
                             LUT,
                             LUT_length, 
                             LUT_units);  

  // Clear the half of memory not currently being displayed.
  oled.clear();

  // Position the text cursor
  // In order to keep the library size small, text can only be positioned
  // with the top of the font aligned with one of the four 8 bit high RAM pages.
  // The Y value therefore can only have the value 0, 1, 2, or 3.
  // usage: oled.setCursor(X IN PIXELS, Y IN ROWS OF 8 PIXELS STARTING WITH 0);
  oled.setCursor(8, 0);

  oled.print(Celsius);
  
  // Write text to oled RAM (which is not currently being displayed).
  //oled.print("°C"); // note values are remapped
  oled.print(F("/:"));

  // Swap which half of RAM is being written to, and which half is being displayed.
  // This is equivalent to calling both switchRenderFrame and switchDisplayFrame.
  oled.switchFrame();
                             
  delay(50);
};

// Board: "ATtiny25/45/85"
// Processor: "ATtiny85"
// Clock: "Internal 8MHz"
// Programmer: "Arduino as ISP"

