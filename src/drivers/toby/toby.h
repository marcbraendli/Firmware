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



//for readAtfromSD function
#define SD_CARD_PATH       "/fs/microsd/toby/at-inits.txt"


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


    virtual int open(device::file_t *filp);
    virtual int	close(device::file_t *filp);
    virtual ssize_t	read(device::file_t *filp, char *buffer, size_t buflen);
    virtual ssize_t	write(device::file_t *filp, const char *buffer, size_t buflen);
    virtual off_t	seek(device::file_t *filp, off_t offset, int whence);
    virtual int	ioctl(device::file_t *filp, int cmd, unsigned long arg);
    virtual int	poll(device::file_t *filp, struct pollfd *fds, bool setup);




private:

    bool initLTEModule(void);
    bool readAtFromSD();
    void printAtCommands();
    int getAtCommandLength(const char* at_command);
    bool setDirectLinkMode();
    bool initTobyModul();
    bool tobyAlive(int times);
    void printStatus(void);


    int set_flowcontrol(int fd, int control);
    void *doClose(void *arg);

    TobyDeviceUart* myTobyDevice;

    struct termios options= {};

    //Enumeration for readAtfromSD function
    enum{
        MAX_AT_COMMANDS = 20,
        MAX_CHAR_PER_AT_COMMANDS =40,
        READ_BUFFER_LENGHT =100
    };

    int   numberOfAt;
    char  atCommandSendArray[MAX_AT_COMMANDS][MAX_CHAR_PER_AT_COMMANDS];
    char* atCommandSendp[MAX_AT_COMMANDS];

    char* temporaryBuffer; // delete later, just for step-by-step test's
    char* temporarySendBuffer;

    const char* atEnterCommand;
    const char* atDirectLinkRequest;
    const char* atResponseOk;
    const char* atReadyRequest;
    const char* atDirectLinkOk;
    const char* stringEnd;
    const char* atResetCommand;
    const char* atExitDirectLink;




};

#endif
