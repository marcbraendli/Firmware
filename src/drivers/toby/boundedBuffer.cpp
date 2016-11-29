#include "boundedBuffer.h"
#include "px4_log.h"

enum{
    BUFDEEPTH = 62,
};


//dirty intializing
//TODO: Do it in a better way?
static pthread_mutex_t bufferlock = PTHREAD_MUTEX_INITIALIZER;

BoundedBuffer::BoundedBuffer(){

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

BoundedBuffer::~BoundedBuffer(){

    for(int i = 0; i < BUFSIZE; ++i ){
        free(myBuffer[i]);
        free(myBuffer[i]);
    }

    free(myBuffer);
    free(mySize);
}



int BoundedBuffer::getString(char *val, size_t size){

    pthread_mutex_lock(&bufferlock);
//    PX4_INFO("getString: hasLock");

    while(this->empty()){
        pthread_cond_wait(&isEmpty,&bufferlock);
    }

    if(mySize[tail] > size ){
        // the requester's buffer isnt' big enough
        PX4_INFO("boundedBuffer getString requested buffer isn't big enough");
        pthread_cond_signal(&isFull);
        pthread_mutex_unlock(&bufferlock);
        return 0;
    }

    int temp = tail;
    tail = next(tail);
    --numElements;
    memcpy(val,myBuffer[temp],mySize[temp]);
    //free(myBuffer[head]); // we call free in deconstructor
    temp = mySize[temp];
    pthread_cond_signal(&isFull);
 //   PX4_INFO("getString: leaveLock");

    pthread_mutex_unlock(&bufferlock);
    return temp;

}

bool BoundedBuffer::putString(const char* val, size_t size){

   // PX4_INFO("boundedBuffer: put String is called() %s",val);

    pthread_mutex_lock(&bufferlock);
   // PX4_INFO("putString: hasLock");

    while(this->full()){
       // PX4_INFO("boundedBuffer: isFull");

        pthread_cond_wait(&isFull,&bufferlock);
    }
    //myBuffer[head] = (char*)malloc(size*sizeof(char));
   while(myBuffer[head]== NULL){
       PX4_INFO("ERROR PUT STRING couldn't get space from malloc");
       sleep(2);

   }
   if(size > 62){
       PX4_INFO("Bounded Buffer: Not enough space!");
   }

    memcpy(myBuffer[head],val,size);
  //  PX4_INFO("boundedBuffer: saved String is %s",myBuffer[head]);

    mySize[head] = size;
    numElements++;
    head = next(head);

    pthread_cond_signal(&isEmpty);
  //  PX4_INFO("putString: leaveLock");

    pthread_mutex_unlock(&bufferlock);



    return true;
}




