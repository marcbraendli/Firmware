#include "boundedBuffer.h"
#include "px4_log.h"

#define BUFSIZE 1


static pthread_mutex_t bufferlock = PTHREAD_MUTEX_INITIALIZER;

BoundedBuffer::BoundedBuffer(){

    pthread_cond_init(&isFull,NULL);
    pthread_cond_init(&isEmpty,NULL);

    mySpaceTest=NULL;
    items = 0;
    mySize = (int*)malloc(BUFSIZE*sizeof(int));
    myBuffer = (char**)malloc(BUFSIZE*sizeof(char*));
}


bool BoundedBuffer::putItem(const char* val, size_t size){

    // = (char *)malloc(_message_buffer.size);

    PX4_INFO("boundedBuffer: putItem()");
    PX4_INFO("boundedBuffer: vor strcpy char: %s",val);
    pthread_mutex_lock(&bufferlock);
    strcpy(space,val);
    PX4_INFO("boundedBuffer: nach strcpy char: %s",space);


    //space = strdup(val);
    //while is needed because spurious wakeup etc
    while(false){
        pthread_cond_wait(&isFull,&bufferlock);
    }


    if(true){
        PX4_INFO("putItem : successfull");
    }
    else{
        PX4_INFO("putItem : fails");

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

int BoundedBuffer::getString(char *val){

    return 0;
}

bool BoundedBuffer::putString(const char* val, size_t size){


    return true;
}


