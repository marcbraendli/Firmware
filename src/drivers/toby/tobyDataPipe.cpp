
#include "tobyDataPipe.h"
#include "px4_log.h"



TobyDataPipe::TobyDataPipe(int inBuflength) : buflength(inBuflength), head(0),tail(0),space(buflength), dataToDelete(0), lastIsDeleted(true){
    myBuffer = (char*)malloc(buflength*sizeof(char));
    pthread_mutex_init(&dataPipeLock,nullptr);

}

TobyDataPipe::~TobyDataPipe() {
    pthread_mutex_destroy(&dataPipeLock);
}

int TobyDataPipe::getItem(char* buffer, int buflen){

    int localSpace = space; // save first, read is atomic operation
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

	return numToCpy;
}

int TobyDataPipe::putItem(const char* val, size_t size){

    int localSpace = space; // save first, read is atomic operation

    /*
    if(localSpace < size){
		return 0;
	}
    */

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

    //space -= numToCpy;
    updateSpace(0 - numToCpy);
	return numToCpy;
}

bool TobyDataPipe::isEmpty(void){
    return (space == buflength);
}

int TobyDataPipe::getItemPointer(char* &val){
    if(!lastIsDeleted){
        PX4_INFO("FAIL getItemPointer");

        return 0;
    }

    val = &myBuffer[tail];
    int local_head = head; // atomic operation, we dont care if head is updated later
    if((local_head >= tail) & (space != 0)){
        dataToDelete = local_head - tail;

    }
    else{

        dataToDelete = buflength - tail;
    }
    lastIsDeleted = false;

    return dataToDelete;
}

bool TobyDataPipe::getItemSuccessful(void){
    if(lastIsDeleted){
        PX4_INFO("FAIL getItemSuccessful");
        return false;
    }

    tail += dataToDelete;
    if(tail == buflength){
        tail = 0;
    }
    updateSpace(dataToDelete);
    lastIsDeleted = true;
    PX4_INFO("deleted successful : %d ",dataToDelete);

    return true;
}



