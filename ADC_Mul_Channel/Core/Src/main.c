#include"main.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
uint16_t data;
uint16_t data1;
void adc_init()
{

	__HAL_RCC_GPIOA_CLK_ENABLE();
	uint32_t* APB2ENR=(uint32_t*)0x40023844;
	*APB2ENR|=(1<<8);//ADC1 clock enable

	uint32_t* CCR=(uint32_t*)0x40012304;
	*CCR|=(2<<16);//ADC prescaler- 10: PCLK2 divided by 6 ADC_CLK = 90/6 = 15MHz

	uint32_t* CR1=(uint32_t*)0x40012004;
	*CR1|=(1<<8);//Scan mode > 1 channel
	*CR1|=(00<<24);//Set resolution 12-bit (15 ADCCLK cycles) 0<=ADC value <= 4095

	uint32_t* CR2=(uint32_t*)0x40012008;

	*CR2|=(1<<1); //1: Continuous conversion mode

	*CR2|=(1<<10);//End of conversion selection
	*CR2&=~(1<<11);//Data alignment
	*CR2|=(1<<8); //Direct memory access mode (for single ADC mode)
	*CR2|=(1<<9);//// Enable Continuous Request

	uint32_t* SMPR2=(uint32_t*)0x40012010;
	*SMPR2|=(000<<3);
	*SMPR2|=(000<<12);// Sampling time of 3 cycles for channel 1 and channel 4 -> 000: 3 cycles
	/*Formula calculate

	 * ADCCLK=30MHZ
	 * Tconv = Sampling+12 CYCLES
	  =>Tconv= (sampling+cycles) / ADC CLOCK =0,5us with APB2 at 60MHZ
	 */

	uint32_t* SQR1=(uint32_t*)0x4001202c;
	*SQR1|=(0001<<20); // Regular channel sequence length - 0001 :2 conversion

	uint32_t* SQR3=(uint32_t*)0x40012034;
	*SQR3|=(1<<0);//Set sequence conver channel 1 first SQ1
	*SQR3|=(4<<5);//Set sequence conver channel 4 second SQ2

	uint32_t* MODER=(uint32_t*)0x40020000;
	*MODER |=(3<<2); //Set Analog mode for PA1 ADC1_1 - 11: Analog mode with 3 = 0011
	*MODER |=(3<<8); //Set Analog mode for PA4

	*CR2|=(1<<0); // Enable ADC  **Always set this bit in the end
}

void adc_start(void)
{
	uint32_t* CR2=(uint32_t*)0x40012008;
	*CR2|=(1<<30);//Start conversion of regular channels
}

uint32_t adc_read()
{
	uint32_t* SR=(uint32_t*)0x40012000;
	*SR &=~(1); //Clear SR before start conversion
	uint16_t* DR= (uint16_t*)0x4001204c;
	return *DR;
}

void main()
{
	adc_init();
	adc_start();
    while(1)
    {
        data=adc_read();   //get 12 bit --> 2^12 =4096 => uint16
    }
}
