#ifndef RC_LED_CONF_H
#define RC_LED_CONF_H

// Pin for button
#define BUTTON_PIN        0
// Pin for Buzzer
#define BUZZER_PIN        25
// Pin for led strip 1
#define PIXEL_PIN_1       33  // Digital IO pin connected to the NeoPixels.
// Pin for led srip 2
#define PIXEL_PIN_2       32  // Digital IO pin connected to the NeoPixels.
// Number of leds on led strip 1
#define PIXEL_COUNT_1     9 // Number of NeoPixels
// Number of leds on led strip 2
#define PIXEL_COUNT_2     6 // Number of NeoPixels

// Input pin for radio chanel 1 PWM
#define CHANEL_1_PIN      27
// Input pin for radio chanel 2 PWM
#define CHANEL_2_PIN      26

// Uncomment to enable logs on serial out
// #define SERIAL_OUT

#endif