
/**
 * @file custom_simple_app.cpp
 * Für die Analyse des Flightstacks. Dieses App "subscribed" den Topic sensor_custom.
 *
 *
 * @author Example User <mail@example.com>
 */

#include <px4_config.h>
#include <px4_posix.h>
#include <unistd.h>
#include <stdio.h>
//#include <poll.h>
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
   //options.c_cflag = B9600| CS8 | CLOCAL | CREAD;		//<Set baud rate
   options.c_cflag &= ~(CSIZE | PARENB);
   options.c_cflag |= CS8;
   options.c_iflag = IGNPAR;
   options.c_oflag = 0;
   options.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

   cfsetispeed(&options, B9600);

   tcflush(uart0_filestream, TCIFLUSH);

   if(tcsetattr(uart0_filestream, TCSANOW, &options)<0)
   {
      PX4_WARN("Wrong Attributes");
   }

   unsigned char tx_buffer[]={"Hallo Michael"};

   if (uart0_filestream != -1)
   {
        int count = write(uart0_filestream, tx_buffer, sizeof(tx_buffer)/sizeof(tx_buffer[0]));		//Filestream, bytes to write, number of bytes to write
        PX4_INFO("count: %d",count);
        if (count < 0)
        {
            PX4_INFO("UART TX error\n");
        }
   }

    //sleep(2);
    close(uart0_filestream);
    PX4_INFO("exiting the Uart App");



    return 0;
}

