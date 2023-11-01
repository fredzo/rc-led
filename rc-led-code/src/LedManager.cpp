#include <LedManager.h>

LedManager *LedManager::ledManagerInstance = nullptr;

// Couleurs de base
uint32_t LedManager::ORANGE = Adafruit_NeoPixel::Color(255,70,0);
uint32_t LedManager::BLACK = Adafruit_NeoPixel::Color(0,0,0);
uint32_t LedManager::WHITE = Adafruit_NeoPixel::Color(255,255,255);
uint32_t LedManager::RED = Adafruit_NeoPixel::Color(255,0,0);
uint32_t LedManager::BLUE = Adafruit_NeoPixel::Color(0,0,255);

void LedManager::init()
{
    strip1 = new Adafruit_NeoPixel(PIXEL_COUNT_1, PIXEL_PIN_1, NEO_GRB + NEO_KHZ800);
    strip2 = new Adafruit_NeoPixel(PIXEL_COUNT_2, PIXEL_PIN_2, NEO_GRB + NEO_KHZ800);
    strip1->begin(); // Initialize NeoPixel strip object (REQUIRED)
    strip1->show();  // Initialize all pixels to 'off'
    colorWipe(Strip::STRIP_1, WHITE, 200);    // White
    strip2->begin(); // Initialize NeoPixel strip object (REQUIRED)
    strip2->show();  // Initialize all pixels to 'off'
    colorWipe(Strip::STRIP_2, RED, 200);    // White
}

u_int32_t LedManager::colorHSV(uint16_t hue, uint8_t sat, uint8_t val)
{
    return strip1->gamma32(strip1->ColorHSV(hue,sat,val));
}


// Fonction générique pour positionner la couleur d'une LED
void LedManager::setLedColor(Led led, uint32_t color) {
  if(led < 0x10)
  { // Badeau 1
    strip1->setPixelColor(led, color);       //  Set pixel's color (in RAM)
    strip1->show();                          //  Update strip to match
  }
  else
  { // Bandeau 2
    led = (Led)(led & 0x0F);
    strip2->setPixelColor(led, color);       //  Set pixel's color (in RAM)
    strip2->show();                          //  Update strip to match
  }
}

// Fonction générique pour éteindre une LED
void LedManager::setLedOff(Led led) {
  setLedColor(led,BLACK);
}

void LedManager::setBackwardLedsState(bool on)
{
  uint32_t color = on ? WHITE : BLACK;
  setLedColor(RIGHT_STOP,color);
  setLedColor(LEFT_STOP,color);
}

void LedManager::setStopLedsState(bool on)
{
  uint32_t color = on ? RED : BLACK;
  setLedColor(RIGHT_STOP,color);
  setLedColor(LEFT_STOP,color);
}

void LedManager::setLeftLedsState(bool on)
{
  uint32_t color = on ? ORANGE : BLACK;
  setLedColor(LEFT_TURN_SIGNAL,color);
  setLedColor(BACK_LEFT_TURN_SIGNAL1,color);
  setLedColor(BACK_LEFT_TURN_SIGNAL2,color);
}

void LedManager::setRightLedsState(bool on)
{
  uint32_t color = on ? ORANGE : BLACK;
  setLedColor(RIGHT_TURN_SIGNAL,color);
  setLedColor(BACK_RIGHT_TURN_SIGNAL1,color);
  setLedColor(BACK_RIGHT_TURN_SIGNAL2,color);
}

void LedManager::setEyesLevel(byte level)
{
  uint32_t color = strip1->gamma32(strip1->ColorHSV(0,0xFF,level));
  setLedColor(LEFT_EYE,color);
  setLedColor(RIGHT_EYE,color);
}





void LedManager::colorWipe(Strip stripNumber, uint32_t color, int wait)
{
  Adafruit_NeoPixel* strip = getStrip(stripNumber);
  for(int i=0; i<strip->numPixels(); i++) { // For each pixel in strip...
    strip->setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip->show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

void LedManager::rainbow(Strip stripNumber, int wait)
{
  Adafruit_NeoPixel* strip = getStrip(stripNumber);
  // Hue of first pixel runs 3 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 3*65536. Adding 256 to firstPixelHue each time
  // means we'll make 3*65536/256 = 768 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 3*65536; firstPixelHue += 256) {
    for(int i=0; i<strip->numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip->numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip->setPixelColor(i, strip->gamma32(strip->ColorHSV(pixelHue)));
    }
    strip->show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

Adafruit_NeoPixel* LedManager::getStrip(Strip stripNumber)
{
  return (stripNumber == Strip::STRIP_1) ? strip1 : strip2;
}


