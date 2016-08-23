/******************************************************************************/
/** @date	2015-02-14
    @author	2014 DTZ
*******************************************************************************/
#ifndef FNR_H
#define FNR_H

#include <stdint.h>
#include "../modules/midi.h"

void     FNR_Init(void);
void     FNR_Reset(void);

void     FNR_Buttons_EspiPull(void);
void     FNR_Buttons_EspiPull_NonSwitching(void);
void     FNR_Buttons_Process(void);
uint32_t FNR_Buttons_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage);

void     FNR_Leds_WriteMsgToRingBuffer(MIDI_MSG_T midiMessage);
void     FNR_Leds_Process(void);
void     FNR_Leds_EspiSend(void);

#endif
