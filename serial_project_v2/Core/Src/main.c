/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>    // add this
#include <string.h>   // for strlen
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

USART_HandleTypeDef husart3;
uint8_t mode = 0;
uint32_t count = 0;
uint32_t counter = 0;
uint8_t modemsgsent = 0;
uint8_t prev_butt = GPIO_PIN_RESET;
uint8_t led_toggle = 0;
uint32_t osc = 500;
int8_t dir = -25;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_Init();
  /* USER CODE BEGIN 2 */
  char mode0msg[] = "[MODE 0] TICK\r\n";
  char mode1msg[] = "[MODE 1] SLOW BLINK\r\n";
  char mode2msg[] = "[MODE 2] FAST BLINK\r\n";
  char mode3msg[] = "[MODE 3] OSCILLATE\r\n";

  char msg[50];
  HAL_Delay(500); // let things settle before starting
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  uint32_t ticks = HAL_GetTick();
	  uint8_t curr_butt = HAL_GPIO_ReadPin(B1_BLUE_USER_BUTTON_GPIO_Port, B1_BLUE_USER_BUTTON_Pin);

	  // Check if the Blue User Button is pressed
	      // Note: Most Nucleo buttons are "Active Low", but some are "Active High".
	      // If it doesn't work with GPIO_PIN_SET, try GPIO_PIN_RESET.
	      if (curr_butt == GPIO_PIN_SET && prev_butt == GPIO_PIN_RESET)
	      {
	          // Toggle the LED

	          modemsgsent = 0;
	          mode++;
	          counter = 0;


	          // Short delay to prevent accidental double-toggles
	          HAL_Delay(50);
	      }
	      prev_butt = curr_butt;

	      if (mode == 0 && HAL_GetTick() - counter >= 1000)
	      {
	    	  count++;
	    	  counter = HAL_GetTick();
	    	  if (modemsgsent == 0)
	    	  {
		    	  HAL_USART_Transmit(&husart3, (uint8_t*)mode0msg, strlen(mode0msg), HAL_MAX_DELAY);
		    	  modemsgsent = 1;
	    	  }

	    	  sprintf(msg, "[%lu] TICK = %lu ms\r\n", count, ticks);
	    	  HAL_USART_Transmit(&husart3, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

	      }

	      if (mode == 1 && HAL_GetTick() - counter >= 500)
	      {
	    	  counter = HAL_GetTick();


	    	  if (modemsgsent == 0)
	    	  {
		    	  HAL_USART_Transmit(&husart3, (uint8_t*)mode1msg, strlen(mode1msg), HAL_MAX_DELAY);
		    	  modemsgsent = 1;
	    	  }

	    	  HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
	    	  led_toggle = !led_toggle;

	    	  if (led_toggle == 1)
	    	  {
		    	  sprintf(msg, "[MODE 1] SLOW BLINK [ON] %lu ms\r\n",ticks);

	    	  }
	    	  if (led_toggle == 0)
	    	  {
	    	      sprintf(msg, "[MODE 1] SLOW BLINK [OFF] %lu ms\r\n",ticks);
	    	  }
	    	  HAL_USART_Transmit(&husart3, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

	      }
	      if (mode == 2 && HAL_GetTick() - counter >= 100)
	      {
	    	  counter = HAL_GetTick();

	    	  if (modemsgsent == 0)
	    	  {
		    	  HAL_USART_Transmit(&husart3, (uint8_t*)mode2msg, strlen(mode2msg), HAL_MAX_DELAY);
		    	  modemsgsent = 1;
	    	  }

	    	  HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
	    	  led_toggle = !led_toggle;

	    	  if (led_toggle == 1)
	    	  {
		    	  sprintf(msg, "[MODE 2] FAST BLINK [ON] %lu ms\r\n",ticks);

	    	  }
	    	  if (led_toggle == 0)
	    	  {
	    	      sprintf(msg, "[MODE 2] FAST BLINK [OFF] %lu ms\r\n",ticks);
	    	  }
	    	  HAL_USART_Transmit(&husart3, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);


	      }
	      if (mode == 3 && HAL_GetTick() - counter >= osc)
	      {
	    	  counter = HAL_GetTick();
	    	  osc += dir;
	    	  if (modemsgsent == 0)
	    	  {
		    	  HAL_USART_Transmit(&husart3, (uint8_t*)mode3msg, strlen(mode3msg), HAL_MAX_DELAY);
		    	  modemsgsent = 1;
	    	  }
	    	  if (osc >= 500)
	    	  {
	    		  dir = -25;
	    	  }
	    	  if (osc <= 50)
	    	  {
	    		  dir = 25;
	    	  }
	    	  HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
	    	  led_toggle = !led_toggle;

	    	  if (led_toggle == 1)
	    	  {
		    	  sprintf(msg, "[MODE 3] OSCILLATING [ON] DELTA = |%lu| %lu ms\r\n",osc,ticks);

	    	  }
	    	  if (led_toggle == 0)
	    	  {
	    	      sprintf(msg, "[MODE 3] OSCILLATING [OFF] DELTA = |%lu| %lu ms\r\n",osc,ticks);
	    	  }
	    	  HAL_USART_Transmit(&husart3, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

	      }

	      if (mode >= 4)
	      {
	    	  mode = 0;
	      }
    /* USER CODE END WHILE */
	  // 1. Toggle the state of the LED pin
//	      HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
//
//	      // 2. Wait for 500 milliseconds (0.5 seconds)
//	      HAL_Delay(500);
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the programming delay
  */
  __HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_1);
}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  husart3.Instance = USART3;
  husart3.Init.BaudRate = 115200;
  husart3.Init.WordLength = USART_WORDLENGTH_8B;
  husart3.Init.StopBits = USART_STOPBITS_1;
  husart3.Init.Parity = USART_PARITY_NONE;
  husart3.Init.Mode = USART_MODE_TX_RX;
  husart3.Init.CLKPolarity = USART_POLARITY_LOW;
  husart3.Init.CLKPhase = USART_PHASE_1EDGE;
  husart3.Init.CLKLastBit = USART_LASTBIT_DISABLE;
  husart3.Init.ClockPrescaler = USART_PRESCALER_DIV1;
  husart3.SlaveMode = USART_SLAVEMODE_DISABLE;
  if (HAL_USART_Init(&husart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_USARTEx_SetTxFifoThreshold(&husart3, USART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_USARTEx_SetRxFifoThreshold(&husart3, USART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_USARTEx_DisableFifoMode(&husart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USER_LED_GPIO_Port, USER_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_BLUE_USER_BUTTON_Pin */
  GPIO_InitStruct.Pin = B1_BLUE_USER_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_BLUE_USER_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USER_LED_Pin */
  GPIO_InitStruct.Pin = USER_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USER_LED_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
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
#ifdef USE_FULL_ASSERT
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
