/******************************************************************************/
/** @date	2015-02-21
    @author	2014 DTZ
    @info	Buttons
    			buttons in uint8_t
	1  2	   	0b1234.xxxx

    3  4
*******************************************************************************/
#include <stdint.h>

#include "../hwmodules/crf.h"

#include "drv/nl_espi_adc.h"
#include "drv/nl_espi_core.h"
#include "drv/nl_espi_shift_io.h"

#include "../modules/midi.h"
#include "../modules/ring_buffer.h"
#include "../modules/button_debounce.h"

#define CRF_NUM_OF_OUT_REGS	1
#define CRF_NUM_OF_IN_REGS	1
#define CRF_PORT				5
#define CRF_DEVICE_BTNS			1
#define CRF_DEVICE_LEDS			2
#define CRF_DEVICE_CRF			3

static ESPI_IO_T  espiButtons;
static ESPI_IO_T  espiLeds;
static ESPI_ADC_T espiAdc;

static RB_BUF_T	rbLeds;
static RB_BUF_T	rbButtons;
static RB_BUF_T	rbAdc;

static BTN_DEBOUNCE_T dbncButtons;

static uint8_t idMapButtons[8] = {0,1,2,3,4,5,6,7};

static MIDI_MSG_T midiAdcMsg = {
	.usb    = MIDI_CONTROL_CHANGE >> 4,
	.status = MIDI_CONTROL_CHANGE | CRF_MIDI_CH
};

static void Leds_SetSingle(uint8_t ledId);
static void Leds_ClearSingle(uint8_t ledId);
static void Leds_ClearAll(void);
static void Leds_SetAll(void);





/******************************************************************************/
void CRF_Init(void)
{
	ESPI_ADC_Init( 	   &espiAdc,
						ESPI_ADC_3201,
						CRF_PORT,
						CRF_DEVICE_CRF);
	ESPI_Shift_IO_Init(&espiLeds,
						ESPI_SHIFT_OUT,
						CRF_NUM_OF_OUT_REGS,
						CRF_PORT,
						CRF_DEVICE_LEDS);
	ESPI_Shift_IO_Init(&espiButtons,
						ESPI_SHIFT_IN,
						CRF_NUM_OF_IN_REGS,
						CRF_PORT,
						CRF_DEVICE_BTNS);

	RB_Init(&rbLeds, 32);
	RB_Init(&rbButtons, 32);
	RB_Init(&rbAdc, 32);

	DBNC_Init(&dbncButtons, 1, idMapButtons, CRF_MIDI_CH);
}



void CRF_Reset(void)
{
	Leds_ClearAll();
}



/******************************************************************************/
void CRF_Buttons_EspiPull(void)
{
	SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_1 | ESPI_CPHA_1);
	ESPI_ShiftIn_Poll_Blocking(&espiButtons);
}

void CRF_Buttons_EspiPull_NonSwitching(void)
{
	ESPI_ShiftIn_Poll_Blocking(&espiButtons);
}



void CRF_Buttons_Process(void)
{
	DBNC_CheckForChanges(&dbncButtons, &rbButtons, espiButtons.val[0]);
}


///@param[out] pointer to midi msg that should be written
uint32_t CRF_Buttons_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage)
{
	return RB_Read(&rbButtons, midiMessage);
}



/******************************************************************************/
void CRF_Leds_WriteMsgToRingBuffer(MIDI_MSG_T midiMessage)
{
	RB_Write(&rbLeds, midiMessage);
}


void CRF_Leds_Process(void)
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



void CRF_Leds_EspiSend(void)
{
	SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_0 | ESPI_CPHA_0);
	ESPI_ShiftOut_Poll_Blocking(&espiLeds);
}



/******************************************************************************/
static void Leds_ClearAll(void)
{
	uint8_t cnt;
	for (cnt = 0; cnt < CRF_NUM_OF_OUT_REGS; cnt++)
		espiLeds.val[cnt] = 0x00;
}

static void Leds_SetAll(void)
{
	uint8_t cnt;
	for (cnt = 0; cnt < CRF_NUM_OF_OUT_REGS; cnt++)
		espiLeds.val[cnt] = 0xFF;
}



static void Leds_SetSingle(uint8_t ledId)
{
	switch (ledId)
	{
	    case 0: espiLeds.val[0] |= 0b11 << 0; break;
		case 1: espiLeds.val[0] |= 0b11 << 2; break;
		case 2: espiLeds.val[0] |= 0b11 << 4; break;
		case 3: espiLeds.val[0] |= 0b11 << 6; break;

		default: break;
	}
}



static void Leds_ClearSingle(uint8_t ledId)
{
	switch (ledId)
	{
	    case  0: espiLeds.val[0] &= ~(0b11 << 0); break;
	    case  1: espiLeds.val[0] &= ~(0b11 << 2); break;
	    case  2: espiLeds.val[0] &= ~(0b11 << 4); break;
	    case  3: espiLeds.val[0] &= ~(0b11 << 6); break;

		default: break;
	}
}




/******************************************************************************/
void CRF_Crossfader_EspiPull(void)
{
	SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_0 | ESPI_CPHA_1);
	ESPI_ADC_Channel_Poll(&espiAdc, 0);
}

void CRF_Crossfader_EspiPull_NonSwitching(void)
{
	ESPI_ADC_Channel_Poll(&espiAdc, 0);
}


/* left: min: 0 - right: max: 4064 */
void CRF_Crossfader_Process(void)
{
	static uint16_t oldValue = 0;
	static uint16_t newValue = 0;

	newValue = ESPI_ADC_Channel_Get(&espiAdc, 0) >> 5;

	if (oldValue != newValue)
	{
		oldValue = newValue;
		midiAdcMsg.id = 0;
		midiAdcMsg.value = newValue;
		RB_Write(&rbAdc, midiAdcMsg);
	}
}



uint32_t CRF_Crossfader_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage)
{
	return RB_Read(&rbAdc, midiMessage);
}


