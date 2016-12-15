#include "tobyRingBuffer.h"
#include "px4_log.h"



//dirty intializing
static pthread_mutex_t bufferlock = PTHREAD_MUTEX_INITIALIZER;

TobyRingBuffer::TobyRingBuffer(int inBufsize, int inBufdeepth) : bufsize (inBufsize), bufdeepth (inBufdeepth) {

    pthread_cond_init(&isFull,NULL);
    pthread_cond_init(&isEmpty,NULL);

    mySize = (int*)malloc(BUFSIZE*sizeof(int));
    myBuffer = (char**)malloc(BUFSIZE*sizeof(char*));

    head = 0;
    tail = 0;
    numElements=0;

    //we allocate the buffersize we need
    for(int i = 0; i < BUFSIZE; ++i){
        myBuffer[i] = (char*)malloc(BUFDEEPTH*sizeof(char));
        if(myBuffer[i] == NULL){
            PX4_INFO("ERROR NO SPACE AVAIABLE");
        }

    }
}

TobyRingBuffer::~TobyRingBuffer(){

    for(int i = 0; i < BUFSIZE; ++i ){
        free(myBuffer[i]);
        free(myBuffer[i]);
    }

    free(myBuffer);
    free(mySize);

}



int TobyRingBuffer::getString(char *val, size_t size){

    pthread_mutex_lock(&bufferlock);
//    PX4_INFO("getString: hasLock");

    while(this->empty()){
        pthread_cond_wait(&isEmpty,&bufferlock);
        PX4_INFO("Buffer is empty");
    }

    if(mySize[tail] > size ){
        PX4_INFO("TobyRingBuffer getString requested buffer isn't big enough");
        pthread_cond_signal(&isFull);
        pthread_mutex_unlock(&bufferlock);
        return 0;
    }

    int temp = tail;
    tail = next(tail);
    --numElements;
    memcpy(val,myBuffer[temp],mySize[temp]);
    temp = mySize[temp];
    pthread_cond_signal(&isFull);
    pthread_mutex_unlock(&bufferlock);
    return temp;

}

bool TobyRingBuffer::putString(const char* val, size_t size){


    pthread_mutex_lock(&bufferlock);

    while(this->full()){
        //this is needed because mavlink does not handle if nothing was written,
        //otherwise here we would return 0 and so no lock would be needed
        pthread_cond_wait(&isFull,&bufferlock);
    }
   while(myBuffer[head]== NULL){
       PX4_INFO("ERROR PUT STRING couldn't get space from malloc");
       sleep(2);

   }
   if(size > BUFDEEPTH){
       PX4_INFO("Bounded Buffer: Not enough space!");
   }

    memcpy(myBuffer[head],val,size);

    mySize[head] = size;
    numElements++;
    head = next(head);

    pthread_cond_signal(&isEmpty);
    pthread_mutex_unlock(&bufferlock);



    return true;
}


// dangerous but more efficenct, because we don't need copy twice!
int TobyRingBuffer::getActualReadBuffer(char* &val){
    val = myBuffer[tail];
    return mySize[tail];
}

bool TobyRingBuffer::gotDataSuccessful(){

    pthread_mutex_lock(&bufferlock);
    pthread_cond_signal(&isFull);
    tail = next(tail);
    --numElements;
    pthread_mutex_unlock(&bufferlock);

    return true;

}



