#include <drivers/device/device.h>
#include <px4_config.h>







class Toby : device::CDev

{
public:
    Toby();
    virtual ~Toby();

    virtual int		init();
    virtual int		ioctl(device::file_t *filp, int cmd, unsigned long arg);
};
