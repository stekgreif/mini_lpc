/******************************************************************************/
/** @brief	This module can be called via midi to get system information like:
			- version (date-time)
	@date	2015-02-17 DTZ
    @author	2015-02-17 DTZ
    @info	formats:  01234567890
    		__DATE__: Feb 17 2015
    		__TIME__: 21:06:17

    		all Get Functions return a two digit value

    		version needs 6 midi messages:
    		id	value
    		=========
    		0	year
    		1	month
    		2	day
    		3	hour
    		4	minute
    		5	second
*******************************************************************************/
#include <stdint.h>
#include "midi.h"
#include "ring_buffer.h"
#include "../hwmodules/seq.h"
#include "../hwmodules/fnl.h"
#include "../hwmodules/fnr.h"
#include "../hwmodules/men.h"
#include "../hwmodules/crf.h"

#define SYS_SEND_HW_VERSION   0
#define SYS_SEND_HW_NAME	  1
#define SYS_HW_NAME_LENGTH	 12
#define SYS_RESET			127

static char sysDate[16];
static char sysTime[16];
static char hwName[SYS_HW_NAME_LENGTH] = {'M','O','S','A','I','K',' ','M','I','N','i','\0'};

static RB_BUF_T   moduleInputBuffer;
static RB_BUF_T   moduleOutputBuffer;

static MIDI_MSG_T midiInputMsg;

static MIDI_MSG_T sysMidiMsg = {
	.usb    = MIDI_CONTROL_CHANGE >> 4,
	.status = MIDI_CONTROL_CHANGE | SYS_MIDI_CH
};

static void WriteHwNameToRingbuffer(void);


/******************************************************************************/
/** prototypes
*******************************************************************************/
static uint8_t GetYear(void);
static uint8_t GetMonth(void);
static uint8_t GetDay(void);
static uint8_t GetHours(void);
static uint8_t GetMinutes(void);
static uint8_t GetSeconds(void);



/******************************************************************************/
/** public functions
*******************************************************************************/
void SYS_Init(char date[16], char time[16])
{
	uint8_t cnt;
	for(cnt = 0; cnt < 16; cnt++)
	{
		sysDate[cnt] = date[cnt];
		sysTime[cnt] = time[cnt];
	}

	RB_Init(&moduleInputBuffer,  32);
	RB_Init(&moduleOutputBuffer, 64);
}



void WriteHwVersionToRingbuffer(void)
{
	sysMidiMsg.id = 0;
	sysMidiMsg.value = GetYear();
	RB_Write(&moduleOutputBuffer, sysMidiMsg);

	sysMidiMsg.id = 1;
	sysMidiMsg.value = GetMonth();
	RB_Write(&moduleOutputBuffer, sysMidiMsg);

	sysMidiMsg.id = 2;
	sysMidiMsg.value = GetDay();
	RB_Write(&moduleOutputBuffer, sysMidiMsg);

	sysMidiMsg.id = 3;
	sysMidiMsg.value = GetHours();
	RB_Write(&moduleOutputBuffer, sysMidiMsg);

	sysMidiMsg.id = 4;
	sysMidiMsg.value = GetMinutes();
	RB_Write(&moduleOutputBuffer, sysMidiMsg);

	sysMidiMsg.id = 5;
	sysMidiMsg.value = GetSeconds();
	RB_Write(&moduleOutputBuffer, sysMidiMsg);
}



void SYS_Process(void)
{
	if (RB_Read(&moduleInputBuffer, &midiInputMsg))
	{
		switch (midiInputMsg.id)
		{
			case SYS_SEND_HW_VERSION:
			{
				WriteHwVersionToRingbuffer();
				break;
			}
			case SYS_SEND_HW_NAME:
			{
				WriteHwNameToRingbuffer();
				break;
			}
			case SYS_RESET:
			{
				MIDI_MSG_T midiMsg;
				midiMsg.usb = 0;
				midiMsg.status = 0;
				midiMsg.id = 127;
				midiMsg.value = 0;

				FNL_Leds_WriteMsgToRingBuffer(midiMsg);
				FNR_Leds_WriteMsgToRingBuffer(midiMsg);
				MEN_Leds_WriteMsgToRingBuffer(midiMsg);
				CRF_Leds_WriteMsgToRingBuffer(midiMsg);

				midiMsg.value = 0xFF;
				SEQ_Leds_WriteMsgToRingBuffer(midiMsg);
				break;
			}
			default:
				break;
		}
	}
}


void SYS_WriteMsgToModule(MIDI_MSG_T midiMessage)
{
	RB_Write(&moduleInputBuffer, midiMessage);
}



uint32_t SYS_ReadMsgFromModule(MIDI_MSG_T* midiMessage)
{
	return RB_Read(&moduleOutputBuffer, midiMessage);
}



/******************************************************************************/
/** private functions
*******************************************************************************/
static uint8_t GetYear(void)
{
	uint8_t tens = (((uint8_t) sysDate[ 9]) - 48) * 10;
	uint8_t ones =  ((uint8_t) sysDate[10]) - 48;
	return tens + ones;
}

static uint8_t GetMonth(void)
{
	if     (sysDate[0] == 'J' && sysDate[1] == 'a' && sysDate[2] == 'n')	return 1;
	else if(sysDate[0] == 'F' && sysDate[1] == 'e' && sysDate[2] == 'b')	return 2;
	else if(sysDate[0] == 'M' && sysDate[1] == 'a' && sysDate[2] == 'b')	return 3;
	else if(sysDate[0] == 'A' && sysDate[1] == 'p' && sysDate[2] == 'r')	return 4;
	else if(sysDate[0] == 'M' && sysDate[1] == 'p' && sysDate[2] == 'r')	return 5;
	else if(sysDate[0] == 'J' && sysDate[1] == 'p' && sysDate[2] == 'r')	return 6;
	else if(sysDate[0] == 'J' && sysDate[1] == 'p' && sysDate[2] == 'r')	return 7;
	else if(sysDate[0] == 'A' && sysDate[1] == 'p' && sysDate[2] == 'r')	return 8;
	else if(sysDate[0] == 'S' && sysDate[1] == 'p' && sysDate[2] == 'r')	return 9;
	else if(sysDate[0] == 'O' && sysDate[1] == 'p' && sysDate[2] == 'r')	return 10;
	else if(sysDate[0] == 'N' && sysDate[1] == 'p' && sysDate[2] == 'r')	return 11;
	else if(sysDate[0] == 'D' && sysDate[1] == 'p' && sysDate[2] == 'r')	return 12;
	else														            return 99;
}


static uint8_t GetDay(void)
{
	uint8_t tens = (((uint8_t) sysDate[4]) - 48) * 10;
	uint8_t ones =  ((uint8_t) sysDate[5]) - 48;
	return tens + ones;
}

static uint8_t GetHours(void)
{
	uint8_t tens = (((uint8_t) sysTime[0]) - 48) * 10;
	uint8_t ones =  ((uint8_t) sysTime[1]) - 48;
	return tens + ones;
}

static uint8_t GetMinutes(void)
{
	uint8_t tens = (((uint8_t) sysTime[3]) - 48) * 10;
	uint8_t ones =  ((uint8_t) sysTime[4]) - 48;
	return tens + ones;
}

static uint8_t GetSeconds(void)
{
	uint8_t tens = (((uint8_t) sysTime[6]) - 48) * 10;
	uint8_t ones =  ((uint8_t) sysTime[7]) - 48;
	return tens + ones;
}



static void WriteHwNameToRingbuffer(void)
{
	uint8_t cnt;

	sysMidiMsg.id = 10;

	for(cnt = 0; cnt < SYS_HW_NAME_LENGTH; cnt++)
	{
		sysMidiMsg.value = (uint8_t) hwName[cnt];
		RB_Write(&moduleOutputBuffer, sysMidiMsg);
	}
}
