#include <drivers/device/ringbuffer.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

#ifndef BOUNDEDBUFFER
#define BOUNDEDBUFFER


//Number of buffered elements in buffer, later, we may change to a parameter in constructor with enum, easier to test)
enum{
    BUFSIZE = 10,
};


/**
 * @file boundedBuffer.h
 *
 *
 *
 */



class PingPongBuffer

{
public:

    PingPongBuffer();
    ~PingPongBuffer();

};


#endif
