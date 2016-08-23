/******************************************************************************/
/** @date	2015-02-17 DTZ
    @author	2015-02-17 DTZ
*******************************************************************************/
#ifndef SYS_H
#define SYS_H

#include <stdint.h>
#include "midi.h"

void	 SYS_Init(char date[16], char time[16]);
void 	 SYS_WriteMsgToModule( MIDI_MSG_T  midiMessage);
uint32_t SYS_ReadMsgFromModule(MIDI_MSG_T* midiMessage);
void 	 SYS_Process(void);



#endif
