#include"main.h"
#include<stdint.h>

float adc_value=0;
uint32_t adc_int_val=0;

void adc_init()
{
	__HAL_RCC_ADC1_CLK_ENABLE();
	uint32_t* SMPR1=(uint32_t*)0x4001200c;
	*SMPR1 |=(0b111<<18);          //Sampling time
	uint32_t* CCR=(uint32_t*)0x40012304;
	uint32_t* JSQR=(uint32_t*)0x40012038;
	*JSQR &=~(0b11<<20);          //Injected sequence length -  1 conversion
	*JSQR &=~(0b11111<<15);      //1st conversion in injected sequence
	*JSQR |=16<<15;        		// choose channel 16 for temp-sensor at JSQ4

	*CCR|=(1<<23); //ENABLE Tempt sensor
	//HAL_Delay(100);
	uint32_t* CR2=(uint32_t*)0x40012008;
	*CR2|=(1<<0) ; //AD Converter ON / OFF
}

float get_adc()
{
	uint32_t* CR2=(uint32_t*)0x40012008;
	*CR2 |=(1<<22);  //Start conversion of injected
	uint32_t* SR=(uint32_t*)0x40012000;
	while(((*SR>>2)&1) !=1);
	*SR &=~(1<<2);
	uint32_t* JDR1=(uint32_t*)0x4001203c; //Data register
	uint32_t temp= *JDR1;
	return 1000*(temp*3)/4095.0;  //3=3v ,*1000->mV

}

void main()
{
	adc_init();
	while (1)
  {
	  //float vsense=(adc_int_val *3000.0)/4095.0;//unit: mV
	  float vsense=get_adc();
	  adc_value=((vsense -760)/2.5) + 25 ; //0,V25=0,76V->760mV  2.5=Avg_Slope
  }
}
