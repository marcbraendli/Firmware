#ifndef atCommander_h
#define atCommander_h

#include "boundedBuffer.h"
#include "tobyDevice.h"



class atCommander

{
public:
    atCommander(TobyDevice* device, BoundedBuffer* read, BoundedBuffer* write);
    virtual ~atCommander();

    enum Event{
        evReadDataAvaiable,
        evWriteDataAvaiable,
        evInitOk,
        evInitFail

    };

    void process(Event e);

    static void *atCommanderStart(void *arg);



private:

    enum State {
        StopState,
        InitState,
        WaitState,
        ReadState,
        WriteState
    };

    State currentState;
    BoundedBuffer* readBuffer;
    BoundedBuffer* writeBuffer;
    TobyDevice* myDevice;

    char* temporaryBuffer; // delete later, just for step-by-step test's

};












#endif
