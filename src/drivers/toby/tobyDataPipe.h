/**
 * @brief The TobyDataPipe class a flexible buffer designed for toby device, optimized use all allocated space
 * @file tobyDataPipe.h
 * @author Michael Lehmann
 *
 */

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#ifndef DATAPIPE_H
#define DATAPIPE_H


class TobyDataPipe {
public:
    /**
     * @brief TobyDataPipe
     * @param inBuflength the total space which is avaiable
     */
    TobyDataPipe(int inBuflength);
    virtual ~TobyDataPipe();
    /**
     * @brief getItem copies the data to destination pointer
     * @param buffer destination pointer where the data should be copied
     * @param buflen the maximum space at destination
     * @return number of total copied elements to destination
     */
	int getItem(char* buffer, int buflen);
    /**
     * @brief putItem copies the data into local buffer
     * @param val points to the data which should be copied into buffer
     * @param size the total number of data
     * @return the number of total copied elements into buffer
     */
	int putItem(const char* val, size_t size);
    /**
     * @brief getItemPointer a not thread safety operation (!), implemented for just copy once. After this method, you have to call getItemSuccessful()
     * @param[in] val the pointer which sould point to the data
     * @return return the number of data could be readed and are valid.
     */

    int getItemPointer(char* &val);

    int getSpace(void){
        return space;
    }

	bool getItemSuccessful(void);
    bool isEmpty(void);


private:

    volatile int buflength;
    volatile int head;
    volatile int tail;
	volatile int space;
	char* myBuffer;
    pthread_mutex_t dataPipeLock;
    int dataToDelete;
    bool lastIsDeleted;
    /**
     * @brief updateSpace a thread safe function to add a value to the parameter space
     * @param update the value which sould be added
     */

    //should be inline but keyword doesn't work
    void updateSpace(int update){
        pthread_mutex_lock(&dataPipeLock);
        space += update;
        pthread_mutex_unlock(&dataPipeLock);
    }



};

#endif
