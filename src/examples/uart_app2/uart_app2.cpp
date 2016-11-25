
/**
 * @file uart_app.cpp
 * Testapplikation zur Inbetriebnahme der Serial4 Schnittstelle
 *
 *
 * @author Example User <mail@example.com>
 */

#include <px4_config.h>
#include <px4_posix.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
#include <stdlib.h>

//#include <sys/socket.h>
//#include <netinet/in.h>
#include <signal.h>

//#include <px4_tasks.h>
//#include <uORB/uORB.h>
//#include <uORB/topics/sensor_custom.h>
//#include <uORB/topics/sensor_combined.h>   // Eigen definierter Topic als Test
//#include <uORB/topics/vehicle_attitude.h>

#include <fcntl.h>
#include <termios.h>


extern "C" __EXPORT  int uart_app2_main(int argc, char *argv[]);

void signal_handler_IO (int status);

int uart_app2_main(int argc, char *argv[])
{

    //http://stackoverflow.com/questions/15119412/setting-serial-port-interruption-in-linux
    struct termios options;
    struct sigaction saio;

    PX4_INFO("This is a Uart Test App");


    int uart0_filestream =-1;

    uart0_filestream =open("/dev/ttyS6", (O_RDWR |O_NOCTTY | O_NONBLOCK));

    if(uart0_filestream == -1)
    {
        PX4_INFO("Unable to Open /dev/ttyS6");

    }
    //PX4_INFO("open return value /dev/ttyS6: %d",uart0_filestream);




    saio.sa_handler = signal_handler_IO;
    saio.sa_flags = 0;
    //saio.sa_restorer = NULL;
    sigaction(1,&saio,NULL);

    fcntl(uart0_filestream , F_SETFL, FNDELAY);
    fcntl(uart0_filestream , F_SETOWN, getpid());
    fcntl(uart0_filestream , F_SETFL,  O_SYNC );

    tcgetattr(uart0_filestream, &options);
    options.c_cflag &= ~CSIZE;
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag |= CS8;
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;

    cfsetispeed(&options,B57600);
    cfsetospeed(&options,B57600);

    tcflush(uart0_filestream, TCIFLUSH);

    if(tcsetattr(uart0_filestream, TCSANOW, &options)<0)
    {
        PX4_WARN("Wrong Options");
    }


    //px4_pollfd_struct_t fds;
    //fds.fd = uart0_filestream;
    //fds.events = (POLLIN | POLLRDNORM);

    char rx_buffer[100]="";

    const char *at_command_send[5]={"AT\r","AT\r","AT+CREG?\r","AT+CGMR\r","AT+CGMM\r"};
    int at_command_send_size[5]={3,3,9,8,8};

    int i =0;
    int send =1;
    //int input=0;
    while(i<5)
    {

        if(send)
        {
             PX4_INFO("Send: %.*s", at_command_send_size[i]-1, at_command_send[i]);
            int count = write(uart0_filestream, at_command_send[i], at_command_send_size[i]);

            if (count < 0)
            {
                PX4_INFO("UART TX error");
            }
             i++;
            send=0;
        }else{

            int poll_ret = px4_poll(&fds, 1, 2500);

            if (poll_ret == 0)
            {
                PX4_INFO("Got no datas!");
                send=1;

            }else if(poll_ret <0){
                PX4_ERR("ERROR return value from poll(): %d", poll_ret);
                break;
            }else
            {
                if (fds.revents & (POLLIN | POLLRDNORM))
                {
                    int count = read(uart0_filestream, rx_buffer,100);
                    if (count < 0)
                    {
                        PX4_ERR("UART RX error");
                    }
                    PX4_INFO("Received: %s", rx_buffer);

                    for (int l=0; l<100;l++)
                    {
                        rx_buffer[l]='\0';
                    }
                    //PX4_INFO("Received: %s", rx_buffer);

                }

            }
        }
    }
    close(uart0_filestream);
    PX4_INFO("exiting the Uart App");



    return 0;
}

void signal_handler_IO (int status)
{
     printf("received data from UART.\n");
}
