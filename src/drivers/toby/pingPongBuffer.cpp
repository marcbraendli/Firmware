#include "pingPongBuffer.h"
#include <stdlib.h>
#include <px4_log.h>





static pthread_mutex_t pingPongBufferlock = PTHREAD_MUTEX_INITIALIZER;


PingPongBuffer::PingPongBuffer() {
    // TODO Auto-generated constructor stub


    head = 0;

    bufferList[0]  = (char*)malloc(AbsolutBufferLength*sizeof(char));
    bufferList[1]  = (char*)malloc(AbsolutBufferLength*sizeof(char));

    if(bufferList[0] == NULL){
        PX4_INFO("Error, malloc funktioniert nicht");

    }
    if(bufferList[1] == NULL){
        PX4_INFO("Error, malloc funktioniert nicht");

    }

    bufferListIndex = 0; // wir beginnen mit buffer 0
    actualWriteBuffer = bufferList[bufferListIndex]; //aktueller zu schreibender Buffer festlegen
    actualReadBuffer = NULL;	// es gibt nichts zu lesen

    pthread_cond_init(&isFull,NULL);
    pthread_cond_init(&isEmpty,NULL);

}

PingPongBuffer::~PingPongBuffer() {
    // TODO Auto-generated destructor stub
}

size_t PingPongBuffer::PutData(const char* val, size_t size){

    pthread_mutex_lock(&pingPongBufferlock);

    if(size >= AbsolutBufferLength - head) {		// wir haben genau Platz im buffer oder benötigen mehr platz
        int ersterTeil = AbsolutBufferLength - head; // den bestehenden Buffer fertig auffüllen
        memcpy(actualWriteBuffer + head,val,ersterTeil);
        head+=ersterTeil;

        while(DataAvaiable()){
            pthread_cond_wait(&isFull,&pingPongBufferlock);

        }

        PX4_INFO("BUFFER : save data into new empty buffer");

        // ab hier buffer tauschen, sofern bereits gelesen

            int zweiterTeil = size - ersterTeil;		// den überschuss berechnen

            //kritische Zone, actualReadBuffer kann
            //pthread_mutex_lock(&pingPongBufferlock);
            actualReadBuffer = actualWriteBuffer;

            changeBufferListIndex();
            head = 0;

            actualWriteBuffer = bufferList[bufferListIndex]; //wir switchen den Buffer
            memcpy(actualWriteBuffer + head,val+zweiterTeil-1,zweiterTeil);
            head = zweiterTeil;
            pthread_mutex_unlock(&pingPongBufferlock);

            return size;

    }

    // normal Buffer füllen, wir haben platz im alten Buffer
    // adresse des elementes, auf welche der Head zeigt
    memcpy(actualWriteBuffer + head ,val,size);

    head += size;
    pthread_mutex_unlock(&pingPongBufferlock);

    return size;

}


int PingPongBuffer::GetData(char *val, size_t size){


    // DON'T work , but whyyyyyyyyyy?!
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

    return_value = (actualReadBuffer != 0);



    return return_value;
}


char* PingPongBuffer::getActualReadBuffer(){
    return actualReadBuffer;
}

