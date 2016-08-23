/******************************************************************************/
/** @file	ring buffer
    @date	2015-02-12 DTZ
    @author	2015-02-08 DTZ
*******************************************************************************/

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include "midi.h"

typedef struct{
	uint32_t    size;
	uint32_t    writePos;
	uint32_t    readPos;
	MIDI_MSG_T* data;
} RB_BUF_T;

void     RB_Init( RB_BUF_T* buffer, uint32_t    bufferSizeInPowerOfTwo);
void     RB_Write(RB_BUF_T* buffer, MIDI_MSG_T  data);
uint32_t RB_Read( RB_BUF_T* buffer, MIDI_MSG_T* dataPtr);

#endif
