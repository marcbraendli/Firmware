#include "tobyDevice.h"


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





/* Hilfs und Testfunktionen deklaration evt später als private in Klasse implementieren!*/
namespace TobyDeviceHelper {
int set_flowcontrol(int fd, int control);
void *doClose(void *arg);
int uart_open(struct termios* options);
void uart_close(int uart0_filestream);

}

using namespace TobyDeviceHelper;

TobyDevice::TobyDevice()
{
    PX4_INFO("this is TobyDeviceHelper()");


    uart0_filestream = uart_open(&options);
    PX4_INFO("opened uart with %d",uart0_filestream);



}

TobyDevice::~TobyDevice()
{
    PX4_INFO("this is ~TobyDeviceHelper() deconstructor");
    uart_close(uart0_filestream);
}






ssize_t	TobyDevice::read(char *buffer, size_t buflen)
{
    int i = px4_read(uart0_filestream,buffer,buflen);
    return i;
}

ssize_t	TobyDevice::write(const char *buffer, size_t buflen){


     //pthread_mutex_lock(&lock);
    //todo : effizienter implementieren, but how?

    //Debugging
   // PX4_INFO("TobyDevice::write() is called with uart0_filestream %d",uart0_filestream);

    int count = 0;
    if (uart0_filestream != -1)
    {


        count = px4_write(uart0_filestream, buffer, buflen);

        if (count < 0)
        {
            //Debugging
            PX4_INFO("UART TX error");
        }

    }
   // close(NULL);

//  pthread_mutex_unlock(&lock);
    return count;
}


// just a studpid stub is needed for some test's of pthreads
void* TobyDevice::writeToUart(void *arg){

    PX4_INFO("received data in writeToUart()");
    return NULL;
}



int
TobyDevice::ioctl(int cmd, unsigned long arg)
{

    //ioctl direct to uart
    return ::ioctl(uart0_filestream,cmd,arg);
}


int	TobyDevice::poll(struct pollfd *fds, bool setup){


    px4_pollfd_struct_t fds1[1];
    fds1[0].fd = uart0_filestream; //4
    fds1[0].events = POLLIN;


    int poll_return = px4_poll(fds1,1,0);
    if(poll_return >0){
        //notify the caller
        /*
        Nicht möglich, weil nicht von CDev geerbt:
        poll_notify(POLLIN);
        poll_notify_one(fds, POLLIN);
        */
    }



    return poll_return;


}



//****************************hilfsfunktionen********************************


int TobyDeviceHelper::uart_open(struct termios* options){



    //Debugging
  //  PX4_INFO("uart_open() is called");
        pid_t x = ::getpid();
        PX4_INFO("actual thread id : %d",x);

    //***********************set options and open uart device************

    //todo: Übernahme der Optionen termios des caller ... but how?
    int localUart0_filestream =px4_open("/dev/ttyS6", O_RDWR |O_NOCTTY);

    if(localUart0_filestream == -1)
    {
        PX4_INFO("Unable to Open /dev/ttys6");

    }
    //PX4_INFO("open return value /dev/ttys6: %d",localUart0_filestream);

    tcgetattr(localUart0_filestream, options);

    //options.c_cflag &= ~(CSIZE | PARENB);
    options->c_cflag = CS8;
    //options.c_iflag = IGNPAR;
    options->c_iflag&= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
    options->c_oflag = 0;

    options->c_oflag = O_NONBLOCK;

    options->c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    //options.c_lflag = ECHO;

    options->c_cflag &= ~(CSTOPB | PARENB);

    cfsetispeed(options, B57600);
    cfsetospeed(options, B57600);


  //  cfsetispeed(&options, B9600);
  //  cfsetospeed(&options, B9600);

    set_flowcontrol(localUart0_filestream,0);
    tcflush(localUart0_filestream, TCIFLUSH);

    if(tcsetattr(localUart0_filestream, TCSANOW, options)<0)
    {
        PX4_WARN("Wrong Options");
    }



    //*******************************************************************

    //PX4_INFO("uart_open() terminates with return %i",localUart0_filestream);

    //if failed, return = -1
    return localUart0_filestream;

}

//TODO : px4_close goes over a hard coded number ... that might not work allways, was just a dirty fix
void *TobyDeviceHelper::doClose(void *arg)
{


    PX4_INFO("Thread started");
    pid_t x = ::getpid();
    PX4_INFO("actual thread id : %d",x);
    int i = px4_close(4);

    PX4_INFO("Thread closed Uart with %d",i);


    return NULL;


}


void TobyDeviceHelper::uart_close(int uart0_filestream){

    PX4_INFO("uart_close() is called, we close with %d",uart0_filestream);
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
   // return 0;


}

int TobyDeviceHelper::set_flowcontrol(int fd, int control)
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



