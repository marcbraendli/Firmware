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
    commandBuffer = (char*)malloc(15*sizeof(char));
    atCommandSend = "AT+USOWR=0,62\r";

    writeDataCommand[] = {"USOCO=0,"};  // write command, Command = Socketnr, number of bytes to write;

}
atCommander::~atCommander(){


}

void atCommander::process(Event e){

    switch(currentState){


    case StopState :

        if(e == evStart){
            bool successful = initTobyModul();
            if(successful){
                PX4_INFO("Beginn with transfer, using command %s ",atCommandSend);
                currentState = WaitState;
                PX4_INFO("switch state to WaitState");
            }
            else{
                currentState = ErrorState;
                PX4_INFO("switch state to ErrorState");

            }
        }
<<<<<<< HEAD
        currentState = WaitState;
        break;

    case InitState:
        PX4_INFO("in InitState");
=======

        break;

    case ErrorState :
        // error handling, maybe reinitialize toby modul?
>>>>>>> f69c1aec2d4ff5443497932d08187ea4de8122dd
        break;


<<<<<<< HEAD
        if(e == evWriteDataAvaiable){
            PX4_INFO("do some writing");
            int sizeofdata = writeBuffer->getString(temporaryBuffer,62);
            myDevice->write("USOCO=0,",7);
            myDevice->write(sizeofdata,7); // sizeofdata in string convert?!


            currentState = waitForResponse;
        }
=======
    case WaitState :

        if(e== evReadDataAvaiable){
            //first check the data avaiable
            myDevice->read(temporaryBuffer,62);
            //parse, we need to know if we can read data or if there is something else
            int dataToRead = 0;
           // int dataToRead = parse(temporaryBuffer);

            if(dataToRead > 0){
                // we want to read all data from toby module
                myDevice->write(temporaryBuffer,12);    // accept data's
>>>>>>> f69c1aec2d4ff5443497932d08187ea4de8122dd

                // fill buffer with data from toby
                dataToRead = myDevice->read(temporaryBuffer,62);

            }

            else if(dataToRead < 0){
                // error handling, parser said that there must be an error message

                currentState = ErrorState;
            }

            else{
                // we do nothing
            }
                readBuffer->putString(temporaryBuffer, dataToRead);

        }



        if(e == evWriteDataAvaiable){
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



            // write an enter
            //const char* stringEnter = " \r";
            // myDevice->write(stringEnter,2);


            PX4_INFO("Number to write : %s",numberToWrite);


            write_return = myDevice->write(temporaryBuffer,62);
            PX4_INFO("write number of character to toby device : %i",write_return);
            PX4_INFO("write number of character to toby device : %s",numberToWrite);

            //flush the count string



            char* stringEnd = '\0';
            strncpy(numberToWrite,stringEnd,10);


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

    usleep(500);

    atCommander *atCommanderFSM = new atCommander(atTobyDevice,atReadBuffer,atWriteBuffer);
    atCommanderFSM->process(evStart);


    sleep(2);
<<<<<<< HEAD

=======
    PX4_INFO("Beginn with transfer");
>>>>>>> f69c1aec2d4ff5443497932d08187ea4de8122dd

    int poll_return = 0;
   // int write_return = 0; // number data to write
   // int read_return = 0;
   // char* temporaryBuffer = (char*)malloc(62*sizeof(char));


    //return NULL;



    while(1){

<<<<<<< HEAD
        poll_return = (atTobyDevice->poll(NULL,true));
        if(poll_return > 0){
            atCommanderFSM->Event(evReadDataAvaiable);
            //*******************************************
            //read_return =  atTobyDevice->read(temporaryBuffer,62);
            //atReadBuffer->putString(temporaryBuffer,read_return);
=======
        //poll_return = (atTobyDevice->poll(NULL,true));
>>>>>>> f69c1aec2d4ff5443497932d08187ea4de8122dd

        if(poll_return > 0){
            atCommanderFSM->process(evReadDataAvaiable);
        }


        atCommanderFSM->process(evWriteDataAvaiable);


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
                                                        "at+usoco=0,\"178.194.112.125\",5555\r",

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
            --i; //we try the last command befor, because if we can't connect to static ip, it closes the socket automatically and we need to reopen
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
