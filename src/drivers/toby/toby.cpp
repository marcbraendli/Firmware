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
#include "drivers/toby/tobyDeviceUart.h"

#include "drivers/toby/atCommander.h"
#include "drivers/drv_toby.h"
#include "toby.h"


extern "C" __EXPORT int toby_main(int argc, char *argv[]);



Toby::Toby() :
#ifdef __PX4_NUTTX
    CDev("Toby", "/dev/toby")

#else
    //copied from other driver implementations
    VDev("toby", "/dev/toby")
#endif
{
	init();
    writeBuffer = new TobyRingBuffer(10,158);
    readBuffer = new TobyRingBuffer(10,158);
    writePongBuffer = new PingPongBuffer();
    writeDataPipe = new TobyDataPipe(256);
    readDataPipe = new TobyDataPipe(256);
    atCommanderThread = nullptr;
}


Toby::~Toby()
{

    if(myTobyDevice != nullptr){
        //should already be deleted in close(), but if close never never is called:
        delete myTobyDevice;
    }

    delete writeBuffer;
    delete readBuffer;
    delete writePongBuffer;
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


    stopAllThreads();
    int closed =  ::device::CDev::close(filp);
    delete myTobyDevice;
    myTobyDevice = nullptr;

    PX4_INFO("closed sucessfully");

    return closed;


}


ssize_t	Toby::read(device::file_t *filp, char *buffer, size_t buflen)
{

    int i = 0;
    i = readDataPipe->getItem(buffer,buflen);
  //  i = (readBuffer->getString(buffer,buflen));

    PX4_INFO("Mavlink reads succesful %d",i);

    return i;

}

ssize_t	Toby::write(device::file_t *filp, const char *buffer, size_t buflen){

    //no space, buffer is full -> does not work, communication lost
    //however we lock the mavlink-thread in buffer
    /*
     if(writeBuffer->full()){
         return 0;
     }
     */

     //writeBuffer->putString(buffer,buflen);
     writeDataPipe->putItem(buffer,buflen);
     return buflen;
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


    if(!(readDataPipe->isEmpty())){
        poll_notify(POLLIN);
        poll_notify_one(fds, POLLIN);
        PX4_INFO("Mavlink polls successful");
        return  1;

    }

    else{
        PX4_INFO("Mavlink poll fails");

        return 0;
    }


/*
    if(!readBuffer->empty()){
        poll_notify(POLLIN);
        poll_notify_one(fds, POLLIN);
        return  1;

    }

    else{
        return 0;
    }

    */

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
       if(gToby != nullptr)
        gToby->stopAllThreads();
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
        if(gToby != nullptr)
         gToby->printStatus();
    }

    //return value is not valid yet
    return 0;


}

void Toby::stopAllThreads(void){

    threadExitSignal = true;
    if(atCommanderThread != nullptr){
         pthread_join(*atCommanderThread,NULL);
         PX4_INFO("Stop successful");
    }




}

void Toby::printStatus(void){
    // may we could print here some net strenght, connection details of toby l210 etc
    if(threadExitSignal){
        PX4_INFO("Toby-Status : Toby L210 is stopped");
    }
    else{
        PX4_INFO("Toby-Status : Toby L210 is currently running");
    }
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
    this->myTobyDevice = new TobyDeviceUart();

    if(this->myTobyDevice == NULL){
        PX4_INFO("ERROR myTobyDevice is a NULL-Pointer!!!!");

    }





    //same for readerthread


    //****************Test some threading thing's****************
    // Dangerous passing the myTobyDevice to Thread ... isn't information hiding anymore???!

    workerParameters.myDevice= myTobyDevice;
    workerParameters.writeBuffer = writeBuffer;
    workerParameters.readBuffer = this->readBuffer;

    //define the worker with the declareted parameters
    workerParameters.writePongBuffer = writePongBuffer;
    //writerThread = new pthread_t;
    //pthread_create(writerThread, NULL, writeWork, (void*)&workerParameters);


    readerParameters.myDevice = myTobyDevice;
    readerParameters.readBuffer = this->readBuffer;
   // readerThread = new pthread_t;
    //pthread_create(readerThread, NULL, readWork, (void*)&readerParameters);



    //***************Initialize the at-CommanderThread which hold's the Statemachine********


    atCommanderParameters.myDevice = myTobyDevice;
    atCommanderParameters.readBuffer = readBuffer;
    atCommanderParameters.writeBuffer= writeBuffer;
    atCommanderParameters.writePongBuffer= writePongBuffer;
    atCommanderParameters.writeDataPipeBuffer = writeDataPipe;
    atCommanderParameters.readDataPipeBuffer = readDataPipe;
    atCommanderParameters.threadExitSignal = &threadExitSignal;
    threadExitSignal = false;

    atCommanderThread = new pthread_t;
    int ret = pthread_create(atCommanderThread, NULL, atCommander::atCommanderStart, (void*)&atCommanderParameters);
    if(ret){
        PX4_INFO("Toby::open() failed");
        return -1;
    }


    //**************************Initialize the polling-Thread******************************

  //  pthread_create(pollingThread, NULL, pollingThreadStart, (void*)&pollingThreadParameters);


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
    //BoundedBuffer* writeBuffer = arguments->writeBuffer;
    TobyDevice* myDevice = arguments->myDevice;
    PingPongBuffer* writePongBuffer = arguments->writePongBuffer;



    //we need some space, only once needed ... the size of the space is not fix yet,
    //TODO : FIX THE SPACE-PROBLEM, how much space should we give? Or maybe, it is saved directly in bufferer (more elegant)
    char* data = (char*)malloc(62*sizeof(char));
    char* readBuffer = NULL;

    // For debugging
    if(data == NULL){
        PX4_INFO("ERROR writeWork couldn't get space from malloc");
        return NULL;
    }

 //   for(int i = 0; i < 3; ++i)
    //TODO : Implement thread should exit logik
    while(1){

        //get data from buffer
        //variante 1) : size = writeBuffer->getString(data,62);

        if(writePongBuffer->DataAvaiable()){
            readBuffer = (writePongBuffer->getActualReadBuffer());
        }
           int write_return =  myDevice->write(readBuffer,PingPongBuffer::AbsolutBufferLength);
           if(write_return != PingPongBuffer::AbsolutBufferLength){
               PX4_INFO("ERROR, only write %d instead of 124",write_return);

           }
            writePongBuffer->GetDataSuccessfull();
            usleep(100000);

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
    TobyRingBuffer* readBuffer = arguments->readBuffer;
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



