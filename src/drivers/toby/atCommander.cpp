#include "atCommander.h"
#include "toby.h"

#include "px4_log.h"

extern pthread_cond_t Toby::pollEventSignal;  // has to be public, otherwise we cant use it from at commander
extern pthread_mutex_t Toby::pollingMutex;


atCommander::atCommander(TobyDevice* device, BoundedBuffer* read, BoundedBuffer* write){

    myDevice = device;
    readBuffer = read;
    writeBuffer = write;

    currentState = WaitState;       //First test with ready state

    temporaryBuffer = (char*)malloc(62*sizeof(char));

}
atCommander::~atCommander(){


}

void atCommander::process(Event e){

    switch (currentState){

    case StopState:
        PX4_INFO("in StopState");

        break;
    case InitState:
        PX4_INFO("in StopState");
        break;

    case WaitState:
        PX4_INFO("in WaitState");

        if(e == evWriteDataAvaiable){
            PX4_INFO("do some writing");

            int sizeofdata = writeBuffer->getString(temporaryBuffer,62);
            myDevice->write(temporaryBuffer,sizeofdata);

            currentState = WaitState;
        }


        if(e == evReadDataAvaiable){
            PX4_INFO("do some reading");

            int i = myDevice->poll(NULL,0);
            if(i>0){
              int u =  myDevice->read(temporaryBuffer,60);
               readBuffer->putString(temporaryBuffer,u);

            }

            currentState = WaitState;
        }





        break;

    case ReadState:
        PX4_INFO("in ReadState");



        break;

    case WriteState:
        PX4_INFO("in WriteState");
        break;


     default:

        break;


    }


}






//******************************start and intercation Function*******************************

void* atCommander::atCommanderStart(void* arg){

    //eventuell in externen Header, saubere Trennung

    myStruct *arguments = static_cast<myStruct*>(arg);

    BoundedBuffer *atWriteBuffer = arguments->writeBuffer;
    BoundedBuffer *atReadBuffer = arguments->readBuffer;
    TobyDevice *atTobyDevice = arguments->myDevice;


    atCommander *atCommanderFSM = new atCommander(atTobyDevice,atReadBuffer,atWriteBuffer);

    while(1){
 //       atCommanderFSM->process(evWriteDataAvaiable);
  //    usleep(100);


        PX4_INFO("AT_COMMANDER: passing cond_wait");

        atCommanderFSM->process(evReadDataAvaiable);
        usleep(100);


    }



    return NULL;
}

