
/**
 * @file px4_daemon_app.c
 * daemon application example for PX4 autopilot
 *
 * @author Example User <mail@example.com>
 */

#include <stdio.h>
#include <assert.h>

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


//Function prototyping for Tests:
namespace TobyTester{

    void testToby();
    int openToby();
    void writeToby(int uart0_filestream);
    void readToby(int uart0_filestream);
    void closeToby(int uart0_filestream);
}



int TobyTester_main(int argc, char *argv[])
{

    PX4_INFO("This is a Toby Test App");


        int uart0_filestream =-1;
        uart0_filestream =::open("/dev/toby", O_RDWR |O_NOCTTY | O_NDELAY);

        if(uart0_filestream == -1)
        {
            PX4_INFO("Unable to Open /dev/toby");

        }
        PX4_INFO("open return value /dev/toby: %d",uart0_filestream);

        //struct termios options= {};

        //tcgetattr(uart0_filestream, &options);

        //-//options.c_cflag &= ~(CSIZE | PARENB);
        //options.c_cflag = CS8;
        //options.c_iflag = IGNPAR;
        //options.c_oflag = 0;
        //-//options.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
        //-//options.c_lflag = ECHO;

        //cfsetispeed(&options, B9600);
        //cfsetospeed(&options, B9600);

        //tcflush(uart0_filestream, TCIFLUSH);

        //if(tcsetattr(uart0_filestream, TCSANOW, &options)<0)
        //{
         //   PX4_WARN("Wrong Options");
        //}





        //*************receive and write  code ******************************
        unsigned char tx_buffer[]={"Hallo Michael"};

        if (uart0_filestream != -1)
        {
            int count = write(uart0_filestream, tx_buffer, 13);
            if (count < 0)
            {
                PX4_INFO("UART TX error");
            }
        }

        px4_pollfd_struct_t fds[1];
        fds[0].fd = uart0_filestream;
        fds[0].events = POLLIN;
        unsigned char rx_buffer[50]={};
        int error_counter = 0;



        for(int i=0;i<10;i++)
            {
                //sleep(1);
                int poll_ret = px4_poll(fds, 1, 2000);

                PX4_INFO("poll_ret = %i!",poll_ret);

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

                        int count = read(uart0_filestream, rx_buffer,40);
                        if (count < 0)
                        {
                            PX4_ERR("UART RX error");
                        }

                        PX4_INFO("Received: %s", rx_buffer);


                        /* set att and publish this information for other apps */

                    }
                }

            }


        sleep(5);
        ::close(uart0_filestream);



        TobyTester::testToby();



         PX4_INFO("Exit");


    return 0;
}

namespace TobyTester{

void testToby(){

    int myFilestream;
    int myTestFilestream;


    //start the toby-driver direct in the Unittest : Not possible yet
/*
    int argc = 1;
    char *argv[2];
    char a[] = {"start"};
    argv[0] = a;

    daemon_task = px4_task_spawn_cmd("daemon",
                     SCHED_DEFAULT,
                     SCHED_PRIORITY_MAX,
                     2000,
                     toby_main,
                     (argv) ? (char *const *)&argv[2] : (char *const *)NULL);

*/


    //open Toby return is the socket-number
    PX4_INFO("openToby Test : ");
    myFilestream = openToby();
    ASSERT(myFilestream != -1);
    PX4_INFO("openToby Test OK");


    //open Toby againg
    PX4_INFO("openToby again Test : ");
    myTestFilestream = openToby();
    ASSERT(myTestFilestream = -1);
    PX4_INFO("openToby again Test OK");

    //test writing to external uart : visual test required
    PX4_INFO("writeToby Test : ");
    writeToby(myFilestream);
    PX4_INFO("writeToby Test OK");


    //read from external uart : visual test is required
    PX4_INFO("readToby Test : ");
    readToby(myFilestream);
    PX4_INFO("readToby Test OK");


    //closing Toby
    PX4_INFO("closeToby Test : ");
    closeToby(myFilestream);
    PX4_INFO("closeToby Test OK");

}


int openToby(){

    int uart0_filestream =-1;
    uart0_filestream =::open("/dev/toby", O_RDWR |O_NOCTTY | O_NDELAY);

    if(uart0_filestream == -1)
    {
        PX4_INFO("Unable to Open /dev/toby");

    }
    PX4_INFO("open return value /dev/toby: %d",uart0_filestream);


  //  ASSERT(uart0_filestream != -1);

    return uart0_filestream;
}

void writeToby(int uart0_filestream){

    unsigned char tx_buffer[]={"Hallo Michael"};

    if (uart0_filestream != -1)
    {
        int count = write(uart0_filestream, tx_buffer, 13);
        if (count < 0)
        {
            PX4_ERR("UART TX error");
            ASSERT(false);

        }
    }

}

void readToby(int uart0_filestream){

    px4_pollfd_struct_t fds[1];
    fds[0].fd = uart0_filestream;
    fds[0].events = POLLIN;
    unsigned char rx_buffer[50]={};



    for(int i=0;i<10;i++)
        {
            //sleep(1);
            int poll_ret = px4_poll(fds, 1, 2000);

            PX4_INFO("poll_ret = %i!",poll_ret);

            /* handle the poll result */
            if (poll_ret == 0) {

                //no data received
                PX4_INFO("Got no datas!");

            } else if (poll_ret < 0) {
                // bad case / Error
                ASSERT(false);

            } else {

                if (fds[0].revents & POLLIN) {

                    //load received data into rx_bufer
                    int count = read(uart0_filestream, rx_buffer,40);
                    if (count < 0)
                    {
                        PX4_ERR("UART RX error");
                        ASSERT(false);
                    }

                    //visual test required
                    PX4_INFO("Received: %s", rx_buffer);

                }
            }

        }

}

void closeToby(int uart0_filestream){
    ::close(uart0_filestream);

}

}





