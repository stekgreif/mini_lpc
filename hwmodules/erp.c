/******************************************************************************/
/** @date	2015-02-21
    @author	2014 DTZ
*******************************************************************************/
#include <stdint.h>

#include "erp.h"

#include "drv/nl_espi_core.h"
#include "drv/nl_espi_shift_io.h"
#include "drv/nl_espi_adc.h"

#include "../modules/midi.h"
#include "../modules/ring_buffer.h"

#define ERP_PORT				3
#define ERP_DEVICE_1TO4			1
#define ERP_DEVICE_5			2

static ESPI_ADC_T espiErp1to4;
static ESPI_ADC_T espiErp5;
static uint16_t   adcValue[10] = {};
static RB_BUF_T	  rbAdc;

static MIDI_MSG_T midiAdcMsg = {
	.usb    = MIDI_CONTROL_CHANGE >> 4,
	.status = MIDI_CONTROL_CHANGE | ERP_MIDI_CH
};


static uint16_t GetPosition(uint16_t x, uint16_t y);



/******************************************************************************/
void ERP_Init(void)
{
	ESPI_ADC_Init( &espiErp1to4,
					ESPI_ADC_3208,
					ERP_PORT,
					ERP_DEVICE_1TO4);
	ESPI_ADC_Init( &espiErp5,
					ESPI_ADC_3202,
					ERP_PORT,
					ERP_DEVICE_5);
	RB_Init(&rbAdc, 32);
}



/* call 10 times to cycle trough all 4 pots */
void ERP_Erps_EspiPull(void)
{
	static uint8_t idCnt = 0;

	SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_0 | ESPI_CPHA_1);

	switch (idCnt)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		{
			ESPI_ADC_Channel_Poll(&espiErp1to4, idCnt);
			adcValue[idCnt] = ESPI_ADC_Channel_Get(&espiErp1to4, idCnt);
			break;
		}
		case 8:
		case 9:
		{
			ESPI_ADC_Channel_Poll(&espiErp5, idCnt-8);
			adcValue[idCnt] = ESPI_ADC_Channel_Get(&espiErp5, idCnt-8);
			break;
		}
	}

	idCnt++;
	if (idCnt == 10)
	{
		idCnt = 0;
	}
}



void ERP_Erps_Espi_SwitchToMode(void)
{
	SPI_DMA_SwitchMode(LPC_SSP0, ESPI_CPOL_0 | ESPI_CPHA_1);
}


void ERP_Erps_EspiPull_NonSwitching(void)
{
	static uint8_t idCnt = 0;

	switch (idCnt)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		{
			ESPI_ADC_Channel_Poll(&espiErp1to4, idCnt);
			adcValue[idCnt] = ESPI_ADC_Channel_Get(&espiErp1to4, idCnt);
			break;
		}
		case 8:
		case 9:
		{
			ESPI_ADC_Channel_Poll(&espiErp5, idCnt-8);
			adcValue[idCnt] = ESPI_ADC_Channel_Get(&espiErp5, idCnt-8);
			break;
		}
	}

	idCnt++;
	if (idCnt == 10)
	{
		idCnt = 0;
	}
}


#if 1
void ERP_Erps_Process(void)
{
	static int16_t erpPositionOld[5];
	static int16_t erpPositionNew[5];
	static int16_t  erpDiff[5];

	uint8_t id = 0;

	for (id = 0; id < 5; id++)
	{
		erpPositionNew[id] = GetPosition( adcValue[id*2] >> 2, adcValue[(id*2)+1] >> 2 );

		if ((erpPositionOld[id]+1 < erpPositionNew[id]) ||
			(erpPositionOld[id]-1 > erpPositionNew[id]))
		{
			erpDiff[id]        = erpPositionOld[id] - erpPositionNew[id];
			erpPositionOld[id] = erpPositionNew[id];

			erpDiff[id]        = erpDiff[id] + 64;

			if( erpDiff[id] < 0 )
				erpDiff[id] = 0;
			else if ( erpDiff[id] > 127 )
				erpDiff[id] = 127;

			midiAdcMsg.id = id;
			midiAdcMsg.value = ((uint8_t) erpDiff[id]) & 0b01111111;
			RB_Write(&rbAdc, midiAdcMsg);
		}
	}
}
#endif



#if 0
void ERP_Erps_Process(void)
{
	static uint16_t erpPositionOld[5];
	static uint16_t erpPositionNew[5];
	static int16_t  erpDiff[5];

	uint8_t  id = 0;
	for (id = 0; id < 5; id++)
	{
		erpPositionNew[id] = GetPosition( adcValue[id*2] >> 2, adcValue[(id*2)+1] >> 2 );

		if (erpPositionOld[id] != erpPositionNew[id])
		{
			erpDiff[id] = erpPositionOld[id] - erpPositionNew[id];
			erpPositionOld[id] = erpPositionNew[id];

			midiAdcMsg.id = id;

			erpDiff[id] = erpDiff[id] + 63;
			if( erpDiff[id] < 0 )
				erpDiff[id] = 0;
			else if ( erpDiff[id] > 127 )
				erpDiff[id] = 127;

			midiAdcMsg.value = (uint8_t) (erpDiff[id]);
			RB_Write(&rbAdc, midiAdcMsg);
		}
	}
}
#endif



#if 0 // 2015-04-29 erps are a bit flickery
/* erp 	id
	0,1 0
	2,3	1
	4,5	2
	6,7	3
	8,9	4   */
void ERP_Erps_Process(void)
{
	uint8_t  id = 0;
	static uint16_t erpPositionOld[5];
	static uint16_t erpPositionNew[5];
	static int16_t  erpDiff[5];

	for (id = 0; id < 5; id++)
	{
		// adcValue: 12 bit -> 10 bit
		uint16_t a = (adcValue[ id*2   ] >> 2);
		uint16_t b = (adcValue[(id*2)+1] >> 2);
		a = a & 0b0000001111111000;
		b = b & 0b0000001111111000;

		erpPositionNew[id] = GetPosition( a, b );

		if (erpPositionOld[id] != erpPositionNew[id])
		{
			erpDiff[id] = erpPositionOld[id] - erpPositionNew[id];
			erpPositionOld[id] = erpPositionNew[id];
			midiAdcMsg.id = id;

			erpDiff[id] = erpDiff[id] + 63;
			if( erpDiff[id] < 0 )
				erpDiff[id] = 0;
			else if ( erpDiff[id] > 127 )
				erpDiff[id] = 127;

			midiAdcMsg.value = (uint8_t) (erpDiff[id]);
			RB_Write(&rbAdc, midiAdcMsg);
		}
	}
}
#endif



uint32_t ERP_Erps_ReadMsgFromRingBuffer(MIDI_MSG_T* midiMessage)
{
	return RB_Read(&rbAdc, midiMessage);
}



/******************************************************************************/
/**	@brief	turns the two ad-values from an erp to a value/position
 	 	 	between 0..2047

	@param x analog outputs A of the erp (10 bit value)
	@param y analog outputs B of the erp (10 bit value)
	@return calculated position (0..2047)
*******************************************************************************/
static uint16_t GetPosition(uint16_t x, uint16_t y)
{
	/* variables for temporary calculation*/
	uint32_t x_c;
	uint32_t y_c;

	uint32_t w = 0;	/* output value */
	uint32_t a;	/* fading coefficient */

	/* B1 */
	if ( ((384<y) && (y<=640)) && (x>768) )
	{
		w = y-256;
	}

	/* B12 */
	else if ( ((640<y) && (y<=896)) && (x>512) )
	{
		a = y-640;					/* calculate coefficient */
		x_c = 1280-x; 				/* mirror and shift x */
		y_c = y-256;				/* shift y */
		w = x_c*a + y_c*(255-a);	/* fading */
		w = w >> 8;					/* normalize result */
	}

	/* B2 */
	else if ( (896<y) && ((256<x) && (x<768)) )
	{
		w = 1280-x;
	}

	/* B23 */
	else if ( ((640<y) && (y<=896)) && (x<512) )
	{
		a = 896-y;						/* calculate coefficient */
		x_c = 1280-x;					/* mirror and shift x */
		y_c = 1792-y;					/* mirror y*/
		w = (y_c*a) + (x_c*(255-a));	/* fading */
		w = w >> 8;						/* normalize result */
	}

	/* B3 */
	else if ( ((384<y) && (y<=640)) && (x<256) )
	{
		w = 1790-y;
	}

	/* B34 */
	else if ( ((128<y) && (y<=384)) && (x<512) )
	{
		a = 384-y;					/* calculate coefficient */
		x_c = 1280+x;				/* shift x */
		y_c = 1792-y;				/* mirror and shift y*/
		w = x_c*a + y_c*(255-a);	/* fading */
		w = w >> 8;					/* normalize result  w = w + 1280; */
	}

	/* B4 */
	else if ( (y<=128) && ((256<x) && (x<768)) )
	{
		w = 1280+x;
	}

	/* B41 */
	else if ( ((128<y) && (y<=384)) && (x>512) )
	{
		a = y-128;					/* calculate coefficient */
		x_c = 1280+x;				/* shift x */
		y_c = 1792+y;				/* mirror and shift y*/
		w = y_c*a + x_c*(255-a);	/* fading */
		w = w >> 8;					/* normalize result */

		if (w > 2047)
		{
			w = w - 2047;
		}
	}

	return (uint16_t) w;
}

