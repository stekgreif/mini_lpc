/******************************************************************************/
/** @date	2015-02-16 DTZ
    @author	2015-02-16 DTZ
    @info	copied from settings.h and mosaik_data_types.h

    Midi Messages:

    s: Status
    		c: USB MIDI Cable / channel
    		[cccc ssss]
	[Byte0] usb
    [Byte1]
    [Byte2]	id

    [Byte3] value

		1000  0x8  Note Off
		1001  0x9  Note On
		1010  0xA  Polyphonic Key Pressure (Aftertouch)
		1011  0xB  Control Change
		1100  0xC  Program Change
		1101  0xD  Channel Pressure (Aftertouch)
		1110  0xE  Pitch Bend Change


*******************************************************************************/
#ifndef MIDI_H
#define MIDI_H

#include <stdint.h>

typedef struct{
	uint8_t usb;
	uint8_t status;
	uint8_t id;
	uint8_t value;
} MIDI_MSG_T;

#define MIDI_TEST_BYTE_0	0x09	// CS
#define MIDI_TEST_BYTE_1	0x90	// SC
#define MIDI_TEST_BYTE_2	0x16
#define MIDI_TEST_BYTE_3	0x17

extern const MIDI_MSG_T midiDummyMessage;

#define MIDI_NOTE_OFF 		0x80
#define MIDI_NOTE_ON  		0x90
#define MIDI_CONTROL_CHANGE 0xB0

/* MIDI PORTs: 0..15 */
#define SEQ_MIDI_CH		0
#define FNL_MIDI_CH		1
#define FNR_MIDI_CH		2
#define MEN_MIDI_CH		3
#define ERP_MIDI_CH		4
#define PAD_MIDI_CH		5
#define CRF_MIDI_CH		6
#define AHO_MIDI_CH		7
#define AHC_MIDI_CH		8
#define APA_MIDI_CH		9
#define AIN_MIDI_CH	   10
#define SPR_MIDI_CH	   12
#define SYS_MIDI_CH	   15


#define MIDI_STATUS_CHANNEL_MASK  	0b00001111
#define MIDI_STATUS_MSG_TYPE_MASK  	0b11110000


#endif
