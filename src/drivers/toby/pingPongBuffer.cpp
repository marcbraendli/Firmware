/**
* @file     pingPongBuffer.cpp
*
* @brief    A PingPongBuffer implementation for LTE-Module dispatching
* @author   Marc Brändli & Michael Lehmann
* @date     12.12.2016
*/


#include "pingPongBuffer.h"
#include <stdlib.h>
#include <px4_log.h>


static pthread_mutex_t pingPongBufferlock = PTHREAD_MUTEX_INITIALIZER;


PingPongBuffer::PingPongBuffer() {

    head = 0;

    bufferList[0]  = (char*)malloc(AbsolutBufferLength*sizeof(char));
    bufferList[1]  = (char*)malloc(AbsolutBufferLength*sizeof(char));

    if(bufferList[0] == nullptr){
        PX4_INFO("Error, malloc does not work");

    }
    if(bufferList[1] == NULL){
        PX4_INFO("Error, malloc does not work");

    }

    bufferListIndex = 0; // begin with buffer 0
    actualWriteBuffer = bufferList[bufferListIndex]; //set actual writeBuffer
    actualReadBuffer = nullptr;	// there is nothing to read

    pthread_cond_init(&isFull,nullptr);
    pthread_cond_init(&isEmpty,nullptr);

}

PingPongBuffer::~PingPongBuffer() {
    free(bufferList[0]);
    free(bufferList[1]);
    pthread_cond_destroy(&isFull);
    pthread_cond_destroy(&isEmpty);


}

size_t PingPongBuffer::PutData(const char* val, size_t size){

    pthread_mutex_lock(&pingPongBufferlock);

    if(size >= AbsolutBufferLength - head) {         // check if we need more space
        int ersterTeil = AbsolutBufferLength - head; // fill actual buffer
        memcpy(actualWriteBuffer + head,val,ersterTeil);
        head+=ersterTeil;

        while(DataAvaiable()){
            pthread_cond_wait(&isFull,&pingPongBufferlock);

        }

        PX4_INFO("BUFFER : save data into new empty buffer");

        // change buffer if is already read

            int zweiterTeil = size - ersterTeil;		// calculate overflow

            //pthread_mutex_lock(&pingPongBufferlock);
            actualReadBuffer = actualWriteBuffer;

            changeBufferListIndex();
            head = 0;

            actualWriteBuffer = bufferList[bufferListIndex]; //switch the buffer
            memcpy(actualWriteBuffer + head,val+zweiterTeil-1,zweiterTeil);
            head = zweiterTeil;
            pthread_mutex_unlock(&pingPongBufferlock);

            return size;

    }

    // fill buffer normal, there is enough space
    memcpy(actualWriteBuffer + head*sizeof(char*) ,val,size);

    head += size;
    pthread_mutex_unlock(&pingPongBufferlock);

    return size;

}


int PingPongBuffer::GetData(char *val, size_t size){


    // DON'T work , but why?!
   // (val) = (actualReadBuffer);

    return 0;
}

bool PingPongBuffer::GetDataSuccessfull(){

    pthread_mutex_lock(&pingPongBufferlock);
    actualReadBuffer = 0;
    pthread_cond_signal(&isFull);
    pthread_mutex_unlock(&pingPongBufferlock);

    return true;

}

bool PingPongBuffer::DataAvaiable(){

    bool return_value = false;
    return_value = (actualReadBuffer != nullptr);
    return return_value;
}


char* PingPongBuffer::getActualReadBuffer(){
    return actualReadBuffer;
}

