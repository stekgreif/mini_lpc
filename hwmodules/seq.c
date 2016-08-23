/******************************************************************************/
/** @date	2015-04-02 DTZ
    @author	2015-02-13 DTZ

    	Button assignment

	byte[0] = 64 48 32 16 15 31 47 63
	byte[1] = 62 46 30 14 13 29 45 61
	byte[2] = 60 44 28 12 11 27 43 59
	byte[3] = 58 42 26 10 09 25 41 57
	byte[4] = 56 40 24 08 07 23 39 55
	byte[5] = 54 38 22 06 05 21 37 53
	byte[6] = 52 36 20 04 03 19 35 51
	byte[7] = 50 34 18 02 01 17 33 49
*******************************************************************************/
#include <stdint.h>
#include "seq.h"

#include "drv/nl_dbg.h"

#include "../modules/midi.h"
#include "../modules/ring_buffer.h"
#include "../modules/button_debounce.h"

#include "drv/nl_espi_core.h"
#include "drv/nl_espi_shift_io.h"

#define SEQ_NUM_OUT_REGS   32
#define SEQ_NUM_IN_REGS 	8
#define SEQ_PORT			1
#define SEQ_DEVICE_LEDS		1
#define SEQ_DEVICE_BTNS 	2

static ESPI_IO_T leds;
static ESPI_IO_T buttons;
static RB_BUF_T  ledRingBuffer;
static RB_BUF_T  buttonRingBuffer;

static void Leds_ClearAll(void);
static void Leds_SetSingleAndClearOthers(uint8_t ledId, uint8_t color);
static void Leds_SetSingle(uint8_t step, uint8_t color);

//[line][row]
static uint8_t btnIdMap0[8] = {48, 32, 16,  0,  1, 17, 33, 49};
static uint8_t btnIdMap1[8] = {50, 34, 18,  2,  3, 19, 35, 51};
static uint8_t btnIdMap2[8] = {52, 36, 20,  4,  5, 21, 37, 53};
static uint8_t btnIdMap3[8] = {54, 38, 22,  6,  7, 23, 39, 55};
static uint8_t btnIdMap4[8] = {56, 40, 24,  8,  9, 25, 41, 57};
static uint8_t btnIdMap5[8] = {58, 42, 26, 10, 11, 27, 43, 59};
static uint8_t btnIdMap6[8] = {60, 44, 28, 12, 13, 29, 45, 61};
static uint8_t btnIdMap7[8] = {62, 46, 30, 14, 15, 31, 47, 63};

static BTN_DEBOUNCE_T btnDbnc[8];


/******************************************************************************/
void SEQ_Init(void)
{
	ESPI_Shift_IO_Init(&leds,
						ESPI_SHIFT_OUT,
						SEQ_NUM_OUT_REGS,
						SEQ_PORT,
						SEQ_DEVICE_LEDS);
	ESPI_Shift_IO_Init(&buttons,
						ESPI_SHIFT_IN,
						SEQ_NUM_IN_REGS,
						SEQ_PORT,
						SEQ_DEVICE_BTNS);
	RB_Init(&ledRingBuffer, 256);
	RB_Init(&buttonRingBuffer, 256);

	DBNC_Init(&btnDbnc[0], 5, btnIdMap0, SEQ_MIDI_CH);
	DBNC_Init(&btnDbnc[1], 5, btnIdMap1, SEQ_MIDI_CH);
	DBNC_Init(&btnDbnc[2], 5, btnIdMap2, SEQ_MIDI_CH);
	DBNC_Init(&btnDbnc[3], 5, btnIdMap3, SEQ_MIDI_CH);
	DBNC_Init(&btnDbnc[4], 5, btnIdMap4, SEQ_MIDI_CH);
	DBNC_Init(&btnDbnc[5], 5, btnIdMap5, SEQ_MIDI_CH);
	DBNC_Init(&btnDbnc[6], 5, btnIdMap6, SEQ_MIDI_CH);
	DBNC_Init(&btnDbnc[7], 5, btnIdMap7, SEQ_MIDI_CH);

}



void SEQ_Reset()
{
	Leds_ClearAll();
}



/******************************************************************************/
void SEQ_Buttons_EspiPull(void)
{
	SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_1 | ESPI_CPHA_1);
	ESPI_ShiftIn_Poll_Blocking(&buttons);
}

void SEQ_Buttons_EspiPull_NonBlocking(void)
{
	SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_1 | ESPI_CPHA_1);
	ESPI_ShiftIn_Poll_NonBlocking(&buttons);
}



void SEQ_Buttons_Process(void)
{
	DBNC_CheckForChanges(&btnDbnc[0], &buttonRingBuffer, buttons.val[0]);
	DBNC_CheckForChanges(&btnDbnc[1], &buttonRingBuffer, buttons.val[1]);
	DBNC_CheckForChanges(&btnDbnc[2], &buttonRingBuffer, buttons.val[2]);
	DBNC_CheckForChanges(&btnDbnc[3], &buttonRingBuffer, buttons.val[3]);
	DBNC_CheckForChanges(&btnDbnc[4], &buttonRingBuffer, buttons.val[4]);
	DBNC_CheckForChanges(&btnDbnc[5], &buttonRingBuffer, buttons.val[5]);
	DBNC_CheckForChanges(&btnDbnc[6], &buttonRingBuffer, buttons.val[6]);
	DBNC_CheckForChanges(&btnDbnc[7], &buttonRingBuffer, buttons.val[7]);
}


uint32_t SEQ_Buttons_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage)
{
	return RB_Read(&buttonRingBuffer, midiMessage);
}



/******************************************************************************/
void SEQ_Leds_WriteMsgToRingBuffer(MIDI_MSG_T midiMessage)
{
	RB_Write(&ledRingBuffer, midiMessage);
}



void SEQ_Leds_Process(void)
{
	// read all data from ring buffer and write them to the led output buffer
	MIDI_MSG_T midiMsg;
	int dbgCnt = 0;

	while (RB_Read(&ledRingBuffer, &midiMsg) != 0)
	{
		Leds_SetSingle(midiMsg.id, midiMsg.value);
		dbgCnt++;
	}
}



void SEQ_Leds_EspiSend(void)
{
	DBG_Led_Error_On();
	SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_0 | ESPI_CPHA_0);
	ESPI_ShiftOut_Poll_Blocking(&leds);
	DBG_Led_Error_Off();
}


void SEQ_Leds_EspiSend_NonBlocking(void)
{
	SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_0 | ESPI_CPHA_0);
	ESPI_ShiftOut_Poll_NonBlocking(&leds);
}



void Leds_SetSingleAndClearOthers(uint8_t ledId, uint8_t color)
{
	Leds_ClearAll();
	Leds_SetSingle(ledId, color);
}



void Leds_SetSingle(uint8_t step, uint8_t color)
{
	switch (step)
	{
	    case  0: leds.val[31] = (leds.val[31] & 0x0F) | (color << 4);	break;
	    case  1: leds.val[28] = (leds.val[28] & 0xF0) | color;			break;
	    case  2: leds.val[27] = (leds.val[27] & 0x0F) | (color << 4);	break;
	    case  3: leds.val[24] = (leds.val[24] & 0xF0) | color;			break;
	    case  4: leds.val[23] = (leds.val[23] & 0x0F) | (color << 4);	break;
	    case  5: leds.val[20] = (leds.val[20] & 0xF0) | color;			break;
	    case  6: leds.val[19] = (leds.val[19] & 0x0F) | (color << 4);	break;
	    case  7: leds.val[16] = (leds.val[16] & 0xF0) | color;			break;

	    case  8: leds.val[15] = (leds.val[15] & 0x0F) | (color << 4);	break;
	    case  9: leds.val[12] = (leds.val[12] & 0xF0) | color;			break;
	    case 10: leds.val[11] = (leds.val[11] & 0x0F) | (color << 4);	break;
	    case 11: leds.val[8]  = (leds.val[8]  & 0xF0) | color;			break;
	    case 12: leds.val[7]  = (leds.val[7]  & 0x0F) | (color << 4);	break;
	    case 13: leds.val[4]  = (leds.val[4]  & 0xF0) | color;			break;
	    case 14: leds.val[3]  = (leds.val[3]  & 0x0F) | (color << 4);	break;
	    case 15: leds.val[0]  = (leds.val[0]  & 0xF0) | color;			break;

	    case 16: leds.val[31] = (leds.val[31] & 0xF0) | color;			break;
	    case 17: leds.val[28] = (leds.val[28] & 0x0F) | (color << 4);	break;
	    case 18: leds.val[27] = (leds.val[27] & 0xF0) | color;			break;
	    case 19: leds.val[24] = (leds.val[24] & 0x0F) | (color << 4);	break;
	    case 20: leds.val[23] = (leds.val[23] & 0xF0) | color;			break;
	    case 21: leds.val[20] = (leds.val[20] & 0x0F) | (color << 4);	break;
	    case 22: leds.val[19] = (leds.val[19] & 0xF0) | color;			break;
	    case 23: leds.val[16] = (leds.val[16] & 0x0F) | (color << 4);	break;

	    case 24: leds.val[15] = (leds.val[15] & 0xF0) | color;			break;
	    case 25: leds.val[12] = (leds.val[12] & 0x0F) | (color << 4);	break;
	    case 26: leds.val[11] = (leds.val[11] & 0xF0) | color;			break;
	    case 27: leds.val[8]  = (leds.val[8]  & 0x0F) | (color << 4);	break;
	    case 28: leds.val[7]  = (leds.val[7]  & 0xF0) | color;			break;
	    case 29: leds.val[4]  = (leds.val[4]  & 0x0F) | (color << 4);	break;
	    case 30: leds.val[3]  = (leds.val[3]  & 0xF0) | color;			break;
	    case 31: leds.val[0]  = (leds.val[0]  & 0x0F) | (color << 4);	break;

	    case 32: leds.val[30] = (leds.val[30] & 0x0F) | (color << 4);	break;
	    case 33: leds.val[29] = (leds.val[29] & 0xF0) | color;			break;
	    case 34: leds.val[26] = (leds.val[26] & 0x0F) | (color << 4);	break;
	    case 35: leds.val[25] = (leds.val[25] & 0xF0) | color;			break;
	    case 36: leds.val[22] = (leds.val[22] & 0x0F) | (color << 4);	break;
	    case 37: leds.val[21] = (leds.val[21] & 0xF0) | color;			break;
	    case 38: leds.val[18] = (leds.val[18] & 0x0F) | (color << 4);	break;
	    case 39: leds.val[17] = (leds.val[17] & 0xF0) | color;			break;

	    case 40: leds.val[14] = (leds.val[14] & 0x0F) | (color << 4);	break;
	    case 41: leds.val[13] = (leds.val[13] & 0xF0) | color;			break;
	    case 42: leds.val[10] = (leds.val[10] & 0x0F) | (color << 4);	break;
	    case 43: leds.val[9]  = (leds.val[9]  & 0xF0) | color;			break;
	    case 44: leds.val[6]  = (leds.val[6]  & 0x0F) | (color << 4);	break;
	    case 45: leds.val[5]  = (leds.val[5]  & 0xF0) | color;			break;
	    case 46: leds.val[2]  = (leds.val[2]  & 0x0F) | (color << 4);	break;
	    case 47: leds.val[1]  = (leds.val[1]  & 0xF0) | color;			break;

	    case 48: leds.val[30] = (leds.val[30] & 0xF0) | color;			break;
	    case 49: leds.val[29] = (leds.val[29] & 0x0F) | (color << 4);	break;
	    case 50: leds.val[26] = (leds.val[26] & 0xF0) | color;			break;
	    case 51: leds.val[25] = (leds.val[25] & 0x0F) | (color << 4);	break;
	    case 52: leds.val[22] = (leds.val[22] & 0xF0) | color;			break;
	    case 53: leds.val[21] = (leds.val[21] & 0x0F) | (color << 4);	break;
	    case 54: leds.val[18] = (leds.val[18] & 0xF0) | color;			break;
	    case 55: leds.val[17] = (leds.val[17] & 0x0F) | (color << 4);	break;

	    case 56: leds.val[14] = (leds.val[14] & 0xF0) | color;			break;
	    case 57: leds.val[13] = (leds.val[13] & 0x0F) | (color << 4);	break;
	    case 58: leds.val[10] = (leds.val[10] & 0xF0) | color;			break;
	    case 59: leds.val[9]  = (leds.val[9]  & 0x0F) | (color << 4);	break;
	    case 60: leds.val[6]  = (leds.val[6]  & 0xF0) | color;			break;
	    case 61: leds.val[5]  = (leds.val[5]  & 0x0F) | (color << 4);	break;
	    case 62: leds.val[2]  = (leds.val[2]  & 0xF0) | color;			break;
	    case 63: leds.val[1]  = (leds.val[1]  & 0x0F) | (color << 4);	break;

	    case 127:
	    {
	    	uint8_t led;
	    	for(led = 0; led < 64; led++)
	    	{
	    		Leds_SetSingle(led, color);
	    	}
	    	break;
	    }

	    default: break;
	}
}



#if 0
void Leds_SetSingle(uint8_t step, uint8_t color)
{
	switch (step)
	{
	    case  0: leds.val[31] &= ((color << 4) | 0x0F); 	break;
	    case  1: leds.val[28] &=   color | 0xF0;      		break;
	    case  2: leds.val[27] &= ((color << 4) | 0x0F); 	break;
	    case  3: leds.val[24] &=   color | 0xF0;      		break;
	    case  4: leds.val[23] &= ((color << 4) | 0x0F); 	break;
	    case  5: leds.val[20] &=   color | 0xF0;      		break;
	    case  6: leds.val[19] &= ((color << 4) | 0x0F); 	break;
	    case  7: leds.val[16] &=   color | 0xF0;      		break;

	    case  8: leds.val[15] &= ((color << 4) | 0x0F); 	break;
	    case  9: leds.val[12] &=   color | 0xF0;      		break;
	    case 10: leds.val[11] &= ((color << 4) | 0x0F); 	break;
	    case 11: leds.val[8]  &=   color | 0xF0;      		break;
	    case 12: leds.val[7]  &= ((color << 4) | 0x0F); 	break;
	    case 13: leds.val[4]  &=   color | 0xF0;      		break;
	    case 14: leds.val[3]  &= ((color << 4) | 0x0F); 	break;
	    case 15: leds.val[0]  &=   color | 0xF0;      		break;

	    case 16: leds.val[31] &=   color | 0xF0;      		break;
	    case 17: leds.val[28] &= ((color << 4) | 0x0F); 	break;
	    case 18: leds.val[27] &=   color | 0xF0;      		break;
	    case 19: leds.val[24] &= ((color << 4) | 0x0F); 	break;
	    case 20: leds.val[23] &=   color | 0xF0;      		break;
	    case 21: leds.val[20] &= ((color << 4) | 0x0F); 	break;
	    case 22: leds.val[19] &=   color | 0xF0;      		break;
	    case 23: leds.val[16] &= ((color << 4) | 0x0F); 	break;

	    case 24: leds.val[15] &=   color | 0xF0;      		break;
	    case 25: leds.val[12] &= ((color << 4) | 0x0F); 	break;
	    case 26: leds.val[11] &=   color | 0xF0;      		break;
	    case 27: leds.val[8]  &= ((color << 4) | 0x0F); 	break;
	    case 28: leds.val[7]  &=   color | 0xF0;      		break;
	    case 29: leds.val[4]  &= ((color << 4) | 0x0F); 	break;
	    case 30: leds.val[3]  &=   color | 0xF0;      		break;
	    case 31: leds.val[0]  &= ((color << 4) | 0x0F); 	break;

	    case 32: leds.val[30] &= ((color << 4) | 0x0F); 	break;
	    case 33: leds.val[29] &=   color | 0xF0;      		break;
	    case 34: leds.val[26] &= ((color << 4) | 0x0F); 	break;
	    case 35: leds.val[25] &=   color | 0xF0;      		break;
	    case 36: leds.val[22] &= ((color << 4) | 0x0F); 	break;
	    case 37: leds.val[21] &=   color | 0xF0;      		break;
	    case 38: leds.val[18] &= ((color << 4) | 0x0F); 	break;
	    case 39: leds.val[17] &=   color | 0xF0;      		break;

	    case 40: leds.val[14] &= ((color << 4) | 0x0F); 	break;
	    case 41: leds.val[13] &=   color | 0xF0;      		break;
	    case 42: leds.val[10] &= ((color << 4) | 0x0F); 	break;
	    case 43: leds.val[9]  &=   color | 0xF0;      		break;
	    case 44: leds.val[6]  &= ((color << 4) | 0x0F); 	break;
	    case 45: leds.val[5]  &=   color | 0xF0;      		break;
	    case 46: leds.val[2]  &= ((color << 4) | 0x0F); 	break;
	    case 47: leds.val[1]  &=   color | 0xF0;      		break;

	    case 48: leds.val[30] &=   color | 0xF0;      		break;
	    case 49: leds.val[29] &= ((color << 4) | 0x0F); 	break;
	    case 50: leds.val[26] &=   color | 0xF0;      		break;
	    case 51: leds.val[25] &= ((color << 4) | 0x0F); 	break;
	    case 52: leds.val[22] &=   color | 0xF0;      		break;
	    case 53: leds.val[21] &= ((color << 4) | 0x0F); 	break;
	    case 54: leds.val[18] &=   color | 0xF0;      		break;
	    case 55: leds.val[17] &= ((color << 4) | 0x0F); 	break;

	    case 56: leds.val[14] &= color | 0xF0;      		break;
	    case 57: leds.val[13] &= ((color << 4) | 0x0F); 	break;
	    case 58: leds.val[10] &= color | 0xF0;      		break;
	    case 59: leds.val[9]  &= ((color << 4) | 0x0F); 	break;
	    case 60: leds.val[6]  &= color | 0xF0;      		break;
	    case 61: leds.val[5]  &= ((color << 4) | 0x0F); 	break;
	    case 62: leds.val[2]  &= color | 0xF0;      		break;
	    case 63: leds.val[1]  &= ((color << 4) | 0x0F); 	break;

	    default: break;
	}
}
#endif



void SEQ_RGB_Process_CountSteps(void)
{
	static uint8_t step = 1;
	static uint8_t rgb_color = RGB_COLOR_RED;
	static uint8_t tgl_color = 0;
	Leds_ClearAll();
	Leds_SetSingle(step, rgb_color);
	step++;

	if(step == 65)
	{
		step = 1;

		tgl_color++;
		if (tgl_color == 3)
			tgl_color = 0;

		switch (tgl_color)
		{
			case 0: rgb_color = RGB_COLOR_RED; break;
			case 1: rgb_color = RGB_COLOR_GREEN; break;
			case 2: rgb_color = RGB_COLOR_BLUE; break;
			default: break;
		}
	}
}






uint8_t SEQ_RGB_TestReadButtons(void)
{
	static uint8_t state = 0;
	uint8_t retValue = 0;


	switch (state)
	{
		case 0: retValue = buttons.val[0]; break;
		case 1: retValue = buttons.val[1]; break;
		case 2: retValue = buttons.val[2]; break;
		case 3: retValue = buttons.val[3]; break;
		case 4: retValue = buttons.val[4]; break;
		case 5: retValue = buttons.val[5]; break;
		case 6: retValue = buttons.val[6]; break;
		case 7: retValue = buttons.val[7]; break;
		case 8: retValue = 0; break;
	}

	state++;
	if (state == 9)
		state = 0;

	return retValue;


#if 0
	uint8_t shiftReg = 0;
	uint8_t bitShift = 0;

	for(shiftReg = 0; shiftReg <= SEQ_NUM_IN_REGS; shiftReg++)
	{
		for(bitShift = 0; bitShift < 8; bitShift++)
		{
			if( ((buttons.val[shiftReg] >> bitShift) & 0b1) == 0 )
			{
				return (shiftReg * bitShift) + 1;
			}
		}
	}
	return 0;
#endif
}



static void Leds_ClearAll(void)
{
	uint8_t cnt;
	for (cnt = 0; cnt < 32; cnt++)
	{
		leds.val[cnt] = 0xFF;
	}
}



#if 0 // check if still needed
void SEQ_RGB_ReadStepAndSetButton(void)
{
	uint8_t step = 0;

	step = SEQ_RGB_ReadButtons();
	Leds_ClearAll();
	SEQ_RGB_SetSingleStepToColor(step, RGB_COLOR_RED);
}
#endif
