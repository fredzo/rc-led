#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <RcLedConf.h>
#include <Adafruit_NeoPixel.h>

class LedManager
{
    public :

        enum Led { 
            // Bandeau 1
            RIGHT_TURN_SIGNAL=0x00, RIGHT_HEADLIGHT=0x01, LEFT_HEADLIGHT=0x02, LEFT_TURN_SIGNAL=0x03, LEFT_EYE=0x04, RIGHT_EYE=0x05,  GYROPHARE_1=0x06, GYROPHARE_2=0x07, GYROPHARE_3=0x08,
            // Bandeau 2
            BACK_RIGHT_TURN_SIGNAL1=0x10, BACK_RIGHT_TURN_SIGNAL2=0x11, RIGHT_STOP=0x12, LEFT_STOP=0x13, BACK_LEFT_TURN_SIGNAL2=0x14, BACK_LEFT_TURN_SIGNAL1=0x15
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