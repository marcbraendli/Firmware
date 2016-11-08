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

#include <px4_config.h>
#include <drivers/device/device.h>
#include <drivers/drv_led.h>
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


int
toby_init(){
    PX4_INFO("toby_init");
    return 0;
}

class Toby : device::CDev

{
public:
    Toby();
    virtual ~Toby();

	virtual int		init();
	virtual int		ioctl(device::file_t *filp, int cmd, unsigned long arg);
};

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
    DEVICE_DEBUG("TOBY::init");
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
    PX4_INFO("ioctl");
    return 1;
}



namespace
{
//Toby *gLTE;
}


