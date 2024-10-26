#include "controlpanel.h"
#include "button.h"

ControlPanel::ControlPanel()
    : up("up", PIN_BUTTON_UP), middle("middle", PIN_BUTTON_MIDDLE), down("down", PIN_BUTTON_DOWN) {
}

void ControlPanel::recv() {
    up.recv();
    middle.recv();
    down.recv();
}

int ControlPanel::GetMostRecentStateChangeTS() const {
    if (up.getLastStateChangeTime() > middle.getLastStateChangeTime() && up.getLastStateChangeTime() > down.getLastStateChangeTime()) {
        return up.getLastStateChangeTime();
    } else if (middle.getLastStateChangeTime() > up.getLastStateChangeTime() && middle.getLastStateChangeTime() > down.getLastStateChangeTime()) {
        return middle.getLastStateChangeTime();
    } else {
        return down.getLastStateChangeTime();
    }
}

Button ControlPanel::GetMostRecentStateChangeAction() const {
    if (up.getLastStateChangeTime() > middle.getLastStateChangeTime() && up.getLastStateChangeTime() > down.getLastStateChangeTime()) {
        return up;
    } else if (middle.getLastStateChangeTime() > up.getLastStateChangeTime() && middle.getLastStateChangeTime() > down.getLastStateChangeTime()) {
        return middle;
    } else {
        return down;
    }
}

Button ControlPanel::guard(Button button) const {
    if (up.getState() == HELD && down.getState() == HELD) {
        return Button("", 0);
    }

    return button;
}

Action ControlPanel::getAction() {
    objToggle = DESK;
    Button mostRecentStateChangeAction = GetMostRecentStateChangeAction();

    if (mostRecentStateChangeAction.getLastStateChangeTime() == action.time) {
        return action;
    }

    Serial.printf("Most recent state change time: %d, action time: %d\n", mostRecentStateChangeAction.getLastStateChangeTime(), action.time);

    String btnPressed = mostRecentStateChangeAction.getName();

    if (btnPressed == "up") {
        if (up.getState() == HELD) {
            updateAction({objToggle, UP, 0, up.getLastStateChangeTime()});
        } else if (up.getState() == DOUBLE_PRESS) {
            updateAction({objToggle, UPBY, 100, up.getLastStateChangeTime()});
        } else {
            updateAction({objToggle, NONE, 0, up.getLastStateChangeTime()});
        }
        return action;
    } 

    if(btnPressed == "middle") {
        if (middle.getState() == SINGLE_PRESS) {
            toggleObj();
            updateAction({objToggle, TOGGLE, 0, middle.getLastStateChangeTime()});
        } else {
            updateAction({objToggle, NONE, 0, middle.getLastStateChangeTime()});
        }
        return action;
    }

    if (btnPressed == "down") {
        if (down.getState() == HELD) {
            updateAction({objToggle, DOWN, 0, down.getLastStateChangeTime()});
        } else if (down.getState() == DOUBLE_PRESS) {
            updateAction({objToggle, DOWNBY, 100, down.getLastStateChangeTime()});
        } else {
            updateAction({objToggle, NONE, 0, down.getLastStateChangeTime()});
        }
        return action;
    }
    return action;
}

void ControlPanel::updateAction(Action newAction) {
    Serial.printf("Updating action to %d\n", newAction.command);
    action = newAction;
}

void ControlPanel::toggleObj()
{
    if (objToggle == DESK)
    {
        Serial.println("Toggling to lights");
        objToggle = LIGHTS;
    }
    else
    {
        objToggle = DESK;
        Serial.println("Toggling to desk");
    }
}

String ControlPanel::string() const {
    return up.string() + " / " + middle.string() + " / " + down.string();
}