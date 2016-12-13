/**
* @file     atCommander.cpp
*
* @brief    Initialize the Toby Modul and handles the communication
* @author   Marc Brändli
* @date     12.12.2016
*/


#include <string.h>
#include "atCommander.h"
#include "toby.h"
#include "px4_log.h"




atCommander::atCommander(TobyDevice* device, BoundedBuffer* read, BoundedBuffer* write, PingPongBuffer* write2)
    : myDevice(device), readBuffer(read), writeBuffer(write), pingPongWriteBuffer(write2)
{


    atReaderThread = 0;

    currentState = InitState;

    temporaryBuffer = (char*)malloc(62*sizeof(char));
    temporarySendBuffer = (char*)malloc(62*sizeof(char));

    //Initialize const strings for sending and checking
    atEnterCommand              = "\"\r\n"; // length 3
    atDirectLinkRequest         = "AT+USODL=0\r";
    atReadyRequest              ="AT\r";
    atDirectLinkOk              ="CONNECT";
    stringEnd                   ='\0';
    atResponseOk                ="OK";

    //
    for(int j=0; j < MAX_AT_COMMANDS; ++j)
        atCommandSendp[j] = &atCommandSendArray[j][0];

}
atCommander::~atCommander(){

    PX4_INFO("AT-Commander deconstructor is called");
    free(temporaryBuffer);
    free(temporarySendBuffer);
    PX4_INFO("free successful");

}

void atCommander::process(Event e){

    switch(currentState){

        case InitState:{

            PX4_INFO("InitState");
            if(e == evStart){

                if(tobyAlive(10)==true){

                    PX4_INFO("InitState: Toby connected");

                    if(readAtfromSD()>0){

                        PX4_INFO("InitState: Read AT-Commands from SD card");

                        printAtCommands();

                        if(initTobyModul()==true){

                            PX4_INFO("InitState: Toby initialized");

                            currentState = SetupState;

                        }else{
                            PX4_INFO("InitState: Toby NOT initialized");
                            currentState = ErrorState;
                        }
                    }else{
                        PX4_INFO("InitState: No SD-Card or corrupted");
                        currentState = ErrorState;
                    }
                }else{
                    PX4_INFO("InitState: Toby NOT connected");
                    currentState = ErrorState;
                }
            }

            if(e == evShutDown){
                shutDown();
            }
            break;
        }

        case SetupState :{

            PX4_INFO("ReadyState");
            PX4_INFO("Direct Link Connection");
            myDevice->write(atDirectLinkRequest,getAtCommandLenght(atDirectLinkRequest));

            int poll_return = 0;
            while(poll_return < 1){
                //some stupid polling;
                poll_return = myDevice->poll(0);
                usleep(100000);
            }

            bzero(temporaryBuffer,62);
            myDevice->read(temporaryBuffer,62);

            PX4_INFO("Direct Link : %s",temporaryBuffer);

            if(strstr(temporaryBuffer, atDirectLinkOk) != 0){
                sleep(1);
                //set up readerParameters and start read thread
                readerParameters.myDevice = myDevice;
                readerParameters.readBuffer = readBuffer;
                readerParameters.threadExitSignal = &readerExitSignal;
                readerExitSignal = false;
                atReaderThread = new pthread_t();
                pthread_create(atReaderThread, NULL, atCommander::readWork, (void*)&readerParameters);

                currentState=WriteState;

            }else{
                currentState = SetupState;
            }

            break;
        }

        case WriteState :{
            //this don't work yet -----------------------------------------------------------------

            //-------------------------------------------------------------------------------------

            if(e == evWriteDataAvailable){
                int buffer_return = writeBuffer->getString(temporarySendBuffer,128);

                int write_return = myDevice->write(temporarySendBuffer,buffer_return); //the number depends on the buffer deepness!!!!
                PX4_INFO("successfull write %d",write_return);

                if(write_return != buffer_return){
                    PX4_INFO("Error writing Data to UART");
                }
                usleep(10000);
                //pingPongWriteBuffer->GetDataSuccessfull(); // message that we can free the buffer
                if(e == evShutDown){
                    shutDown();
                }

            }
            break;
        }

    case ErrorState :{
        PX4_INFO("ErrorState");
        sleep(5); // do here some sleeping, so we can better read the error messages output from nsh
        // error handling, maybe reinitialize32 toby modul?
        if(e == evShutDown){
            shutDown();
        }
        break;
    }

        default :
        break;
            //break; every other State went to the ErrorState


    }
}

void* atCommander::atCommanderStart(void* arg){

    PX4_INFO("AT-Commander start");
    //**************get arguments*************************
    myStruct *arguments = static_cast<myStruct*>(arg);
    BoundedBuffer *atWriteBuffer = arguments->writeBuffer;
    BoundedBuffer *atReadBuffer = arguments->readBuffer;
    TobyDevice *atTobyDevice = arguments->myDevice;
    PingPongBuffer *pingPongWriteBuffer = arguments->writePongBuffer;
    volatile bool* shouldExitSignal = arguments->threadExitSignal;


    atCommander *atCommanderFSM = new atCommander(atTobyDevice,atReadBuffer,atWriteBuffer,pingPongWriteBuffer);


    PX4_INFO("Beginn with transfer, should exit : ");

   if(*shouldExitSignal){
       PX4_INFO("should exit");
   }
   else if(*shouldExitSignal == false){
       PX4_INFO("no exit signal");

   }



    //to start all
    atCommanderFSM->process(evStart);


    while(!*shouldExitSignal){

            if(!(atWriteBuffer->empty())){ // a bit inconsistent, should be done in FSM, but for actual state of play useful
                atCommanderFSM->process(evWriteDataAvailable);
                usleep(10000);
            }
            else{
                usleep(10000);
            }



    }

    PX4_INFO("Thread terminate");

    //has to leave, clear all
    atCommanderFSM->process(evShutDown);
    delete atCommanderFSM;
    return NULL;
}

bool atCommander::tobyAlive(int times){

    PX4_INFO("Check %d times with 1 Second in between for Toby", times);
    bool    returnValue     =false;
    int     returnPollValue =0;
    int     returnWriteValue=0;
    int     i               =0;

    do{

        returnWriteValue = myDevice->write(atReadyRequest,getAtCommandLenght(atReadyRequest));

        if(returnWriteValue > 0){

            returnPollValue = myDevice->poll(0);
            //PX4_INFO("tobyAlive returnPollValue :%d",returnPollValue);
            sleep(1);
        }
        if(returnPollValue>0){
            bzero(temporaryBuffer,62);
            //PX4_INFO("Jetzt wird gelesen");
            myDevice->read(temporaryBuffer,62);
            //PX4_INFO("tobyAlive answer :%s",temporaryBuffer);
            if(strstr(temporaryBuffer,atResponseOk) != 0){
                //PX4_INFO("tobyAlive Command Successfull : %s",atReadyRequest);
                //PX4_INFO("Toby is connected",temporaryBuffer);
                returnValue=true;
            }else{
                bzero(temporaryBuffer,62);
            }
        }else{
           sleep(1);
        }
        ++i;
    }
    while((returnValue != true)&(i<times));

    return returnValue;
}


bool atCommander::initTobyModul(){

    int         returnValue= 0;
    int i = 0;


    PX4_INFO("Beginn with Initialization");

    while(i < numberOfAt){
        myDevice->write(atCommandSendp[i],getAtCommandLenght(atCommandSendp[i]));
        while(returnValue < 1){
            //some stupid polling, we wait for answer
            returnValue = myDevice->poll(0);
            usleep(5000);
        }
        sleep(1);
        returnValue = myDevice->read(temporaryBuffer,62);
        if(strstr(temporaryBuffer,atResponseOk) != 0){
            PX4_INFO("Command Successfull : %s",atCommandSendp[i]);
            PX4_INFO("answer: %s",temporaryBuffer);

            ++i; //sucessfull, otherwise, retry
        }
        else{
            PX4_INFO("Command failed: %s", atCommandSendp[i]);
            if(i > 0){
                --i; //we try the last command befor, because if we can't connect to static ip, it closes the socket automatically and we need to reopen
            }
        }

        bzero(temporaryBuffer,62);
        returnValue = 0;
    }
    PX4_INFO("sucessfull init");

    return true;

}


int atCommander::getAtCommandLenght(const char* at_command)
{
    int k=0;
    while(at_command[k] != '\0')
    {
        k++;
    }

    return k;
}


void atCommander::printAtCommands()
{
    PX4_INFO("%s beinhaltet %d Zeilen",SD_CARD_PATH,numberOfAt);
    for(int j=0 ; j < numberOfAt ; j++)
    {
        PX4_INFO("Inhalt %d %s",j ,atCommandSendp[j]);
    }
}


bool atCommander::readAtfromSD()
{
    FILE*   sd_stream               = fopen(SD_CARD_PATH, "r");
    int 	atcommandbufferstand    = 0;
    int 	inputbufferstand        = 0;
    bool     returnValue            =false;
    int 	c                       =-1;
    char 	string_end              ='\0';
    char 	inputbuffer[MAX_CHAR_PER_AT_COMMANDS]="";

    if(sd_stream) {
        returnValue=true;
        PX4_INFO("SD card open");
        do { // read all lines in file
            do{ // read one line until EOF
                c = fgetc(sd_stream);
                if(c != EOF){
                    //(EOF = End of File mit -1 im System definiert)
                    inputbuffer[inputbufferstand]=(char)c;
                    inputbufferstand++;
                }
            }while(c != EOF && c != '\n');

            inputbuffer[inputbufferstand]='\r';
            inputbufferstand++;
            inputbuffer[inputbufferstand]=string_end; //wirklich Notwendig ?

            //Speicherplatz konnte nicht mit malloc alloziert werden! Griff auf ungültigen Speicherbereich zu.
            //atcommandbuffer[atcommandbufferstand] =(char*) malloc(20);
            //assert(atcommandbuffer[atcommandbufferstand]!=0);
            strcpy(atCommandSendp[atcommandbufferstand],inputbuffer);

            inputbufferstand=0;
            atcommandbufferstand++;
        } while(c != EOF);
    }else{
        PX4_INFO("No SD card or corrupted");
    }

    fclose(sd_stream);
    PX4_INFO("SD card closed");

    atcommandbufferstand--;
    numberOfAt=atcommandbufferstand;

    //For Debbuging
    //printAtCommands(atcommandbuffer,atcommandbufferstand);
    //PX4_INFO("atcommandbufferstand in readATfromSD fkt: %d", numberOfAt);


    return returnValue;
}


void* atCommander::readWork(void *arg){

    PX4_INFO("readWork Thread started");
    //extract arguments :
    threadParameter *arguments = static_cast<threadParameter*>(arg);
    BoundedBuffer* readBuffer = arguments->readBuffer;
    TobyDevice* myDevice = arguments->myDevice;
    volatile bool* shouldExitSignal = arguments->threadExitSignal;




    // a ty
    char* buffer = (char*)malloc(64*sizeof(char));


    if(myDevice == NULL || readBuffer == NULL){
        PX4_INFO("readWork Thread parameters invalid");
        return NULL; // may do here some error handling
    }
    int i = 0; // poll result handle
    int u = 0; //size of data received
    while(!*shouldExitSignal){
        i = myDevice->poll(10000);
        if(i>0){
            usleep(10000);
            u =  myDevice->read(buffer,64);
            readBuffer->putString(buffer,u);
            PX4_INFO("readWorker successfull read %d",u);
        }
        else{
            usleep(10000);
        }

        usleep(10000);
    }


    PX4_INFO("readWork exit");

    return NULL;

}

int atCommander::shutDown(void){

    PX4_INFO("shutDown is called");
    readerExitSignal = true;
    if(atReaderThread != 0){
        pthread_join(*atReaderThread,NULL);
        PX4_INFO("atReaderThread terminates");
    }


    return 0;
}
