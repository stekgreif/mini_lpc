#ifndef APA_H
#define APA_H

#include <stdint.h>
#include "../modules/midi.h"
#include "../modules/ring_buffer.h"

void     APA_Init(void);
void     APA_Pedals_Process(void);
uint16_t APA_Pedals_GetValue(uint8_t id);

void     APA_Attenuator_Process(void);
void     APA_Attenuator_EspiSend(void);
void     APA_Attenuator_WriteMsgToRingBuffer(MIDI_MSG_T midiMessage);


#if 0
void APA_SetAudioVolume(uint8_t ch, uint8_t val);
void ESPI_Attenuator_Init(void);
void ESPI_Attenuator_Channel_Set(uint8_t ch, uint8_t val);
uint32_t ESPI_Attenuator_Poll(uint8_t ch);
#endif


#endif
