#ifndef atCommander_h
#define atCommander_h

#include "boundedBuffer.h"
#include "pingPongBuffer.h"

#include "tobyDevice.h"



class atCommander

{
public:
    atCommander(TobyDevice* device, BoundedBuffer* read, BoundedBuffer* write, PingPongBuffer* write2);
    virtual ~atCommander();

    enum Event{
        evReadDataAvaiable,
        evWriteDataAvaiable,
        evInitOk,
        evInitFail,
        evStart

    };

    void process(Event e);

    static void *atCommanderStart(void *arg);
    bool initTobyModul();


private:

    enum State {
        StopState,
        InitState,
        WaitState,
        ReadState,
        WriteState,
        ErrorState
    };

    State currentState;
    BoundedBuffer* readBuffer;
    BoundedBuffer* writeBuffer;
    PingPongBuffer* pingPongWriteBuffer;

    TobyDevice* myDevice;


    char* writeDataCommand;
    char* temporaryBuffer; // delete later, just for step-by-step test's
    char* commandBuffer; // delete later, just for step-by-step test's

    const char* atCommandSend ;

    const char* atCommandPingPongBufferSend ;


};












#endif
