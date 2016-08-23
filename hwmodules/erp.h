/******************************************************************************/
/** @date	2015-02-21 DTZ
    @author	2014 DTZ
*******************************************************************************/
#ifndef ERP_H
#define ERP_H

#include <stdint.h>

#include "../modules/midi.h"
#include "../modules/ring_buffer.h"

void ERP_Init(void);

void	 ERP_Erps_EspiPull(void);
void     ERP_Erps_EspiPull_NonSwitching(void);
void     ERP_Erps_Espi_SwitchToMode(void);
void 	 ERP_Erps_Process(void);
uint32_t ERP_Erps_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage);

#endif
