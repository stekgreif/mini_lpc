/******************************************************************************/
/** @date	2015-02-17 DTZ
    @author	2014-11-10 DTZ
*******************************************************************************/
#include <stdint.h>

#include "drv/nl_espi_core.h"
#include "drv/nl_espi_shift_io.h"
#include "drv/nl_espi_adc.h"

#include "../hwmodules/ahc.h"

#include "../modules/ring_buffer.h"
#include "../modules/midi.h"
#include "../modules/button_debounce.h"

#define AHC_NUM_OUT_REGS	1
#define AHC_NUM_IN_REGS		1
#define AHC_PORT			9
#define AHC_DEVICE_POTS		1
#define AHC_DEVICE_SWIS		2

static ESPI_ADC_T     pots;
static RB_BUF_T       potsRingbuffer;
static ESPI_IO_T      switches;
static RB_BUF_T       switchesRingbuffer;
static uint8_t        switchesIdMap[8] = {0, 1, 2, 3, 4, 5, 6, 7};
static BTN_DEBOUNCE_T switchesDebounce;

static MIDI_MSG_T midiMsg = {
	.usb    = MIDI_CONTROL_CHANGE >> 4,
	.status = MIDI_CONTROL_CHANGE | AHC_MIDI_CH
};



void AHC_Init(void)
{
	ESPI_ADC_Init( 		&pots,
						ESPI_ADC_3202,
						AHC_PORT,
						AHC_DEVICE_POTS);
	ESPI_Shift_IO_Init( &switches,
						ESPI_SHIFT_IN,
						AHC_NUM_IN_REGS,
						AHC_PORT,
						AHC_DEVICE_SWIS);
	RB_Init(&potsRingbuffer, 32);
	RB_Init(&switchesRingbuffer, 32);

	DBNC_Init(&switchesDebounce, 1, switchesIdMap, AHC_MIDI_CH);
}



void AHC_Switches_EspiPullData(void)
{
	SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_1 | ESPI_CPHA_1);
	ESPI_ShiftIn_Poll_Blocking(&switches);
}

void AHC_Switches_EspiPullData_NonSwitching(void)
{
	ESPI_ShiftIn_Poll_Blocking(&switches);
}


void AHC_Switches_Process(void)
{
	DBNC_CheckForChanges(&switchesDebounce, &switchesRingbuffer, switches.val[0]);
}


uint32_t AHC_Switches_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage)
{
	return RB_Read(&switchesRingbuffer, midiMessage);
}



/******************************************************************************/
/** @brief	This function will alternating call the volume poti and the pan poti
  	  	  	id 0: VOL - id 1: PAN
*******************************************************************************/
void AHC_Potis_EspiPullData(void)
{
	static uint8_t tglId = 0;

	SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_0 | ESPI_CPHA_1);

	if (tglId == 0)
	{
		ESPI_ADC_Channel_Poll(&pots, tglId);
		tglId = 1;
	}
	else
	{
		ESPI_ADC_Channel_Poll(&pots, tglId);
		tglId = 0;
	}
}



void AHC_Potis_Process(void)
{
	static uint16_t oldValue[2] = {};
	static uint16_t newValue[2] = {};

	// both pots are inverted: left is max (4096) and right is min (0),
	newValue[0] = (4095 - ESPI_ADC_Channel_Get(&pots, 0)) >> 5;
	newValue[1] = (4095 - ESPI_ADC_Channel_Get(&pots, 1)) >> 5;

	if (oldValue[0] != newValue[0])
	{
		oldValue[0] = newValue[0];
		midiMsg.id = 0;
		midiMsg.value = newValue[0];
		RB_Write(&potsRingbuffer, midiMsg);
	}

	if (oldValue[1] != newValue[1])
	{
		oldValue[1] = newValue[1];
		midiMsg.id = 1;
		midiMsg.value = newValue[1];
		RB_Write(&potsRingbuffer, midiMsg);
	}
}



/******************************************************************************/
/** public functions
*******************************************************************************/
uint32_t AHC_Adc_ReadMsgFromRingbuffer(MIDI_MSG_T* midiMessage)
{
	return RB_Read(&potsRingbuffer, midiMessage);
}


