//   @slash128 2018
//   CompuNet DC26
//
//   To upload the sketch from the Arduino IDE:
//     Requires the FastLED 2.1 or higher library to be installed in Arduino.
//       The FastLED library can be found by searching under "Sketch/Include Library/Manage Libraries" menu.
//     Set the power switch to "USB" and connect to computer via USB. Note the COM port detected.
//     Under the "Tools/Board" menu select the “Arduino Pro or Pro Mini” option.
//     Under the "Tools/Processor" menu select “ATmega328 (5V, 16MHz)”.
//     Under the "Tools/Port" menu select the COM port detected when plugging in the board to USB.
//     Click the "Upload" button.
//     After the sketch compiles it should upload and the "TX/RX" lights will flash.
//     Once upload is complete the NeoPixels will start to cycle through the patterns.
//
//   Operation:
//     [Press] the SW1 button to cycle through available effects
//     [Press and hold] the SW1 button (one second) to switch between auto and manual mode
//       * Auto Mode (two green blinks): Effects automatically cycle over time per cycleTime
//       * Manual Mode (two red blinks): Effects must be selected manually with SW1 button
//
//     [Press] the SW2 button to cycle through available brightness levels
//     [Press and hold] the SW2 button (one second) to reset brightness to startup value
//
//     Brightness, selected effect, and auto-cycle are saved in EEPROM after a delay
//     The badge will automatically start up with the last-selected settings


// RGB LED data output to LEDs is on pin 5
#define LED_PIN 5

// RGB LED color order (Green/Red/Blue)
#define COLOR_ORDER GRB
#define CHIPSET WS2811

// Global maximum brightness value, maximum 255
#define MAXBRIGHTNESS 64
#define STARTBRIGHTNESS 128

// Cycle time (milliseconds between pattern changes)
#define cycleTime 15000

// Hue time (milliseconds between hue increments)
#define hueTime 30

// Time after changing settings before settings are saved to EEPROM
#define EEPROMDELAY 2000

// Include FastLED library and other useful files
#include <FastLED.h>
#include <EEPROM.h>
#include "XYmatrix.h"
#include "utils.h"
#include "effects.h"
#include "buttons.h"


// list of functions that will be displayed
functionList effectList[] = {	rainbow,
								rainbowWithGlitter,
								confetti,
								sinelon,
								bpm,
								juggle,
								pulsar,
								rotateWhite,
								threeSine,
								threeSineWithGlitter,
								plasma,
								rider,
								glitter,
								colorFill,
								threeDee,
								sideRain,
								confetti2,
								slantBars
								};
								
const byte numEffects = (sizeof(effectList)/sizeof(effectList[0]));

// Runs one time at the start of the program (power up or reset)
void setup() {

  // check to see if EEPROM has been used yet
  // if so, load the stored settings
  byte eepromWasWritten = EEPROM.read(0);
  if (eepromWasWritten == 99) {
    currentEffect = EEPROM.read(1);
    autoCycle = EEPROM.read(2);
    currentBrightness = EEPROM.read(3);
  }

  if (currentEffect > (numEffects - 1)) currentEffect = 0;

  // write FastLED configuration data
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);

  // set global brightness value
  FastLED.setBrightness( scale8(currentBrightness, MAXBRIGHTNESS) );

  // configure input buttons
  pinMode(MODEBUTTON, INPUT_PULLUP);
  pinMode(BRIGHTNESSBUTTON, INPUT_PULLUP);

}


// Runs over and over until power off or reset
void loop()
{
  currentMillis = millis(); // save the current timer value
  updateButtons();          // read, debounce, and process the buttons
  doButtons();              // perform actions based on button state
  checkEEPROM();            // update the EEPROM if necessary

  // switch to a new effect every cycleTime milliseconds
  if (currentMillis - cycleMillis > cycleTime && autoCycle == true) {
    cycleMillis = currentMillis;
    if (++currentEffect >= numEffects) currentEffect = 0; // loop to start of effect list
    effectInit = false; // trigger effect initialization when new effect is selected
  }

  // increment the global hue value every hueTime milliseconds
  if (currentMillis - hueMillis > hueTime) {
    hueMillis = currentMillis;
    hueCycle(1); // increment the global hue value
  }

  // run the currently selected effect every effectDelay milliseconds
  if (currentMillis - effectMillis > effectDelay) {
    effectMillis = currentMillis;
    effectList[currentEffect](); // run the selected effect function
    random16_add_entropy(1); // make the random values a bit more random-ish
  }

  // run a fade effect too if the confetti2 effect is running
  if (effectList[currentEffect] == confetti2) fadeAll(1);

  FastLED.show(); // send the contents of the led memory to the LEDs

}