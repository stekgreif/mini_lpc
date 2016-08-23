/******************************************************************************/
/** @date	2015-02-18 DTZ
    @author	2015-02-17 DTZ
    @todo	add a simplified version?
    @todo	add a variable for number of buttons
*******************************************************************************/
#ifndef BUTTON_DEBOUNCE_H
#define BUTTON_DEBOUNCE_H

#include <stdint.h>
#include "ring_buffer.h"

/* for 8 buttons */
typedef struct {
	uint32_t  debounceTimeInSysTicks;
	uint8_t*  stateMachineState;
	uint32_t* debounceCounterValue;
	uint8_t*  buttonIdMap;
	uint8_t   midiChannel;
} BTN_DEBOUNCE_T;

void DBNC_Init(				BTN_DEBOUNCE_T* btnDbc,
							uint32_t 		debounceTimeInSysTicks,
							uint8_t*		buttonIdMap,
							uint8_t 		midiChannel);

void DBNC_CheckForChanges(	BTN_DEBOUNCE_T* btnDbc,
							RB_BUF_T* 		btnRingBuf,
							uint8_t 		buttonByte);


#endif
