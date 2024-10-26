#include "buttonevtbuffer.h"
#include <Arduino.h>

ButtonEvtBuffer::ButtonEvtBuffer() : head(0) {
    for (int i = 0; i < 4; i++) {
        buffer[i] = ButtonStateChangeEvent{false, 0};
    }
}

bool ButtonEvtBuffer::add(ButtonStateChangeEvent value) {
    // Do not add the same value twice
    if (buffer[(head - 1 + 4) % 4].state == value.state) {
        return false;
    }
    buffer[head] = value;
    head = (head + 1) % 4; 
    return true;
}

ButtonStateChangeEvent ButtonEvtBuffer::get(int index) const {
    return buffer[(head - 1 - index + 4) % 4];
}

unsigned long ButtonEvtBuffer::mostRecentStateTS(bool state) const {
    for (int i = 0; i < 4; i++) {
        if (buffer[(head - 1 - i + 4) % 4].state == state) {
            return buffer[(head - 1 - i + 4) % 4].time;
        }
    }
    return 0;
}

String ButtonEvtBuffer::string() const {
    String str = "";
    for (int i = 0; i < 4; i++) {
        str += buffer[(head + i) % 4].state ? "1" : "0";
    }
    return str;
}