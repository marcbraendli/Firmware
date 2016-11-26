#include "boundedBuffer.h"
#include "px4_log.h"

#define BUFSIZE 1


static pthread_mutex_t bufferlock = PTHREAD_MUTEX_INITIALIZER;

BoundedBuffer::BoundedBuffer(){

    pthread_cond_init(&isFull,NULL);
    pthread_cond_init(&isEmpty,NULL);

    mySize = (int*)malloc(BUFSIZE*sizeof(int));
    myBuffer = (char**)malloc(BUFSIZE*sizeof(char*));

    head = 0;
    tail = 0;
    numElements=0;
}

BoundedBuffer::~BoundedBuffer(){

    for(int i = 0; i < BUFSIZE; ++i ){
        free(myBuffer[i]);
    }

    free(myBuffer);
    free(mySize);
}

//TODO : Delete after documentation

/*

bool BoundedBuffer::putItem(const char* val, size_t size){

    // = (char *)malloc(_message_buffer.size);

    PX4_INFO("boundedBuffer: putItem()");
    PX4_INFO("boundedBuffer: vor strcpy char: %s",val);
    pthread_mutex_lock(&bufferlock);
    PX4_INFO("boundedBuffer: nach strcpy char: %s",space);


    //space = strdup(val);
    //while is needed because spurious wakeup etc
    while(this->full()){
        pthread_cond_wait(&isFull,&bufferlock);
    }






    myBuffer[0] = (char*)malloc(size*sizeof(char));
    strcpy(myBuffer[0],val);
    mySize[0] = size;
    PX4_INFO("put() got %s",myBuffer[0]);
    ++items;

    //char testPut[] = {"XXXXXXXXXXXXXXXXXX"};

  //  PX4_INFO("boundedBuffer: direct get: %s",testPut);
    sleep(1);
    pthread_cond_signal(&isEmpty);
    pthread_mutex_unlock(&bufferlock);
    return true;
}

*/

/*

int BoundedBuffer::getItem(char *val){

    pthread_mutex_lock(&bufferlock);
    //while is needed because spurious wakeup etc
    while(items == 0){
        PX4_INFO("getItem : has to wait there is no element");
        pthread_cond_wait(&isEmpty,&bufferlock);

    }
    PX4_INFO("boundedBuffer: getItem()");
  //  buffer->get(val);
    PX4_INFO("boundedBuffer getItem() lokaler wert space: %s",space);
    PX4_INFO("boundedBuffer getItem : is %s",myBuffer[0]);

    --items;

    strcpy(val,myBuffer[0]);

    free(myBuffer[0]);
    myBuffer[0] = NULL;
    PX4_INFO("boundedBuffer getItem : is %s",val);
    pthread_cond_signal(&isFull);
    pthread_mutex_unlock(&bufferlock);
    return mySize[0];
}

*/

int BoundedBuffer::getString(char *val){

    pthread_mutex_lock(&bufferlock);

    while(this->empty()){
        pthread_cond_wait(&isEmpty,&bufferlock);
    }
    int temp = tail;
    tail = next(tail);
    --numElements;
    strcpy(val,myBuffer[temp]);
    pthread_cond_signal(&isFull);
    pthread_mutex_unlock(&bufferlock);
    pthread_cond_signal(&isFull);
    pthread_mutex_unlock(&bufferlock);
    return mySize[temp];

}

bool BoundedBuffer::putString(const char* val, size_t size){

    PX4_INFO("boundedBuffer: put String is called()");


    pthread_mutex_lock(&bufferlock);
    PX4_INFO("boundedBuffer: hasLock");

    while(this->full()){
        PX4_INFO("boundedBuffer: isFull");

        pthread_cond_wait(&isFull,&bufferlock);
    }
    myBuffer[head] = (char*)malloc(size);

    strcpy(myBuffer[head],val);
    free(myBuffer[head]);
    mySize[head] = size;
    numElements++;
    head = next(head);
    pthread_cond_signal(&isEmpty);
    pthread_mutex_unlock(&bufferlock);



    return true;
}


