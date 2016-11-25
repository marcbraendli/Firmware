#include <drivers/device/device.h>
#include <px4_config.h>
#include <drivers/device/ringbuffer.h>

#include <drivers/device/device_nuttx.h>
#include <termios.h>
#include "tobyDevice.h"





struct myStruct
{
  TobyDevice* myDevice;
  ringbuffer::RingBuffer* writeBuffer;
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

    TobyDevice* myTobyDevice;


private:
    ringbuffer::RingBuffer *writeBuffer;
    struct termios options= {};
    pthread_t *writerThread;
    myStruct workerParameters;

    // our worker thread function, needs to be static, otherwise pthread can't execute (is C, not C++)
    static void *writeWork(void *arg);

    static pthread_cond_t v;
    static pthread_mutex_t m;

    //



};
