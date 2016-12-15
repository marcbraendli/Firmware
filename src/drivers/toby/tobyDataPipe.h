/*
 * LongBuffer.h
 *
 *  Created on: 11.12.2016
 *      Author: michaellehmann
 */
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>



#ifndef DATAPIPE_H
#define DATAPIPE_H

/**
 * @brief The TobyDataPipe class a flexible buffer designed for toby device, optimized use all allocated space
 */

class TobyDataPipe {
public:
    TobyDataPipe(int inBuflength);
    virtual ~TobyDataPipe();
	int getItem(char* buffer, int buflen);
	int putItem(const char* val, size_t size);
    // only one copy needed, not implemented yet;
	char* getItemPointer(char* buffer, int buflen);
    int getSpace(void){
        return space;
    }

	bool getItemSuccessful(void);
    bool isEmpty(void){
        return (space == buflength);
    }


private:

    /**
     * @brief updateSpace a thread safety function to update the parameter space
     * @param update the value which sould be added
     */
    inline void updateSpace(int update);
	volatile int buflength;
	int head;
	int tail;
	volatile int space;
	char* myBuffer;
    pthread_cond_t pipeIsFull;
    pthread_cond_t pipeIsEmpty;



};

#endif /* LONGBUFFER_H_ */
