#include <Arduino.h>
#include <RcLedConf.h>
#include <EasyBuzzer.h>
#include <LedManager.h>
#include <EffectManager.h>

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

//////////// Gestion de la radio //////////////

// Etat des canaux radio
enum ChanelState { LEFT, CENTER, RIGHT, FOWRWARD, STOP, BACKWARD };
// Etat du déplacement (avant / arrière / frein)
enum MotionState { FRONT, BRAKE, BACK, NONE };

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

// Détection du Konami code
void detectKonamiCode()
{
#ifdef SERIAL_OUT
  Serial.print("Current code :");
  for(int i = KONAMI_CODE_SIZE-1 ; i >= 0 ; i--)
  {
    Serial.print(" ");
    Serial.print((int)chStates[i]);
  }
  Serial.println();
#endif
  for(int i = 0 ; i < KONAMI_CODE_SIZE ; i++)
  { // Check command stack in reverse order since it is a FIFO
    if(chStates[KONAMI_CODE_SIZE-1-i] != konamiCode[i]) return;
  }
  // Konami code found
  effectManager->changeSpecialEffect(true);
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

  ledManager = LedManager::getLedManager();
  ledManager->init();

  EasyBuzzer.singleBeep(440,500);
  
  ledManager->colorWipe(LedManager::Strip::STRIP_2,LedManager::BLACK,0);    // Turn off back leds

  effectManager = EffectManager::getEffectManager();
  effectManager->init(ledManager);

#ifdef SERIAL_OUT
  Serial.begin(115200);
  Serial.println("Démarrage RC LED !");
#endif
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

  // Animation des yeux et effets speciaux
  effectManager->updateEffects();

  // Lecture cannaux radio (basée sur une interruption)
  int newCh1 = ch1PulseWidth; // Lecture canal 1 (gauche / droite)
  int newCh2 = ch2PulseWidth; // Lecture canal 2 (avant / arrière)

  ChanelState newChanelState;

  if(abs(ch1 - newCh1) >= CHANNEL_DELTA_TRIGGER )
  {
    ch1 = newCh1;
#ifdef SERIAL_OUT
    //Serial.print("Channel 1:"); // Print the value of 
    //Serial.println(ch1);        // each channel
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
    //Serial.print("Channel 2:"); // Print the value of 
    //Serial.println(ch2);        // each channel
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
          if(curMotionState != BRAKE && curMotionState != BACK)
          { // Keep braking and back state when trigger is released
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

  if(curMotionState == BRAKE)
  {
    braking();
  }
  else
  {
    stopBraking();
  }


  if(curMotionState == BACK)
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

