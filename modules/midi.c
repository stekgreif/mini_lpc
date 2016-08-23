/******************************************************************************/
/** @date	2015-02-17 DTZ
    @author	2015-02-17 DTZ
*******************************************************************************/

#include <stdint.h>
#include "midi.h"

#if 1 // has to be tested
const MIDI_MSG_T midiDummyMessage = {
	.usb 	= 0x0A,
	.status	= 0xA0,
	.id		= 0x11,
	.value	= 0xAA
};
#endif
