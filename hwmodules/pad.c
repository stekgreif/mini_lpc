/******************************************************************************/
/** @date	2015-03-28 DTZ
    @author	2014 DTZ
*******************************************************************************/
#include <stdint.h>

#include "pad.h"

#include "drv/nl_espi_core.h"
#include "drv/nl_espi_shift_io.h"
#include "drv/nl_espi_adc.h"

#include "../modules/midi.h"
#include "../modules/ring_buffer.h"

#define PAD_PORT	4
#define PAD_DEVICE	1

static ESPI_ADC_T espiPads;
static RB_BUF_T	  rbAdc;

static MIDI_MSG_T midiAdcMsg = {
	.usb    = MIDI_CONTROL_CHANGE >> 4,
	.status = MIDI_CONTROL_CHANGE | PAD_MIDI_CH
};



/******************************************************************************/
void PAD_Init(void)
{
	ESPI_ADC_Init( &espiPads,
					ESPI_ADC_3204,
					PAD_PORT,
					PAD_DEVICE);
	RB_Init(&rbAdc, 32);
}



/******************************************************************************/
/**	@brief	Every time this function gets called it will call the next adc. So
			it takes for times to cover all 4 channels.
*******************************************************************************/
void PAD_Pads_EspiPull(void)
{
	static uint8_t cnt = 0;

	SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_0 | ESPI_CPHA_1);
	ESPI_ADC_Channel_Poll(&espiPads, cnt);

	cnt++;
	if (cnt == 4)
		cnt = 0;
}

void PAD_Pads_Espi_SwitchToMode(void)
{
	SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_0 | ESPI_CPHA_1);
}


void PAD_Pads_EspiPull_NonSwitching(void)
{
	static uint8_t cnt = 0;

	ESPI_ADC_Channel_Poll(&espiPads, cnt);

	cnt++;
	if (cnt == 4)
		cnt = 0;
}


void PAD_Pads_Process(void)
{
#if 0
	static uint16_t oldValue[4] = {};
	static uint16_t newValue[4] = {};

	uint8_t cnt;
	for (cnt = 0; cnt < 4; cnt++)
	{
		newValue[cnt] = ESPI_ADC_Channel_Get(&espiPads, cnt) >> 6;

		if (oldValue[cnt] != newValue[cnt])
		{
			oldValue[cnt] = newValue[cnt];
			midiAdcMsg.id = cnt;
			midiAdcMsg.value = newValue[cnt];
			RB_Write(&rbAdc, midiAdcMsg);
		}
	}
#endif
}



uint32_t PAD_Pads_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage)
{
	return RB_Read(&rbAdc, midiMessage);
}
