#include "atCommander.h"
#include "toby.h"


#include "px4_log.h"

extern pthread_cond_t Toby::pollEventSignal;  // has to be public, otherwise we cant use it from at commander
extern pthread_mutex_t Toby::pollingMutex;
int at_command_lenght2(const char* at_command);

atCommander::atCommander(TobyDevice* device, BoundedBuffer* read, BoundedBuffer* write, PingPongBuffer* write2){

    myDevice = device;
    readBuffer = read;
    writeBuffer = write;
    pingPongWriteBuffer = write2;

    currentState = StopState;       //First test with ready state

    temporaryBuffer = (char*)malloc(62*sizeof(char));
    temporarySendBuffer = (char*)malloc(62*sizeof(char));

    commandBuffer = (char*)malloc(15*sizeof(char));
    atCommandSend = "AT+USOWR=0,62\r";
    atCommandPingPongBufferSend = "at+usost=0,\"178.196.15.59\",44444,10,\""; // 37    the value 0,XX depends on the PingPongBuffer::AbsolutBufferLength!!!
    atEnterCommand = "\"\r\n"; // length 3


    atDirectLinkCommand = "AT+USODL=0\r";




}
atCommander::~atCommander(){


}

void atCommander::process(Event e){

    switch(currentState){


    case StopState :

        if(e == evStart){
            bool successful = initTobyModul();
            if(successful){
                currentState = WaitState;
                PX4_INFO("switch state to WaitState");
                PX4_INFO("Direct Link Connection");
                myDevice->write(atDirectLinkCommand,11);
                const char* pch = "CONNECT";
                const char* stringEnd = '\0';
                strncpy(temporaryBuffer,stringEnd,62);
                PX4_INFO("Polling for CommandOK");
                int poll_return = 0;
                while(poll_return < 1){
                    //some stupid polling;
                    poll_return = myDevice->poll(NULL,true);
                    usleep(100000);
            }
                int read_return = myDevice->read(temporaryBuffer,62);
                PX4_INFO("read returns : %d",read_return);
                if(strstr(temporaryBuffer, pch) != NULL){
                    PX4_INFO("response sucessfull : %s",temporaryBuffer);

                }
                else{

                    PX4_INFO("response : %s",temporaryBuffer);
                    strncpy(temporaryBuffer,stringEnd,62);
                    read_return = myDevice->read(temporaryBuffer,62);
                    PX4_INFO("ERROR response : %s",temporaryBuffer);


                }








                sleep(1);
                readerParameters.myDevice = myDevice;
                readerParameters.readBuffer = readBuffer;
                atReaderThread = new pthread_t();
                pthread_create(atReaderThread, NULL, atCommander::readWork, (void*)&readerParameters);

            }
            else{
                currentState = ErrorState;
                PX4_INFO("switch state to ErrorState");
            }
        }
        currentState = WaitState;
        break;

    case InitState:
        PX4_INFO("in InitState");

        break;

    case ErrorState :
        // error handling, maybe reinitialize32 toby modul?
        break;

    case WaitState :
    //this don't work yet -----------------------------------------------------------------

       //-------------------------------------------------------------------------------------

        if(e == evWriteDataAvaiable){



           int buffer_return = writeBuffer->getString(temporarySendBuffer,128);

           int write_return = myDevice->write(temporarySendBuffer,buffer_return); //the number depends on the buffer deepness!!!!
           PX4_INFO("successfull write %d",write_return);

           if(write_return != buffer_return){
               PX4_INFO("Error writing Data to UART");
           }
           usleep(10000);
           //pingPongWriteBuffer->GetDataSuccessfull(); // message that we can free the buffer


        }


        break;

    default :

        break;


    }


}






//******************************start and intercation Function*******************************

void* atCommander::atCommanderStart(void* arg){

    //eventuell in externen Header, saubere Trennung
    PX4_INFO("AT-Commander says hello");

    myStruct *arguments = static_cast<myStruct*>(arg);

    BoundedBuffer *atWriteBuffer = arguments->writeBuffer;
    BoundedBuffer *atReadBuffer = arguments->readBuffer;
    TobyDevice *atTobyDevice = arguments->myDevice;
    PingPongBuffer *pingPongWriteBuffer = arguments->writePongBuffer;


    usleep(500);

    atCommander *atCommanderFSM = new atCommander(atTobyDevice,atReadBuffer,atWriteBuffer,pingPongWriteBuffer);
    atCommanderFSM->process(evStart);


    sleep(2);

    PX4_INFO("Beginn with transfer");

    int poll_return = 0;
   // int write_return = 0; // number data to write

    //char* temporaryReadBuffer = (char*)malloc(62*sizeof(char));


    //return NULL;





    while(1){
        while(1){


            atCommanderFSM->process(evWriteDataAvaiable);
            usleep(10000);
        }







       // poll_return = (atTobyDevice->poll(NULL,true));
        if(poll_return > 0){
            atCommanderFSM->process(evReadDataAvaiable);
            //*******************************************
            //read_return =  atTobyDevice->read(temporaryBuffer,62);
            //atReadBuffer->putString(temporaryBuffer,read_return);
            //poll_return = (atTobyDevice->poll(NULL,true));
        }

        // Nur Senden, wir testen einzig ob wir daten empfangen
        if(pingPongWriteBuffer->DataAvaiable()){
            //PX4_INFO("process put evWriteDataAvaiable");
            atCommanderFSM->process(evWriteDataAvaiable);
        }
        else{
            usleep(10000);
            PX4_INFO("no data avaiable");

        }


    }
    return NULL;

}


bool atCommander::initTobyModul(){

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
                                                        "at+usocr=6\r",                         // 6 TCP, 17 UDP
                                                        "at+usoco=0,\"178.196.15.59\",44444\r",

                                                       };


    PX4_INFO("Beginn with Initialization");
    int i = 0;
    char* stringEnd = '\0';
    while(i < 18){
        myDevice->write(at_command_send[i],at_command_lenght2(at_command_send[i]));
        while(poll_return < 1){
            //some stupid polling;
            //PX4_INFO("Polling");
            poll_return = myDevice->poll(NULL,true);
            usleep(100);
        }
        sleep(1);
        poll_return = myDevice->read(temporaryBuffer,62);
        if(strstr(temporaryBuffer, pch) != NULL){
            PX4_INFO("Command Successfull : %s",at_command_send[i]);
            PX4_INFO("answer :  : %s",temporaryBuffer);

            ++i; //sucessfull, otherwise, retry
        }
        else{
            PX4_INFO("Command failed : %s", at_command_send[i]);
            if(i > 0){
                --i; //we try the last command befor, because if we can't connect to static ip, it closes the socket automatically and we need to reopen
            }
        }


        //dirty way to flush the read buffer
        strncpy(temporaryBuffer,stringEnd,62);


       poll_return = 0;
    }
    PX4_INFO("sucessfull init");

    return true;

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



void* atCommander::readWork(void *arg){

    PX4_INFO("readWork Thread started");
    //extract arguments :
    threadParameter *arguments = static_cast<threadParameter*>(arg);
    BoundedBuffer* readBuffer = arguments->readBuffer;
    TobyDevice* myDevice = arguments->myDevice;




    // a ty
    char* buffer = (char*)malloc(64*sizeof(char));


    if(myDevice == NULL || readBuffer == NULL){
        PX4_INFO("readWork Thread parameters invalid");

    }
    int i = 0; // poll result handle
    int u = 0; //size of data received
    while(1){

        i = myDevice->poll(NULL,0);
        if(i>0){
            usleep(10000);
          u =  myDevice->read(buffer,128);
           readBuffer->putString(buffer,u);
           PX4_INFO("readWorker successfull read %d",u);


        }
        else{
             usleep(10000);


        }
        if(buffer == NULL){
            PX4_INFO("READ WORKER NULLPOINTER");
        }
        usleep(10000);

    }


    PX4_INFO("readWork exit");

    return NULL;

}
