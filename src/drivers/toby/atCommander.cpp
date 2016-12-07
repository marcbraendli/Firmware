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
    commandBuffer = (char*)malloc(15*sizeof(char));
    atCommandSend = "AT+USOWR=0,62\r";
    atCommandPingPongBufferSend = "AT+USOWR=0,64\r"; // the value 0,XX depends on the PingPongBuffer::AbsolutBufferLength!!!
    atEnterCommand = "\r";




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

            //**************old function******************
            /*
            PX4_INFO("evWriteDataAvaiable : ");

            int write_return = (writeBuffer->getString(temporaryBuffer,62));
            PX4_INFO("writeBuffer has to write :  %d",write_return);
            sleep(1);

            //send "AT+USOWR=0,"
            myDevice->write(atCommandSend,14);
            //PX4_INFO("AT COMMAND: write %d of 14 commands",write_return);
            char numberToWrite[10] = "";
            //parse number to write from int to string
            itoa(write_return,numberToWrite,10);

            PX4_INFO("Number to write : %s",numberToWrite);
            write_return = myDevice->write(temporaryBuffer,62);
            PX4_INFO("write number of character to toby device : %i",write_return);
            PX4_INFO("write number of character to toby device : %s",numberToWrite);

            //flush the count string
            char* stringEnd = '\0';
            strncpy(numberToWrite,stringEnd,10);


            */

            // new function********************************************


            /*
            PX4_INFO("StateMachine: evWriteDataAavaiable : send Test data");
            const char* testText = "HelloFromDrone";
            const char* atSendtestText = "AT+USOWR=0,14\r";

            myDevice->write(atSendtestText,14);
            usleep(20000);
            myDevice->write(testText,14);

            */


           //PX4_INFO("StateMachine: evWriteDataAavaiable : send data");
           char* stringEnd = '\0';
           const char* pch = "@";
           int poll_return = 0;
           int read_return = 0;
           strncpy(temporaryBuffer,stringEnd,62);
           char* sendDataPointer = NULL;
           bool dataIsAvaiable = false;
           sendDataPointer =  pingPongWriteBuffer->getActualReadBuffer();
           PX4_INFO("Send Command : ");
           myDevice->write(atCommandPingPongBufferSend,14);

           while(!dataIsAvaiable){
               PX4_INFO("Polling for CommandOK");
               poll_return = 0;
               while(poll_return < 1){
                   //some stupid polling;
                   poll_return = myDevice->poll(NULL,true);
                   usleep(10000);
           }
               read_return = myDevice->read(temporaryBuffer,62);
               PX4_INFO("read returns : %d",read_return);
               if(strstr(temporaryBuffer, pch) != NULL){
                   dataIsAvaiable = true;
                   PX4_INFO("response sucessfull : %s",temporaryBuffer);

               }
               else{

                   PX4_INFO("response : %s",temporaryBuffer);
                   strncpy(temporaryBuffer,stringEnd,62);
                   dataIsAvaiable = false;
                   read_return = myDevice->read(temporaryBuffer,62);
                   PX4_INFO("ERROR response : %s",temporaryBuffer);


               }

           }


           PX4_INFO("Begin to send");
           int write_return = myDevice->write(sendDataPointer,64); //the number depends on the buffer deepness!!!!
           if(write_return != 64){
               PX4_INFO("Error writing Data to UART");
           }







           strncpy(temporaryBuffer,stringEnd,62);
           bool transmittOk = false;
           const char* pch2 = "0,64";

           while(!transmittOk){
               PX4_INFO("Polling for sendOK");
               poll_return = 0;
               while(poll_return < 1){
                   //some stupid polling;
                   poll_return = myDevice->poll(NULL,true);

           }
               usleep(30000);
               read_return = myDevice->read(temporaryBuffer,62);
               PX4_INFO("read returns : %d",read_return);
               if(strstr(temporaryBuffer, pch2) != NULL){
                   transmittOk = true;
                   PX4_INFO("response sucessfull: %s",temporaryBuffer);

               }
               else{

                   PX4_INFO("ERROR response : %s",temporaryBuffer);
                   strncpy(temporaryBuffer,stringEnd,62);
               }

          }

           pingPongWriteBuffer->GetDataSuccessfull(); // message that we can free the buffer


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
   // int read_return = 0;
   // char* temporaryBuffer = (char*)malloc(62*sizeof(char));


    //return NULL;





    while(1){

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
            usleep(100);
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
                                                        "at+usocr=6\r",
                                                        "at+usoco=0,\"178.196.15.59\",44444\r",

                                                       };



    int i = 0;
    char* stringEnd = '\0';
    while(i < 18){
        myDevice->write(at_command_send[i],at_command_lenght2(at_command_send[i]));
        while(poll_return < 1){
            //some stupid polling;
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
