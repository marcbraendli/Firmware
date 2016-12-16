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


struct threadParameters
{
    TobyDevice* myDevice;
    TobyDataPipe* writeDataPipeBuffer;
    TobyDataPipe* readDataPipeBuffer;
    volatile bool* threadExitSignal;
    volatile bool* threadStartCommSignal;
};

// struct which is used to signaling special events to thread


/**
* @brief Toby Class, damit das interface im /dev/ Ordner von NuttX erscheint
*
* Genaue beschreibung der Klasse hier....
*
* @date ?
*/
class Toby : device::CDev
{
public:
    Toby();
    virtual ~Toby();
    virtual int	init();


    int open(device::file_t *filp);
    int	close(device::file_t *filp);
    ssize_t	read(device::file_t *filp, char *buffer, size_t buflen);
    ssize_t	write(device::file_t *filp, const char *buffer, size_t buflen);
    off_t	seek(device::file_t *filp, off_t offset, int whence);


    int	ioctl(device::file_t *filp, int cmd, unsigned long arg);
    int	poll(device::file_t *filp, struct pollfd *fds, bool setup);

    void printStatus(void);
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



    // our worker thread function, needs to be static, otherwise pthread can't execute (is C, not C++)
    static void *writeWork(void *arg);
    static void *readWork(void *arg);


};

#endif
