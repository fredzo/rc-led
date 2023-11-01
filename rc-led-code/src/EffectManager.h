#ifndef EFFECT_MANAGER_H
#define EFFECT_MANAGER_H

#include <RcLedConf.h>
#include <LedManager.h>

//////////// Gestion des effets spéciaux ////////////
#define SIREN_STROBO_DURATION 80  // 80 ms
#define GYRO_FADE_TIME        1   // 1 ms
#define GYRO_MAX_LEVEL        255 
//////////// Gestion des yeux //////////////
#define EYES_FADE_TIME    1     // 1 ms
#define EYES_PAUSE_TIME   1000  // 1 s

class EffectManager
{
    public :

        // Effets spéciaux
        enum SpecialEffect { NO_EFFECT = 0, SIREN = 1 };

        static EffectManager *getEffectManager()
        {
            if (effectManagerInstance == nullptr) {
                effectManagerInstance = new EffectManager();
            }
            return effectManagerInstance;
        }

        void init(LedManager* ledManager);

        // Changement d'effets spéciaux (up/down)
        void changeSpecialEffect(bool up);

        bool specialEffectHasSound();

        // A appeler dans la boucle loop() pour mettre à jour l'affichage des effets
        void updateEffects();

    private :
        // Declare our NeoPixel strip object:
        LedManager *ledManager;

        // Effet spécial en cours
        SpecialEffect curSpecialEffect = NO_EFFECT;

        // Gestion des yeux
        unsigned long lastEyesFadeTime = 0;
        byte eyesFadeLevel = 0;
        bool eyesPaused = false;
        bool eyesRising = true;

        void updateEyes();

        // Gestion effets spéciaux
        unsigned long lastEffectUpdateTime = 0;
        byte gyro1FadeLevel = 0;
        byte gyro2FadeLevel = GYRO_MAX_LEVEL/3;
        byte gyro3FadeLevel = 2*GYRO_MAX_LEVEL/3;
        byte sirenSequenceIndex = 0;
        #define SIREN_SEQUENCE_SIZE 25
        static byte sirenSequence[SIREN_SEQUENCE_SIZE];

        byte updateGyroFadeLeve(byte gyroFadeLevel);
        void updateSpecialEffects();

        EffectManager()
        {
        };

        ~EffectManager()
        {
        };

        static EffectManager *effectManagerInstance;
};

#endif 