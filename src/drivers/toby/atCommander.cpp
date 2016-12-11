#include <string.h>
#include "atCommander.h"
#include "toby.h"


#include "px4_log.h"

extern pthread_cond_t Toby::pollEventSignal;  // has to be public, otherwise we cant use it from at commander
extern pthread_mutex_t Toby::pollingMutex;
int at_command_lenght2(const char* at_command);

volatile int uart_poll;
volatile int pollingThreadStatus;


atCommander::atCommander(TobyDevice* device, BoundedBuffer* read, BoundedBuffer* write, PingPongBuffer* write2){

    myDevice = device;
    readBuffer = read;
    writeBuffer = write;
    pingPongWriteBuffer = write2;

    // currentState = StopState;       //First test with ready state

    currentState = InitState;

    temporaryBuffer = (char*)malloc(62*sizeof(char));
    temporarySendBuffer = (char*)malloc(62*sizeof(char));

    commandBuffer = (char*)malloc(15*sizeof(char));
    atCommandSend = "AT+USOWR=0,62\r";
    atCommandPingPongBufferSend = "at+usost=0,\"178.196.15.59\",44444,10,\""; // 37    the value 0,XX depends on the PingPongBuffer::AbsolutBufferLength!!!
    atEnterCommand = "\"\r\n"; // length 3
    atDirectLinkCommand = "AT+USODL=0\r";
    atReadyRequest="AT\r";
    atDirectLinkOk="CONNECT";
    stringEnd='\0';
    atResponseOk="OK";

    for(int j=0; j < MAX_AT_COMMANDS; ++j)
        atCommandSendp[j] = &atCommandSendArray[j][0];

    uart_poll=0;
    pollingThreadStatus=0;

 //   pollingThreadParameters.myDevice = device;
 //   pollingThread = new pthread_t();
 //  pthread_create(pollingThread, NULL,atCommander::pollingThreadStart, (void*)&pollingThreadParameters);

}
atCommander::~atCommander(){


}

void atCommander::process(Event e){

    switch(currentState){


    case StopState :{
        bool successful=false;
        if(e == evStart){
            //bool successful = initTobyModul();
            if(successful){
                currentState = WaitState;
                PX4_INFO("switch state to WaitState");
                PX4_INFO("Direct Link Connection");
                myDevice->write(atDirectLinkCommand,11);
                //const char* pch = "CONNECT";
                //const char* stringEnd = '\0';

                bzero(temporaryBuffer,62);
                PX4_INFO("Polling for CommandOK");
                int poll_return = 0;
                while(poll_return < 1){
                    //some stupid polling;
                    poll_return = myDevice->poll(0);
                    usleep(100000);
                }
                int read_return = myDevice->read(temporaryBuffer,62);
                PX4_INFO("read returns : %d",read_return);
                if(strstr(temporaryBuffer, atDirectLinkOk) != NULL){
                    PX4_INFO("response sucessfull : %s",temporaryBuffer);

                }
                else{
                    PX4_INFO("ERROR response: %s",temporaryBuffer);
                    currentState = ErrorState;
                    //PX4_INFO("response : %s",temporaryBuffer);
                    //strncpy(temporaryBuffer,stringEnd,62);
                    //read_return = myDevice->read(temporaryBuffer,62);
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

        PX4_INFO("InitState");
        if(e == evStart){

            if(tobyAlive(100)==true){

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
        break;
    }
    case ErrorState :{
        PX4_INFO("ErrorState");
        // error handling, maybe reinitialize32 toby modul?
        break;

    case SetupState :{

            PX4_INFO("ReadyState");
            PX4_INFO("Direct Link Connection");
            myDevice->write(atDirectLinkCommand,getAtCommandLenght(atDirectLinkCommand));
            //const char* pch = "CONNECT";
            //const char* stringEnd = '\0';


            PX4_INFO("Polling for CommandOK");
            int poll_return = 0;
            while(poll_return < 1){
                //some stupid polling;
                poll_return = myDevice->poll(0);
                usleep(100000);
            }

            bzero(temporaryBuffer,62);
            PX4_INFO("temporaryBuffer check : %s",temporaryBuffer);

            int read_return = myDevice->read(temporaryBuffer,62);

            PX4_INFO("read returns : %d",read_return);
            PX4_INFO("Direct Link : %s",temporaryBuffer);

            if(strstr(temporaryBuffer, atDirectLinkOk) != 0){
                sleep(1);
                readerParameters.myDevice = myDevice;
                readerParameters.readBuffer = readBuffer;
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


            }
            break;
        }
        default :

            break;


        }

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

    //usleep(500);

    /*bool check = atCommanderFSM->tobyAlive(100);

    PX4_INFO("tobyAlive %d", check);

    int sdreturn = atCommanderFSM->readAtfromSD();

    PX4_INFO("read from sd card %d", sdreturn);

    atCommanderFSM->printAtCommands();

    bool initcheck = atCommanderFSM->initTobyModul();
    //atCommanderFSM->process(evStart);

    PX4_INFO("initcheck %d", initcheck);

    atCommanderFSM->printAtCommands();*/

    sleep(2);

    PX4_INFO("Beginn with transfer");

    int poll_return = 0;
    // int write_return = 0; // number data to write

    //char* temporaryReadBuffer = (char*)malloc(62*sizeof(char));


    //return NULL;

    //to start all
    //atCommanderFSM->process(evStart);

    char buf[20];
    char sendbuf[]="Hallo";

    while(1){
        while (1) {


            if(!pollingThreadStatus){
                atTobyDevice->write(sendbuf,5);
                pollingThreadStatus=1;

            }

            if(pollingThreadStatus)
            {

                poll_return= atTobyDevice->poll(0);
                //PX4_INFO("polling Thread : I'am polling");
                if(poll_return > 0){
                    //PX4_INFO("polling Thread : poll was successfull");
                    uart_poll =1;
                }
            }

            if(poll_return > 0){
                pollingThreadStatus=0;
                usleep(10000);
                atTobyDevice->read(buf,20);
                PX4_INFO("buf %s", buf);
            }
            usleep(10000);

        }



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

}


/**
 * @brief Checks the connection to Toby
 *
 *Sends a "AT" and checks if a "AT OK" returns
 *
 * @param times, the pixhawk trys so many times to get a answer from Toby
 */

bool atCommander::tobyAlive(int times){

    PX4_INFO("Check %d times for Toby", times);
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
        }
        ++i;
        sleep(1);
    }
    while((returnValue != true)&(i<times));

    return returnValue;
}

/**
 * @brief Initialized the Toby
 *
 * initialized the Toby Modul with the At-Commands from atCommandSendArray
 */
bool atCommander::initTobyModul(){

    //int         returnPollValue= 0;
    //int         returnWriteValue= 0;
    int         returnValue= 0;

    //const char* pch = "OK";
    int i = 0;
    //bool tobyReady =false;
    //int i   =0;

    /*const char *at_command_send[18]={"ATE0\r",
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

                                                       };*/

    //Anpassung da malloc nicht funktioniert,
    //damit Funktionen weiterverwendet werden können
    //char* atCommandSendp[MAX_AT_COMMANDS];

    // for(int j=0; j < MAX_AT_COMMANDS; ++j)
    // atCommandSendp[j] = &atCommandSendArray[j][0];

    //int numberOfAT=readAtfromSD(atCommandSendp);

    //printAtCommands(atCommandSendp,numberOfAT);

    PX4_INFO("Beginn with Initialization");

    //char* stringEnd = '\0';
    while(i < numberOfAt){
        myDevice->write(atCommandSendp[i],getAtCommandLenght(atCommandSendp[i]));
        while(returnValue < 1){
            //some stupid polling;
            //PX4_INFO("Polling");
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


        //dirty way to flush the read buffer
        //strncpy(temporaryBuffer,stringEnd,62);
        bzero(temporaryBuffer,62);


        returnValue = 0;
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

/**
 * @brief returns the lenght of a Chararray
 *
 * return value includes '\r'
 * this function is needed because strleng() doesn't work
 */
int atCommander::getAtCommandLenght(const char* at_command)
{
    int k=0;
    while(at_command[k] != '\0')
    {
        k++;
    }

    return k;
}

/**
 * @brief Gibt die AT-Commands als PX4_Info aus
 *
 *
 */
void atCommander::printAtCommands()
{
    PX4_INFO("%s beinhaltet %d Zeilen",PATH,numberOfAt);
    for(int j=0 ; j < numberOfAt ; j++)
    {
        PX4_INFO("Inhalt %d %s",j ,atCommandSendp[j]);
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
bool atCommander::readAtfromSD()
{
    FILE*   sd_stream               = fopen(PATH, "r");
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

/**
 * @brief Polls on the Uart
 *
 * sets the Global variable poll_return if Datas are available
 * we need this function because there's no SIGIO
 */
void *atCommander::pollingThreadStart(void *arg)
{
    threadParameter *arguments = static_cast<threadParameter*>(arg);
    TobyDevice* myDevice = arguments->myDevice;

    int poll_return;
    while(1)
    {
        if(pollingThreadStatus)
        {
            poll_return= myDevice->poll(0);
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
