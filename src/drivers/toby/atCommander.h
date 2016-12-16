/**
* @file     atCommander.h
*
* @brief    Initialize the Toby Modul and handles the communication
* @author   Marc Brändli
* @date     12.12.2016
*/


#ifndef atCommander_h
#define atCommander_h

#include "tobyRingBuffer.h"
#include "pingPongBuffer.h"
#include "tobyDataPipe.h"


#include "tobyDevice.h"
#include "tobyDeviceUart.h"


//for readAtfromSD function
#define SD_CARD_PATH       "/fs/microsd/toby/at-inits.txt"

struct threadParameter //TODO rename
{
    TobyDevice* myDevice;
    TobyRingBuffer* readBuffer;
    TobyDataPipe* readPipeBuffer;
    volatile bool* threadExitSignal;

};



class atCommander

{
public:
    //event Signals for the FSM
    enum Event{
        evReadDataAvailable,
        evWriteDataAvailable,
        evInitOk,
        evInitFail,
        evStart,
        evInit,
        evShutDown
    };

    /**
     * @brief
     *
     * @param
     * @return
     */
    atCommander(TobyDevice* tobyDevice, TobyRingBuffer* read, TobyRingBuffer* write, PingPongBuffer* write2, TobyDataPipe* write3, TobyDataPipe* read3);

    /**
     * @brief
     *
     * @param
     * @return
     */
    virtual ~atCommander();

    /**
     * @brief
     *
     * @param
     * @return
     */
    void process(Event e);

    /**
     * @brief
     *
     * @param
     * @return
     */
    static void *atCommanderStart(void *arg);

private:
    //States for the FSM
    enum State {
        InitState,
        WriteState,
        ErrorState,
        SetupState
    };

    enum ModuleState {
        DirectLinkMode,
        ATCommandMode
    };
    //Enumeration for readAtfromSD function
    enum{
        MAX_AT_COMMANDS = 20,
        MAX_CHAR_PER_AT_COMMANDS =40,
        READ_BUFFER_LENGHT =100
    };

    //we don't want that this object could be copied
    atCommander(atCommander& other);

    /**
     * @brief Prints the AT-Commands from the parameter
     *
     * @param start-adress from the Command
     */
    void printAtCommands();

    /**
     * @brief returns the lenght of a char-array
     *
     * this function is needed because strleng() doesn't work for us
     *
     * @return lenght of a char-array includes '\r'
     *
     */
    int getAtCommandLenght(const char* atCommand);

    /**
     * @brief Initialized the Toby
     *
     * initialized the Toby Modul with the At-Commands from atCommandSendArray
     *
     * @return true if initialisation was succssesfull
     */
    bool initTobyModul();

    /**
     * @brief read the AT-Commands from the SD-Card
     *
     * read the AT-Commands from the SD-Card into the atCommandSendArray
     *
     * @return Number of AT-Commands, if return value = -1, this means there's no SD card
     */
    bool readAtfromSD();

    /**
     * @brief
     *
     * @param
     * @return
     */
    static void* readWork(void *arg);

    /**
     * @brief Checks the connection to Toby
     *
     *Sends a "AT" and checks if a "AT OK" returns
     *
     * @param times, the pixhawk trys so many times to get a answer from Toby
     */
    bool tobyAlive(int times);

    /**
     * @brief shutDown handles  clear shutdown with the LTE-Modul
     * @return returns true if successful
     */
    int shutDown(void);

    /**
     * @brief shutdownModule does shutDown the LTE-Module, preparing for reinitializing. Do NOT work because
     * a reinitializing on the VAB-600 board is needed (socat reconnect)!!  Just for showing how it could be implemented if there is no
     * board between the LTE-Module and PX4-HW
     * @return true if successful, false if failed : actually, always returns fail
     */
    bool shutdownModule(void);

    /**
     * @brief setDirectLinkMode initialize the LTE-Module to the DiretLinkMode. The Module has to be initialized normally!
     * @return true if success
     */
    bool setDirectLinkMode(void);

    /**
     * @brief setReaderThread set up and starts the reader thread
     * @return true if success
     */
    bool setReaderThread(void);


    State currentState;
    ModuleState moduleState;
    TobyDevice* myDevice;
    TobyRingBuffer* readBuffer;
    TobyRingBuffer* writeBuffer;
    PingPongBuffer* pingPongWriteBuffer;
    TobyDataPipe* dataPipeWriteBuffer;
    TobyDataPipe* dataPipeReadBuffer;

    int buffer_return;
    int write_return;

    pthread_t* atReaderThread;
    threadParameter readerParameters;
    volatile bool readerExitSignal;

    int         numberOfAt;
    char  atCommandSendArray[MAX_AT_COMMANDS][MAX_CHAR_PER_AT_COMMANDS];
    char* atCommandSendp[MAX_AT_COMMANDS];

    char* writeDataCommand;
    char* temporaryBuffer; // delete later, just for step-by-step test's
    char* temporarySendBuffer;

    const char* atEnterCommand;
    const char* atDirectLinkRequest;
    const char* atResponseOk;
    const char* atReadyRequest;
    const char* atDirectLinkOk;
    const char* stringEnd;
    const char* atResetCommand;
    const char* atExitDirectLink;

};












#endif
