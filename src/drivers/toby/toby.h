#include <drivers/device/device.h>
#include <px4_config.h>

#include <drivers/device/device_nuttx.h>
#include <termios.h>







class Toby : device::CDev

{
public:
    Toby();
    virtual ~Toby();

    virtual int		init();


    virtual int open(device::file_t *filp);
    virtual int	close(device::file_t *filp);
    virtual ssize_t	read(device::file_t *filp, char *buffer, size_t buflen);
    virtual ssize_t	write(device::file_t *filp, const char *buffer, size_t buflen);
    virtual off_t	seek(device::file_t *filp, off_t offset, int whence);


    virtual int		ioctl(device::file_t *filp, int cmd, unsigned long arg);
    virtual int	poll(device::file_t *filp, struct pollfd *fds, bool setup);

private:
    int uart0_filestream;

    struct termios options= {};



};
