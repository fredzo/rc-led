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

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip1.numPixels(); i++) { // For each pixel in strip...
    strip1.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip1.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

void colorWipe2(uint32_t color, int wait) {
  for(int i=0; i<strip2.numPixels(); i++) { // For each pixel in strip...
    strip2.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip2.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
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

// Couleurs de base
uint32_t ORANGE = Adafruit_NeoPixel::Color(255,70,0);
uint32_t BLACK = Adafruit_NeoPixel::Color(0,0,0);
uint32_t WHITE = Adafruit_NeoPixel::Color(255,255,255);
uint32_t RED = Adafruit_NeoPixel::Color(255,0,0);

// Fonction générique pour positionner la couleur d'une LED
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

// Fonction générique pour éteindre une LED
void setLedOff(byte led) {
  setLedColor(led,BLACK);
}

//////////// Gestion du recul //////////////
bool backwardNotif = false;

void setBackwardLedsState(bool on)
{
  uint32_t color = on ? WHITE : BLACK;
  setLedColor(STOP_DROIT,color);
  setLedColor(STOP_GAUCHE,color);
}

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
    EasyBuzzer.stop();
    backwardNotif = false;
  }
}

//////////// Gestion du Stop //////////////
bool stopNotif = false;

void setStopLedsState(bool on)
{
  uint32_t color = on ? RED : BLACK;
  setLedColor(STOP_DROIT,color);
  setLedColor(STOP_GAUCHE,color);
}

void braking()
{
  if(!stopNotif)
  {
    setStopLedsState(true);
    stopNotif = true;
  }
}

void stopBraking()
{
  if(stopNotif)
  {
    setStopLedsState(false);
    stopNotif = false;
  }
}

//////////// Gestion des clignotants //////////////

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

//////////// Gestion des yeux //////////////

#define EYES_FADE_TIME    1     // 1 ms
#define EYES_PAUSE_TIME   1000  // 1 s
unsigned long lastEyesFadeTime = 0;
byte eyesFadeLevel = 0;
bool eyesPaused = false;
bool eyesRising = true;

void setEyesLevel(byte level)
{
  uint32_t color = strip1.gamma32(strip1.ColorHSV(0,0xFF,level));
  setLedColor(OEIL_GAUCHE,color);
  setLedColor(OEIL_DROIT,color);
}

void updateEyes()
{
  unsigned long now = millis();
  if(eyesPaused)
  {
    if(now - lastEyesFadeTime >= EYES_PAUSE_TIME)
    {
      eyesPaused = false;
      lastEyesFadeTime = now;
    }
  }
  else
  {
    if(now - lastEyesFadeTime >= EYES_FADE_TIME)
    {
      if(eyesRising)
      {
        eyesFadeLevel++;
        if(eyesFadeLevel >= 0xFF)
        {
          eyesRising = false;
        }
      }
      else
      {
        eyesFadeLevel--;
        if(eyesFadeLevel <= 0)
        {
          eyesRising = true;
          eyesPaused = true;
        }
      }
      setEyesLevel(eyesFadeLevel);
      lastEyesFadeTime = now;
    }
  }
}


//////////// Gestion de la radio //////////////

// Etat des canaux radio
enum ChanelState { LEFT, CENTER, RIGHT, FOWRWARD, STOP, BACKWARD };
// Etat du déplacement (avant / arrière / frein)
enum MotionState { FRONT, BRAKE, BACK, NONE };
// Effets spéciaux
enum SpecialEffect { NO_EFFECT = 0, SIREN = 1 };

// Gestion des piles de commande
#define COMMAND_STACK_SIZE   8
// Pile de commande du canal 1
ChanelState ch1States[COMMAND_STACK_SIZE] = {CENTER, CENTER, CENTER, CENTER, CENTER, CENTER, CENTER, CENTER};
// Pile de commande du canal 2
ChanelState ch2States[COMMAND_STACK_SIZE] = {STOP, STOP, STOP, STOP, STOP, STOP, STOP, STOP};
// Pile de commande commune (ch1 + ch2)
ChanelState chStates[COMMAND_STACK_SIZE] = {CENTER, CENTER, CENTER, CENTER, CENTER, CENTER, CENTER, CENTER};
// Code Konami pour passer en mode réglage
#define KONAMI_CODE_SIZE   5
ChanelState konamiCode[KONAMI_CODE_SIZE] = {LEFT, RIGHT, LEFT, RIGHT, FOWRWARD };

// Etat en cours pour ch1
ChanelState curChan1State = CENTER;
// Etat en cours pour ch2
ChanelState curChan2State = STOP;
// Etat en cours pour le déplacement
MotionState curMotionState = NONE;
// Effet spécial en cours
SpecialEffect curSpecialEffect = NO_EFFECT;

// Changement d'effets spéciaux (up/down)
void changeSpecialEffect(bool up)
{
  if(up)
  {
    curSpecialEffect = (SpecialEffect)(curSpecialEffect + 1);
    if(curSpecialEffect > SIREN)
    {
      curSpecialEffect = NO_EFFECT;
    }
  }
  else
  {
    curSpecialEffect = (SpecialEffect)(curSpecialEffect - 1);
    if(curSpecialEffect < NO_EFFECT)
    {
      curSpecialEffect = SIREN;
    }
  }
  (curSpecialEffect == SIREN) ? EasyBuzzer.siren(true) : EasyBuzzer.stop();
}

// Détection du Konami code
void detectKonamiCode()
{
  for(int i = 0 ; i < KONAMI_CODE_SIZE ; i++)
  {
    if(chStates[i] != konamiCode[i]) return;
  }
  // Konami code found
  changeSpecialEffect(true);
}

// Traitement d'un nouvel état pour le ch1
bool processCh1State(ChanelState ch1State)
{
  if(ch1State != curChan1State)
  {
    curChan1State = ch1State;
    if(ch1State != CENTER)
    { // Only track left and right commands
      for(int i = COMMAND_STACK_SIZE-1; i >=1 ; i--)
      { // Push new command to the stacks
        ch1States[i] = ch1States[i-1];
        chStates[i] = chStates[i-1];
      }
      // Add the new state at the top of the stack
      ch1States[0] = ch1State;
      chStates[0] = ch1State;
      detectKonamiCode();
    }
    return true;
  }
  else
  {
    return false;
  }
}

// Traitement d'un nouvel état pour le ch2
bool processCh2State(ChanelState ch2State)
{
  if(ch2State != curChan2State)
  {
    curChan2State = ch2State;
    if(ch2State != STOP)
    { // Only track fwd and bwd commands
      for(int i = COMMAND_STACK_SIZE-1; i > 0 ; i--)
      { // Push new command to the stacks
        ch2States[i] = ch2States[i-1];
        chStates[i] = chStates[i-1];
      }
      // Add the new state at the top of the stack
      ch2States[0] = ch2State;
      chStates[0] = ch2State;
      detectKonamiCode();
    }
    return true;
  }
  else
  {
    return false;
  }
}

//////////// Lecture PWM des entrées radio ////////////
#define MAX_PULSE_WIDTH     3000 // 3000us
volatile unsigned long ch1StartTime = 0;
volatile unsigned long ch1CurTime = 0;
volatile int ch1PulseWidth = 0;
volatile unsigned long ch2StartTime = 0;
volatile unsigned long ch2CurTime = 0;
volatile int ch2PulseWidth = 0;

void IRAM_ATTR pwmCh1()
{
  ch1CurTime = micros();
  if(ch1CurTime > ch1StartTime)
  {
    unsigned long pulseWidth = ch1CurTime - ch1StartTime;
    if(pulseWidth <= MAX_PULSE_WIDTH)
    { // Only read rise -> fall width
      ch1PulseWidth = pulseWidth;
    }
  }
  ch1StartTime = ch1CurTime;
}

void IRAM_ATTR pwmCh2()
{
  ch2CurTime = micros();
  if(ch2CurTime > ch2StartTime)
  {
    unsigned long pulseWidth = ch2CurTime - ch2StartTime;
    if(pulseWidth <= MAX_PULSE_WIDTH)
    { // Only read rise -> fall width
      ch2PulseWidth = pulseWidth;
    }
  }
  ch2StartTime = ch2CurTime;
}

//////////// Initialisation du programme //////////////

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(CHANEL_1_PIN, INPUT); // Set our input pins for RC chanels
  attachInterrupt(digitalPinToInterrupt(CHANEL_1_PIN), pwmCh1, CHANGE);  // interruption sur Rise et Fall
  pinMode(CHANEL_2_PIN, INPUT); // Set our input pins for RC chanels
  attachInterrupt(digitalPinToInterrupt(CHANEL_2_PIN), pwmCh2, CHANGE);  // interruption sur Rise et Fall
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

//////////// Boucle principale //////////////

#define CHANNEL_DELTA_TRIGGER  10
#define CHANNEL_CENTER_VALUE   1500
#define CHANNEL_THRESHOLD      100

// Button detection
boolean oldButtonState = HIGH;

void loop() {
  // Gestion du buzzer
  EasyBuzzer.update();

  // Animation des yeux
  updateEyes();

  // Lecture cannaux radio (basée sur une interruption)
  int newCh1 = ch1PulseWidth;
  int newCh2 = ch2PulseWidth;
  //int newCh1 = pulseIn(CHANEL_1_PIN, HIGH, 25000); // Lecture canal 1 (gauche / droite)
  //int newCh2 = pulseIn(CHANEL_2_PIN, HIGH, 25000); // Lecture canal 2 (avant / arrière)

  ChanelState newChanelState;

  if(abs(ch1 - newCh1) >= CHANNEL_DELTA_TRIGGER )
  {
    ch1 = newCh1;
#ifdef SERIAL_OUT
    Serial.print("Channel 1:"); // Print the value of 
    Serial.println(ch1);        // each channel
#endif
    if(ch1 < CHANNEL_CENTER_VALUE - CHANNEL_THRESHOLD)
    {
      newChanelState = RIGHT;
    }
    else if(ch1 > CHANNEL_CENTER_VALUE + CHANNEL_THRESHOLD)
    {
      newChanelState = LEFT;
    }
    else
    {
      newChanelState = CENTER;
    }
    processCh1State(newChanelState);
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
      newChanelState = BACKWARD;
    }
    else if(ch2 > CHANNEL_CENTER_VALUE + CHANNEL_THRESHOLD)
    {
      newChanelState = FOWRWARD;
    }
    else
    {
      newChanelState = STOP;
    }
    if(processCh2State(newChanelState))
    { // State changed
      switch(curChan2State)
      {
        case BACKWARD :
          if(ch2States[1] == FOWRWARD)
          {
            curMotionState = BRAKE;
          }
          else
          {
            curMotionState = BACK;
          }
          break;
        case FOWRWARD :
          curMotionState = FRONT;
          break;
        case STOP :
        default :
          if(curMotionState != BRAKE)
          { // Keep braking state when trigger is released
            curMotionState = NONE;
          }
          break;
      }
    }
  }

  switch(curChan1State)
  {
    case LEFT :
      blinkRight(false);
      blinkLeft(true);
      break;
    case RIGHT :
      blinkLeft(false);
      blinkRight(true);
      break;
    case CENTER :
    default :
      blinkLeft(false);
      blinkRight(false);
      break;
  }

  if(curMotionState == BACK)
  {
    backward();
  }
  else
  {
    stopBackward();
  }

  if(curMotionState == BRAKE)
  {
    braking();
  }
  else
  {
    stopBraking();
  }


  // Get current button state.
  boolean newButtonState = digitalRead(BUTTON_PIN);

  // Check if state changed from high to low (button press).
  if((newButtonState == LOW) && (oldButtonState == HIGH)) {
    // Short delay to debounce button.
    delay(20);
    // Check if button is still low after debounce.
    newButtonState = digitalRead(BUTTON_PIN);
    if(newButtonState == LOW) {      // Yes, still low
        changeSpecialEffect(true);
    }
  }
  
  // Set the last-read button state to the old state.
  oldButtonState = newButtonState;

}

