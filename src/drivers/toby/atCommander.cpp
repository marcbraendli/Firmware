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

              int u =  myDevice->read(temporaryBuffer,60);
               readBuffer->putString(temporaryBuffer,u);
                if(u>0){
                    PX4_INFO("atCommander : successfull reading");

            }


            usleep(200);
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


  //  atCommander *atCommanderFSM = new atCommander(atTobyDevice,atReadBuffer,atWriteBuffer);
  //  atCommanderFSM->process(evInitOk);
    int poll_return = 0;
    int write_return = 0; // number data to write
    int read_return = 0;
    char* temporaryBuffer = (char*)malloc(62*sizeof(char));

    while(1){
 //       atCommanderFSM->process(evWriteDataAvaiable);
  //    usleep(100);

        /*
        pthread_mutex_lock(&Toby::pollingMutex);

        pthread_cond_wait(&Toby::pollEventSignal,&Toby::pollingMutex);

        PX4_INFO("AT_COMMANDER: passing cond_wait");

        atCommanderFSM->process(evReadDataAvaiable);
       // pthread_mutex_unlock(&Toby::pollingMutex);
        usleep(100);
*/

        poll_return = (atTobyDevice->poll(NULL,true));
        if(poll_return > 0){
           read_return =  atTobyDevice->read(temporaryBuffer,62);
           atReadBuffer->putString(temporaryBuffer,read_return);
        }

        write_return =  atWriteBuffer->getString(temporaryBuffer,62);
        atTobyDevice->write(temporaryBuffer,write_return);




    }



    return NULL;
}

