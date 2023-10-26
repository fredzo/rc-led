#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <EasyBuzzer.h>

// Digital IO pin connected to the button. This will be driven with a
// pull-up resistor so the switch pulls the pin to ground momentarily.
// On a high -> low transition the button press logic will execute.
#define BUTTON_PIN        0
#define BUZZER_PIN        25

#define PIXEL_PIN_1       33  // Digital IO pin connected to the NeoPixels.
#define PIXEL_PIN_2       32  // Digital IO pin connected to the NeoPixels.

#define CHANEL_1_PIN      27
#define CHANEL_2_PIN      26

#define PIXEL_COUNT_1     9 // Number of NeoPixels
#define PIXEL_COUNT_2     6 // Number of NeoPixels

// Bandeau 1
#define CLIGNOTANT_DROIT      0x00
#define PHARE_DROIT           0x01
#define PHARE_GAUCHE          0x02
#define CLIGNOTANT_GAUCHE     0x03
#define OEIL_GAUCHE           0x04
#define OEIL_DROIT            0x05
#define GYROPHARE_1           0x06
#define GYROPHARE_2           0x07
#define GYROPHARE_3           0x08
// Bandeau 2
#define CLIGNOTANT_AR_DROIT1  0x10
#define CLIGNOTANT_AR_DROIT2  0x11
#define STOP_DROIT            0x12
#define STOP_GAUCHE           0x13
#define CLIGNOTANT_AR_GAUCHE2 0x14
#define CLIGNOTANT_AR_GAUCHE1 0x15

//#define SERIAL_OUT

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip1(PIXEL_COUNT_1, PIXEL_PIN_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(PIXEL_COUNT_2, PIXEL_PIN_2, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

boolean oldState = HIGH;
int     mode     = 0;    // Currently-active animation mode, 0-9


// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip1.numPixels(); i++) { // For each pixel in strip...
    strip1.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip1.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe2(uint32_t color, int wait) {
  for(int i=0; i<strip2.numPixels(); i++) { // For each pixel in strip...
    strip2.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip2.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip1.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip1.numPixels(); c += 3) {
        strip1.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip1.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 3 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 3*65536. Adding 256 to firstPixelHue each time
  // means we'll make 3*65536/256 = 768 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 3*65536; firstPixelHue += 256) {
    for(int i=0; i<strip1.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip1.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip1.setPixelColor(i, strip1.gamma32(strip1.ColorHSV(pixelHue)));
    }
    strip1.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip1.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip1.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip1.numPixels();
        uint32_t color = strip1.gamma32(strip1.ColorHSV(hue)); // hue -> RGB
        strip1.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip1.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}

uint32_t ORANGE = Adafruit_NeoPixel::Color(255,70,0);
uint32_t BLACK = Adafruit_NeoPixel::Color(0,0,0);
uint32_t WHITE = Adafruit_NeoPixel::Color(255,255,255);
uint32_t RED = Adafruit_NeoPixel::Color(255,0,0);

void setLedColor(byte led, uint32_t color) {
  if(led < 0x10)
  { // Badeau 1
    strip1.setPixelColor(led, color);       //  Set pixel's color (in RAM)
    strip1.show();                          //  Update strip to match
  }
  else
  { // Bandeau 2
    led = led & 0x0F;
    strip2.setPixelColor(led, color);       //  Set pixel's color (in RAM)
    strip2.show();                          //  Update strip to match
  }
}

void setLedOff(byte led) {
  setLedColor(led,BLACK);
}

void setBackwardLedsState(bool on)
{
  uint32_t color = on ? WHITE : BLACK;
  setLedColor(CLIGNOTANT_AR_DROIT2,color);
  setLedColor(CLIGNOTANT_AR_GAUCHE2,color);
}

bool backwardNotif = false;
void backward()
{
  if(!backwardNotif)
  {
    setBackwardLedsState(true);
    // Use squence number = 0 for infinite sequence
    // Pause duration must not be 0 (infinite beep) so we set off duration at 0 and pause duration at 500 ms 
    EasyBuzzer.beep(880,500,0,1,500,0);
    backwardNotif = true;
  }
}

void stopBackward()
{
  if(backwardNotif)
  {
    setBackwardLedsState(false);
    EasyBuzzer.stopBeep();
    backwardNotif = false;
  }
}

void setStopLedsState(bool on)
{
  uint32_t color = on ? RED : BLACK;
  setLedColor(STOP_DROIT,color);
  setLedColor(STOP_GAUCHE,color);
}

bool stopNotif = false;
void stop()
{
  if(!stopNotif)
  {
    setStopLedsState(true);
    stopNotif = true;
  }
}

void stopStop()
{
  if(stopNotif)
  {
    setStopLedsState(false);
    stopNotif = false;
  }
}



#define BLINK_TIME      500 // 500 ms
unsigned long lastBlinkTime;
bool lastBlinkState = false;
bool blinkingLeft = false;
bool blinkingRight = false;

void setLeftLedsState(bool on)
{
  uint32_t color = on ? ORANGE : BLACK;
  setLedColor(CLIGNOTANT_GAUCHE,color);
  setLedColor(CLIGNOTANT_AR_GAUCHE1,color);
  setLedColor(CLIGNOTANT_AR_GAUCHE2,color);
}

void setRightLedsState(bool on)
{
  uint32_t color = on ? ORANGE : BLACK;
  setLedColor(CLIGNOTANT_DROIT,color);
  setLedColor(CLIGNOTANT_AR_DROIT1,color);
  setLedColor(CLIGNOTANT_AR_DROIT2,color);
}

void blinkLeft(bool on)
{
  if(on)
  {
    blinkingLeft = true;
    unsigned long now = millis();
    if(now-lastBlinkTime > BLINK_TIME)
    {
      lastBlinkTime = now;
      lastBlinkState = !lastBlinkState;
      setLeftLedsState(lastBlinkState);
      if(!backwardNotif) lastBlinkState ? EasyBuzzer.singleBeep(880,5) : EasyBuzzer.singleBeep(440,5);
    }
  }
  else if(blinkingLeft)
  {
      setLeftLedsState(false);
      blinkingLeft = false;
      lastBlinkState = false;
      lastBlinkTime = 0;
  }
}

void blinkRight(bool on)
{
  if(on)
  {
    blinkingRight = true;
    unsigned long now = millis();
    if(now-lastBlinkTime > BLINK_TIME)
    {
      lastBlinkTime = now;
      lastBlinkState = !lastBlinkState;
      setRightLedsState(lastBlinkState);
      if(!backwardNotif) lastBlinkState ? EasyBuzzer.singleBeep(880,5) : EasyBuzzer.singleBeep(440,5);
    }
  }
  else if(blinkingRight)
  {
      setRightLedsState(false);
      blinkingRight = false;
      lastBlinkState = false;
      lastBlinkTime = 0;
  }
}


void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(CHANEL_1_PIN, INPUT); // Set our input pins for RC chanels
  pinMode(CHANEL_2_PIN, INPUT); // Set our input pins for RC chanels
  EasyBuzzer.setPin(BUZZER_PIN);

#ifdef SERIAL_OUT
  Serial.begin(115200);
  Serial.println("Démarrage RC LED !");
#endif
  strip1.begin(); // Initialize NeoPixel strip object (REQUIRED)
  strip1.show();  // Initialize all pixels to 'off'
  colorWipe(strip1.Color(255,   255,   255), 200);    // White
  strip2.begin(); // Initialize NeoPixel strip object (REQUIRED)
  strip2.show();  // Initialize all pixels to 'off'
  colorWipe2(strip2.Color(255,   0,   0), 200);    // White

  EasyBuzzer.singleBeep(440,500);
  colorWipe2(strip2.Color(0,   0,   0), 0);    // Turn off back leds
}

int ch1=-1; // Here's where we'll keep our channel values
int ch2=-1;

#define CHANNEL_DELTA_TRIGGER  10
#define CHANNEL_CENTER_VALUE   1500
#define CHANNEL_THRESHOLD      100

bool goLeft = false;
bool goRight = false;
bool braking = false;
bool goBackward = false;
bool goForward = false;

byte volume = 15;

void loop() {
  // Gestion du buzzer
  EasyBuzzer.update();

  // Lecture cannaux radio
  int newCh1 = pulseIn(CHANEL_1_PIN, HIGH, 25000); // Lecture canal 1 (gauche / droite)
  int newCh2 = pulseIn(CHANEL_2_PIN, HIGH, 25000); // Lecture canal 2 (avant / arrière)

  if(abs(ch1 - newCh1) >= CHANNEL_DELTA_TRIGGER )
  {
    ch1 = newCh1;
#ifdef SERIAL_OUT
    Serial.print("Channel 1:"); // Print the value of 
    Serial.println(ch1);        // each channel
#endif
    if(ch1 < CHANNEL_CENTER_VALUE - CHANNEL_THRESHOLD)
    {
      goLeft = false;
      goRight = true;
    }
    else if(ch1 > CHANNEL_CENTER_VALUE + CHANNEL_THRESHOLD)
    {
      goLeft = true;
      goRight = false;
    }
    else
    {
      goLeft = false;
      goRight = false;
    }
  }

  if(abs(ch2 - newCh2) >= CHANNEL_DELTA_TRIGGER )
  {
    ch2 = newCh2;
#ifdef SERIAL_OUT
    Serial.print("Channel 2:"); // Print the value of 
    Serial.println(ch2);        // each channel
#endif
    if(ch2 < CHANNEL_CENTER_VALUE - CHANNEL_THRESHOLD)
    {
      if(goForward)
      {
        goForward = false;
        braking = true;
        goBackward = false;
      }
      else
      {
        goForward = false;
        braking = false;
        goBackward = true;
      }
    }
    else if(ch2 > CHANNEL_CENTER_VALUE + CHANNEL_THRESHOLD)
    {
        goForward = true;
        braking = false;
        goBackward = false;
    }
/*  else
    {
      if(goForward)
      {
        goForward = false;
        braking = true;
        goBackward = false;
      }
      else
      {
        goForward = false;
        braking = false;
        goBackward = false;
      }
    }*/
  }

  if(goLeft)
  {
    blinkRight(false);
    blinkLeft(true);
  }
  else if(goRight)
  {
    blinkLeft(false);
    blinkRight(true);
  }
  else
  {
    blinkLeft(false);
    blinkRight(false);
  }

  if(goBackward)
  {
    backward();
  }
  else
  {
    stopBackward();
  }

  if(braking)
  {
    stop();
  }
  else
  {
    stopStop();
  }


  // Get current button state.
  boolean newState = digitalRead(BUTTON_PIN);

  // Check if state changed from high to low (button press).
  if((newState == LOW) && (oldState == HIGH)) {
    // Short delay to debounce button.
    delay(20);
    // Check if button is still low after debounce.
    newState = digitalRead(BUTTON_PIN);
    if(newState == LOW) {      // Yes, still low
        goBackward = !goBackward;
/*      EasyBuzzer.setVolume(volume);
      EasyBuzzer.singleBeep(880,1000);
#ifdef SERIAL_OUT
    Serial.print("Volume set to :"); // Print the value of 
    Serial.println(volume);        // each channel
#endif
      volume+=10; */
    }
  }

  // Set the last-read button state to the old state.
  oldState = newState;

}

