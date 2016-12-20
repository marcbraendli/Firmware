/**
* @file     toby.h
*
* @brief    Device Class für das Toby Modul
* @author   Michael Lehmann
* @date     10.10.2016
*/

#include <drivers/device/device.h>
#include <px4_config.h>

#include <drivers/device/device_nuttx.h>
#include <termios.h>
#include "tobyDevice.h"
#include "tobyRingBuffer.h"
#include "pingPongBuffer.h"
#include "tobyDataPipe.h"






struct myStruct //TODO rename
{
    TobyDevice* myDevice;
    TobyRingBuffer* writeBuffer;  //TODO rename
    TobyRingBuffer* readBuffer;
    PingPongBuffer* writePongBuffer;
    TobyDataPipe* writeDataPipeBuffer;
    TobyDataPipe* readDataPipeBuffer;
};

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




private:

    TobyDevice* myTobyDevice;

    struct termios options= {};
    pthread_t *writerThread;
    pthread_t *readerThread;


    myStruct pollingThreadParameters;
    myStruct workerParameters; //TODO rename
    myStruct readerParameters;

    TobyRingBuffer* writeBuffer;
    TobyRingBuffer* readBuffer;
    PingPongBuffer* writePongBuffer;
    TobyDataPipe* writeDataPipeBuffer;
    TobyDataPipe* readDataPipeBuffer;


    // our worker thread function, needs to be static, otherwise pthread can't execute (is C, not C++)
    static void *writeWork(void *arg);
    static void *readWork(void *arg);
    bool done;


};
