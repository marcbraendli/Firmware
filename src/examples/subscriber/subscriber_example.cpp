/****************************************************************************
 *
 *   Copyright (C) 2014 PX4 Development Team. All rights reserved.
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
 * @file subscriber_example.cpp
 * Example subscriber for ros and px4
 *
 * @author Thomas Gubler <thomasgubler@gmail.com>
 */

#include "subscriber_params.h"
#include "subscriber_example.h"



using namespace px4;

void rc_channels_callback_function(const px4_rc_channels &msg)
{
	PX4_INFO("I heard: [%" PRIu64 "]", msg.data().timestamp_last_valid);
}

SubscriberExample::SubscriberExample() :
	_n(_appState),
	_p_sub_interv("SUB_INTERV", PARAM_SUB_INTERV_DEFAULT),
    _p_test_float("SUB_TESTF", PARAM_SUB_TESTF_DEFAULT)
    //_work{}
{
	/* Read the parameter back as example */
	_p_sub_interv.update();
	_p_test_float.update();
	PX4_INFO("Param SUB_INTERV = %d", _p_sub_interv.get());
	PX4_INFO("Param SUB_TESTF = %.3f", (double)_p_test_float.get());

    //nötig da sonst dekonstruktor streikt gemäss lis3dml.cpp
    memset(&_work, 0, sizeof(_work));

//	/* Do some subscriptions */
//	/* Function */
//	_n.subscribe<px4_rc_channels>(rc_channels_callback_function, _p_sub_interv.get());

//	/* No callback */
//	_sub_rc_chan = _n.subscribe<px4_rc_channels>(500);

//	/* Class method */
//	_n.subscribe<px4_rc_channels>(&SubscriberExample::rc_channels_callback, this, 1000);

//	/* Another class method */
//	_n.subscribe<px4_vehicle_attitude>(&SubscriberExample::vehicle_attitude_callback, this, 1000);

//	/* Yet antoher class method */
//    _n.subscribe<px4_parameter_update>(&SubscriberExample::parameter_update_callback, this, 1000);


//    /*Custom subscribtion */
//    _n.subscribe<px4_actuator_armed>(&SubscriberExample::actuator_armed_callback, this, 1000);

//    _n.subscribe<px4_vehicle_status>(&SubscriberExample::vehicle_status_callback, this, 1000);


    _n.subscribe<px4_sensor_custom>(&SubscriberExample::sensor_custom_callback, this, 5000);



	PX4_INFO("subscribed");
}

/**
 * This tutorial demonstrates simple receipt of messages over the PX4 middleware system.
 * Also the current value of the _sub_rc_chan subscription is printed
 */
void SubscriberExample::rc_channels_callback(const px4_rc_channels &msg)
{
	PX4_INFO("rc_channels_callback (method): [%" PRIu64 "]",
		 msg.data().timestamp_last_valid);
	PX4_INFO("rc_channels_callback (method): value of _sub_rc_chan: [%" PRIu64 "]",
		 _sub_rc_chan->data().timestamp_last_valid);
}

void SubscriberExample::vehicle_attitude_callback(const px4_vehicle_attitude &msg)
{
	PX4_INFO("vehicle_attitude_callback (method): [%" PRIu64 "]",
         msg.data().timestamp);
}

void SubscriberExample::parameter_update_callback(const px4_parameter_update &msg)
{
	PX4_INFO("parameter_update_callback (method): [%" PRIu64 "]",
		 msg.data().timestamp);
	_p_sub_interv.update();
	PX4_INFO("Param SUB_INTERV = %d", _p_sub_interv.get());
	_p_test_float.update();
	PX4_INFO("Param SUB_TESTF = %.3f", (double)_p_test_float.get());
}


//**********************myFunction**************************

void SubscriberExample::actuator_armed_callback(const px4_actuator_armed &msg)
{
    PX4_INFO("actuator_armed_callback (method): [%" PRIu64 "]",
         msg.data().timestamp);
}

void SubscriberExample::vehicle_status_callback(const px4_vehicle_status &msg)
{
    PX4_INFO("vehicle_status_callback (method): [%" PRIu64 "]",
         msg.data().timestamp);

    PX4_INFO("vehicle_status_callback (method): [%" PRIu64 "]",
         msg.data().arming_state);

}

void SubscriberExample::sensor_custom_callback(const px4_sensor_custom &msg)
{
    PX4_INFO("sensor_custom_callback (method): [%" PRIu64 "]",
         msg.data().custom_parameter_for_test);
        //_n.~NodeHandle();

        //define some work for analyzing

       // work_queue(HPWORK, &_work, (worker_t)&SubscriberExample::subscriber_trampoline, this, 1000);

       // Does not work, why ?
       // work_queue(LPWORK, &_work, (worker_t)&SubscriberExample::subscriber_trampoline2, this, 500);

       // work_queue(HPWORK, &_work, (worker_t)&SubscriberExample::subscriber_trampoline2, this, 100);
       // work_queue(USRWORK, &_workLP, (worker_t)&SubscriberExample::subscriber_trampoline, this, 100);



        work_queue(HPWORK, &_work, (worker_t)&SubscriberExample::subscriber_trampoline2, this, 100);
        work_queue(HPWORK, &_workLP, (worker_t)&SubscriberExample::subscriber_trampoline, this, 100);
        PX4_INFO("scheduled some work");

}

void SubscriberExample::subscriber_trampoline(void *arg){
    PX4_INFO("This is SubscriberExample Trampoline()");

    sleep(1);
    PX4_INFO("Trampoline() finished");


}

void SubscriberExample::subscriber_trampoline2(void *arg){
    PX4_INFO("This is SubscriberExample Trampoline2()");

    sleep(8);
    PX4_INFO("Trampoline2() finished");


}

