
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
//#include <px4_tasks.h>

//#include <uORB/uORB.h>
//#include <uORB/topics/sensor_custom.h>
//#include <uORB/topics/sensor_combined.h>   // Eigen definierter Topic als Test
//#include <uORB/topics/vehicle_attitude.h>

#include <fcntl.h>
#include <termios.h>


__EXPORT int uart_app_main(int argc, char *argv[]);


int uart_app_main(int argc, char *argv[])
{

    PX4_INFO("This is a Uart Test App");


    int uart0_filestream =-1;
    uart0_filestream =open("/dev/ttyS6", O_RDWR |O_NOCTTY | O_NDELAY);

    if(uart0_filestream == -1)
    {
        PX4_INFO("Unable to Open /dev/ttyS6");

    }
    PX4_INFO("open return value /dev/ttyS6: %d",uart0_filestream);

    struct termios options= {};
    tcgetattr(uart0_filestream, &options);

    options.c_cflag &= ~(CSIZE | PARENB);
    options.c_cflag |= CS8;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    tcflush(uart0_filestream, TCIFLUSH);

    if(tcsetattr(uart0_filestream, TCSANOW, &options)<0)
    {
        PX4_WARN("Wrong Options");
    }

    px4_pollfd_struct_t fds[] =
    {
        {
            .fd = uart0_filestream,
            .events = POLLIN,

        },

    };
    int error_counter = 0;



    unsigned char tx_buffer[]={"Hallo Michael"};
    unsigned char rx_buffer[50]={};

    if (uart0_filestream != -1)
    {
        int count = write(uart0_filestream, tx_buffer, sizeof(tx_buffer)/sizeof(tx_buffer[0]));
        if (count < 0)
        {
            PX4_INFO("UART TX error");
        }
    }

    for(int i=0;i<10;i++)
    {
        //sleep(1);
        int poll_ret = px4_poll(fds, 1, 1000);

        /* handle the poll result */
        if (poll_ret == 0) {
            /* this means none of our providers is giving us data */
            PX4_INFO("Got no datas!");

        } else if (poll_ret < 0) {
            /* this is seriously bad - should be an emergency */
            if (error_counter < 10 || error_counter % 50 == 0) {
                /* use a counter to prevent flooding (and slowing us down) */
                PX4_ERR("ERROR return value from poll(): %d", poll_ret);
            }

            error_counter++;

        } else {

            if (fds[0].revents & POLLIN) {
                //fds[0].revents=0;
                /* obtained data for the first file descriptor */
                //PX4_INFO("Something to Receive!");
                /* copy sensors raw data into local buffer */

                int count = read(uart0_filestream, rx_buffer,8);
                if (count < 0)
                {
                    PX4_ERR("UART RX error");
                }

                PX4_INFO("Received: %s", rx_buffer);


                /* set att and publish this information for other apps */

            }

            /* there could be more file descriptors here, in the form like:
        * if (fds[1..n].revents & POLLIN) {}
        */
        }
    }
    close(uart0_filestream);
    PX4_INFO("exiting the Uart App");



    return 0;
}

