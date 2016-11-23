
/**
 * @file TobyTester.cpp
 * Test for toby-Interface
 *
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
//#include <drivers/toby/tobyDevice.h>


#include <modules/mavlink/mavlink_main.h>






//extern int main(int argc, char **argv);
extern "C" __EXPORT int TobyTester_main(int argc, char *argv[]);
int testTastMavlink;

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
void startMavlink();
}

namespace TobyDeviceTester{

void testTobyDevice();

}


/*                      INFO
 *
 * Assert's brechen nur ab und geben keine Rückmeldung
 * Eine eigene Assert Definition mit Output könnte zu
 * diesem Zweck definiert werden, ist bis jetzt aber nicht
 * umgesetzt
 *
 */

int TobyTester_main(int argc, char *argv[])
{

    PX4_INFO("This is a Toby Test App");

    /*****************my testfunctions*********************/

    //TobyDeviceTester::testTobyDevice();
    //PX4_INFO("TobyDevice finished, start testToby()");

    TobyTester::testToby();







    //*******Test external start Mavlink

 //   unsigned char tx_buffer[]={"Hallo Michael"};


    /*
   {
    const char *myargv[3] = {"start","-d","/dev/toby"};
    //char **myargv2 = (char**)myargv;

    Mavlink::start(3,(char**)myargv);

    PX4_INFO("myargv: %s", myargv[0]);



    return 0;
    }
    */

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
   // ASSERT(myTestFilestream != -1);
    PX4_INFO("openToby again Test OK with %d",myTestFilestream);

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

    PX4_INFO("Let's terminate");

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

    px4_pollfd_struct_t fds[1
            ];
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

void startMavlink(){
    // myargv[][0] = {"start"};

     //{{"start"}, {"-d"}, {"/dev/toby"}};

   /*
     Mavlink::start()
     testTaskMavlink = px4_task_spawn_cmd("Mavlink to Toby",
                      SCHED_DEFAULT,
                      SCHED_PRIORITY_MAX,
                      2000,
                      mavlink_main,
                      (argv) ? (char *const *)&argv[2] : (char *const *)NULL);

 */


     // PX4_INFO("Exit");
}

}


namespace TobyDeviceTester{

void testTobyDevice(){


    char tx_buffer[]={"Hallo Michael"};





    TobyDevice *myDevice = new TobyDevice();
    myDevice->write(tx_buffer,13);
    delete myDevice;
}





}


