/****************************************************************************
 *
 *   Copyright (c) 2013 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file led.cpp
 *
 * LED driver.
 */



#include "toby.h"
#include <drivers/drv_toby.h>
#include <drivers/toby/tobyDevice.h>


#include <drivers/toby/atCommander.h>


#include <stdio.h>
#include <termios.h>

#include <px4_config.h>
#include <px4_posix.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
#include <fcntl.h>

/*
 * Ideally we'd be able to get these from up_internal.h,
 * but since we want to be able to disable the NuttX use
 * of leds for system indication at will and there is no
 * separate switch, we need to build independent of the
 * CONFIG_ARCH_LEDS configuration switch.
 */

//is not needed
/*
__BEGIN_DECLS
extern void toby_init();
__END_DECLS
*/

extern "C" __EXPORT int toby_main(int argc, char *argv[]);


/* Hilfs und Testfunktionen deklaration evt später als private in Klasse implementieren!*/

int set_flowcontrol(int fd, int control);
void *doClose(void *arg);
void *writeHard(void *arg);
int toby_init();
int writeHardHelper(TobyDevice* myDevice);


//signaling mutex for test, TODO: delet
//static pthread_mutex_t pollingMutex = PTHREAD_MUTEX_INITIALIZER;




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
    pthread_cond_init(&pollEventSignal,NULL);




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

    /* old function
    {
    PX4_INFO("close() is called, we close with %d",uart0_filestream);
    ::device::CDev::close(filp);

    //for Debugging
    pid_t x = ::getpid();
    PX4_INFO("actual thread id : %d",x);
    // **************************************


    tcflush(uart0_filestream, TCIFLUSH);

    //force a thread to close the uart
    pthread_t Toby_thread;
    pthread_create(&Toby_thread, NULL, doClose, NULL);


    //int i = set_flowcontrol(uart0_filestream,1);
    //PX4_INFO("set_flowcontrol returns with %d");
    //tcgetattr(uart0_filestream, &options);
    //this->unlock();
    //px4_close(uart0_filestream);

    //for Debugging
    PX4_INFO("toby::close() terminate");

    //return value not valid yet!
    return 0;
    }
    */

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
    /* old function
    {
    //PX4_INFO("read() is called");
    ::device::CDev::read(filp,buffer,buflen);
    int i = px4_read(uart0_filestream,buffer,buflen);

    return i;
}
    */

    //new function
    PX4_INFO("Toby::read() is called");








    //readBuffer->getString()
    //return myTobyDevice->read(buffer,buflen);
    int i = 0;
    i = (readBuffer->getString(buffer,buflen));
    PX4_INFO("Toby::read() read %d",i);

    return i;

}

ssize_t	Toby::write(device::file_t *filp, const char *buffer, size_t buflen){

 /* old function:
  {
    //todo : effizienter implementieren, but how?

    //Debugging
    // PX4_INFO("write() is called");

    int count = 0;
    if (uart0_filestream != -1)
    {
 //       PX4_INFO("::write() uart_filstream %d",uart0_filestream);


        count = px4_write(uart0_filestream, buffer, buflen);
        if (count < 0)
        {
            //Debugging
            PX4_INFO("UART TX error");
        }

    }
   // close(NULL);

  }
    */
    //the new function
  //  PX4_INFO("Toby::write() is called");


     writeBuffer->putString(buffer,buflen);
     //sleep(1);
    // return myTobyDevice->write(buffer,buflen);
     return buflen;



}


off_t Toby::seek(device::file_t *filp, off_t offset, int whence){
    PX4_INFO("seek() is called");

    return ::device::CDev::seek(filp,offset,whence);

}




int
Toby::ioctl(device::file_t *filp, int cmd, unsigned long arg)
{
    //PX4_INFO("ioctl mit cmd:  %d",cmd);
    //ein versuch :
    //int i = ::device::CDev::ioctl(filp,cmd,arg);
    //PX4_INFO("ioctl() return %d",i);

    //ioctl direct to uart
    //new function
    return myTobyDevice->ioctl(cmd,arg);
    //old function
    //return ::ioctl(uart0_filestream,cmd,arg);
}


int	Toby::poll(device::file_t *filp, struct pollfd *fds, bool setup){

    /* old Function
{
    px4_pollfd_struct_t fds1[1];
    fds1[0].fd = uart0_filestream;
    fds1[0].events = POLLIN;

    int poll_return = px4_poll(fds1,1,500);
    if(poll_return >0){
        //notify the caller
        poll_notify(POLLIN);
        poll_notify_one(fds, POLLIN);
    }

    //  PX4_INFO("poll() is called with return %d",poll_return);
}
*/
    // new function:

   // int poll_return = 0;
   // int poll_return = myTobyDevice->poll(fds,setup);
    PX4_INFO("poll() is called");





    if(!readBuffer->empty()){
        PX4_INFO("Toby : poll() return 1, data avaiable");
        poll_notify(POLLIN);
        poll_notify_one(fds, POLLIN);
        return  1;

    }

    else{
        PX4_INFO("Toby : poll() return 0, no data avaiable");
        return 0;
    }

/*

    PX4_INFO("poll() is called with return %d",&poll_return);

    if(poll_return > 0){

        poll_notify(POLLIN);
        poll_notify_one(fds, POLLIN);
    }
    return poll_return;

    */
}





int toby_init(){

    //für Debuggingzwecke
    PX4_INFO("toby_init");
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
        PX4_INFO("ERROR myTobyDevice is a NULL-Pointer!!!!");

    }





    //same for readerthread


    //****************Test some threading thing's****************
    // Dangerous passing the myTobyDevice to Thread ... isn't information hiding anymore???!

    workerParameters.myDevice= myTobyDevice;
    workerParameters.writeBuffer = writeBuffer;
    workerParameters.readBuffer = this->readBuffer;

    //define the worker with the declareted parameters


   // writerThread = new pthread_t;
    pthread_create(writerThread, NULL, writeWork, (void*)&workerParameters);


    readerParameters.myDevice = myTobyDevice;
    readerParameters.readBuffer = this->readBuffer;
    readerThread = new pthread_t;
   // pthread_create(readerThread, NULL, readWork, (void*)&readerParameters);



    //***************Initialize the at-CommanderThread which hold's the Statemachine********


    atCommanderParameters.myDevice = myTobyDevice;
    atCommanderParameters.readBuffer = readBuffer;
    atCommanderParameters.writeBuffer= writeBuffer;
    atCommanderThread = new pthread_t;
    pthread_create(atCommanderThread, NULL, atCommander::atCommanderStart, (void*)&atCommanderParameters);


    pollingThreadParameters.myDevice = myTobyDevice;
    pollingThread = new pthread_t;


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
    BoundedBuffer* writeBuffer = arguments->writeBuffer;
    TobyDevice* myDevice = arguments->myDevice;


    //we need some space, only once needed ... the size of the space is not fix yet,
    //TODO : FIX THE SPACE-PROBLEM, how much space should we give? Or maybe, it is saved directly in bufferer (more elegant)
    char* data = (char*)malloc(62*sizeof(char));


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
   //     int size = writeBuffer->getItem(data);
        size = writeBuffer->getString(data,62);
       // PX4_INFO("Thread got data: %s", data);
        //write data to hardware
        if(size > 62){
            PX4_INFO("ERROR Thread has to write size: %d",size);
        }
         // PX4_INFO("write worker is working");

          myDevice->write(data,size);

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

        i = myDevice->poll(NULL,0);
        if(i>0){
          u =  myDevice->read(buffer,60);
           readBuffer->putString(buffer,u);

        }
        else{
            PX4_INFO("readWorker : poll got 0");

        }
        if(buffer == NULL){
            PX4_INFO("READ WORKER NULLPOINTER");
        }

    }


    PX4_INFO("readWork exit");

    return NULL;

}





//****************************Helperfunctions, may going into a separate header File********************************


// not needed anymore, just for debugging
//TODO: Delete
void *doClose(void *arg)
{


    PX4_INFO("Thread started");
    pid_t x = ::getpid();
    PX4_INFO("actual thread id : %d",x);
    int i = px4_close(4);

    PX4_INFO("Thread closed Uart with %d",i);


    return NULL;
}

/*
void *writeHard(void *arg)
{
    PX4_INFO("writeHard Thread started");
    myStruct *testing = static_cast<myStruct*>(arg);
  //  PX4_INFO("Thread got the value %s",testing->text);

    const char* myTestText = "HaKLo";

    sleep(5);
    if(testing->myDevice == NULL){
        PX4_INFO("Thread :: got a null-pointer!!");

    }

    PX4_INFO("Thread : Let's write to device");
    testing->myDevice->write(myTestText,5);

    //TobyDevice *pointer = dynamic_cast<TobyDevice*>(testing->myDevice);




    // pthread_exit();
    return NULL;
}


*/

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



