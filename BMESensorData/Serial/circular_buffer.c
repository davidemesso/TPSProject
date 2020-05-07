/*
 * circular_buffer.c
 *
 *  Created on: 17/ago/2015
 *      Author: Paolo Santinelli
 */


//--------------------------------------------------------------------------------------
#include <stdlib.h>
#include "circular_buffer.h"
/* Circular buffer */
//typedef struct {                                        /* Circular buffer struct */
//    int                 size;   /* maximum number of elements         */
//    int                 w_pos;  /* write index                        */
//    int                 r_pos;  /* read index			        */
//    int                 count;  /* number of items in the buffer      */
//    unsigned char       *buff;  /* vector of elements                 */
//} CircularBuffer;

void cbInit(CircularBuffer *cb, int size) {             /* Inizialize circular buffer */
    cb->size  = size;
    cb->w_pos = 0;
    cb->r_pos = 0;
    cb->count = 0;
    cb->buff  = (unsigned char *)malloc(cb->size*sizeof(unsigned char));
}

void cbFree(CircularBuffer *cb) {                       /* Free circular buffer */
    free(cb->buff); /* OK if null */
}

int cbIsFull(CircularBuffer *cb) {                      /* Check circular buffer full condition */
    return cb->count == cb->size;
}

int cbIsEmpty(CircularBuffer *cb) {                     /* Check circular buffer empty condition */
    return cb->count == 0;
}

int cbWrite(CircularBuffer *cb, unsigned char elem) {   /* Write data in to circular buffer */

    if (cb->count < cb->size){
          cb->count ++;
          cb->buff[cb->w_pos] = elem;
          cb->w_pos = ++cb->w_pos < cb->size ? cb->w_pos: 0; // cb->w_pos = (cb->w_pos+1) % cb->size;
          return(0); // Done
    }
  return(1); // TX buffer full
}

int cbRead(CircularBuffer *cb, unsigned char *elem) {   /* Read data from circular buffer */
    if (cb->count > 0){
          cb->count --;
          *elem = cb->buff[cb->r_pos];
          cb->r_pos = ++cb->r_pos < cb->size ? cb->r_pos: 0; // cb->r_pos = (cb->r_pos+1) % cb->size;
          return(0); // Done
    }
  return(1); // RX buffer empty
}
//------------------------------------------------------------------------------------

