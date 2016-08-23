/******************************************************************************/
/** @date	2015-02-19 DTZ
    @author	2014 DTZ
    @info
			LEDs/BTNs
			0    1
			2    3
			4    5
			6    7
			8 	 9
			10  11
			12	13
			14	15

			Button Bytes	Button IDs
			buttons.val[0]:  8| 9|10|11|12|13|14|15|
			buttons.val[1]:  0| 1| 2| 3| 4| 5| 6| 7|
*******************************************************************************/
#include "stdint.h"
#include "fnr.h"

#include "../modules/midi.h"
#include "../modules/ring_buffer.h"
#include "../modules/button_debounce.h"

#include "drv/nl_espi_core.h"
#include "drv/nl_espi_shift_io.h"

#define FNR_NUM_OUT_REGS 	4
#define FNR_NUM_IN_REGS 	2
#define FNR_PORT			7
#define FNR_DEVICE_LEDS		1
#define FNR_DEVICE_BTNS		2

static ESPI_IO_T leds;
static ESPI_IO_T buttons;

static RB_BUF_T ledRingBuffer;
static RB_BUF_T buttonRingBuffer;

static BTN_DEBOUNCE_T btnDebounceByte0;
static BTN_DEBOUNCE_T btnDebounceByte1;

static uint8_t buttonIds0[8] = { 0, 1, 2, 3, 4, 5, 6, 7};
static uint8_t buttonIds1[8] = { 8, 9,10,11,12,13,14,15};

static void Leds_SetSingle(uint8_t ledId);
static void Leds_ClearSingle(uint8_t ledId);
static void Leds_ClearAll(void);
static void Leds_SetAll(void);

/******************************************************************************/
void FNR_Init(void)
{
	ESPI_Shift_IO_Init(&leds,
						ESPI_SHIFT_OUT,
						FNR_NUM_OUT_REGS,
						FNR_PORT,
						FNR_DEVICE_LEDS);
	ESPI_Shift_IO_Init(&buttons,
						ESPI_SHIFT_IN,
						FNR_NUM_IN_REGS,
						FNR_PORT,
						FNR_DEVICE_BTNS);

	RB_Init(  &ledRingBuffer, 64);
	RB_Init(  &buttonRingBuffer, 64);

	DBNC_Init(&btnDebounceByte0, 1, buttonIds0, FNR_MIDI_CH);
	DBNC_Init(&btnDebounceByte1, 1, buttonIds1, FNR_MIDI_CH);
}



void FNR_Reset(void)
{
	Leds_ClearAll();
}



/******************************************************************************/
void FNR_Buttons_EspiPull(void)
{
	SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_1 | ESPI_CPHA_1);
	ESPI_ShiftIn_Poll_Blocking(&buttons);
}

void FNR_Buttons_EspiPull_NonSwitching(void)
{
	ESPI_ShiftIn_Poll_Blocking(&buttons);
}



void FNR_Buttons_Process(void)
{
	DBNC_CheckForChanges(&btnDebounceByte0, &buttonRingBuffer, buttons.val[0]);
	DBNC_CheckForChanges(&btnDebounceByte1, &buttonRingBuffer, buttons.val[1]);
}



uint32_t FNR_Buttons_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage)
{
	return RB_Read(&buttonRingBuffer, midiMessage);
}



/******************************************************************************/
void FNR_Leds_WriteMsgToRingBuffer(MIDI_MSG_T midiMessage)
{
	RB_Write(&ledRingBuffer, midiMessage);
}



void FNR_Leds_Process(void)
{
	MIDI_MSG_T ledTemp;

	while (RB_Read(&ledRingBuffer, &ledTemp) != 0)
	{
		if (ledTemp.id != 127)
		{
			if (ledTemp.value > 0)
				Leds_SetSingle(ledTemp.id);
			else
				Leds_ClearSingle(ledTemp.id);
		}
		/* set/clear all leds */
		else
	    {
			if (ledTemp.value > 0)
				Leds_SetAll();
			else
				Leds_ClearAll();
	    }
	}
}



void FNR_Leds_EspiSend(void)
{
	SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_0 | ESPI_CPHA_0);
	ESPI_ShiftOut_Poll_Blocking(&leds);
}



/******************************************************************************/
static void Leds_ClearAll(void)
{
	uint8_t cnt;
	for (cnt = 0; cnt < FNR_NUM_OUT_REGS; cnt++)
		leds.val[cnt] = 0x00;
}

static void Leds_SetAll(void)
{
	uint8_t cnt;
	for (cnt = 0; cnt < FNR_NUM_OUT_REGS; cnt++)
		leds.val[cnt] = 0xFF;
}



static void Leds_SetSingle(uint8_t ledId)
{
	switch (ledId)
	{
	    case  0: leds.val[3] |= 0b11 << 0; break;
	    case  1: leds.val[3] |= 0b11 << 2; break;
	    case  2: leds.val[3] |= 0b11 << 4; break;
	    case  3: leds.val[3] |= 0b11 << 6; break;

	    case  4: leds.val[2] |= 0b11 << 0; break;
		case  5: leds.val[2] |= 0b11 << 2; break;
		case  6: leds.val[2] |= 0b11 << 4; break;
		case  7: leds.val[2] |= 0b11 << 6; break;

	    case  8: leds.val[1] |= 0b11 << 0; break;
		case  9: leds.val[1] |= 0b11 << 2; break;
		case 10: leds.val[1] |= 0b11 << 4; break;
		case 11: leds.val[1] |= 0b11 << 6; break;

	    case 12: leds.val[0] |= 0b11 << 0; break;
		case 13: leds.val[0] |= 0b11 << 2; break;
		case 14: leds.val[0] |= 0b11 << 4; break;
		case 15: leds.val[0] |= 0b11 << 6; break;

		default: break;
	}
}


# if 0 // not used at the moment
static void Leds_SetSingleAndClearOthers(uint8_t ledId)
{
	Leds_ClearAll();
	Leds_SetSingle(ledId);
}



static void Leds_SetAll(void)
{
	uint8_t cnt;
	for (cnt = 0; cnt < FNR_NUM_OUT_REGS; cnt++)
	{
		leds.val[cnt] = 0xFF;
	}
}
#endif


static void Leds_ClearSingle(uint8_t ledId)
{
	switch (ledId)
	{
	    case  0: leds.val[3] &= ~(0b11 << 0); break;
	    case  1: leds.val[3] &= ~(0b11 << 2); break;
	    case  2: leds.val[3] &= ~(0b11 << 4); break;
	    case  3: leds.val[3] &= ~(0b11 << 6); break;

	    case  4: leds.val[2] &= ~(0b11 << 0); break;
		case  5: leds.val[2] &= ~(0b11 << 2); break;
		case  6: leds.val[2] &= ~(0b11 << 4); break;
		case  7: leds.val[2] &= ~(0b11 << 6); break;

		case  8: leds.val[1] &= ~(0b11 << 0); break;
		case  9: leds.val[1] &= ~(0b11 << 2); break;
		case 10: leds.val[1] &= ~(0b11 << 4); break;
		case 11: leds.val[1] &= ~(0b11 << 6); break;

		case 12: leds.val[0] &= ~(0b11 << 0); break;
		case 13: leds.val[0] &= ~(0b11 << 2); break;
		case 14: leds.val[0] &= ~(0b11 << 4); break;
		case 15: leds.val[0] &= ~(0b11 << 6); break;

		default: break;
	}
}


