#include <drivers/device/device.h>
#include <px4_config.h>
#include <drivers/device/ringbuffer.h>

#include <drivers/device/device_nuttx.h>
#include <termios.h>







class TobyDevice

{
public:
    TobyDevice();
    virtual ~TobyDevice();


    ssize_t	read(char *buffer, size_t buflen);
    ssize_t	write(const char *buffer, size_t buflen);



    virtual int ioctl(int cmd, unsigned long arg);
    virtual int	poll(struct pollfd *fds, bool setup);

private:
    int uart0_filestream;
    struct termios options= {};



};
