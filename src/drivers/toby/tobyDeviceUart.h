#ifndef tobyDeviceUart_h
#define tobyDeviceUart_h



#include <drivers/device/device.h>
#include <px4_config.h>
#include <drivers/device/ringbuffer.h>
#include "tobyDevice.h"


#include <drivers/device/device_nuttx.h>
#include <termios.h>

#include <fcntl.h>




class TobyDeviceUart : public TobyDevice

{
public:
    TobyDeviceUart();
    virtual ~TobyDeviceUart();


    ssize_t	read(char *buffer, size_t buflen);
    ssize_t	write(const char *buffer, size_t buflen);

    static void *writeToUart(void *arg);

    int ioctl(int cmd, unsigned long arg);
    int	poll(int timeout);

private:
    bool uart_open(void);
    void uart_close();
    int uart0_filestream;
    struct termios options= {};



};




#endif
