#include<stdint.h>
#define RCC_ADDR_BASE 0x40023800  //RCC addr
#define GPIOD_ADDR_BASE 0x40020C00 //GIPO D ADDR
uint32_t systick_count=0;

/*Delay Function*/
void SysTick_Init()
{
    uint32_t* Systick_Ctrl=(uint32_t*)0xe00e010; //Check in ARM datasheet
    uint32_t* Systick_reload=(uint32_t*)0xe00e014;
    *Systick_reload=16000-1; //  16Mhz:16KHZ =1KHZ-> 1 count counts 1ms
    *Systick_Ctrl|=(1)|(1<<1); //enable and interrupt
}

void SysTick_Handler() //interrupt jump into this function (startup file)
{
    systick_count++;
}

void Delay(uint32_t time)
{
    systick_count=0;
    while(systick_count<time);
}

void SystemInit()
{
}

void LED_Init()
{
    uint32_t* RCC_AHB1ENR=(uint32_t*)(RCC_ADDR_BASE+0x30);
    *RCC_AHB1ENR|=(1<<3); //Enable Clock Port D
    uint32_t* GPIOD_MODER=(uint32_t*)(GPIOD_ADDR_BASE + 0x00);
    *GPIOD_MODER|=(0b01<<28);
}

typedef enum
{
    OFF,
    ON
}Led_state_t;

void LED_Control(Led_state_t state)
{
    uint32_t* GPIO_ODR =(uint32_t*)(GPIOD_ADDR_BASE + 0x14);
    if(state == OFF)
        *GPIO_ODR=~(1<<14);
    else
        *GPIO_ODR|=(1<<14);
}


void main()
{    //HAL_Init();
    SystemInit();
    LED_Init();
    while(1)
    {
        LED_Control(ON);
        Delay(10000);
        LED_Control(OFF);
        Delay(10000);
    }
}