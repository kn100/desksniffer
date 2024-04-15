#ifndef MANUALCONTROLS
#define MANUALCONTROLS
#include <Arduino.h>
class ManualControls
{
    public: 
        ManualControls(int upPin, int downPin);
        int handleButtons();
    private:
        int upPin;
        int downPin;
};

#endif