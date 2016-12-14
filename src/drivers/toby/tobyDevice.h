#ifndef tobyDevice_h
#define tobyDevice_h



#include <drivers/device/device.h>
#include <px4_config.h>
#include <drivers/device/ringbuffer.h>

#include <drivers/device/device_nuttx.h>
#include <termios.h>

#include <fcntl.h>

/**
 * @brief The TobyDevice class is a interface for the Toby-L210
 */


class TobyDevice

{
public:
    TobyDevice(){

    }

    virtual ~TobyDevice(){

    }

    virtual ssize_t	read(char *buffer, size_t buflen) = 0;
    virtual ssize_t	write(const char *buffer, size_t buflen) = 0;
    virtual int ioctl(int cmd, unsigned long arg)= 0;
    virtual int	poll(int timeout) = 0;

private:


};




#endif
