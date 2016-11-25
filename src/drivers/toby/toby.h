#include <drivers/device/device.h>
#include <px4_config.h>
#include <drivers/device/ringbuffer.h>

#include <drivers/device/device_nuttx.h>
#include <termios.h>
#include "tobyDevice.h"








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

private:
    ringbuffer::RingBuffer *writeBuffer;
    struct termios options= {};
    TobyDevice* myTobyDevice;
    pthread_t *myThread;



};
