#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>


#include "atCommander.h"
#include "toby.h"
#include "px4_log.h"

//for readATfromSD
#define PATH_TO_AT_COMMAND       "/fs/microsd/toby/at-inits.txt"
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

    currentState = InitModulState;       //First test with ready state
    //currentState = InitWriteState;

    temporaryBuffer = (char*)malloc(62*sizeof(char));
    commandBuffer = (char*)malloc(15*sizeof(char));

    atCommandSend   ="AT+USOWR=0,62\r";
    atCommandPingPongBufferSend = "AT+USOWR=0,62\r"; // the value 0,XX depends on the PingPongBuffer::AbsolutBufferLength!!!
    response_at     ="@";
    response_ok     ="OK";
    atEnterCommand  ="\r";
    stringEnd       ='\0';

    uart_poll=0;
    pollingThreadStatus=0;

    //pollingThreadParameters.myDevice = myDevice;
    //pthread_create(pollingThread,NULL ,atCommander::pollingThreadStart,(void*)&pollingThreadParameters);

}
atCommander::~atCommander(){


}

void atCommander::process(Event e){

    char    inputBuffer[100]    ="";
    int     write_return        =0;
    char*   sendDataPointer     =0;

    switch(currentState){

    case InitModulState:{

        PX4_INFO("StateMachine: InitModulState");

        if(e == evStart){
            bool successful = initTobyModul();

            if(successful){
                PX4_INFO("sucessfull init, switch to WaitState");
                currentState = WaitState;

            }
            else{
                PX4_INFO("switch to ErrorState");
                currentState = ErrorState;

            }
        }

        /*switch(e){
        case evReadDataAvailable:
            PX4_INFO("switch state to InitModuleReadState");
            currentState = InitModuleReadState;
            break;
        case evStart:
            currentState = InitModuleWriteState;
            break;
        default:
            break;
        }*/

        break;
    }
        /*    case InitModuleWriteState :{
        PX4_INFO("StateMachine: InitModuleWriteState ");

        atCommandLenght =getAtCommandLenght(atCommandSendp[i]);
        write_return = myDevice->write(atCommandSendp[i], atCommandLenght);
        if(write_return ==  atCommandLenght)
        {
            PX4_INFO("StateMachine: InitModuleWriteState, change to InitModulState");
            pollingThreadStatus =1;
            currentState=InitModulState;

        }else{
            PX4_INFO("StateMachine: InitModuleWriteState, Fu**");
            //Falls schreibanfrage nicht akzeptiert wurde, delay und nochmals versuchen
            usleep(5000);
        }
        break;
    }
    case InitModuleReadState:{
        PX4_INFO("StateMachine: InitModuleReadState");

        myDevice->read(inputBuffer,READ_BUFFER_LENGHT);
        PX4_INFO("answer :%s",inputBuffer);

        if(strstr(inputBuffer,response_ok) != NULL){
            PX4_INFO("answer :%s",inputBuffer);
            ++i; //damit nächster Befehl gesendet wird
        }
        bzero(inputBuffer,READ_BUFFER_LENGHT); //buffer leeren
        if(i>numberOfAT){
            //Initialisierung abgeschlossen, nun kann gesendet werden
            currentState = WaitState;
        }else{
            currentState = InitModuleWriteState;
        }
        break;
    }*/

    case WaitState :{
        PX4_INFO("StateMachine: WaitState");
        switch(e){
        case evWriteDataAvailable:
            currentState = InitWriteState;
            break;
        case evReadDataAvailable:
            currentState = ReadState;
            break;
        default:
            PX4_INFO("switch to ErrorState");
            currentState = ErrorState;
            break;
        }
    }
    case InitWriteState:{
        PX4_INFO("StateMachine: InitWriteState");

        write_return = myDevice->write(atCommandPingPongBufferSend,14);
        //PX4_INFO("StateMachine: InitWriteState, write_return: %d", write_return);
        if(write_return == 14)
        {
            PX4_INFO("StateMachine: InitWriteState, change to Waitstate");
            pollingThreadStatus =1;
            currentState=WriteState;

        }else{
            PX4_INFO("StateMachine: InitWriteState, no sucsessfull request");
            //Falls schreibanfrage wurde nicht akzeptiert, delay und nochmals versuchen
        }

        break;
    }
    case InitReadState:{
        PX4_INFO("StateMachine: InitReadState");
        myDevice->read(inputBuffer,READ_BUFFER_LENGHT);

        if(strstr(inputBuffer,response_at) != NULL){
            PX4_INFO("answer :%s",inputBuffer);
            currentState =ReadState;
        }else{ // Antwork enthält kein '@'
            PX4_INFO("StateMachine: InitReadState:, No @ ...");
            //Falls schreibanfrage nicht akzeptiert wurde, delay und nochmals versuchen
            usleep(5000);
            currentState =InitWriteState;
        }

        bzero(inputBuffer,READ_BUFFER_LENGHT); //buffer leeren
        break;
    }
    case ReadState:{
        PX4_INFO("StateMachine: ReadState");

        myDevice->read(inputBuffer,READ_BUFFER_LENGHT);
        PX4_INFO("answer :%s",inputBuffer);

        //Copy into ReadBuffer

        break;
    }
    case WriteState:{
        PX4_INFO("StateMachine: WriteState");


        sendDataPointer =  pingPongWriteBuffer->getActualReadBuffer();

        //usleep(50);

        write_return = myDevice->write(sendDataPointer,64); //the number depends on the buffer deepness!!!!
        if(write_return != 64){
            PX4_INFO("Error writing Data to UART");
            //Nochmaliges schreiben Initialisieren
            currentState=InitWriteState;
        }else{
            pingPongWriteBuffer->GetDataSuccessfull();
            // message that we can free the buffer
            pollingThreadStatus =1;
            currentState=WaitState;
        }
        //write_return = myDevice->write(atEnterCommand,1); //the number depends on the buffer deepness!!!!
        //usleep(50); //Test zwecke tracking an konsole da toby l210 noch nicht ufnktoniert

        break;
    default:
            break;
        }

    case ErrorState :{
        PX4_INFO("StateMachine: ErrorState");
        // error handling, maybe reinitialize toby modul?
        //reboot ?
        break;

    }//from case
    }//from switch


}//from void atCommander::process(Event e)


void* atCommander::atCommanderStart(void* arg){

    //eventuell in externen Header, saubere Trennung
    PX4_INFO("AT-Commander says hello");

    myStruct *arguments = static_cast<myStruct*>(arg);

    BoundedBuffer *atWriteBuffer = arguments->writeBuffer;
    BoundedBuffer *atReadBuffer = arguments->readBuffer;
    TobyDevice *atTobyDevice = arguments->myDevice;
    PingPongBuffer *pingPongWriteBuffer = arguments->writePongBuffer;

    //sleep(1);

    atCommander *atCommanderFSM = new atCommander(atTobyDevice,atReadBuffer,atWriteBuffer,pingPongWriteBuffer);
    //usleep(500);
    sleep(1);

    PX4_INFO("atCommanderFSM->process(evStart);");
    atCommanderFSM->process(evStart);
    //sleep(1);

    sleep(2); //Delay für Toby Initialisierung

    PX4_INFO("atCommanderStart::Beginn with transfer");

    while(1){

        if(pollingThreadStatus)
        {
            uart_poll = atTobyDevice->poll(0);
        }
        sleep(1);
        if(uart_poll > 0){
            uart_poll =0;
            PX4_INFO("Poll arrived");
            pollingThreadStatus =0;

            atCommanderFSM->process(evResponse);
            //atCommanderFSM->process(evReadDataAvailable);


            //*******************************************
            //read_return =  atTobyDevice->read(temporaryBuffer,62);
            //atReadBuffer->putString(temporaryBuffer,read_return);
            //poll_return = (atTobyDevice->poll(NULL,true));
        }

        // Nur Senden, wir testen einzig ob wir daten empfangen
        /*if(pingPongWriteBuffer->DataAvaiable()){
            PX4_INFO("process put evWriteDataAvaiable");
            atCommanderFSM->process(evWriteDataAvailable);
        }*/

    }
    return 0;
}
/**
 * @brief Takes the AT-Commands from the SD-Card and Initilazied Toby with these
 *
 * @return True uf sucessful
 */
bool atCommander::initTobyModul(){

    int pollReturn  = 0;
    int numberOfAt  = 0;
    int i           = 0;

    char   atCommandSendArray[MAX_AT_COMMANDS][MAX_CHAR_PER_AT_COMMANDS];


    //Anpassung da malloc nicht funktioniert,
    //damit aber Funktionen weiterverwendet werden können
    char* atCommandSendp[MAX_AT_COMMANDS];
    for(int j=0; j < MAX_AT_COMMANDS; ++j)
        atCommandSendp[j] = &atCommandSendArray[j][0];



    numberOfAt=readAtfromSD(atCommandSendp);
    PX4_INFO("number of AT Commands:: %d",numberOfAt);
    printAtCommands(atCommandSendp, numberOfAt);

    while(i < numberOfAt)
    {
        myDevice->write(atCommandSendp[i], getAtCommandLenght(atCommandSendp[i]));
        while(pollReturn < 1){
            //some stupid polling;
            // Senden & Empfangen von  50 Zeichen bei 57600 Baud dauert 2 x 0.86 ms =1,73ms
            pollReturn = myDevice->poll(2);
            usleep(100);
        }
        sleep(1);
        myDevice->read(temporaryBuffer,62);
        if(strstr(temporaryBuffer, response_ok) != NULL){
            PX4_INFO("Command Successfull : %s",atCommandSendp[i]);
            PX4_INFO("answer : %s",temporaryBuffer);

            ++i; //sucessfull, otherwise, retry
        }
        else{
            PX4_INFO("Command failed : %s", atCommandSendp[i]);
            if(i > 0){
                --i; //we try the last command before, because if we can't connect to static ip, it closes the socket automatically and we need to reopen
            }
        }
        //dirty way to flush the read buffer
        //strncpy(temporaryBuffer,stringEnd,62);
        bzero(temporaryBuffer,62);


        pollReturn = 0;
    }


    return true;

}

/**
 * @brief gibt die länge des AT-Commands zurück
 *
 * '\r'
 */
int atCommander::getAtCommandLenght(const char* atCommand)
{
    int k=0;
    while(atCommand[k] != *atEnterCommand)
    {
        k++;
    }

    return k;
}

/**
 * @brief Prints the AT-Commands in the Shell
 *
 *
 */
void atCommander::printAtCommands(char **atcommandbuffer, int atcommandbufferstand)
{
    PX4_INFO("%s beinhaltet %d Zeilen",PATH_TO_AT_COMMAND ,atcommandbufferstand);
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
{   FILE*   sd_stream               = fopen(PATH_TO_AT_COMMAND, "r");
    int 	atcommandbufferstand    = 0;
    int 	inputbufferstand        = 0;
    int 	c                       =-1;
    char 	inputbuffer[MAX_CHAR_PER_AT_COMMANDS]="";


    if(sd_stream) {
        PX4_INFO("SD-Karte offen");
        do { // read all lines in file
            do{ // read one line until EOF
                c = fgetc(sd_stream);
                if(c != EOF){
                    //(EOF = End of File mit -1 im System definiert)
                    inputbuffer[inputbufferstand]=(char)c;
                    inputbufferstand++;
                }
            }while(c != EOF && c != '\n');

            inputbuffer[inputbufferstand]=*atEnterCommand;


            //inputbufferstand++;
            //inputbuffer[inputbufferstand]=stringEnd; //Now we could work with string-fkt

            //Speicherplatz konnte nicht mit malloc alloziert werden! Griff auf ungültigen Speicherbereich zu.
            //atcommandbuffer[atcommandbufferstand] =(char*) malloc(20);
            //assert(atcommandbuffer[atcommandbufferstand]!=0);
            strcpy(atcommandbuffer[atcommandbufferstand],inputbuffer);

            inputbufferstand=0;
            atcommandbufferstand++;
        } while(c != EOF);
    }else{
        PX4_INFO("SD-Card NOT Open");
    }

    fclose(sd_stream);
    PX4_INFO("SD-Card Closed");

    atcommandbufferstand--;

    return atcommandbufferstand;
}


/**
 * @brief compares two char array NOT USED
 *
 * Compares two arrays until the first one is terminatet with '\0'
 *
 *
 * @param first char-array pointer (defines lenght)
 * @param second char-array pointer
 * @return Number of identical values
 */
int atCommander::string_compare(const char* pointer1,const char* pointer2)
{
    int i=0;
    do{
        i++;
    }while((pointer1[i]!='\0')&& (pointer2[i]==pointer2[i]));


    return i;
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
        sleep(1);
    }
}
