#include <drivers/device/ringbuffer.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>



//Number of buffered elements in buffer, later, we may change to a parameter in constructor with enum, easier to test)
enum{
    BUFSIZE = 5,
};


/**
 * @file boundedBuffer.h
 *
 * The Name "BoundedBuffer" isnt valid anymore, its actual a CircularBuffer
 * is Threadsafe, works only with the toby Write-Worker
 *
 * preimplemented RingBuffer of PX4 didn't work
 *
 */



class BoundedBuffer

{
public:
    BoundedBuffer();
    ~BoundedBuffer();

    // artefacts, not needed anymore just for debugging and documentation
    bool putItem(const char* val, size_t size);
    int getItem(char* val);
    //********************************************************

    // the right ones ****************************************
    bool putString(const char* val, size_t size);
    int  getString(char* val);

    bool empty(void){
        return (numElements == 0);
    }

    bool full(void){

        return (numElements == BUFSIZE);
    }

    int next(int actual){
        if(actual == BUFSIZE-1){
            return 0;
        }

        return (++actual);
    }


private:


    pthread_cond_t isFull;
    pthread_cond_t isEmpty;

    char** myBuffer;
    int* mySize;


    int head;
    int tail;
    int numElements;


};
