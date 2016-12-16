/**
* @file     toby.cpp
*
* @brief    Device Class for Toby LTE-Module
* @author   Marc Brändli & Michael Lehmann
* @date     10.10.2016
 */

#include <stdio.h>
#include <termios.h>
#include <px4_config.h>
#include <px4_posix.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
#include <fcntl.h>

#include "drivers/toby/tobyDevice.h"
#include "drivers/toby/tobyDeviceUart.h"

#include "drivers/toby/atCommander.h"
#include "drivers/drv_toby.h"
#include "toby.h"


extern "C" __EXPORT int toby_main(int argc, char *argv[]);

enum{
    INITIALIZINGSTEP = 10   // after 10 times a initializing step failed, it produces an error
};


Toby::Toby() :
#ifdef __PX4_NUTTX
    CDev("Toby", "/dev/toby")

#else
    //copied from other driver implementations
    VDev("toby", "/dev/toby")
#endif
{
    init();
    for(int j=0; j < MAX_AT_COMMANDS; ++j)
        atCommandSendp[j] = &atCommandSendArray[j][0];

    temporarySendBuffer = (char*)malloc(62*sizeof(char));
    temporaryBuffer = (char*)malloc(62*sizeof(char));


    atEnterCommand              = "\"\r\n"; // length 3
    atDirectLinkRequest         = "AT+USODL=0\r";
    atReadyRequest              ="AT\r";
    atDirectLinkOk              ="CONNECT";
    stringEnd                   ='\0';
    atResponseOk                ="OK";
    atResetCommand              ="AT+CFUN=16\r";
    atExitDirectLink            ="+++\r";




    this->myTobyDevice = new TobyDeviceUart();

    if(this->myTobyDevice == NULL){
        PX4_INFO("ERROR myTobyDevice is a NULL-Pointer!!!!");

    }


    initLTEModule();

}


Toby::~Toby()
{

    if(myTobyDevice != nullptr){
        //should already be deleted in close(), but if close never never is called:
        delete myTobyDevice;
    }



}


int Toby::init()
{
    PX4_INFO("TOBY::init");
#ifdef __PX4_NUTTX
	CDev::init();
#else
	VDev::init();
#endif

    return 0;
}




int	Toby::close(device::file_t *filp){


    int closed =  ::device::CDev::close(filp);
    delete myTobyDevice;
    myTobyDevice = nullptr;

    PX4_INFO("closed sucessfully");

    return closed;


}


ssize_t	Toby::read(device::file_t *filp, char *buffer, size_t buflen)
{
    int ret = myTobyDevice->read(buffer,buflen);
    PX4_INFO("READ is called &d", ret);

    return ret;

}

ssize_t	Toby::write(device::file_t *filp, const char *buffer, size_t buflen){


    return myTobyDevice->write(buffer,buflen);
}


off_t Toby::seek(device::file_t *filp, off_t offset, int whence){

    return ::device::CDev::seek(filp,offset,whence);
}




int
Toby::ioctl(device::file_t *filp, int cmd, unsigned long arg)
{
    return myTobyDevice->ioctl(cmd,arg);

}


int	Toby::poll(device::file_t *filp, struct pollfd *fds, bool setup){

    int ret =  myTobyDevice->poll(100);
    PX4_INFO("POLL is called %d",ret);

    return ret;


}


namespace
{
Toby *gToby;
}

int
toby_main(int argc, char *argv[])
{

    //Load start parameter laden
    int myoptind = 1;
    const char *verb = argv[myoptind];


    if (argc < 2) {

        //stop test reset und status in planung
        PX4_ERR("unrecognized command, try 'start', 'stop', 'test' or 'status'");
        PX4_ERR("[-d " TOBY_DEVICE_PATH "][-f (for enabling fake)][-s (to enable sat info)]");
        return 1;
    }

    /*
     * Start/load the driver.
     */

    if (!strcmp(verb, "start")) {
        if (gToby != nullptr) {
            warnx("already started");
            return 1;
        }


        //only one instance
        gToby = new Toby();

        return 0;
    }


    /** This is only needed because mavlink don't close his comm-channel
     *  so Toby:close is never called
     *
     */

    if (!strcmp(argv[1], "stop")) {
    }



    /*
     * delete the driver. Be sure what you do, so if mavlink is allready running,
     * it will crash!
     */
    if (!strcmp(argv[1], "delete")) {
        if(gToby != nullptr){
            delete gToby;
            gToby = nullptr;
        }
    }

    /*
     * Print driver status.
     */
    if (!strcmp(argv[1], "status")) {
    }



    //return value is not valid yet
    return 0;


}




void Toby::printStatus(void){
    // may we could print here some net strenght, connection details of toby l210 etc
}

int Toby::open(device::file_t *filp){
    PX4_INFO("Toby::open() is called");
    //filp->f_oflags =  OK;

    // we don't wan't that toby is opened twice
    if(CDev::is_open()){
        PX4_INFO("Toby::open() already open, return -1");
        return -1;
    }

    CDev::open(filp);
    //open TobyDevice, is not possible in an other way

    //



    return OK;
}





void *Toby::doClose(void *arg)
{


    PX4_INFO("Thread started");
    pid_t x = ::getpid();
    PX4_INFO("actual thread id : %d",x);
    int i = px4_close(4);

    PX4_INFO("Thread closed Uart with %d",i);


    return NULL;
}


int Toby::set_flowcontrol(int fd, int control)
{
    PX4_INFO("set_flowcontrol started");

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0)
    {
        perror("error from tggetattr");
        return -1;
    }

    if(control) tty.c_cflag |= CRTSCTS;
    else tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        perror("error setting term attributes");
        return -1;
    }
    return 0;
}




//*******************************************************************************************

bool Toby::initLTEModule(void){

    bool ret = tobyAlive(10);
    if(!ret){
        PX4_INFO("tobyAlive failed");

        return false;
    }

    int numberOfCommands = readAtFromSD();

    if(numberOfCommands < 1){

        PX4_INFO("readAtFromSD failed");

        return false;
    }

    printAtCommands();
    ret = initTobyModul();

    if(!ret){
        PX4_INFO("init failed");
        return false;
    }

    ret = setDirectLinkMode();
    if(!ret){
        PX4_INFO("directLInkMode failed");

        return false;
    }




    return true;
}


bool Toby::tobyAlive(int times){

    PX4_INFO("Check %d times for Toby", times);
    bool    returnValue     =false;
    int     returnPollValue =0;
    int     returnWriteValue=0;
    int     i               =0;

    do{

        returnWriteValue = myTobyDevice->write(atReadyRequest,getAtCommandLength(atReadyRequest));

        if(returnWriteValue > 0){

            returnPollValue = myTobyDevice->poll(0);
            PX4_INFO("tobyAlive returnPollValue :%d",returnPollValue);
            sleep(1);
        }
        if(returnPollValue>0){
            bzero(temporaryBuffer,62);
            PX4_INFO("Jetzt wird gelesen");
            myTobyDevice->read(temporaryBuffer,62);
            PX4_INFO("tobyAlive answer :%s",temporaryBuffer);
            if(strstr(temporaryBuffer,atResponseOk) != 0){
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




bool Toby::initTobyModul(){

    int         returnValue= 0;
    int i = 0;


    PX4_INFO("Beginn with Initialization");
    int timeout = 0;
    bool success = true;

    while((i < numberOfAt) & success){
        myTobyDevice->write(atCommandSendp[i],getAtCommandLength(atCommandSendp[i]));
        while(returnValue < 1){
            //some stupid polling, we wait for answer
            returnValue = myTobyDevice->poll(0);
            usleep(5000);
        }
        sleep(1);
        returnValue = myTobyDevice->read(temporaryBuffer,62);
        if(strstr(temporaryBuffer,atResponseOk) != 0){
            PX4_INFO("Command successful : %s",atCommandSendp[i]);
            PX4_INFO("answer: %s",temporaryBuffer);

            ++i; //sucessfull, otherwise, retry
        }
        else{
            PX4_INFO("Command failed: %s", atCommandSendp[i]);
            if(i > 0){
                --i; //we try the last command befor, because if we can't connect to static ip, it closes the socket automatically and we need to reopen

                if(timeout > INITIALIZINGSTEP){ //check 10 times otherwise we failed
                    success = false;
                }

                ++timeout; // count the times it failed
            }
        }

        bzero(temporaryBuffer,62);
        returnValue = 0;
    }

    PX4_INFO("sucessfull init");

    return success;

}

bool Toby::setDirectLinkMode(void){


    PX4_INFO("Direct Link Connection");
    myTobyDevice->write(atDirectLinkRequest,getAtCommandLength(atDirectLinkRequest));

    int poll_return = 0;
    while(poll_return < 1){
        //some stupid polling;
        poll_return = myTobyDevice->poll(200);
        usleep(100000); //wait for all data in uart-buffer
    }

    bzero(temporaryBuffer,62);
    myTobyDevice->read(temporaryBuffer,62);

    PX4_INFO("Direct Link : %s",temporaryBuffer);

    if(strstr(temporaryBuffer, atDirectLinkOk) != 0){
        return true;
    }
    else{
        return false;
    }
}




int Toby::getAtCommandLength(const char* at_command)
{
    int k=0;
    while(at_command[k] != '\0')
    {
        k++;
    }

    return k;
}


void Toby::printAtCommands()
{
    PX4_INFO("%s contents %d rows",SD_CARD_PATH,numberOfAt);
    for(int j=0 ; j < numberOfAt ; j++)
    {
        PX4_INFO("content %d %s",j ,atCommandSendp[j]);
    }
}


bool Toby::readAtFromSD()
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

            /*There was no possibilty to allocate space for at-commands using malloc
             * didn't work
             */
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
