/**
* @file     toby.cpp
*
* @brief    Device Class für das Toby Modul
* @author   Michael Lehmann
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
#include "drivers/drv_toby.h"
#include "toby.h"


extern "C" __EXPORT int toby_main(int argc, char *argv[]);
bool AT_RESPONSE= false;

/* Hilfs und Testfunktionen deklaration evt später als private in Klasse implementieren!*/
int set_flowcontrol(int fd, int control);
void *doClose(void *arg);
void *writeHard(void *arg);
int toby_init();
int writeHardHelper(TobyDevice* myDevice);

/**
* @brief Default Konstruktor
*
* Initialisiert dies und jenes
*
* @date ?
*/
Toby::Toby() :
#ifdef __PX4_NUTTX
    CDev("Toby", "/dev/toby")

#else
    //Übernommen aus anderen Treiberimplementationen
    VDev("toby", "/dev/toby")
#endif
{
	init();
    writeBuffer = new BoundedBuffer();
    readBuffer = new BoundedBuffer();
    done = true;
    writePongBuffer = new PingPongBuffer();
}

Toby::~Toby()
{

    if(myTobyDevice != nullptr){
        //should already be deleted in close(), but if close never never is called:
        delete myTobyDevice;
    }
}

/**
* @brief Initialisierungsroutine
*
*<br><b>Hier wird folgendes Initialisiert:</b>
*<br>dieses
*<br>und jenes
*
* @date ?
*/
int Toby::init()
{
    PX4_INFO("TOBY::init");
#ifdef __PX4_NUTTX
	CDev::init();
#else
	VDev::init();
#endif
    //myTobyDevice = new TobyDevice();
    toby_init();

    /*Unfortunately, this is not possible, because if we open the TobyDevice here, which would open the uart,
    the file descrptor to the uart is only valid in the process(the shell or autostart) which start's toby, but if
    Mavlink open() the toby itself, the uart descriptor in the tobyDevice isn't valid anymore. It would be possible
    if we share the file descriptor over an InterProcessCommunication, but uORB can't do that.*/
    //myTobyDevice = new TobyDevice();

    return 0;
}




int	Toby::close(device::file_t *filp){


    //new function
    PX4_INFO("Toby::close() is called, we close with");
    pthread_join(*writerThread,NULL);
    PX4_INFO("Thread terminated");


    int closed =  ::device::CDev::close(filp);
    //TobyDevice closes the uart himself
    delete myTobyDevice;
    myTobyDevice = nullptr;


    return closed;


}


ssize_t	Toby::read(device::file_t *filp, char *buffer, size_t buflen)
{

    int i = 0;
    i = (readBuffer->getString(buffer,buflen));


    return i;

}

ssize_t	Toby::write(device::file_t *filp, const char *buffer, size_t buflen){



     writeBuffer->putString(buffer,buflen);

     return buflen;

}


off_t Toby::seek(device::file_t *filp, off_t offset, int whence){
    PX4_INFO("seek() is called");

    return ::device::CDev::seek(filp,offset,whence);

}




int
Toby::ioctl(device::file_t *filp, int cmd, unsigned long arg)
{

    return myTobyDevice->ioctl(cmd,arg);
}


int	Toby::poll(device::file_t *filp, struct pollfd *fds, bool setup){


    if(!readBuffer->empty()){
      //  PX4_INFO("Toby : poll() return 1, data avaiable");
        poll_notify(POLLIN);
        poll_notify_one(fds, POLLIN);
        return  1;

    }

    else{
        return 0;
    }


}





int toby_init(){

    return 0;
    //todo : initalization toby L210 Module with AT-Command

}

namespace
{
Toby *gToby;
}

int
toby_main(int argc, char *argv[])
{
    /* set to default */
    const char *device_name = TOBY_DEVICE_PATH;

    //Load start parameter laden
    int myoptind = 1;
    const char *verb = argv[myoptind];


    if (argc < 2) {

        //stop test reset und status in planung
        PX4_ERR("unrecognized command, try 'start', 'stop', 'test', 'reset' or 'status'");
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


        //"einmalige" Instanzierung
        gToby = new Toby();

        return 0;
    }


    //todo : Alle input parameter handler implementieren
    // Stubs für andere start-argumente
    if (!strcmp(argv[1], "stop")) {
    }

    /*
     * Test the driver/device.
     */
    if (!strcmp(argv[1], "test")) {
    }

    /*
     * Reset the driver.
     */
    if (!strcmp(argv[1], "reset")) {
    }

    /*
     * Print driver status.
     */
    if (!strcmp(argv[1], "status")) {
    }

    if(device_name){

    }


    //return value is not valid yet
    return 0;


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
    this->myTobyDevice = new TobyDevice();

    if(this->myTobyDevice == NULL){
        //some debug
        PX4_INFO("ERROR myTobyDevice is a NULL-Pointer!!!!");

    }



    //****************Test some threading thing's****************
    // Dangerous passing the myTobyDevice to Thread ... isn't information hiding anymore???!
    workerParameters.myDevice= myTobyDevice;
    workerParameters.writeBuffer = writeBuffer;
    workerParameters.readBuffer = this->readBuffer;

    //define the worker with the declareted parameters
    workerParameters.writePongBuffer = writePongBuffer;
    writerThread = new pthread_t;
    pthread_create(writerThread, NULL, writeWork, (void*)&workerParameters);


    readerParameters.myDevice = myTobyDevice;
    readerParameters.readBuffer = this->readBuffer;
    readerThread = new pthread_t;
    pthread_create(readerThread, NULL, readWork, (void*)&readerParameters);

    PX4_INFO("Toby:: exit open()");


    usleep(100);


    return OK;
}




// this is the write thread, reads from the buffer and writes to tobyDevice
// only for testing of concept and Buffer threadsafety needed, later, we make a AT-Commander, which
// will control our MavLink data
void* Toby::writeWork(void *arg){

    PX4_INFO("writeWork Thread started");
    //extract arguments :
    myStruct *arguments = static_cast<myStruct*>(arg);
    TobyDevice* myDevice = arguments->myDevice;

    PingPongBuffer* writePongBuffer = arguments->writePongBuffer;
    //BoundedBuffer* writeBuffer = arguments->writeBuffer;




    //we need some space, only once needed ... the size of the space is not fix yet,
    //TODO : FIX THE SPACE-PROBLEM, how much space should we give? Or maybe, it is saved directly in buffer (more elegant)
    char* data = (char*)malloc(62*sizeof(char));
    char* readBuffer = NULL;

    // For debugging
    if(data == NULL){
        PX4_INFO("ERROR writeWork couldn't get space from malloc");
        return NULL;
    }

    int size = 0;


 //   for(int i = 0; i < 3; ++i)
    //TODO : Implement thread should exit logik
    while(1){

        //get data from buffer
        //variante 1) : size = writeBuffer->getString(data,62);

        if(writePongBuffer->DataAvaiable()){
            readBuffer = (writePongBuffer->getActualReadBuffer());
           int write_return =  myDevice->write(readBuffer,PingPongBuffer::AbsolutBufferLength);
           if(write_return != PingPongBuffer::AbsolutBufferLength){
               PX4_INFO("ERROR, only write %d instead of 124",write_return);

           }
            writePongBuffer->GetDataSuccessfull();
            usleep(100000);
        }

        else{
            usleep(50);
        }

        //write data to hardware
        if(size > 62){
            PX4_INFO("ERROR Thread has to write size: %d",size);
        }

          // write data to device
          // variante 1) : myDevice->write(data,size)

    }

    sleep(2);


    free(data);
    return NULL;

}


// this is the read thread, reads from the buffer and returns it in the toby read function.
// only for testing of concept and Buffer threadsafety needed, later, we make a AT-Commander, which
// will control our MavLink data
void* Toby::readWork(void *arg){

    PX4_INFO("readWork Thread started");
    //extract arguments :
    myStruct *arguments = static_cast<myStruct*>(arg);
    BoundedBuffer* readBuffer = arguments->readBuffer;
    TobyDevice* myDevice = arguments->myDevice;




    // a ty
    char* buffer = (char*)malloc(60*sizeof(char));


    if(myDevice == NULL || readBuffer == NULL){
        PX4_INFO("readWork Thread parameters invalid");

    }
    int i = 0; // poll result handle
    int u = 0; //size of data received
    while(1){

        i = myDevice->poll(0);
        if(i>0){
          u =  myDevice->read(buffer,60);
           readBuffer->putString(buffer,u);

        }
        else{
          //  PX4_INFO("readWorker : poll got 0");

        }
        if(buffer == NULL){
            PX4_INFO("READ WORKER NULLPOINTER");
        }

    }


    PX4_INFO("readWork exit");

    return NULL;

}




//****************************Helperfunctions, may going into a separate header File********************************


int set_flowcontrol(int fd, int control)
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



