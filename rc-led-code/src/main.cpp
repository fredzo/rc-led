#include <Arduino.h>
#include <RcLedConf.h>
#include <EasyBuzzer.h>
#include <LedManager.h>
#include <EffectManager.h>
#include <Radio.h>

LedManager* ledManager = nullptr;
EffectManager* effectManager = nullptr;

//////////// Gestion du recul //////////////
bool backwardNotif = false;

void backward()
{
  if(!backwardNotif)
  {
    ledManager->setBackwardLedsState(true);
    if(!effectManager->specialEffectHasSound())
    { // Use squence number = 0 for infinite sequence
      // Pause duration must not be 0 (infinite beep) so we set off duration at 0 and pause duration at 500 ms 
      EasyBuzzer.beep(880,500,0,1,500,0);
    }
    backwardNotif = true;
  }
}

void stopBackward()
{
  if(backwardNotif)
  {
    ledManager->setBackwardLedsState(false);
    if(!effectManager->specialEffectHasSound()) EasyBuzzer.stop();
    backwardNotif = false;
  }
}

//////////// Gestion du Stop //////////////
bool stopNotif = false;

void braking()
{
  if(!stopNotif)
  {
    ledManager->setStopLedsState(true);
    stopNotif = true;
  }
}

void stopBraking()
{
  if(stopNotif)
  {
    ledManager->setStopLedsState(false);
    stopNotif = false;
  }
}

//////////// Gestion des clignotants //////////////

#define BLINK_TIME      500 // 500 ms
unsigned long lastBlinkTime;
bool lastBlinkState = false;
bool blinkingLeft = false;
bool blinkingRight = false;

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
      ledManager->setLeftLedsState(lastBlinkState);
      if(!backwardNotif && !effectManager->specialEffectHasSound()) lastBlinkState ? EasyBuzzer.singleBeep(880,50) : EasyBuzzer.singleBeep(440,50);
    }
  }
  else if(blinkingLeft)
  {
      ledManager->setLeftLedsState(false);
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
      ledManager->setRightLedsState(lastBlinkState);
      if(!backwardNotif && !effectManager->specialEffectHasSound()) lastBlinkState ? EasyBuzzer.singleBeep(880,50) : EasyBuzzer.singleBeep(440,50);
    }
  }
  else if(blinkingRight)
  {
      ledManager->setRightLedsState(false);
      blinkingRight = false;
      lastBlinkState = false;
      lastBlinkTime = 0;
  }
}



//////////// Initialisation du programme //////////////

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  EasyBuzzer.setPin(BUZZER_PIN);

  ledManager = LedManager::getLedManager();
  ledManager->init();

  EasyBuzzer.singleBeep(440,500);
  
  ledManager->colorWipe(LedManager::Strip::STRIP_2,LedManager::BLACK,0);    // Turn off back leds

  effectManager = EffectManager::getEffectManager();
  effectManager->init(ledManager);

  radioInit(effectManager);

#ifdef SERIAL_OUT
  Serial.begin(115200);
  Serial.println("Démarrage RC LED !");
#endif
}

//////////// Boucle principale //////////////

// Button detection
boolean oldButtonState = HIGH;

void loop() {
  // Gestion du buzzer
  EasyBuzzer.update();

  // Animation des yeux et effets speciaux
  effectManager->updateEffects();

  // Traitement du signal radio
  radioProcessSignal();


  switch(radioGetChannel1State())
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

  MotionState motionState = radioGetMotionState();
  if(motionState == BRAKE)
  {
    braking();
  }
  else
  {
    stopBraking();
  }


  if(motionState == BACK)
  {
    backward();
  }
  else
  {
    stopBackward();
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
        //Serial.println("Change special effect !");
        effectManager->changeSpecialEffect(true);
    }
  }
  
  // Set the last-read button state to the old state.
  oldButtonState = newButtonState;

}

