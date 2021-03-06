/****************************************************************************
 *
 *   Copyright (c) 2012-2015 PX4 Development Team. All rights reserved.
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

using namespace DriverFramework;

//extern int main(int argc, char **argv);
extern "C" __EXPORT int custom_daemon_app_main(int argc, char *argv[]);

using namespace DriverFramework;

extern int main(int argc, char **argv);
extern "C" __EXPORT int custom_daemon_app_main(int argc, char *argv[]);

static bool thread_should_exit = false;		/**< daemon exit flag */
static bool thread_running = false;		/**< daemon status flag */
static int daemon_task;				/**< Handle of daemon task / thread */


static DevHandle h_rgbleds_custom;
//void rgbled_set_mode(rgbled_mode_t mode);

/**
 * daemon management function.
 */
__EXPORT int custom_daemon_app_main(int argc, char *argv[]);

/**
 * Mainloop of daemon.
 */
int custom_daemon_thread_main(int argc, char *argv[]);

/**
 * Print the correct usage.
 */
static void usage(const char *reason);

/**
 * Function for analizing architecture
 */
void doSomeOtherShit(void);
void myInit();

static DevHandle h_leds;

static void
usage(const char *reason)
{
	if (reason) {
		warnx("%s\n", reason);
	}

	warnx("usage: daemon {start|stop|status} [-p <additional params>]\n\n");
}

/**
 * The daemon app only briefly exists to start
 * the background job. The stack size assigned in the
 * Makefile does only apply to this management task.
 *
 * The actual stack size should be set in the call
 * to task_create().
 */
int custom_daemon_app_main(int argc, char *argv[])
{
	if (argc < 2) {
		usage("missing command");
		return 1;
	}

	if (!strcmp(argv[1], "start")) {

		if (thread_running) {
			warnx("daemon already running\n");
			/* this is not an error */
			return 0;
		}

		thread_should_exit = false;
		daemon_task = px4_task_spawn_cmd("daemon",
						 SCHED_DEFAULT,
                         SCHED_PRIORITY_MAX,
						 2000,
                         custom_daemon_thread_main,
						 (argv) ? (char *const *)&argv[2] : (char *const *)NULL);
		return 0;
	}

	if (!strcmp(argv[1], "stop")) {
		thread_should_exit = true;
		return 0;
	}

	if (!strcmp(argv[1], "status")) {
		if (thread_running) {
			warnx("\trunning\n");

		} else {
			warnx("\tnot started\n");
		}

		return 0;
	}

	usage("unrecognized command");
	return 1;
}

int custom_daemon_thread_main(int argc, char *argv[])
{
    DevMgr::getHandle(RGBLED0_DEVICE_PATH, h_rgbleds_custom);

        struct sensor_custom_s att;
        memset(&att, 0, sizeof(att));
        orb_advert_t att_pub = orb_advertise(ORB_ID(sensor_custom), &att);


       // DevMgr::getHandle("/dev/led0", h_leds);



	warnx("[daemon] starting\n");

	thread_running = true;
    warnx("Hello daemon!\n");
    PX4_INFO("Thread actually running with high priority");

    rgbled_set_mode(RGBLED_MODE_BLINK_NORMAL);
    rgbled_set_color(RGBLED_COLOR_GREEN);


	while (!thread_should_exit) {

  //      rgbled_set_mode(RGBLED_MODE_PATTERN);

        att.custom_parameter_for_test = 100;
        orb_publish(ORB_ID(sensor_custom), att_pub, &att);




        sleep(1);
	}

	warnx("[daemon] exiting.\n");

	thread_running = false;

	return 0;
}

void doSomeOtherShit(void){
static int i;


/*
if(false){
//how to start a new daemon
    daemon_task = px4_task_spawn_cmd("daemon2",
                     SCHED_DEFAULT,
                     SCHED_PRIORITY_MAX,
                     2000,
                     custom_daemon_thread_main,
                     NULL);
}
*/
    ++i;

}





void myInit(){
        DevMgr::getHandle(LED0_DEVICE_PATH, h_leds);
}
