/******************************************************************************/
/** @date	2015-02-18 DTZ
    @author	2015-02-18 DTZ
    @info

    LEDS
	1  2  3
    4     5
    6  7  8

	BUTTONS/INC
	1  2  3
	4 INC 5
	6  7  8

	.val[1]
	A: ENC_A, B: ENC_B,	P: ENC push button
	7 6 5 4 3 2 1 0  // Byte
    A B P x x x x x

	.val[0]
	7 6 5 4 3 2 1 0  // Byte
	3 2 1 5 4 8 7 6  // Button IDs

*******************************************************************************/
#include <stdint.h>

#include "../hwmodules/men.h"

#include "drv/nl_espi_core.h"
#include "drv/nl_espi_shift_io.h"

#include "../modules/midi.h"
#include "../modules/ring_buffer.h"
#include "../modules/button_debounce.h"

#define MEN_NUM_OF_OUT_REGS	2
#define MEN_NUM_OF_IN_REGS 	2
#define MEN_PORT			2
#define MEN_DEVICE_LEDS		1
#define MEN_DEVICE_BTN_ENC	2

static ESPI_IO_T espiLeds;
static ESPI_IO_T espiEncBtns;

static RB_BUF_T  rbLeds;
static RB_BUF_T  rbEnc;
static RB_BUF_T  rbBtns;

static BTN_DEBOUNCE_T dbncBtns;
static BTN_DEBOUNCE_T dbncEnc;

static uint8_t idMapBtns[8] = { 2, 1, 0, 4, 3, 7, 6, 5};
static uint8_t idMapEnc[8] = {99,99,99,99,99, 8,99,99};

static MIDI_MSG_T midiCcMsg = {
	.usb    = MIDI_CONTROL_CHANGE >> 4,
	.status = MIDI_CONTROL_CHANGE | MEN_MIDI_CH
};


static void Leds_SetSingle(uint8_t ledId);
static void Leds_ClearSingle(uint8_t ledId);
static void Leds_ClearAll(void);
static void Leds_SetAll(void);


/******************************************************************************/
void MEN_Init(void)
{
	ESPI_Shift_IO_Init(&espiLeds,
						ESPI_SHIFT_OUT,
						MEN_NUM_OF_OUT_REGS,
						MEN_PORT,
						MEN_DEVICE_LEDS);

	ESPI_Shift_IO_Init(&espiEncBtns,
						ESPI_SHIFT_IN,
						MEN_NUM_OF_IN_REGS,
						MEN_PORT,
						MEN_DEVICE_BTN_ENC);

	RB_Init(&rbLeds, 32);
	RB_Init(&rbEnc,  32);
	RB_Init(&rbBtns, 32);

	DBNC_Init(&dbncBtns, 1, idMapBtns, MEN_MIDI_CH);
	DBNC_Init(&dbncEnc,  1, idMapEnc,  MEN_MIDI_CH);
}


void MEN_Reset(void)
{
	Leds_ClearAll();
}


/******************************************************************************/
void MEN_ButtonsAndEnc_EspiPull(void)
{
	SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_1 | ESPI_CPHA_1);
	ESPI_ShiftIn_Poll_Blocking(&espiEncBtns);
}

void MEN_ButtonsAndEnc_EspiPull_NonSwitching(void)
{
	ESPI_ShiftIn_Poll_Blocking(&espiEncBtns);
}

void MEN_Buttons_Process(void)
{
	DBNC_CheckForChanges(&dbncBtns, &rbBtns, espiEncBtns.val[0]);
}

uint32_t MEN_Buttons_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage)
{
	return RB_Read(&rbBtns, midiMessage);
}



static int32_t encDelta  = 0;
/******************************************************************************/
/** @brief	This function should be called at least every 1 ms.
*******************************************************************************/
void MEN_Encoder_Process(void)
{
#if 0 // 2015-04-29 original
	static int8_t encPreVal = 0;

	int8_t encNewVal = 0;
	int8_t encDiff   = 0;

	uint8_t tempBit6 = espiEncBtns.val[1];
	uint8_t tempBit7 = espiEncBtns.val[1];

	 /* bits invertieren */
	tempBit6 = ~tempBit6;
	tempBit7 = ~tempBit7;

	/* bits maskieren */
	tempBit6 = tempBit6 & 0b01000000;
	tempBit7 = tempBit7 & 0b10000000;

	/* bits shiften */
	tempBit6 = tempBit6 >> 6;
	tempBit7 = tempBit7 >> 7;


	if (tempBit6)
		encNewVal = 3;
	if (tempBit7)
		encNewVal ^= 1;

	encDiff = encPreVal - encNewVal;

	if (encDiff & 1)
	{
		encPreVal = encNewVal;
		encDelta += (encDiff & 2) - 1;
	}

	//debounce encoder click
	DBNC_CheckForChanges(&dbncEnc, &rbEnc, (espiEncBtns.val[1] | 0b11011111));
#endif
#if 1
	static int8_t encPreVal = 0;

	int8_t encNewVal = 0;
	int8_t encDiff   = 0;

	uint8_t tempBit6 = espiEncBtns.val[1];
	uint8_t tempBit7 = espiEncBtns.val[1];

	 /* bits invertieren */
	tempBit6 = ~tempBit6;
	tempBit7 = ~tempBit7;

	/* bits maskieren */
	tempBit6 = tempBit6 & 0b01000000;
	tempBit7 = tempBit7 & 0b10000000;

	/* bits shiften */
	tempBit6 = tempBit6 >> 6;
	tempBit7 = tempBit7 >> 7;


	if (tempBit7)
		encNewVal = 3;
	if (tempBit6)
		encNewVal ^= 1;

	encDiff = encPreVal - encNewVal;

	if (encDiff & 1)
	{
		encPreVal = encNewVal;
		encDelta += (encDiff & 2) - 1;
	}

	//debounce encoder click
	DBNC_CheckForChanges(&dbncEnc, &rbEnc, (espiEncBtns.val[1] | 0b11011111));
#endif
}



int8_t GetEncoderValue(void)
{
	int8_t retVal = 0;

	if (encDelta > 63)
		retVal = 63;
	else if (encDelta < -63)
		retVal = -63;
	else
		retVal = (int8_t) encDelta;

#if 0 // 1 Step
	encDelta = 0;
#endif
#if 1 // 2 Step
	encDelta &= 1;
	retVal  >>= 1;
#endif
#if 0 // 4 Step
	encDelta &= 3;
	retVal >>= 2;
#endif

	return retVal;
}



uint32_t MEN_Encoder_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage)
{
	int8_t sendCheck = GetEncoderValue();

	if(sendCheck != 0)
	{
		midiCcMsg.value = ((uint8_t) (sendCheck + 64)) & 0b01111111;
		midiCcMsg.id = 9;
		RB_Write(&rbEnc, midiCcMsg);
	}

	return RB_Read(&rbEnc, midiMessage);
}









#if 1
/******************************************************************************/
void MEN_Leds_WriteMsgToRingBuffer(MIDI_MSG_T midiMessage)
{
	RB_Write(&rbLeds, midiMessage);
}


void MEN_Leds_Process(void)
{
	MIDI_MSG_T ledTemp;

	while (RB_Read(&rbLeds, &ledTemp) != 0)
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



void MEN_Leds_EspiSend(void)
{
	SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_0 | ESPI_CPHA_0);
	ESPI_ShiftOut_Poll_Blocking(&espiLeds);
}




static void Leds_ClearAll(void)
{
	uint8_t cnt;
	for (cnt = 0; cnt < MEN_NUM_OF_OUT_REGS; cnt++)
		espiLeds.val[cnt] = 0x00;
}

static void Leds_SetAll(void)
{
	uint8_t cnt;
	for (cnt = 0; cnt < MEN_NUM_OF_OUT_REGS; cnt++)
		espiLeds.val[cnt] = 0xFF;
}




static void Leds_SetSingle(uint8_t ledId)
{
	switch (ledId)
	{
		case 0: espiLeds.val[1] |= 0b11 << 4; break;
		case 1: espiLeds.val[1] |= 0b11 << 2; break;
	    case 2: espiLeds.val[1] |= 0b11 << 0; break;
	    case 3: espiLeds.val[0] |= 0b11 << 0; break;

		case 4: espiLeds.val[1] |= 0b11 << 6; break;
		case 5: espiLeds.val[0] |= 0b11 << 6; break;
		case 6: espiLeds.val[0] |= 0b11 << 4; break;
	    case 7: espiLeds.val[0] |= 0b11 << 2; break;

		default: break;
	}
}



static void Leds_ClearSingle(uint8_t ledId)
{
	switch (ledId)
	{
	    case 0: espiLeds.val[1] &= ~(0b11 << 4); break;
	    case 1: espiLeds.val[1] &= ~(0b11 << 2); break;
	    case 2: espiLeds.val[1] &= ~(0b11 << 0); break;
	    case 3: espiLeds.val[0] &= ~(0b11 << 0); break;

	    case 4: espiLeds.val[1] &= ~(0b11 << 6); break;
	    case 5: espiLeds.val[0] &= ~(0b11 << 6); break;
	    case 6: espiLeds.val[0] &= ~(0b11 << 4); break;
	    case 7: espiLeds.val[0] &= ~(0b11 << 2); break;

		default: break;
	}
}

#endif
