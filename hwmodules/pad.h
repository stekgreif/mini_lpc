/******************************************************************************/
/** @date	2015-02-21 DTZ
    @author	2014 DTZ
*******************************************************************************/
#ifndef PAD_H
#define PAD_H

#include <stdint.h>

#include <stdint.h>
#include "../modules/midi.h"

void 	 PAD_Init(void);

void     PAD_Pads_Espi_SwitchToMode(void);
void	 PAD_Pads_EspiPull(void);
void     PAD_Pads_EspiPull_NonSwitching(void);

void 	 PAD_Pads_Process(void);
uint32_t PAD_Pads_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage);
#endif
