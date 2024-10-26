#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

// Define button pin numbers
#define PIN_BUTTON_UP 4
#define PIN_BUTTON_DOWN 15
#define PIN_BUTTON_MIDDLE 5
#include "button.h"
#include <Arduino.h>

enum Object {
    DESK,
    LIGHTS
};

enum Command {
    UP,
    UPBY,
    DOWN,
    DOWNBY,
    TOGGLE,
    NONE, // actually more like halt
};

struct Action {
    Object object;
    Command command;
    int value;
    unsigned long time;
};

class ControlPanel {
public:
    ControlPanel();
    void recv();
    Action getAction();
    String string() const;

    
private:
    Button up;
    Button middle;
    Button down;
    Object objToggle;
    Action action = {DESK, NONE, 0, 0};
    Button guard(Button button) const;
    int GetMostRecentStateChangeTS() const;
    Button GetMostRecentStateChangeAction() const;
    void toggleObj();
    void updateAction(Action action);
};

#endif