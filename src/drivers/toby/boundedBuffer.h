#include <drivers/device/ringbuffer.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>







class BoundedBuffer

{
public:
    BoundedBuffer();
    ~BoundedBuffer();

    bool putItem(const char* val, size_t size);
    int getItem(char* val);

    bool putString(const char* val, size_t size);
    int  getString(char* val);


private:

    bool empty(void){
        return (head == tail);
    }


    pthread_cond_t isFull;
    pthread_cond_t isEmpty;
    char space[100] = {};
    char* mySpaceTest;
    char** myBuffer;
    int* mySize;
    int items;

    int head;
    int tail;


};
