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

#include <stdio.h>
#include <stdlib.h>

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
int toby_init();
void ringBufferTest();


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
}

Toby::~Toby()
{
}



int Toby::init()
{
    PX4_INFO("TOBY::init");

    ringBufferTest();   //Test
    writeBuffer = new ringbuffer::RingBuffer(20,sizeof(char));
#ifdef __PX4_NUTTX
	CDev::init();
#else
	VDev::init();
#endif
    toby_init();

	return 0;
}


int Toby::open(device::file_t *filp){

    //Debugging
    PX4_INFO("open() is called");
        pid_t x = ::getpid();
        PX4_INFO("actual thread id : %d",x);


    //*******************************************************************
    int l = ::device::CDev::open(filp);
    PX4_INFO("CDev:Open() mit return %d",l);



    //***********************set options and open uart device************

    //todo: Übernahme der Optionen termios des caller ... but how?
    uart0_filestream =px4_open("/dev/ttyS6", O_RDWR |O_NOCTTY);

    if(uart0_filestream == -1)
    {
        PX4_INFO("Unable to Open /dev/ttys6");

    }
    PX4_INFO("open return value /dev/ttys6: %d",uart0_filestream);

    tcgetattr(uart0_filestream, &options);

    //options.c_cflag &= ~(CSIZE | PARENB);
    options.c_cflag = CS8;
    //options.c_iflag = IGNPAR;
    options.c_iflag&= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
    options.c_oflag = 0;
    //options.c_oflag = O_NONBLOCK;
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    //options.c_lflag = ECHO;

    options.c_cflag &= ~(CSTOPB | PARENB);

    cfsetispeed(&options, B57600);
    cfsetospeed(&options, B57600);


  //  cfsetispeed(&options, B9600);
  //  cfsetospeed(&options, B9600);

    set_flowcontrol(uart0_filestream,0);
    tcflush(uart0_filestream, TCIFLUSH);

    if(tcsetattr(uart0_filestream, TCSANOW, &options)<0)
    {
        PX4_WARN("Wrong Options");
    }


   //Debugging :
    union DeviceId deviceID =  CDev::Device::_device_id;
    PX4_INFO("my DeviceID = %i", deviceID.devid);


    //*******************************************************************


    return l;
}


int	Toby::close(device::file_t *filp){
    PX4_INFO("close() is called, we close with %d",uart0_filestream);
    ::device::CDev::close(filp);

    //for Debugging
    pid_t x = ::getpid();
    PX4_INFO("actual thread id : %d",x);
    //**************************************


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


ssize_t	Toby::read(device::file_t *filp, char *buffer, size_t buflen)
{
    //PX4_INFO("read() is called");
    ::device::CDev::read(filp,buffer,buflen);
    int i = px4_read(uart0_filestream,buffer,buflen);

    return i;
}

ssize_t	Toby::write(device::file_t *filp, const char *buffer, size_t buflen){

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
    return count;
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
    return ::ioctl(uart0_filestream,cmd,arg);
}


int	Toby::poll(device::file_t *filp, struct pollfd *fds, bool setup){


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


    return 10;


}





int toby_init(){



    PX4_INFO("toby_init");

    //int i =0;
    //int send =1;

    //const char *at_command_send[2]={"AT","ATE0"};

    /*at_command_send[0][]="AT";
    at_command_send[1][]="ATE0";
    at_command_send[2][]="";
    at_command_send[3][]="";
    at_command_send[4][]="";
    at_command_send[5][]="";
    at_command_send[6][]="";
    at_command_send[7][]="";*/

   /* char* at_command_receive[8];
    at_command_receive[0]="AT";
    at_command_receive[1]="OK";
    at_command_receive[2]="ATE0";
    at_command_receive[3]="OK";
    at_command_receive[4]="";
    at_command_receive[5]="";
    at_command_receive[6]="";
    at_command_receive[7]="";*/

    unsigned char buffer[50][8]={};


    FILE *f = fopen("/fs/microsd/test/myfile.txt", "r");
    int pos = 0;
    int c;
    if(f) {
        PX4_INFO("SD-Karte offen");

        do{ // read one line until EOF
            c = fgetc(f);
            if((c != EOF)||(c != '\n'))
            {
                buffer[0][pos++] = (char)c;
            }
            }while(c != EOF && c != '\n');
        }
    fclose(f);

    PX4_INFO("Von SD Karte gelesen: %s", buffer);




    /*
    int uart0_filestream =-1;

    uart0_filestream =open("/dev/ttyS6", O_RDWR |O_NOCTTY | O_NDELAY);

    if(uart0_filestream == -1)
    {
        PX4_INFO("Unable to Open /dev/ttyS6");

    }
    PX4_INFO("open return value /dev/ttyS6: %d",uart0_filestream);

    struct termios options= {};
    tcgetattr(uart0_filestream, &options);

    //options.c_cflag &= ~(CSIZE | PARENB);
    options.c_cflag = CS8;
    //options.c_iflag = IGNPAR;
    options.c_iflag&= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
    options.c_oflag = 0;
    //options.c_oflag = O_NONBLOCK;
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    //options.c_lflag = ECHO;

    options.c_cflag &= ~(CSTOPB | PARENB);

    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    set_flowcontrol(uart0_filestream,0);

    tcflush(uart0_filestream, TCIFLUSH);

    if(tcsetattr(uart0_filestream, TCSANOW, &options)<0)
    {
        PX4_WARN("Wrong Options");
    }

    px4_pollfd_struct_t fds[1];
    fds[0].fd = uart0_filestream;
    fds[0].events = POLLIN;

    while(i<10)
    {

        if(send)
        {
            int count = write(uart0_filestream, at_command_send[0], sizeof(at_command_send)/sizeof(at_command_send[0]));
            if (count < 0)
            {
                PX4_INFO("UART TX error");
            }
            send=0;
        }else{

            int poll_ret = px4_poll(fds, 1, 150);
            if (poll_ret == 0)
            {
                PX4_INFO("Got no datas!");
            }else if(poll_ret <0){
                PX4_ERR("ERROR return value from poll(): %d", poll_ret);
                break;
            }else
            {
                if (fds[0].revents & POLLIN)
                {
                    int count = read(uart0_filestream, rx_buffer,1);
                    if (count < 0)
                    {
                        PX4_ERR("UART RX error");
                    }
                    PX4_INFO("Received: %s", rx_buffer);
                    send=1;
                }
                i++;
            }
        }
     }

    close(uart0_filestream);*/

    return 0;
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
        PX4_INFO("Toby wird beendet");
        delete gToby;
        gToby= nullptr;

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

int set_flowcontrol(int fd, int control)
{
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

