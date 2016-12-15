/*
 * LongBuffer.cpp
 *
 *  Created on: 11.12.2016
 *      Author: michaellehmann
 */


#include "tobyDataPipe.h"

enum{
	BUFLENGTH = 12,
};

//dirty initialisation
static pthread_mutex_t dataPipeLock = PTHREAD_MUTEX_INITIALIZER;


TobyDataPipe::TobyDataPipe(int inBuflength) : buflength(inBuflength), head(0),tail(0),space(buflength){
    myBuffer = (char*)malloc(buflength*sizeof(char));

    pthread_cond_init(&pipeIsFull,NULL);
    pthread_cond_init(&pipeIsEmpty,NULL);

}

TobyDataPipe::~TobyDataPipe() {
	// TODO Auto-generated destructor stub
}

int TobyDataPipe::getItem(char* buffer, int buflen){
  //  pthread_mutex_lock(&dataPipeLock);


    int localSpace = space; // save first
    if(localSpace == buflength){
		return 0;
	}

	int numToCpy = 0;
    if((buflength - localSpace) <= buflen){
        numToCpy = buflength -  localSpace;
	}
	else{
		numToCpy = buflen;
	}

	int delta = buflength - tail;
	if(delta >= numToCpy){
		memcpy(buffer,&myBuffer[tail],numToCpy);
		tail += numToCpy;
	}
	else{
		memcpy(buffer,&myBuffer[tail],delta);
		memcpy(&buffer[delta],&myBuffer[0],numToCpy - (delta));
		tail = numToCpy - (delta);
	}

   // space += numToCpy;
    updateSpace(numToCpy);

  //  pthread_mutex_unlock(&dataPipeLock);
	return numToCpy;
}

int TobyDataPipe::putItem(const char* val, size_t size){

    int localSpace = space; // save first

    if(localSpace < size){
		return 0;
	}

	int numToCpy;
    if(size >= localSpace){
        numToCpy = localSpace;
	}
	else{
		numToCpy = size;
	}

	int delta = buflength - head;
	if(delta >= numToCpy){
		memcpy(&myBuffer[head], val,numToCpy);
		head += numToCpy;
	}
	else{
		memcpy(&myBuffer[head], val,delta);
		memcpy(&myBuffer[0], &val[delta],numToCpy - (delta));
		head = numToCpy - (delta);
	}
    updateSpace(-numToCpy);
	return numToCpy;
}

void TobyDataPipe::updateSpace(int update){
    pthread_mutex_lock(&dataPipeLock);
    space += update;
    pthread_mutex_unlock(&dataPipeLock);
}



