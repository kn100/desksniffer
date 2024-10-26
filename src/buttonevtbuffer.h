#ifndef BUTTONEVTBUFFER_H
#define BUTTONEVTBUFFER_H
#include <Arduino.h>

struct ButtonStateChangeEvent
{
    bool state;
    unsigned long time;
};

class ButtonEvtBuffer
{
private:
    ButtonStateChangeEvent buffer[4];
    int head;

public:
    ButtonEvtBuffer();
    bool add(ButtonStateChangeEvent e);
    ButtonStateChangeEvent get(int index) const;
    String string() const;
    unsigned long mostRecentStateTS(bool state) const;
};

#endif