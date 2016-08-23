/******************************************************************************/
/** @date	2015-02-19 DTZ
    @author	2014 DTZ
*******************************************************************************/
#ifndef FNL_H
#define FNL_H

#include <stdint.h>

#include "../modules/ring_buffer.h"
#include "../modules/midi.h"



void     FNL_Init(void);
void     FNL_Reset(void);

void     FNL_Buttons_EspiPull(void);
void     FNL_Buttons_Process(void);
uint32_t FNL_Buttons_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage);

void     FNL_Leds_WriteMsgToRingBuffer(MIDI_MSG_T midiMessage);
void     FNL_Leds_Process(void);
void     FNL_Leds_EspiSend(void);

#endif
