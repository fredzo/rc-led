#include <EffectManager.h>
#include <EasyBuzzer.h>

EffectManager *EffectManager::effectManagerInstance = nullptr;


void EffectManager::init(LedManager* ledManager)
{
  EffectManager::ledManager = ledManager;
}

// Changement d'effets spéciaux (up/down)
void EffectManager::changeSpecialEffect(bool up)
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

bool EffectManager::specialEffectHasSound()
{
  return (curSpecialEffect == SIREN);
}

byte EffectManager::updateGyroFadeLeve(byte gyroFadeLevel)
{
  gyroFadeLevel++;
  if(gyroFadeLevel > GYRO_MAX_LEVEL)
  {
    gyroFadeLevel = 0;
  }
  return gyroFadeLevel;
}

void EffectManager::updateEffects()
{
  updateEyes();
  updateSpecialEffects();
}

void EffectManager::updateEyes()
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
      ledManager->setEyesLevel(eyesFadeLevel);
      lastEyesFadeTime = now;
    }
  }
}

byte EffectManager::sirenSequence[] = {0,1,2,0,1,2,0,1,2,3,8,3,8,4,8,4,8,5,8,5,8,6,7,6,7};

void EffectManager::updateSpecialEffects()
{
  unsigned long now = millis();
  if(curSpecialEffect == SIREN)
  {
    if(now - lastEffectUpdateTime >= SIREN_STROBO_DURATION)
    {
      lastEffectUpdateTime = now;
      switch(sirenSequence[sirenSequenceIndex])
      {
        case 0:
          ledManager->setLedColor(LedManager::Led::GYROPHARE_1,LedManager::RED);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_2,LedManager::BLACK);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_3,LedManager::BLACK);
          break;
        case 1:
          ledManager->setLedColor(LedManager::Led::GYROPHARE_1,LedManager::BLACK);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_2,LedManager::WHITE);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_3,LedManager::BLACK);
          break;
        case 2:
          ledManager->setLedColor(LedManager::Led::GYROPHARE_1,LedManager::BLACK);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_2,LedManager::BLACK);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_3,LedManager::BLUE);
          break;
        case 3:
          ledManager->setLedColor(LedManager::Led::GYROPHARE_1,LedManager::RED);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_2,LedManager::RED);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_3,LedManager::RED);
          break;
        case 4:
          ledManager->setLedColor(LedManager::Led::GYROPHARE_1,LedManager::WHITE);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_2,LedManager::WHITE);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_3,LedManager::WHITE);
          break;
        case 5:
          ledManager->setLedColor(LedManager::Led::GYROPHARE_1,LedManager::BLUE);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_2,LedManager::BLUE);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_3,LedManager::BLUE);
          break;
        case 6:
          ledManager->setLedColor(LedManager::Led::GYROPHARE_1,LedManager::RED);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_2,LedManager::RED);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_3,LedManager::BLACK);
          break;
        case 7:
          ledManager->setLedColor(LedManager::Led::GYROPHARE_1,LedManager::BLACK);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_2,LedManager::BLUE);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_3,LedManager::BLUE);
          break;
        case 8:
        default:
          ledManager->setLedColor(LedManager::Led::GYROPHARE_1,LedManager::BLACK);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_2,LedManager::BLACK);
          ledManager->setLedColor(LedManager::Led::GYROPHARE_3,LedManager::BLACK);
          break;
      }
      sirenSequenceIndex++;
      if(sirenSequenceIndex >= SIREN_SEQUENCE_SIZE)
      {
        sirenSequenceIndex = 0;
      }
    }
  }
  else
  {
    if(now - lastEffectUpdateTime >= GYRO_FADE_TIME)
    {
      gyro1FadeLevel = updateGyroFadeLeve(gyro1FadeLevel);
      gyro2FadeLevel = updateGyroFadeLeve(gyro2FadeLevel);
      gyro3FadeLevel = updateGyroFadeLeve(gyro3FadeLevel);
      uint32_t color1 = ledManager->colorHSV(7140,0xFF,gyro1FadeLevel);
      uint32_t color2 = ledManager->colorHSV(7140,0xFF,gyro2FadeLevel);
      uint32_t color3 = ledManager->colorHSV(7140,0xFF,gyro3FadeLevel);
      ledManager->setLedColor(LedManager::Led::GYROPHARE_1,color1);
      ledManager->setLedColor(LedManager::Led::GYROPHARE_2,color2);
      ledManager->setLedColor(LedManager::Led::GYROPHARE_3,color3);
      lastEffectUpdateTime = now;
    }
  }
}