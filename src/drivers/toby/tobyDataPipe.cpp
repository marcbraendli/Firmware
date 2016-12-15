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



DataPipe::DataPipe(int inBuflength) : buflength(inBuflength), head(0),tail(0),space(buflength){
    myBuffer = (char*)malloc(buflength*sizeof(char));

}

DataPipe::~DataPipe() {
	// TODO Auto-generated destructor stub
}

int DataPipe::getItem(char* buffer, int buflen){

	if(space == buflength){
		return 0;
	}

	int numToCpy = 0;
	if((buflength - space) <= buflen){
		numToCpy = buflength -  space;
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

	space += numToCpy;

	return numToCpy;
}

int DataPipe::putItem(const char* val, size_t size){

	//no space available
	if(space < size){
		return 0;
	}

	int numToCpy;
	if(size >= space){
		numToCpy = space;
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
	space -= numToCpy;

	return numToCpy;
}



