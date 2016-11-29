#include <drivers/device/ringbuffer.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>



//Number of buffered elements in buffer, later, we may change to a parameter in constructor with enum, easier to test)
enum{
    BUFSIZE = 10,
};


/**
 * @file boundedBuffer.h
 *
 * The Name "BoundedBuffer" isnt valid anymore, its actual a CircularBuffer
 * is Threadsafe, works only with the toby-module
 *
 * BUFSIZE: Number of total items
 * BUFDEEPTH: Maximal size of a single item
 *
 *
 *
 */



class BoundedBuffer

{
public:

    BoundedBuffer();
    ~BoundedBuffer();


    /**
     * @brief putString put data in buffer
     * @param val pointer to the data should be stored
     * @param size sizeof(char) of the data should be stored
     * @return true if successfull
     */
    bool putString(const char* val, size_t size);


    /**
     * @brief getString get data from buffer
     * @param val pointer to the destination where the data should be stored
     * @param size maximum sizeof(char) of the destination
     * @return returns the total number of sizeof(char) were stored in the destnation, if return 0, no data were stored
     */
    int  getString(char* val, size_t size);

    /**
     * @brief empty
     * @return true if the buffer is empty
     */
    bool empty(void){
        return (numElements == 0);
    }

    /**
     * @brief full
     * @return true if the buffer is full
     */
    bool full(void){

        return (numElements == BUFSIZE);
    }

private:

    int next(int actual){
        if(actual == BUFSIZE-1){
            return 0;
        }

        return (++actual);
    }


    pthread_cond_t isFull;
    pthread_cond_t isEmpty;

    char** myBuffer;
    int* mySize;
    int head;
    int tail;
    int numElements;


};
