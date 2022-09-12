#include "main.h"

void gpio_d12_init()
{
	__HAL_RCC_GPIOD_CLK_ENABLE();
	uint32_t* GPIOD_MODER =(uint32_t*)0x40020c00;// tro toi dia chi Moder
	*GPIOD_MODER &= ~(0b11<<26);
	*GPIOD_MODER |= 0b01<<26;//Set PD13
}

/* READ BUTTON Function*/
void gpio_button_init()
{
__HAL_RCC_GPIOA_CLK_ENABLE();
uint32_t* GPIOA_MODER=0x40020000;
*GPIOA_MODER &= ~(0b11<<0);//clear -set PA0 as input

uint32_t* GPIOA_PUPDR=0x4002000c;
*GPIOA_PUPDR &= ~(0b11<<0);  //set floating
}

int gpio_button_read()
{
  uint32_t* GPIOA_IDR=(uint32_t*)0x40020010;
  if((*GPIOA_IDR>>0 & 1)==1)
    return 1;
  else
    return 0;
}

void rcc_init()
{
	uint32_t* CSR=(uint32_t*)0x40023874;
	*CSR|=(1<<0);//LSI ON

//	uint32_t* CR=(uint32_t*)0x40023800;
//	*CR|=(0b01<<21);//



	uint32_t* KR=(uint32_t*)0x40003000;
	*KR=0x5555;//unlock PR and RLR register

	uint32_t* PR=(uint32_t*)0x40003004;
	*PR=29-1; // Divide 32KHZ  1/32.10^-3 =31,35 ms-> /30 =1,0416 ms
	//*PR=(3<<0);
	uint32_t* RLR=(uint32_t*)0x40003008;
	*RLR=4000;// thres value 4000ms

	*KR=0xCCCC;//Enable


}

int main(void)
{
  HAL_Init();
  gpio_d12_init();
  gpio_button_init();
  rcc_init();

  while (1)
  {
	  HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
	  HAL_Delay(500);
	  if(gpio_button_read(GPIOA, GPIO_PIN_0)==1) //If press button, program will be stacked-->after 4S Led On again
	  {
		  while(1);
	  }
  }

}


