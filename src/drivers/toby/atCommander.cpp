#include "atCommander.h"
#include "toby.h"

#include "px4_log.h"

extern pthread_cond_t Toby::pollEventSignal;  // has to be public, otherwise we cant use it from at commander
extern pthread_mutex_t Toby::pollingMutex;
int at_command_lenght2(const char* at_command);

atCommander::atCommander(TobyDevice* device, BoundedBuffer* read, BoundedBuffer* write){

    myDevice = device;
    readBuffer = read;
    writeBuffer = write;

    currentState = StopState;       //First test with ready state

    temporaryBuffer = (char*)malloc(62*sizeof(char));

}
atCommander::~atCommander(){


}

void atCommander::process(Event e){

    switch (currentState){

    case StopState:
        PX4_INFO("in StopState");
        if(e == evInitOk){
            initTobyModul();
        }
        break;
    case InitState:
        PX4_INFO("in InitState");
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


    atCommander *atCommanderFSM = new atCommander(atTobyDevice,atReadBuffer,atWriteBuffer);
    atCommanderFSM->process(evInitOk);


    sleep(2);
    return NULL;


    //unrechable , just for test
    //********************************************************************
    int poll_return = 0;
    int write_return = 0; // number data to write
    int read_return = 0;
    char* temporaryBuffer = (char*)malloc(62*sizeof(char));





    while(1){

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


void atCommander::initTobyModul(){

    int poll_return= 0;
    const char* pch = "OK";
    const char *at_command_send[18]={"ATE0\r",
                                                        "AT+IPR=57600\r",
                                                        "AT+CMEE=2\r",
                                                        "AT+CGMR\r",
                                                        "ATI9\r",
                                                        "AT+CPIN=\"4465\"\r",
                                                        "AT+CLCK=\"SC\",2\r",
                                                        "AT+CREG=2\r",
                                                        "AT+CREG=0\r",
                                                        "AT+CSQ\r",
                                                        "AT+UREG?\r",
                                                        "AT+CLCK=\"SC\",2\r",
                                                        "AT+CGMR\r",
                                                        "ATI9\r",
                                                        "at+upsd=0,100,3\r",
                                                        "at+upsda=0,3\r",                       //dangerous command, may we have to check the activated socket, now we code hard!
                                                        "at+uping=\"www.google.ch\"\r",
                                                        "at+usocr=6\r",

                                                       };



    int i = 0;
    while(i < 18){
        myDevice->write(at_command_send[i],at_command_lenght2(at_command_send[i]));
        while(poll_return < 1){
            poll_return = myDevice->poll(NULL,true);
        }
        sleep(1);
        poll_return = myDevice->read(temporaryBuffer,62);
        if(strstr(temporaryBuffer, pch) != 0){
            PX4_INFO("Command Successfull : %s",at_command_send[i]);
            ++i; //sucessfull, otherwise, retry
        }
        else{
            PX4_INFO("Command failed : %s", at_command_send[i]);
        }


       poll_return = 0;
    }
    PX4_INFO("sucessfull init");








}


int at_command_lenght2(const char* at_command)
{
   int k=0;
   while(at_command[k] != '\r')
    {
        k++;
    }

    return k+1;
}
