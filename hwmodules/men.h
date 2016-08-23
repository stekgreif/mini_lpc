/******************************************************************************/
/** @date	2015-02-21 DTZ
    @author	2014 DTZ
*******************************************************************************/

#ifndef MEN_H
#define MEN_H

#include <stdint.h>
#include "../modules/midi.h"

void	 MEN_Init(void);
void	 MEN_Reset(void);

void     MEN_ButtonsAndEnc_EspiPull(void);
void     MEN_ButtonsAndEnc_EspiPull_NonSwitching(void);

void     MEN_Buttons_Process(void);
uint32_t MEN_Buttons_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage);

void	 MEN_Encoder_Process(void);
uint32_t MEN_Encoder_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage);

void     MEN_Leds_WriteMsgToRingBuffer(MIDI_MSG_T midiMessage);
void     MEN_Leds_Process(void);
void     MEN_Leds_EspiSend(void);

#endif
