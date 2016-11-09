
/**
 * @file px4_daemon_app.c
 * daemon application example for PX4 autopilot
 *
 * @author Example User <mail@example.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <uORB/topics/sensor_custom.h>
#include <uORB/topics/vehicle_attitude.h>
#include <drivers/drv_led.h>
#include <drivers/drv_rgbled.h>
#include <modules/commander/commander_helper.h>
#include <drivers/drv_hrt.h>
#include "DevMgr.hpp"

#include <drivers/device/device.h>
#include <drivers/drv_hrt.h>
#include "DevMgr.hpp"

#include <px4_config.h>
#include <nuttx/sched.h>

#include <systemlib/systemlib.h>
#include <systemlib/err.h>
#include <termios.h>
#include <drivers/toby/toby.h>


//extern int main(int argc, char **argv);
extern "C" __EXPORT int TobyTester_main(int argc, char *argv[]);



/**
 * Function for analizing architecture
 */
int TobyTester_main(int argc, char *argv[])
{


    PX4_INFO("This is a Uart Test App");


        int uart0_filestream =-1;
        uart0_filestream =::open("/dev/toby", O_RDWR |O_NOCTTY | O_NDELAY);

        if(uart0_filestream == -1)
        {
            PX4_INFO("Unable to Open /dev/toby");

        }
        PX4_INFO("open return value /dev/toby: %d",uart0_filestream);

        struct termios options= {};
        tcgetattr(uart0_filestream, &options);

        //options.c_cflag &= ~(CSIZE | PARENB);
        options.c_cflag = CS8;
        options.c_iflag = IGNPAR;
        options.c_oflag = 0;
        //options.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
        //options.c_lflag = ECHO;

        cfsetispeed(&options, B9600);
        cfsetospeed(&options, B9600);

        tcflush(uart0_filestream, TCIFLUSH);

        if(tcsetattr(uart0_filestream, TCSANOW, &options)<0)
        {
            PX4_WARN("Wrong Options");
        }





        unsigned char tx_buffer[]={"Hallo Michael"};

        if (uart0_filestream != -1)
        {
            int count = write(uart0_filestream, tx_buffer, sizeof(tx_buffer)/sizeof(tx_buffer[0]));
            if (count < 0)
            {
                PX4_INFO("UART TX error");
            }
        }




        px4_close(uart0_filestream);

         PX4_INFO("Exit");


    return 0;
}



