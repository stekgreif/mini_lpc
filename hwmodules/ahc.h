#ifndef AHC_H
#define AHC_H

#include <stdint.h>
#include "../modules/midi.h"

void     AHC_Init(void);

void     AHC_Potis_EspiPullData(void);
void     AHC_Potis_Process(void);
uint32_t AHC_Adc_ReadMsgFromRingbuffer(MIDI_MSG_T* midiMessage);

void     AHC_Switches_EspiPullData(void);
void     AHC_Switches_EspiPullData_NonSwitching(void);
void     AHC_Switches_Process(void);
uint32_t AHC_Switches_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage);

#endif

