#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <RcLedConf.h>
#include <Adafruit_NeoPixel.h>

class LedManager
{
    public :

        enum Led { 
            // Bandeau 1
            CLIGNOTANT_DROIT=0x00, PHARE_DROIT=0x01, PHARE_GAUCHE=0x02, CLIGNOTANT_GAUCHE=0x03, OEIL_GAUCHE=0x04, OEIL_DROIT=0x05,  GYROPHARE_1=0x06, GYROPHARE_2=0x07, GYROPHARE_3=0x08,
            // Bandeau 2
            CLIGNOTANT_AR_DROIT1=0x10, CLIGNOTANT_AR_DROIT2=0x11, STOP_DROIT=0x12, STOP_GAUCHE=0x13, CLIGNOTANT_AR_GAUCHE2=0x14, CLIGNOTANT_AR_GAUCHE1=0x15
         };

        enum class Strip { STRIP_1, STRIP_2};

        // Couleurs de base
        static uint32_t ORANGE;
        static uint32_t BLACK;
        static uint32_t WHITE;
        static uint32_t RED;
        static uint32_t BLUE;

        static LedManager *getLedManager()
        {
            if (ledManagerInstance == nullptr) {
                ledManagerInstance=new LedManager();
            }
            return ledManagerInstance;
        }

        void init();

        void setLedColor(Led led, uint32_t color);
        void setLedOff(Led led);

        void setBackwardLedsState(bool on);
        void setStopLedsState(bool on);

        void setLeftLedsState(bool on);
        void setRightLedsState(bool on);

        void setEyesLevel(byte level);

        void colorWipe(Strip strip, uint32_t color, int wait);
        void rainbow(Strip strip, int wait);

        u_int32_t colorHSV(uint16_t hue, uint8_t sat, uint8_t val); 


    private :
        // Declare our NeoPixel strip object:
        Adafruit_NeoPixel *strip1;
        Adafruit_NeoPixel *strip2;

        Adafruit_NeoPixel* getStrip(Strip stripNumber);

        LedManager()
        {
        };

        ~LedManager()
        {
        };

        static LedManager *ledManagerInstance;
};

#endif 