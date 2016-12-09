#ifndef atCommander_h
#define atCommander_h

#include "boundedBuffer.h"
#include "pingPongBuffer.h"

#include "tobyDevice.h"

struct threadParameter //TODO rename
{
    TobyDevice* myDevice;
    BoundedBuffer* readBuffer;

};



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

    static void* readWork(void *arg);
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

    pthread_t* atReaderThread;
    threadParameter readerParameters;


    char* writeDataCommand;
    char* temporaryBuffer; // delete later, just for step-by-step test's
    char* commandBuffer; // delete later, just for step-by-step test's
    char* temporarySendBuffer;


    const char* atCommandSend ;
    const char* atEnterCommand;
    const char* atCommandPingPongBufferSend ;
    const char* atDirectLinkCommand;


};












#endif
