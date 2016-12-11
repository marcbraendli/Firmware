#ifndef atCommander_h
#define atCommander_h

#include "boundedBuffer.h"
#include "pingPongBuffer.h"

#include "tobyDevice.h"

//for readATfromSD
#define PATH       "/fs/microsd/toby/at-inits.txt"

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
        evReadDataAvailable,
        evWriteDataAvailable,
        evInitOk,
        evInitFail,
        evStart

    };

    void process(Event e);

    static void *atCommanderStart(void *arg);



private:

    void printAtCommands();
    int getAtCommandLenght(const char* atCommand);
    bool initTobyModul();
    bool readAtfromSD();

    static void* pollingThreadStart(void *arg);
    static void* readWork(void *arg);
    bool tobyAlive(int times);

    enum State {
        StopState,
        InitState,
        WaitState,
        ReadState,
        WriteState,
        ErrorState,
        SetupState
    };

    //for readATfromSD

    enum{
        MAX_AT_COMMANDS = 20,
        MAX_CHAR_PER_AT_COMMANDS =40,
        READ_BUFFER_LENGHT =100
    };

    State currentState;
    BoundedBuffer* readBuffer;
    BoundedBuffer* writeBuffer;
    PingPongBuffer* pingPongWriteBuffer;

    TobyDevice* myDevice;

    pthread_t* atReaderThread;
    pthread_t* pollingThread;

    threadParameter readerParameters;
    threadParameter pollingThreadParameters;

    char  atCommandSendArray[MAX_AT_COMMANDS][MAX_CHAR_PER_AT_COMMANDS];
    char* atCommandSendp[MAX_AT_COMMANDS];

    char* writeDataCommand;
    char* temporaryBuffer; // delete later, just for step-by-step test's
    char* commandBuffer; // delete later, just for step-by-step test's
    char* temporarySendBuffer;


    int         numberOfAt;
    const char* atCommandSend ;
    const char* atEnterCommand;
    const char* atCommandPingPongBufferSend ;
    const char* atDirectLinkCommand;
    const char* response_at;
    const char* atResponseOk;
    const char* atReadyRequest;
    const char* atDirectLinkOk;
    const char* stringEnd;


};












#endif
