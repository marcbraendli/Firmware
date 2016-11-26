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

#include <drivers/device/ringbuffer.h>

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
void ringBufferTest();
int writeHardHelper(TobyDevice* myDevice);



Toby::Toby() :
#ifdef __PX4_NUTTX
    CDev("Toby", "/dev/toby")

#else
    //Übernommen aus anderen Treiberimplementationen
    VDev("toby", "/dev/toby")
#endif
{

	// force immediate init/device registration
	init();
  //  writeBuffer = new ringbuffer::RingBuffer(16,sizeof(char));
   // ringbuffer::RingBuffer *myBuffer = new ringbuffer::RingBuffer(2,sizeof(&writeBuffer));


    writeBuffer= new ringbuffer::RingBuffer(16,sizeof(char*));

    unsigned char tx_buffer[]={"Hallo Michael"};
    unsigned char dest_buffer[13];

    memcpy(dest_buffer, tx_buffer, 13);
  //  pthread_mutex_init(&Toby::m, NULL);

    buffer2 = new BoundedBuffer();




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
   // ringBufferTest();   //Test
   // writeBuffer = new ringbuffer::RingBuffer(20,sizeof(char));
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

    return myTobyDevice->read(buffer,buflen);

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
    PX4_INFO("Toby::write() is called");


    buffer2->putItem(buffer, buflen);
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
    int poll_return = myTobyDevice->poll(fds,setup);
    PX4_INFO("poll() is called with return %d",poll_return);

    if(poll_return > 0){

        poll_notify(POLLIN);
        poll_notify_one(fds, POLLIN);
    }
    return poll_return;
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


    this->myTobyDevice = new TobyDevice();
   // myTobyDevice->write(myTestText,5);

    if(this->myTobyDevice == NULL){
        PX4_INFO("myTobyDevice is a NULL-Pointer!!!!");

    }

    //****************Test some threading thing's****************
    // Dangerous passing the myTobyDevice to Thread ... isn't information hiding anymore???!

    workerParameters.myDevice= myTobyDevice;
    workerParameters.writeBuffer= writeBuffer;
    workerParameters.buffer2 = buffer2;

    writerThread = new pthread_t;
    pthread_create(writerThread, NULL, writeWork, (void*)&workerParameters);






    return OK;
}


void* Toby::writeWork(void *arg){

    PX4_INFO("writeHard Thread started");
    //extract arguments :
    myStruct *arguments = static_cast<myStruct*>(arg);
    BoundedBuffer* buffer2 = arguments->buffer2;
    TobyDevice* myDevice = arguments->myDevice;

    //we need some space, only once needed
    char* data = (char*)malloc(20*sizeof(char));



    for(int i = 0; i < 1; ++i){
        //get data from buffer
        int size = buffer2->getItem(data);
        //write data to hardware
        myDevice->write(data,size);
    }


    free(data);
    return NULL;

}




//****************************hilfsfunktionen********************************

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

void ringBufferTest(){


    unsigned char tx_buffer[]={"Hallo Michael"};

    ringbuffer::RingBuffer *x = new ringbuffer::RingBuffer(10,sizeof(char));
    x->put('h');


    x->get(tx_buffer[1]);

    PX4_INFO("Received: %s", tx_buffer);
}

