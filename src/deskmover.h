#ifndef DESKMOVER
#define DESKMOVER
#include <Arduino.h>

class DeskMover
{
public:
    DeskMover();
    bool handleManualMovement(bool manualUp, bool manualDown);
    bool requestHeight(uint16_t currHeight, uint16_t reqHeight);
    void wakeDesk();
    bool haltMovement();
private:
    uint16_t requestedHeight;
    bool moveTickCycle;
    uint16_t prevHeight;
    bool requestedMove;
    void moveDesk(bool slow, int pin);
    bool moveDeskUp();
    bool moveDeskDown();
    bool deskIsMoving(uint16_t currHeight);

};
#endif