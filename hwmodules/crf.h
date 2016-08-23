/******************************************************************************/
/** @date	2015-02-19
    @author	2014 DTZ
*******************************************************************************/
#ifndef CRF_H
#define CRF_H

#include <stdint.h>
#include "../modules/midi.h"

void 	 CRF_Init(void);
void     CRF_Reset(void);

void     CRF_Buttons_EspiPull(void);
void     CRF_Buttons_EspiPull_NonSwitching(void);
void     CRF_Buttons_Process(void);
uint32_t CRF_Buttons_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage);

void     CRF_Leds_WriteMsgToRingBuffer(MIDI_MSG_T midiMessage);
void     CRF_Leds_Process(void);
void     CRF_Leds_EspiSend(void);

void	 CRF_Crossfader_EspiPull(void);
void     CRF_Crossfader_EspiPull_NonSwitching(void);
void 	 CRF_Crossfader_Process(void);
uint32_t CRF_Crossfader_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage);

#endif
