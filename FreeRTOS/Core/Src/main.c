#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>

UART_HandleTypeDef huart2;
uint32_t adc_data;
//uint16_t adc_data;

/* Definitions for Task1 */
osThreadId_t Task1Handle;
const osThreadAttr_t Task1_attributes = {
  .name = "Task1",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Definitions for Task2 */
osThreadId_t Task2Handle;
const osThreadAttr_t Task2_attributes = {
  .name = "Task2",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Definitions for Timer1 */
osTimerId_t Timer1Handle;
const osTimerAttr_t Timer1_attributes = {
  .name = "Timer1"
};

/* Definitions for Timer2 */
osTimerId_t Timer2Handle;
const osTimerAttr_t Timer2_attributes = {
  .name = "Timer2"
};
/* Definitions for adc_queue */
osMessageQueueId_t adc_queueHandle;
const osMessageQueueAttr_t adc_queue_attributes = {
  .name = "adc_queue"
};

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void Task1_function(void *argument);
void Task2_function(void *argument);
void Timer1_function(void *argument);
void Timer2_func(void *argument);
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
	*CR2|=(1<<8); //Direct memory access mode (for single ADC mode)
	*CR2|=(1<<9);
	*CR2|=(1<<10);//End of conversion selection
	*CR2&=~(1<<11);//Data alignment

	uint32_t* SMPR2=(uint32_t*)0x40012010;
	*SMPR2|=(000<<3);  //Channel x sampling time selection -> 000: 3 cycles

	uint32_t* SQR1=(uint32_t*)0x4001202c;
	*SQR1|=(0000<<20); // Regular channel sequence length - 0000 :1 conversion

	uint32_t* SQR3=(uint32_t*)0x40012034;
	*SQR3|=(1<<0);//SQ1  1st conversion in regular sequence

	uint32_t* MODER=(uint32_t*)0x40020000;
	*MODER |=(11<<2); //Set Analog mode for PA1 ADC1_1

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
	uint32_t* DR= (uint32_t*)0x4001204c;
	//uint16_t* DR= (uint16_t*)0x4001204c;
	return *DR;
}

int main(void)
{
  //HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  adc_init();
  osKernelInitialize();
  /* creation of Timer1 */
  Timer1Handle = osTimerNew(Timer1_function, osTimerPeriodic, NULL, &Timer1_attributes);
  /* creation of Timer2 */
  Timer2Handle = osTimerNew(Timer2_func, osTimerPeriodic, NULL, &Timer2_attributes);

  /* creation of Task1 */
  Task1Handle = osThreadNew(Task1_function, NULL, &Task1_attributes);

  /* creation of Task2 */
  Task2Handle = osThreadNew(Task2_function, NULL, &Task2_attributes);
  /* Start scheduler */

/* creation of adc_queue */
  adc_queueHandle = osMessageQueueNew (16, sizeof(uint32_t), &adc_queue_attributes);

  osKernelStart();
  while (1)
  {

  }

}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_USART2_UART_Init(void)
{


  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : PD12 PD13 PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

void Task1_function(void *argument)
{
  uint8_t* msg1="Hello world\r\n";
  osTimerStart(Timer1Handle,1000);//Goi timer 1 chay

  for(;;)
  {
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
    char msg2[12]={0};
    uint32_t msg_count=osMessageQueueGetCount(adc_queueHandle);//Get number of queued messages in a Message Queue
    sprintf(msg2,"adc value: %ld",adc_data);  //Read the first times
    for(int i=1; i< msg_count ;i++)            //Read the second times
    {
    	osMessageQueueGet(adc_queueHandle, &adc_data, 0,HAL_MAX_DELAY);
    	sprintf(msg2, "%s,%ld" ,msg2, adc_data);
    }

    sprintf(msg2,"%s\r\n",msg2);// luu du lieu adc vao vung nho format sprint gui qua Uart
	HAL_UART_Transmit(&huart2,(uint8_t*)msg2, strlen(msg2),HAL_MAX_DELAY);//Use UART transmit adc_value to hercules
    osDelay(5000);
    //HAL_UART_Transmit(&huart2,msg1, sizeof(msg1)+2,HAL_MAX_DELAY);//Use UART transmit string to hercules
  }

}

/* USER CODE END Header_Task2_function */
void Task2_function(void *argument)
{
  osTimerStart(Timer2Handle,2500);
  for(;;)
  {
	  adc_start();
	  adc_data=adc_read();
	  osMessageQueuePut(adc_queueHandle, &adc_data, 0,HAL_MAX_DELAY); //put data in the Queue
	  HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
	  osDelay(1000);
  }

}

/* Timer1_function function */
void Timer1_function(void *argument)
{
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
}

/* Timer2_func function */
void Timer2_func(void *argument)
{
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
