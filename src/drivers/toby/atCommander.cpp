#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>


#include "atCommander.h"
#include "toby.h"
#include "px4_log.h"

//for readATfromSD
#define PATH       "/fs/microsd/toby/at-inits.txt"
enum{
    MAX_AT_COMMANDS = 20,
    MAX_CHAR_PER_AT_COMMANDS =40,
    READ_BUFFER_LENGHT =100
};

//Globales Poll Flag
static volatile int uart_poll;
static volatile int pollingThreadStatus;


extern pthread_cond_t Toby::pollEventSignal;  // has to be public, otherwise we cant use it from at commander
extern pthread_mutex_t Toby::pollingMutex;


atCommander::atCommander(TobyDevice* device, BoundedBuffer* read, BoundedBuffer* write, PingPongBuffer* write2){

    myDevice = device;
    readBuffer = read;
    writeBuffer = write;
    pingPongWriteBuffer = write2;

    //currentState = InitModulState;       //First test with ready state
    currentState = StopState;

    temporaryBuffer = (char*)malloc(62*sizeof(char));
    commandBuffer = (char*)malloc(62*sizeof(char));

    commandBuffer = (char*)malloc(15*sizeof(char));
    atCommandSend = "AT+USOWR=0,62\r";

    atCommandPingPongBufferSend = "at+usost=0,\"178.196.15.59\",44444,10,\""; // the value 0,XX depends on the PingPongBuffer::AbsolutBufferLength!!!
    atEnterCommand = "\r";
    response_ok="OK";
    response_at="@";

    atDirectLinkCommand = "AT+USODL=0\r";

    uart_poll=0;
    pollingThreadStatus=0;

    //pollingThreadParameters.myDevice = myDevice;
    //pthread_create(pollingThread,0,atCommander::pollingThreadStart,(void*)&pollingThreadParameters);


}
atCommander::~atCommander(){


}

void atCommander::process(Event e){

    switch(currentState){

    case StopState:{

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
                PX4_INFO("Polling for Command OK");
                int poll_return = 0;
                while(poll_return < 1){
                    //some stupid polling;
                    poll_return = myDevice->poll(0);
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
    }
    case InitState:{
        PX4_INFO("in InitState");

        break;
    }
    case ErrorState :{
        // error handling, maybe reinitialize32 toby modul?
        break;
    }
    case WaitState :
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
        }
        break;

    default :

        break;

    }



    /*    char    inputBuffer[100]    ="";
    int     write_return        =0;
    char*   sendDataPointer     =0;
    bool    successful          =FALSE;

    switch(currentState){
        case InitModulState:{

            PX4_INFO("StateMachine: InitModulState");

            if(e == evStart){
                successful = initTobyModul();

                if(successful){
                    PX4_INFO("sucessfull init, switch to WaitState");
                    currentState = WaitState;
                }
                else{
                    PX4_INFO("switch to ErrorState");
                    currentState = ErrorState;
                }
            }
            break;
        }
        case WaitState :{
            PX4_INFO("StateMachine: WaitState");

            switch(e){
            case evWriteDataAvailable:{
                PX4_INFO("StateMachine: WaitState, case: evWriteDataAvailable");
                write_return = myDevice->write(atCommandPingPongBufferSend,14);
                pollingThreadStatus =1;
                if(write_return == 14)
                {
                    PX4_INFO("change to writestate");
                    currentState=WriteState;

                }else{
                    PX4_INFO("StateMachine: WaitState, no sucsessfull request");
                    currentState = WaitState;
                }
                break;
            }
                //QGC Response not implementet yet
            case evResponse:{
                PX4_INFO("StateMachine: WaitState, case: evResponse");
                myDevice->read(inputBuffer,READ_BUFFER_LENGHT);
                PX4_INFO("answer :%s",inputBuffer);

                //Copy into ReadBuffer

                break;
            }

            default:
                break;
            }
        }
        case WriteState:{
            PX4_INFO("StateMachine: WriteState");

            if(e==evResponse){
                PX4_INFO("StateMachine: WriteState case evResponse");
                myDevice->read(inputBuffer,READ_BUFFER_LENGHT);

                if(strstr(inputBuffer,response_at) != 0){
                    PX4_INFO("answer :%s",inputBuffer);
                    sendDataPointer =  pingPongWriteBuffer->getActualReadBuffer();
                    write_return = myDevice->write(sendDataPointer,64); //the number depends on the buffer deepness!!!!
                    pollingThreadStatus =1;
                    if(write_return == 64){
                        PX4_INFO("Send sucessfull");
                        pingPongWriteBuffer->GetDataSuccessfull();
                        // message that we can free the buffer

                        currentState=WaitState;
                    }else{
                        PX4_INFO("Error writing Data to UART");
                        //Nochmaliges schreiben Initialisieren ?
                        currentState = WaitState;
                    }
                }else{ // Antwork enthält kein '@'
                    PX4_INFO("StateMachine: WriteState::, No @ ...");
                    currentState = WaitState;
                }

                bzero(inputBuffer,READ_BUFFER_LENGHT); //buffer leeren


            }else{
                PX4_INFO("AHA");
                //pollingThreadStatus =1;
            }
            break;
        }
        default:{
            PX4_INFO("switch to ErrorState because of default");
            currentState = ErrorState;
            break;
        }
    }*/
}


void* atCommander::atCommanderStart(void* arg){

    //eventuell in externen Header, saubere Trennung
    PX4_INFO("AT-Commander says hello");

    myStruct *arguments = static_cast<myStruct*>(arg);

    BoundedBuffer *atWriteBuffer = arguments->writeBuffer;
    BoundedBuffer *atReadBuffer = arguments->readBuffer;
    TobyDevice *atTobyDevice = arguments->myDevice;
    PingPongBuffer *pingPongWriteBuffer = arguments->writePongBuffer;


    usleep(500); //Delay for cast, really needed ?

    atCommander *atCommanderFSM = new atCommander(atTobyDevice,atReadBuffer,atWriteBuffer,pingPongWriteBuffer);
    atCommanderFSM->process(evStart);


    sleep(2); //Delay for Toby Initialisation

    PX4_INFO("Beginn with transfer");

    int poll_return = 0;

    while(1){
        while(1){


            atCommanderFSM->process(evWriteDataAvailable);
            usleep(10000);
        }







        // poll_return = (atTobyDevice->poll(NULL,true));
        if(poll_return > 0){
            atCommanderFSM->process(evReadDataAvailable);
            //*******************************************
            //read_return =  atTobyDevice->read(temporaryBuffer,62);
            //atReadBuffer->putString(temporaryBuffer,read_return);
            //poll_return = (atTobyDevice->poll(NULL,true));
        }

        // Nur Senden, wir testen einzig ob wir daten empfangen
        if(pingPongWriteBuffer->DataAvaiable()){
            //PX4_INFO("process put evWriteDataAvaiable");
            atCommanderFSM->process(evWriteDataAvailable);
        }
        else{
            usleep(10000);
            PX4_INFO("no data avaiable");

        }


    }
    return NULL;


    /*while(1){
        if(pollingThreadStatus){
            PX4_INFO("Allow to poll!!!");
            uart_poll = atTobyDevice->poll(0);
        }

        if(poll_return > 0){
            PX4_INFO("process put evWriteDataAvaiable");
            uart_poll =0;
            pollingThreadStatus =0;
            atCommanderFSM->process(evResponse);
        }
        // Nur Senden, wir testen einzig ob wir daten empfangen
        if(pingPongWriteBuffer->DataAvaiable()){
            PX4_INFO("process put evWriteDataAvaiable");
            atCommanderFSM->process(evWriteDataAvailable);
        }
        sleep(1);
        return 0;
    }*/
}


bool atCommander::initTobyModul(){

    int poll_return= 0;
    //const char* pch = "OK";

    //char   atCommandSendArray[MAX_AT_COMMANDS][MAX_CHAR_PER_AT_COMMANDS];
    const char* atCommandSendp[18]={"ATE0\r",
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
                                     "at+usoco=0,\"178.196.15.59\",44444\r"

                                    };
    int i = 0;
    //Anpassung da malloc nicht funktioniert,
    //damit Funktionen weiterverwendet werden können
   /* char* atCommandSendp[MAX_AT_COMMANDS];
    for(int j=0; j < MAX_AT_COMMANDS; ++j)
        atCommandSendp[j] = &atCommandSendArray[j][0];*/

    int numberOfAT=18;
            //readAtfromSD( atCommandSendp);
    PX4_INFO("numberOfAT from return:: %d",numberOfAT);

    //printAtCommands(atCommandSendp,numberOfAT);





    //char* stringEnd = '\0';
    while(i < numberOfAT){
        myDevice->write( atCommandSendp[i],getAtCommandLenght(atCommandSendp[i]));
        PX4_INFO("Command lenght get  : %d", getAtCommandLenght(atCommandSendp[i]));
        //PX4_INFO("Command lenght strlen : %d", strlen(atCommandSendp[i]));
        while(poll_return < 1){
            //some stupid polling;
            poll_return = myDevice->poll(0);
            usleep(100);
        }
        sleep(1);
        poll_return = myDevice->read(temporaryBuffer,62);
        if(strstr(temporaryBuffer, response_ok) != NULL){
            PX4_INFO("Command Successfull!");
            PX4_INFO("answer :  : %s",temporaryBuffer);

            ++i; //sucessfull, otherwise, retry
        }
        else{
            PX4_INFO("Command failed!");
            if(i > 0){
                --i; //we try the last command befor, because if we can't connect to static ip, it closes the socket automatically and we need to reopen
            }
        }


        //dirty way to flush the read buffer
        //strncpy(temporaryBuffer,stringEnd,62);
        bzero(temporaryBuffer,62);


        poll_return = 0;
    }
    //PX4_INFO("sucessfull init");

    return true;

}

/**
 * @brief returns the lenght of a Chararray
 *
 * return value includes '\r'
 * this function is needed because strleng() doesn't work
 */
int atCommander::getAtCommandLenght(const char* at_command)
{
    int k=0;
    while(at_command[k] != '\r')
    {
        k++;
    }

    return k+1;
}

/**
 * @brief Gibt die AT-Commands als PX4_Info aus
 *
 *
 */
void atCommander::printAtCommands(char **atcommandbuffer, int atcommandbufferstand)
{
    PX4_INFO("%s beinhaltet %d Zeilen",PATH,atcommandbufferstand);
    for(int j=0 ; j < atcommandbufferstand ; j++)
    {
        PX4_INFO("Inhalt %d %s",j ,atcommandbuffer[j]);
    }
}

/**
 * @brief read the AT-Commands from the SD-Card into the char-pointer Array
 *
 * #define MAX_AT_COMMANDS beachten !!!
 *
 * @return Number of AT-Commands
 * @param char-array pointer
 */
int atCommander::readAtfromSD(char **atcommandbuffer)
{
    FILE*   sd_stream               = fopen(PATH, "r");
    int 	atcommandbufferstand    = 0;
    int 	inputbufferstand        = 0;
    int 	c                       =-1;
    char 	string_end              ='\0';
    char 	inputbuffer[MAX_CHAR_PER_AT_COMMANDS]="";

    if(sd_stream) {
        PX4_INFO("SD-Card open");
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
            strcpy(atcommandbuffer[atcommandbufferstand],inputbuffer);

            inputbufferstand=0;
            atcommandbufferstand++;
        } while(c != EOF);
    }else{
        PX4_INFO("SD-Card NOT open");
    }

    fclose(sd_stream);
    PX4_INFO("SD-Card-Closed");

    atcommandbufferstand--;

    //For Debbuging
    //printAtCommands(atcommandbuffer,atcommandbufferstand);
    //PX4_INFO("atcommandbufferstand in readATfromSD fkt: %d",atcommandbufferstand);


    return atcommandbufferstand;
}

/**
 * @brief Polls on the Uart
 *
 * sets the Global variable poll_return if Datas are available
 * we need this function because there's no SIGIO
 */
void *atCommander::pollingThreadStart(void *arg)
{
    myStruct *arguments = static_cast<myStruct*>(arg);
    TobyDevice* myDevice = arguments->myDevice;

    int poll_return;
    while(1)
    {
        if(pollingThreadStatus)
        {
            poll_return= myDevice->poll(5);
            //PX4_INFO("polling Thread : I'am polling");
            if(poll_return > 0){
                //PX4_INFO("polling Thread : poll was successfull");
                uart_poll =1;
            }
        }
        usleep(50000);
    }
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

        i = myDevice->poll(0);
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

