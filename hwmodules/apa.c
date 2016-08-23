// 2015-01-21

#include "../hwmodules/apa.h"

#include "stdint.h"
#include "drv/nl_espi_core.h"
#include "drv/nl_espi_shift_io.h"
#include "drv/nl_espi_adc.h"

#include "../modules/midi.h"
#include "../modules/ring_buffer.h"

#define APA_PORT_AUDIO				11
#define APA_PORT_PEDAL				10
#define APA_DEVICE_PEDAL_ADC	 	 1
#define APA_DEVICE_PEDAL_DETECT	 	 2
#define APA_DEVICE_PEDAL_PULL_R	 	 3
#define APA_DEVICE_AUDIO_ATTENUATOR	 1
#define APA_NUM_OUT_REGS 			 1
#define APA_NUM_IN_REGS 			 1

static ESPI_ADC_T pedals;
static ESPI_IO_T  pullResistors;
static ESPI_IO_T  detect;

static RB_BUF_T rbAttenuator;

static uint8_t rxb[2];
static uint8_t txb[2];
static uint8_t attenuation = 0;



/******************************************************************************/
static void ESPI_Attenuator_Callback(uint32_t status)
{
	ESPI_SCS_Select(ESPI_PORT_OFF, APA_DEVICE_AUDIO_ATTENUATOR);
}


void APA_Init(void)
{
	ESPI_ADC_Init( 		&pedals,
						ESPI_ADC_3208,
						APA_PORT_PEDAL,
						APA_DEVICE_PEDAL_ADC);
	ESPI_Shift_IO_Init( &pullResistors,
						ESPI_SHIFT_OUT,
						APA_NUM_OUT_REGS,
						APA_PORT_PEDAL,
						APA_DEVICE_PEDAL_PULL_R);
	ESPI_Shift_IO_Init( &detect,
						ESPI_SHIFT_IN,
						APA_NUM_IN_REGS,
						APA_PORT_PEDAL,
						APA_DEVICE_PEDAL_DETECT);

	RB_Init( &rbAttenuator, 64);
}



/******************************************************************************/
void APA_Attenuator_WriteMsgToRingBuffer(MIDI_MSG_T midiMessage)
{
	RB_Write(&rbAttenuator, midiMessage);
}



void APA_Attenuator_Process(void)
{
	MIDI_MSG_T tempMsg;

	while( RB_Read( &rbAttenuator, &tempMsg ) != 0 )
	{
		attenuation = tempMsg.value;
	}
}



void APA_Attenuator_EspiSend(void)
{
	static uint8_t lastAttVal = 0;
	static uint8_t channel = 0;

	if(lastAttVal != attenuation)
	{
		ESPI_SCS_Select(APA_PORT_AUDIO, APA_DEVICE_AUDIO_ATTENUATOR);
		SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_0 | ESPI_CPHA_0);

		if (channel == 0)
		{
			txb[0] = 0;
			txb[1] = attenuation;
			channel = 1;
		}
		else
		{
			txb[0] = 1;
			txb[1] = attenuation;
			channel = 0;
			lastAttVal = attenuation;
		}
		ESPI_TransferBlocking(txb, rxb, 2, ESPI_Attenuator_Callback);
	}
}






/******************************************************************************/












/******************************************************************************/
/** @brief	Read ADCs from selected 4 channels
*******************************************************************************/
void APA_Pedals_Process(void)
{
	static uint8_t cnt = 0;

	switch (cnt)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		/* ADCs */
		case 7:
		{
			SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_0 | ESPI_CPHA_1);
			ESPI_ADC_Channel_Poll(&pedals, cnt);
			break;
		}

		default:
			break;
	}

	cnt++;
	if (cnt == 11)
		cnt = 0;
}


/******************************************************************************/
/** @param[]  id: 1 .. 4
    @return   12 adc value, right aligned
*******************************************************************************/
uint16_t APA_Pedals_GetValue(uint8_t id)
{
	return ESPI_ADC_Channel_Get(&pedals, (8 - (2 * id)));
}






#if 0


static void ESPI_Attenuator_Callback(uint32_t status) {
	ESPI_SCS_Select(ESPI_PORT_OFF, ESPI_ATTENUATOR_DEVICE);
	//scu_pinmux(3,7, MD_PLN_FAST, 4);
}

void ESPI_Attenuator_Init(void) {
	attenuation[0]=attenuation[1] = 0;
	is_changed[0]=is_changed[1]=0;
}

void ESPI_Attenuator_Channel_Set(uint8_t ch, uint8_t val)
{
	if(ch >= 2)
		return;
	if(attenuation[ch] == val)
		return;
	attenuation[ch] = val;
	is_changed[ch] = 1;
}

uint32_t ESPI_Attenuator_Poll(uint8_t ch) {
	if(ch >=2)
		return 0;

	if(is_changed[ch] == 0)
		return 2;
	is_changed[ch] = 0;

	ESPI_SCS_Select(ESPI_ATTENUATOR_PORT, ESPI_ATTENUATOR_DEVICE);

	txb[0] = ch;
	txb[1] = attenuation[ch];
	//scu_pinmux(3,7, MD_PLN_FAST, 5);
	return ESPI_TransferBlocking(txb, rxb, 2, ESPI_Attenuator_Callback);
}
#endif
