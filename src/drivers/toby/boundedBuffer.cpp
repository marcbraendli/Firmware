#include "boundedBuffer.h"
#include "px4_log.h"


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
        myBuffer[i] = (char*)malloc(62*sizeof(char));
        if(myBuffer[i] == NULL){
            PX4_INFO("ERROR NO SPACE AVAIABLE");
        }

    }
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
      // myBuffer[head] = (char*)malloc(size*sizeof(char));
       sleep(2);

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


bool BoundedBuffer::putString2(const char* val, size_t size){

/*
    //char* temporärbuffer = (char*)malloc(0*sizeof(char));
    char** temporaryBuffer = NULL;

    int x = 1;
    memcpy(temporaryBuffer[head],val,x*8);





   // ***************************************

   //rest auffüllen
 //  memcpy(myBuffer[head],rest,restsize);


   int i = 0;
   while(size >= 8){

       memcpy(myBuffer[head],val+i*8,8);
       size = size - 8;

       //abfrage is Full fehlt hier noch

       //next element put in
       mySize[head] = size;
       numElements++;
       head = next(head);


   }

   if(size >= 0){
       //rest kopieren
       restsize = size%8;
       memcpy(rest,val + i*8,size%8);
   }



    pthread_cond_signal(&isEmpty);
  //  PX4_INFO("putString: leaveLock");

    pthread_mutex_unlock(&bufferlock);


*/
    return true;
}


