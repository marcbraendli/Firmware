#include <drivers/device/ringbuffer.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

#ifndef PINGPONGBUFFER
#define PINGPONGBUFFER



/**
 * @file boundedBuffer.h
 *
 *
 *
 */




class PingPongBuffer {
public:
    PingPongBuffer();
    virtual ~PingPongBuffer();
    size_t PutData(const char* val, size_t size);
    int GetData(char* val, size_t size);
    bool GetDataSuccessfull();
    bool DataAvaiable();
    char* getActualReadBuffer(void);

    enum{
        AbsolutBufferLength = 64,
    };

private:


    int next(int sizeWritten);
    void changeBufferListIndex(){
        if(bufferListIndex > 0){
            bufferListIndex = 0;
        }
        else{
            bufferListIndex = 1;
        }

    }


    int head;
    int bufferListIndex;
    char* bufferList[2];
    //the value which could be reloaded should be volatile ...
    //but then we can't compile?
    char* actualWriteBuffer;
    char* volatile actualReadBuffer;

};




#endif
