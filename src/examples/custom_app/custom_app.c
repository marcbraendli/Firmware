/**
 * @file    custom_app.c
 *
 * @brief   Testapplication in C, to show how a App has to look like
 *
 *          This App subscribes itself to the Topic "sensor_custom" and print the next four
 *          Values, after this it close itself.
 *
 * @author  Marc Brändli
 * @date    3.12.2016
 */

#include <px4_config.h>
#include <px4_posix.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
#include <px4_tasks.h>

#include <uORB/uORB.h>
#include <uORB/topics/sensor_custom.h>
#include <uORB/topics/vehicle_attitude.h>

// Own Topic for Test
#include <uORB/topics/sensor_combined.h>


//Exportiert die App in die System Console
__EXPORT int custom_app_main(int argc, char *argv[]);



int custom_app_main(int argc, char *argv[])
{
    //Output in the System Console
    PX4_INFO("This is Custom_Simple_App");

    //subscribe to sensor_combined topic
    int sensor_sub_fd = orb_subscribe(ORB_ID(sensor_custom));

    //limit the update rate to 5 Hz / 200ms
    orb_set_interval(sensor_sub_fd, 200);


    px4_pollfd_struct_t fds[]=  // bracket
    {

        {
            .fd = sensor_sub_fd,
            .events = POLLIN,

        },
    };




    int error_counter = 0;

    for (int i = 0; i < 5; i++) {
        // wait for sensor update of 1 file descriptor for 2000 ms (2 second)
        int poll_ret = px4_poll(fds, 1, 2000);

        // handle the poll result
        if (poll_ret == 0) {
            // this means none of our providers is giving us data
            PX4_ERR("Got no data within a second");

        } else if (poll_ret < 0) {
            // this is seriously bad - should be an emergency
            if (error_counter < 10 || error_counter % 50 == 0) {
                // use a counter to prevent flooding (and slowing us down)
                PX4_ERR("ERROR return value from poll(): %d", poll_ret);
            }

            error_counter++;

        } else {

            if (fds[0].revents & POLLIN) {
                // obtained data for the first file descriptor
                struct sensor_custom_s raw;
                // copy sensors raw data into local buffer
                orb_copy(ORB_ID(sensor_custom), sensor_sub_fd, &raw);
                PX4_INFO("Accelerometer:\t%8.4f\t%8.4f\t%8.4f",
                         (double)raw.accelerometer_m_s2[0],
                        (double)raw.accelerometer_m_s2[1],
                        (double)raw.accelerometer_m_s2[2]);

                PX4_INFO("custom_parameter : \t%8.4f ", (double)raw.custom_parameter_for_test);

            }
        }
    }

    PX4_INFO("exiting");
    return 0;
}

