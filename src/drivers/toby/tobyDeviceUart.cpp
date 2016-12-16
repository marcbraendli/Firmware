/**
 * @brief TobyDeviceUart implements TobyDevice Interface handles the communication over uart
 * @file tobyDevice.h
 * @author Marc Brändli & Michael Lehmann
 *
 */

#include "tobyDeviceUart.h"
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

#define TOBY_L210_UART_PATH  "/dev/ttyS6"

namespace TobyDeviceUartHelper {
int set_flowcontrol(int fd, int control);
void *doClose(void *arg);
int uart_open(struct termios* options);

}
using namespace TobyDeviceUartHelper;




TobyDeviceUart::TobyDeviceUart()
{

    if(uart_open()){
        PX4_INFO("opened uart successful with %d",uart0_filestream);
    }



}

TobyDeviceUart::~TobyDeviceUart()
{
    PX4_INFO("this is ~TobyDeviceUart() deconstructor");
    uart_close();
}






ssize_t	TobyDeviceUart::read(char *buffer, size_t buflen)
{
    int i = px4_read(uart0_filestream,buffer,buflen);
    return i;
}

ssize_t	TobyDeviceUart::write(const char *buffer, size_t buflen){

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

    return count;
}


int TobyDeviceUart::ioctl(int cmd, unsigned long arg)
{

    //ioctl direct to uart
    return ::ioctl(uart0_filestream,cmd,arg);
}


int	TobyDeviceUart::poll(int timeout){


    px4_pollfd_struct_t fds;
    fds.fd = uart0_filestream;
    fds.events = POLLIN;


    int poll_return = px4_poll(&fds,1,timeout);
    if(poll_return >0){

    }

    return poll_return;


}


/**
 * @brief TobyDeviceUartHelper::uart_close this function is needed because we're not able to close the uart from the toby device class itself
 *
 * @param uart0_filestream Filedescriptor
 */
void TobyDeviceUart::uart_close(void){


    tcflush(uart0_filestream, TCIFLUSH);
    //force a thread to close the uart
    pthread_t uart_close_thread;
    pthread_create(&uart_close_thread, NULL, doClose, NULL);

}



bool TobyDeviceUart::uart_open(void){

        pid_t x = ::getpid();
        PX4_INFO("actual thread id : %d",x); // Debug
    //***********************set options and open uart device************

    uart0_filestream =px4_open(TOBY_L210_UART_PATH, O_RDWR |O_NOCTTY);

    if(uart0_filestream == -1)
    {
        PX4_INFO("Unable to Open /dev/ttys6");
        return false;

    }
    tcgetattr(uart0_filestream, &options);
    //options.c_cflag &= ~(CSIZE | PARENB);
    options.c_cflag = CS8;
    //options.c_iflag = IGNPAR;
    options.c_iflag&= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
    options.c_oflag = 0;
    options.c_oflag = O_NONBLOCK;
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    //options.c_lflag = ECHO;
    options.c_cflag &= ~(CSTOPB | PARENB);

    cfsetispeed(&options, B57600);
    cfsetospeed(&options, B57600);

    set_flowcontrol(uart0_filestream,0);
    tcflush(uart0_filestream, TCIFLUSH);

    if(tcsetattr(uart0_filestream, TCSANOW, &options)<0)
    {
        PX4_WARN("Wrong Options");
        return false;
    }



    return true;

}


void TobyDeviceUart::printStatus(void){

    PX4_INFO("Current filedescriptor: %d", uart0_filestream);

}




void *TobyDeviceUartHelper::doClose(void *arg)
{


    PX4_INFO("Thread started");
    pid_t x = ::getpid();
    PX4_INFO("actual thread id : %d",x);
    int i = px4_close(4);

    PX4_INFO("Thread closed Uart with %d",i);


    return NULL;

}

int TobyDeviceUartHelper::set_flowcontrol(int fd, int control)
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



