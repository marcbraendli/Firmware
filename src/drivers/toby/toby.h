#include <drivers/device/device.h>
#include <px4_config.h>

#include <drivers/device/device_nuttx.h>
#include <termios.h>
#include "tobyDevice.h"
#include "boundedBuffer.h"




struct myStruct //TODO rename
{
  TobyDevice* myDevice;
  BoundedBuffer* writeBuffer;  //TODO rename
  BoundedBuffer* readBuffer;
};




class Toby : device::CDev

{
public:
    Toby();
    virtual ~Toby();

    virtual int		init();


    int open(device::file_t *filp);
    int	close(device::file_t *filp);
    ssize_t	read(device::file_t *filp, char *buffer, size_t buflen);
    ssize_t	write(device::file_t *filp, const char *buffer, size_t buflen);
    off_t	seek(device::file_t *filp, off_t offset, int whence);


    int	ioctl(device::file_t *filp, int cmd, unsigned long arg);
    int	poll(device::file_t *filp, struct pollfd *fds, bool setup);



    static pthread_cond_t pollEventSignal;  // has to be public, otherwise we cant use it from at commander
    static pthread_mutex_t pollingMutex;

private:
    TobyDevice* myTobyDevice;

    struct termios options= {};
    pthread_t *writerThread;
    pthread_t *readerThread;

    pthread_t *atCommanderThread;
    myStruct atCommanderParameters;

    pthread_t *pollingThread;

    myStruct pollingThreadParameters;



    myStruct workerParameters; //TODO rename
    myStruct readerParameters;



    BoundedBuffer* writeBuffer;
    BoundedBuffer* readBuffer;


    // our worker thread function, needs to be static, otherwise pthread can't execute (is C, not C++)
    static void *writeWork(void *arg);
    static void *readWork(void *arg);
    static void *atCommanderStart(void *arg);



    bool done;


};
