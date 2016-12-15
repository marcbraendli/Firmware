/*
 * LongBuffer.h
 *
 *  Created on: 11.12.2016
 *      Author: michaellehmann
 */
#include <stdlib.h>
#include <string.h>


#ifndef DATAPIPE_H
#define DATAPIPE_H



class DataPipe {
public:
    DataPipe(int inBuflength);
	virtual ~DataPipe();
	int getItem(char* buffer, int buflen);
	int putItem(const char* val, size_t size);
	// only one copy needed:
	char* getItemPointer(char* buffer, int buflen);
	bool getItemSuccessful(void);


private:
	volatile int buflength;
	int head;
	int tail;
	volatile int space;
	char* myBuffer;



};

#endif /* LONGBUFFER_H_ */
