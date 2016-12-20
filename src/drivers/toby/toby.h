/**
* @file     toby.cpp
*
* @brief    Device Class for Toby LTE-Module
* @author   Marc Brändli & Michael Lehmann
* @date     10.10.2016
 */


#ifndef TOBY_H
#define TOBY_H


#include <drivers/device/device.h>
#include <px4_config.h>

#include <drivers/device/device_nuttx.h>
#include <termios.h>
#include "tobyDevice.h"
#include "tobyDeviceUart.h"

#include "tobyRingBuffer.h"
#include "pingPongBuffer.h"
#include "tobyDataPipe.h"

// struct which is used to signaling special events to thread
struct threadParameters
{
    TobyDevice* myDevice;
    TobyDataPipe* writeDataPipeBuffer;
    TobyDataPipe* readDataPipeBuffer;
    volatile bool* threadExitSignal;
    volatile bool* threadStartCommSignal;
};



/**
 * @brief The Toby class device driver for Toby L210 LTE-Module.
 * Usage: start toby in NSH and start a mavlink-instance with /dev/toby
 * @date 20.12.2016
 * @author Marc Brändli, Michael Lehmann
 */
class Toby : device::CDev
{
public:
    Toby();
    virtual ~Toby();
    /**
     * @brief init register tobys as a device
     * @return positive if success
     */
    virtual int	init();

    /**
     * @brief open open the character device driver
     * @param filp filepointer to toby
     * @return fileDescriptor
     */
    int open(device::file_t *filp);
    /**
     * @brief close close the character device driver
     * @param filp filepointer to toby
     * @return positive if success
     */
    int	close(device::file_t *filp);
    /**
     * @brief read read data from character device
     * @param filp  filepointer to toby
     * @param buffer pointer to place where data will be stored
     * @param buflen the maximal space of buffer
     * @return number of characters were read
     */
    ssize_t	read(device::file_t *filp, char *buffer, size_t buflen);
    /**
     * @brief write write data to character device
     * @param filp filepointer to toby
     * @param buffer pointer to place where data is stored
     * @param buflen the number of characters to write
     * @return number of characters were written
     */
    ssize_t	write(device::file_t *filp, const char *buffer, size_t buflen);
    /**
     * @brief seek seek function from character device interface
     * @param filp filepointer to toby
     * @param offset
     * @param whence
     * @return positive if success
     */
    off_t	seek(device::file_t *filp, off_t offset, int whence);

    /**
     * @brief ioctl io-control function from character device interface
     * @param filp  filepointer to toby
     * @param cmd   command for io-control
     * @param arg   argument
     * @return positive if success
     */
    int	ioctl(device::file_t *filp, int cmd, unsigned long arg);
    /**
     * @brief poll poll function
     * @param filp filepointer to toby
     * @param fds filedescriptor for notify
     * @param setup
     * @return positive if data avaiable, zero if no data avaiable, negativ if error occours
     */
    int	poll(device::file_t *filp, struct pollfd *fds, bool setup);

    void printStatus(void);
    /**
     * @brief stopAllThreads stop all threading interaction. Used for shutdown
     */
    void stopAllThreads(void);



private:

    int set_flowcontrol(int fd, int control);
    void *doClose(void *arg);

    TobyDeviceUart* myTobyDevice;

    struct termios options= {};
    pthread_t *atCommanderThread;
    threadParameters atCommanderParameters;
    volatile bool threadExitSignal;

    TobyDataPipe* writeDataPipe;
    TobyDataPipe* readDataPipe;


};

#endif
