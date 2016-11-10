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


int set_flowcontrol(int fd, int control);
void *doClose(void *arg);
int toby_init();


Toby::Toby() :
#ifdef __PX4_NUTTX
    CDev("Toby", "/dev/toby")
#else
    VDev("toby", "/dev/toby")
#endif
{
	// force immediate init/device registration
	init();
}

Toby::~Toby()
{
}




int
Toby::init()
{
    PX4_INFO("TOBY::init");
#ifdef __PX4_NUTTX
	CDev::init();
#else
	VDev::init();
#endif
    toby_init();

	return 0;
}


int Toby::open(device::file_t *filp){
    PX4_INFO("open() is called");
        pid_t x = ::getpid();
        PX4_INFO("actual thread id : %d",x);


    //*******************************************************************
    int l = ::device::CDev::open(filp);
    PX4_INFO("CDev:Open() mit return %d",l);

    uart0_filestream =px4_open("/dev/ttyS6", O_WRONLY );

    if(uart0_filestream == -1)
    {
        PX4_INFO("Unable to Open /dev/ttys6");

    }
    PX4_INFO("open return value /dev/ttys6: %d",uart0_filestream);

    tcgetattr(uart0_filestream, &options);

    //options.c_cflag &= ~(CSIZE | PARENB);
    options.c_cflag = CS8;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_oflag = O_NONBLOCK;
    //options.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    //options.c_lflag = ECHO;

    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    set_flowcontrol(uart0_filestream,0);

    tcflush(uart0_filestream, TCIFLUSH);

    if(tcsetattr(uart0_filestream, TCSANOW, &options)<0)
    {
        PX4_WARN("Wrong Options");
    }


    //*******************************************************************


   // int i = ::device::CDev::open(filp);
    return 0;
}


int	Toby::close(device::file_t *filp){
    PX4_INFO("close() is called, we close with %d",uart0_filestream);
    ::device::CDev::close(filp);
    pid_t x = ::getpid();
    PX4_INFO("actual thread id : %d",x);
    tcflush(uart0_filestream, TCIFLUSH);

    //pthread_t Toby_thread;
   // pthread_create(&Toby_thread, NULL, doClose, NULL);


   // int i = set_flowcontrol(uart0_filestream,1);
    PX4_INFO("set_flowcontrol returns with %d");
    tcgetattr(uart0_filestream, &options);


    this->unlock();

    px4_close(uart0_filestream);


   PX4_INFO("uart closed mit %d");

   return 0;




}


ssize_t	Toby::read(device::file_t *filp, char *buffer, size_t buflen)
{
    PX4_INFO("read() is called");

    ::device::CDev::read(filp,buffer,buflen);
    return 0;
}

ssize_t	Toby::write(device::file_t *filp, const char *buffer, size_t buflen){

    PX4_INFO("write() is called");

    int i = 0;
    //i = ::device::CDev::write(filp, buffer, buflen);

    if (uart0_filestream != -1)
    {
        PX4_INFO("::write() uart_filstream %d",uart0_filestream);

        int count = px4_write(uart0_filestream, buffer, buflen);
        i = count;
        if (count < 0)
        {
            PX4_INFO("UART TX error");
        }

    }


   // close(NULL);

    return i;
}


off_t Toby::seek(device::file_t *filp, off_t offset, int whence){
    PX4_INFO("seek() is called");

    return ::device::CDev::seek(filp,offset,whence);



}




int
Toby::ioctl(device::file_t *filp, int cmd, unsigned long arg)
{
    PX4_INFO("ioctl mit cmd:  %d",cmd);

    //ein versuch :
    int i = ::device::CDev::ioctl(filp,cmd,arg);
PX4_INFO("ioctl() return %d",i);
return i;
}


int	Toby::poll(device::file_t *filp, struct pollfd *fds, bool setup){


    PX4_INFO("poll() is called");
    return ::device::CDev::poll(filp, fds,setup);

}





int
toby_init(){
    PX4_INFO("toby_init");
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
   // const char *device_name2 = nullptr;
    int myoptind = 1;
    const char *verb = argv[myoptind];




    if (argc < 2) {
        goto out;
    }

    /*
     * Start/load the driver.
     */

    if (!strcmp(verb, "start")) {
        if (gToby != nullptr) {
            warnx("already started");
            return 1;
        }

        gToby = new Toby();



        return 0;
    }

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

    return 0;

out:
    PX4_ERR("unrecognized command, try 'start', 'stop', 'test', 'reset' or 'status'");
    PX4_ERR("[-d " TOBY_DEVICE_PATH "][-f (for enabling fake)][-s (to enable sat info)]");
    return 1;
}


//****************************hilfsfunktionen********************************

void *doClose(void *arg)
{


    PX4_INFO("Thread started");
    pid_t x = ::getpid();
    PX4_INFO("actual thread id : %d",x);
    PX4_INFO("Thread closed Uart");
    px4_close(4);

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

