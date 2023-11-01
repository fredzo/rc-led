#include <Radio.h>

#include <Arduino.h>

// Gestion des piles de commande
#define COMMAND_STACK_SIZE   8
// Pile de commande du canal 1
ChannelState ch1States[COMMAND_STACK_SIZE] = {CENTER, CENTER, CENTER, CENTER, CENTER, CENTER, CENTER, CENTER};
// Pile de commande du canal 2
ChannelState ch2States[COMMAND_STACK_SIZE] = {STOP, STOP, STOP, STOP, STOP, STOP, STOP, STOP};
// Pile de commande commune (ch1 + ch2)
ChannelState chStates[COMMAND_STACK_SIZE] = {CENTER, CENTER, CENTER, CENTER, CENTER, CENTER, CENTER, CENTER};
// Code Konami pour passer en mode réglage
#define KONAMI_CODE_SIZE   5
ChannelState konamiCode[KONAMI_CODE_SIZE] = {LEFT, RIGHT, LEFT, RIGHT, FOWRWARD };

// Etat en cours pour ch1
ChannelState curChan1State = CENTER;
// Etat en cours pour ch2
ChannelState curChan2State = STOP;
// Etat en cours pour le déplacement
MotionState curMotionState = NONE;

ChannelState radioGetChannel1State()
{
  return curChan1State;
}

MotionState radioGetMotionState()
{
  return curMotionState;
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

static EffectManager* effectManager;

// Initialisation du module radio
void radioInit(EffectManager* effectManagerParam)
{
  pinMode(CHANEL_1_PIN, INPUT); // Set our input pins for RC chanels
  attachInterrupt(digitalPinToInterrupt(CHANEL_1_PIN), pwmCh1, CHANGE);  // interruption sur Rise et Fall
  pinMode(CHANEL_2_PIN, INPUT); // Set our input pins for RC chanels
  attachInterrupt(digitalPinToInterrupt(CHANEL_2_PIN), pwmCh2, CHANGE);  // interruption sur Rise et Fall
  effectManager = effectManagerParam;
}

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
bool processCh1State(ChannelState ch1State)
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
bool processCh2State(ChannelState ch2State)
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

//////// Traitement des cannaux ///////

#define CHANNEL_DELTA_TRIGGER  10
#define CHANNEL_CENTER_VALUE   1500
#define CHANNEL_THRESHOLD      100

int ch1=-1; // Here's where we'll keep our channel values
int ch2=-1;

void radioProcessSignal()
{
  // Lecture cannaux radio (basée sur une interruption)
  int newCh1 = ch1PulseWidth; // Lecture canal 1 (gauche / droite)
  int newCh2 = ch2PulseWidth; // Lecture canal 2 (avant / arrière)

  ChannelState newChanelState;

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
}