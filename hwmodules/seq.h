/******************************************************************************/
/** @date	2015-02-14
    @author	2015-02-13

	R: 0xD  0b1101
	G: 0xB	0b1011
	B: 0x7	0b0111

	The color LEDs have a common anode, so a 0 means on.
			___
	BGR-	BGR -	COLOR		HEX

	0000	111 1	WHITE		0
	0001	111 0	WHITE		1	x
	0010	110 1	CYAN		2
	0011	110 0	CYAN		3	x
	0100	101 1	MAGENTA		4
	0101	101 0	MAGENTA		5	x
	0110	100 1	BLUE		6
	0111	100 0	BLUE		7	x
	1000	011 1	YELLOW		8
	1001	011 0	YELLOW		9	x
	1010	010 1	GREEN		A
	1011	010 0	GREEN		B	x
	1100	001 1	RED			C
	1101	001 0	RED			D	x
	1110	000 1	OFF			E
	1111	000 0	OFF			F	x

	LED UI Table
	01 02 ... 15 16
	17 18 ... 31 32
	33 34 ... 47 48
	49 50 ... 63 64


*******************************************************************************/
#ifndef SEQ_H
#define SEQ_H

#include <stdint.h>
#include "../modules/midi.h"

#define RGB_COLOR_NO		0b1110
#define RGB_COLOR_RED		0b1101
#define RGB_COLOR_GREEN		0b1011
#define RGB_COLOR_BLUE		0b0111


void 	 SEQ_Init(void);
void	 SEQ_Reset(void);

void     SEQ_Buttons_EspiPull(void);
void     SEQ_Buttons_EspiPull_NonBlocking(void);
void     SEQ_Buttons_Process(void);
uint32_t SEQ_Buttons_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage);

void     SEQ_Leds_WriteMsgToRingBuffer(MIDI_MSG_T midiMessage);
void     SEQ_Leds_Process(void);
void     SEQ_Leds_EspiSend(void);
void     SEQ_Leds_EspiSend_NonBlocking(void);

#endif
