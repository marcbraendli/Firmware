/****************************************************************************
 *
 *   Copyright (c) 2013 PX4 Development Team. All rights reserved.
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
 * @file led.cpp
 *
 * LED driver.
 */



#include "toby.h"
#include <drivers/drv_toby.h>
#include <stdio.h>

/*
 * Ideally we'd be able to get these from up_internal.h,
 * but since we want to be able to disable the NuttX use
 * of leds for system indication at will and there is no
 * separate switch, we need to build independent of the
 * CONFIG_ARCH_LEDS configuration switch.
 */

//is not needed
/*
__BEGIN_DECLS
extern void toby_init();
__END_DECLS
*/

extern "C" __EXPORT int toby_main(int argc, char *argv[]);

int toby_init();



Toby::Toby() :
#ifdef __PX4_NUTTX
    CDev("Toby", "/dev/toby")
#else
    VDev("toby", LED0_DEVICE_PATH)
#endif
{
	// force immediate init/device registration
	init();
}

Toby::~Toby()
{
}




int
Toby::init()
{
    PX4_INFO("TOBY::init");
#ifdef __PX4_NUTTX
	CDev::init();
#else
	VDev::init();
#endif
    toby_init();

	return 0;
}

int
Toby::ioctl(device::file_t *filp, int cmd, unsigned long arg)
{
    PX4_INFO("ioctl mit cmd:  %d",cmd);

    //ein versuch :
    int i = ::device::CDev::ioctl(filp,cmd,arg);
   // CDev::ioctl(*filp, cmd,arg);
sleep(2);
PX4_INFO("ioctl() return %d",i);
return i;
}

int
toby_init(){
    PX4_INFO("toby_init");
    return 0;
}

namespace
{
Toby *gToby;
}

int
toby_main(int argc, char *argv[])
{
    /* set to default */
    const char *device_name = TOBY_DEVICE_PATH;
   // const char *device_name2 = nullptr;
    int myoptind = 1;
    const char *verb = argv[myoptind];




    if (argc < 2) {
        goto out;
    }

    /*
     * Start/load the driver.
     */

    if (!strcmp(verb, "start")) {
        if (gToby != nullptr) {
            warnx("already started");
            return 1;
        }

        gToby = new Toby();



        return 0;
    }

    if (!strcmp(argv[1], "stop")) {
    }

    /*
     * Test the driver/device.
     */
    if (!strcmp(argv[1], "test")) {
    }

    /*
     * Reset the driver.
     */
    if (!strcmp(argv[1], "reset")) {
    }

    /*
     * Print driver status.
     */
    if (!strcmp(argv[1], "status")) {
    }

    if(device_name){

    }

    return 0;

out:
    PX4_ERR("unrecognized command, try 'start', 'stop', 'test', 'reset' or 'status'");
    PX4_ERR("[-d " TOBY_DEVICE_PATH "][-f (for enabling fake)][-s (to enable sat info)]");
    return 1;
}



