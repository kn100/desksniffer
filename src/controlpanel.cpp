#include "controlpanel.h"
#include "button.h"

ControlPanel::ControlPanel()
    : up("up", PIN_BUTTON_UP), middle("middle", PIN_BUTTON_MIDDLE), down("down", PIN_BUTTON_DOWN), objToggle(DESK), action({DESK, HALT, 0, 0})
{
}

void ControlPanel::recv()
{
    up.recv();
    middle.recv();
    down.recv();
}

Button ControlPanel::GetMostRecentStateChangeAction() const
{
    if (up.getLastStateChangeTS() > middle.getLastStateChangeTS() && up.getLastStateChangeTS() > down.getLastStateChangeTS())
        return up;
    if (middle.getLastStateChangeTS() > up.getLastStateChangeTS() && middle.getLastStateChangeTS() > down.getLastStateChangeTS())
        return middle;
    return down;
}

Action ControlPanel::getAction()
{
    Button mostRecentStateChangeAction = GetMostRecentStateChangeAction();

    if (mostRecentStateChangeAction.getLastStateChangeTS() == action.time)
        return action;

    String btnPressed = mostRecentStateChangeAction.getName();

    if (btnPressed == "up")
    {
        if (up.getState() == HELD)
            return updateAction({objToggle, UP, 0, up.getLastStateChangeTS()});
        if (up.getState() == DOUBLE_PRESS)
            return updateAction({objToggle, UPBY, 320, up.getLastStateChangeTS()});
        return updateAction({objToggle, HALT, 0, up.getLastStateChangeTS()});
    }
    else if (btnPressed == "middle")
    {
        if (middle.getState() == SINGLE_PRESS)
        {
            toggleObj();
            return updateAction({objToggle, TOGGLE, 0, middle.getLastStateChangeTS()});
        }
        return updateAction({objToggle, HALT, 0, middle.getLastStateChangeTS()});
    }
    else if (btnPressed == "down")
    {
        if (down.getState() == HELD)
            return updateAction({objToggle, DOWN, 0, down.getLastStateChangeTS()});
        if (down.getState() == DOUBLE_PRESS)
            return updateAction({objToggle, DOWNBY, 320, down.getLastStateChangeTS()});
        return updateAction({objToggle, HALT, 0, down.getLastStateChangeTS()});
    }
    return action;
}

Action ControlPanel::updateAction(Action newAction)
{
    Serial.printf("Updating action to %d\n", newAction.command);
    action = newAction;
    return action;
}

void ControlPanel::toggleObj()
{
    if (objToggle == DESK)
    {
        objToggle = LIGHTS;
        return;
    }
    objToggle = DESK;
}

String ControlPanel::string() const
{
    return up.string() + " / " + middle.string() + " / " + down.string();
}