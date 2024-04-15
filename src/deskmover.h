#ifndef DESKMOVER
#define DESKMOVER
#include <Arduino.h>

class DeskMover
{
public:
    DeskMover(int upPin, int downPin);
    void initialize();
    bool handle(bool manualUp, bool manualDown, uint16_t currHeight);
    void requestHeight(uint16_t height);
    void wakeDesk();
    void haltMovement();
private:
    uint16_t requestedHeight;
    bool moveTickCycle;
    uint16_t prevHeight;
    int valueSameForNumberOfCycles;
    bool requestedMove;
    int upPin;
    int downPin;
    void moveDesk(bool slow, int pin);
    bool deskIsMoving(uint16_t currHeight);

};
#endif