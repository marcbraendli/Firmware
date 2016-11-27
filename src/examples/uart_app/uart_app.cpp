
/**
 * @file uart_app.cpp
 * Testapplikation zur Inbetriebnahme der Serial4 Schnittstelle
 * mit der poll fkt
 *
 * @author Example User <mail@example.com>
 */

#include <px4_config.h>
#include <px4_posix.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>

#include <fcntl.h>
#include <termios.h>

typedef enum State
{
    Receive = 1,
    Send,
    Edit
}State;

extern "C" __EXPORT  int uart_app_main(int argc, char *argv[]);



int uart_app_main(int argc, char *argv[])
{

    PX4_INFO("This is a Uart Test App");


    int uart0_filestream =-1;

    uart0_filestream =open("/dev/ttyS6", (O_RDWR |O_NOCTTY | O_NONBLOCK));

    if(uart0_filestream == -1)
    {
        PX4_INFO("Unable to Open /dev/ttyS6");

    }
    PX4_INFO("open return value /dev/ttyS6: %d",uart0_filestream);

    struct termios options;
    tcgetattr(uart0_filestream, &options);

    options.c_iflag=0;
    options.c_iflag = IGNPAR;
    options.c_iflag &= ~(IGNBRK | BRKINT | ICRNL| INLCR | PARMRK | INPCK | ISTRIP | IXON);
    options.c_lflag=0;
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    options.c_oflag=0;
    options.c_cflag=0;
    options.c_cflag &= ~(CSIZE | PARENB);
    options.c_cflag |= (CS8|CREAD|CLOCAL);

    cfsetispeed(&options,B57600);
    cfsetospeed(&options,B57600);

    tcflush(uart0_filestream, TCIFLUSH);

    if(tcsetattr(uart0_filestream, TCSANOW, &options)<0)
    {
        PX4_WARN("Wrong Options");
    }

    px4_pollfd_struct_t fds;
    fds.fd = uart0_filestream;
    fds.events = (POLLIN | POLLRDNORM);

    char rx_buffer[100]="";
    char output[100]="";
    char* string_end ='\0';
    int received =0;

    const char *at_command_send[5]={"AT\r","AT\r","AT+CREG?\r","AT+CGMR\r","AT+CGMM\r"};
    int at_command_send_size[5]={3,3,9,8,8};

    int i =0;
    State state =Send;
    while(i<5)
    {

        switch(state)
        {
        case Send:
        {
            PX4_INFO("Send: %.*s", at_command_send_size[i]-1, at_command_send[i]);
            int count = write(uart0_filestream, at_command_send[i], at_command_send_size[i]);

            if (count < 0)
            {
                PX4_INFO("UART TX error");
            }
            i++;
            state=Receive;
            break;
        }

        case Receive:
        {
            int poll_ret = px4_poll(&fds, 1, 500);

            if (poll_ret == 0)
            {
                PX4_INFO("Got no (more) datas!");
                if(received)
                {
                    state=Edit;
                }
            }else if(poll_ret <0){
                PX4_ERR("ERROR return value from poll(): %d", poll_ret);
            }else
            {
                if (fds.revents & (POLLIN | POLLRDNORM))
                {
                    int count = read(uart0_filestream, rx_buffer,100);
                    if (count < 0)
                    {
                        PX4_ERR("UART RX error");
                    }
                    strcat(output, rx_buffer);
                    received =1;
                }
            }
            break;
        }

        case Edit://verarbeiten
        {
            PX4_INFO("Received: %s", output);

            strncpy(rx_buffer, string_end,100);
            strncpy(output, string_end,100);

            //PX4_INFO("RX-Buff check: %s", rx_buffer);
            //PX4_INFO("Output check: %s", output);
            state=Send;
            received=0;
            break;
        }
        default:
            break;
        }
    }
    close(uart0_filestream);
    PX4_INFO("exiting the Uart App");

    return 0;
}

