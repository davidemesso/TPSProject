/*
 * circular_buffer.h
 *
 *  Created on: 17/ago/2015
 *      Author: Paolo Santinelli
 */

#ifndef CIRCULAR_BUFFER_H_
#define CIRCULAR_BUFFER_H_
//#include "serial.h"

typedef struct {                       /* Circular buffer struct */
    int                 size;   /* maximum number of elements           */
    int                 w_pos;  /* write index                          */
    int                 r_pos;  /* read index				*/
    int                 count;  /* number of items in the buffer        */
    unsigned char       *buff;  /* vector of elements                   */
} CircularBuffer;

void cbInit(CircularBuffer *cb, int size);
void cbFree(CircularBuffer *cb);
int cbIsFull(CircularBuffer *cb);
int cbIsEmpty(CircularBuffer *cb);
int cbWrite(CircularBuffer *cb, unsigned char elem);
int cbRead(CircularBuffer *cb, unsigned char *elem);

#endif /* CIRCULAR_BUFFER_H_ */
