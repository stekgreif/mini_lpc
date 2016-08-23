/******************************************************************************/
/** @file	ring buffer
    @date	2015-02-13 DTZ
    @author	2015-02-08 DTZ
*******************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include "ring_buffer.h"
#include "midi.h"



/******************************************************************************/
/** @example	RB_BUF_T buffer;
				RB_Init(&buffer, 32);
				buffer size should be 2,4,8,16,32,...
*******************************************************************************/
void RB_Init(RB_BUF_T* buffer, uint32_t bufferSizeInPowerOfTwo)
{
	buffer->size = bufferSizeInPowerOfTwo;
	buffer->writePos = 0;
	buffer->readPos  = 0;
	buffer->data = malloc( buffer->size * sizeof(MIDI_MSG_T) );
}



/******************************************************************************/
/** @example	RB_Write(&buffer, midiMessage);
	@todo		change MIDI_MSG_T data to MIDI_MSG_T* data?!
*******************************************************************************/
void RB_Write(RB_BUF_T* buffer, MIDI_MSG_T data)
{
	uint8_t next = ((buffer->writePos + 1) & (buffer->size - 1));

	if (buffer->readPos == next)
	{
		/* delete oldest element */
		buffer->readPos = (buffer->readPos + 1) & (buffer->size - 1);
	}

	buffer->data[buffer->writePos] = data;
	buffer->writePos = next;

	// allet ok
}




/******************************************************************************/
/** @example	RB_Read(&buffer, &data);
	@return		0: nothing returned
				1: one message returned
*******************************************************************************/
uint32_t RB_Read(RB_BUF_T* buffer, MIDI_MSG_T* dataPtr)
{
	/* buffer empty -> nothing to read */
	if (buffer->readPos == buffer->writePos)
	{
		return 0;
	}

	/* data to return */
	else
	{
		*dataPtr = buffer->data[buffer->readPos];
		buffer->readPos = (buffer->readPos + 1) & (buffer->size - 1);
	}

	return 1;
}
