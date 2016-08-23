/******************************************************************************/
/** @date	2015-02-17 DTZ
    @author	2015-02-15 DTZ
*******************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include "button_debounce.h"
#include "ring_buffer.h"
#include "midi.h"


#define DBC_STATE_CHECK_IF_BUTTON_HIGH  0
#define DBC_STATE_DEBOUNCE_BUTTON_HIGH	1
#define DBC_STATE_CHECK_IF_BUTTON_LOW	2
#define DBC_STATE_DEBOUNCE_BUTTON_LOW  	3


static MIDI_MSG_T midiMsgButtonPressed = {
	.value 	= 127
};

static MIDI_MSG_T midiMsgButtonReleased = {
	.value 	= 0
};



void DBNC_Init(	BTN_DEBOUNCE_T* btnDbc,
				uint32_t 		debounceTimeInSysTicks,
				uint8_t*		buttonIdMap,
				uint8_t			midiChannel)
{
	btnDbc->debounceTimeInSysTicks 	= debounceTimeInSysTicks;
	btnDbc->midiChannel 			= midiChannel;
	btnDbc->buttonIdMap 		 	= malloc( 8 * sizeof(uint8_t)  );
	btnDbc->stateMachineState 	 	= malloc( 8 * sizeof(uint8_t)  );
	btnDbc->debounceCounterValue 	= malloc( 8 * sizeof(uint32_t) );

	uint8_t cnt;
	for (cnt = 0; cnt < 8; cnt++)
	{
		btnDbc->stateMachineState[cnt] 		= 0;
		btnDbc->debounceCounterValue[cnt] 	= 0;
		btnDbc->buttonIdMap[cnt] = buttonIdMap[cnt];
	}
}


/******************************************************************************/
/**	@param[in]	dbncBtn - debounce struct
	@param[out] rbBtn	- ring buffer to wrtie data to
	@param[in]	espiBtn - one byte from espi data
*******************************************************************************/
void DBNC_CheckForChanges(BTN_DEBOUNCE_T* dbncBtn, RB_BUF_T* rbBtn, uint8_t espiBtn)
{
	uint8_t buttonInputValue;
	uint8_t bit;

	for(bit = 0; bit < 8; bit++)
	{
		buttonInputValue = ((~espiBtn) >> bit) & 0b00000001;

		switch (dbncBtn->stateMachineState[bit])
		{
			case DBC_STATE_CHECK_IF_BUTTON_HIGH:
			{
				if (buttonInputValue == 1)
				{
					dbncBtn->debounceCounterValue[bit] = dbncBtn->debounceTimeInSysTicks;
					dbncBtn->stateMachineState[bit]    = DBC_STATE_DEBOUNCE_BUTTON_HIGH;
					midiMsgButtonPressed.usb 	= MIDI_NOTE_ON >> 4;
					midiMsgButtonPressed.status = MIDI_NOTE_ON | dbncBtn->midiChannel;
					midiMsgButtonPressed.id 	= dbncBtn->buttonIdMap[bit];
					RB_Write(rbBtn, midiMsgButtonPressed);
				}
				break;
			}

			case DBC_STATE_DEBOUNCE_BUTTON_HIGH:
			{
				dbncBtn->debounceCounterValue[bit]--;
				if (dbncBtn->debounceCounterValue[bit] == 0)
				{
					dbncBtn->stateMachineState[bit] = DBC_STATE_CHECK_IF_BUTTON_LOW;
				}
				break;
			}

			case DBC_STATE_CHECK_IF_BUTTON_LOW:
			{
				if (buttonInputValue == 0)
				{
					dbncBtn->debounceCounterValue[bit] = dbncBtn->debounceTimeInSysTicks;
					dbncBtn->stateMachineState[bit] = DBC_STATE_DEBOUNCE_BUTTON_LOW;
					midiMsgButtonReleased.usb = MIDI_NOTE_OFF >> 4;
					midiMsgButtonReleased.status = MIDI_NOTE_OFF | dbncBtn->midiChannel;
					midiMsgButtonReleased.id = dbncBtn->buttonIdMap[bit];
					RB_Write(rbBtn, midiMsgButtonReleased);
				}
				break;
			}

			case DBC_STATE_DEBOUNCE_BUTTON_LOW:
			{
				dbncBtn->debounceCounterValue[bit]--;
				if (dbncBtn->debounceCounterValue[bit] == 0)
				{
					dbncBtn->stateMachineState[bit] = DBC_STATE_CHECK_IF_BUTTON_HIGH;
				}
				break;
			}
		}
	}
}
