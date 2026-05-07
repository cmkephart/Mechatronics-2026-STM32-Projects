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
#include <stdio.h>
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

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
const char* modes[] = {"Plate", "Cup", "Bowl", "Manual"};
int current_mode = 0; // Index to keep track of selection
uint8_t state = 0;
uint8_t cycle = 0;
uint8_t top_prev_butt = GPIO_PIN_SET;
uint8_t bot_prev_butt = GPIO_PIN_SET;
uint8_t select = GPIO_PIN_RESET;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// --- Pulse the Enable pin to latch data ---
static void LCD_Pulse_Enable(void)
{
    HAL_GPIO_WritePin(GPIOA, E_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOA, E_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
}

// --- Send the upper nibble (bits 7-4) via D4-D7 ---
static void LCD_Send_Nibble(uint8_t nibble)
{
    HAL_GPIO_WritePin(D4_GPIO_Port,  D4_Pin, (nibble >> 0) & 1);
    HAL_GPIO_WritePin(GPIOC,         D5_Pin, (nibble >> 1) & 1);
    HAL_GPIO_WritePin(GPIOC,         D6_Pin, (nibble >> 2) & 1);
    HAL_GPIO_WritePin(GPIOC,         D7_Pin, (nibble >> 3) & 1);
    LCD_Pulse_Enable();
}

// --- Send a full byte as two nibbles ---
static void LCD_Send_Byte(uint8_t byte, uint8_t rs)
{
    HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, rs);   // RS=0: command, RS=1: data
    LCD_Send_Nibble(byte >> 4);             // High nibble first
    LCD_Send_Nibble(byte & 0x0F);           // Low nibble second
    HAL_Delay(2);
}

// --- Initialize the LCD in 4-bit mode ---
static void LCD_Init(void)
{
    HAL_Delay(50);                          // Wait for LCD power-up

    // Special 3-step init to reliably enter 4-bit mode
    HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET);
    LCD_Send_Nibble(0x03); HAL_Delay(5);
    LCD_Send_Nibble(0x03); HAL_Delay(1);
    LCD_Send_Nibble(0x03); HAL_Delay(1);
    LCD_Send_Nibble(0x02); HAL_Delay(1);    // Switch to 4-bit mode

    LCD_Send_Byte(0x28, 0);                 // 4-bit, 2 lines, 5x8 font
    LCD_Send_Byte(0x0C, 0);                 // Display ON, cursor OFF, blink OFF
    LCD_Send_Byte(0x06, 0);                 // Entry mode: increment, no shift
    LCD_Send_Byte(0x01, 0);                 // Clear display
    HAL_Delay(2);                           // Clear needs extra time
}

// --- Print a string at the current cursor position ---
static void LCD_Print(const char *str)
{
    while (*str)
        LCD_Send_Byte((uint8_t)*str++, 1);
}

// --- Move cursor: row 0 or 1, col 0-15 ---
static void LCD_Set_Cursor(uint8_t row, uint8_t col)
{
    uint8_t addr = (row == 0) ? (0x80 + col) : (0xC0 + col);
    LCD_Send_Byte(addr, 0);
}

void delay_us(uint32_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

uint32_t Ultrasonic_Read(void)
{
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);

    uint32_t timeout = 0;
    while (HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_RESET)
    {
        timeout++;
        if (timeout > 100000) return 0;
    }

    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while (HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_SET);
    return __HAL_TIM_GET_COUNTER(&htim2);
}
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
  MX_TIM2_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);

  HAL_TIM_Base_Start(&htim2);
  LCD_Init();
  LCD_Set_Cursor(0, 0);
  LCD_Print("Dirty Dishes!");

  char buf[4];
  for (int i = 5; i >= 1; i--)
  {
      LCD_Set_Cursor(1, 0);       // Row 2, column 0
      sprintf(buf, "%d", i);
      LCD_Print(buf);
      HAL_Delay(1000);
  }

  LCD_Send_Byte(0x01, 0);         // Clear display
  HAL_Delay(2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  uint8_t top_curr_butt = HAL_GPIO_ReadPin(TOP_BUTT_GPIO_Port, TOP_BUTT_Pin);
	  uint8_t bot_curr_butt = HAL_GPIO_ReadPin(BOT_BUTT_GPIO_Port, BOT_BUTT_Pin);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	    char row1[32];
	    char row2[32];

	    uint32_t echo_us = Ultrasonic_Read();
	    uint32_t dist_whole = echo_us / 58;
	    uint32_t dist_frac  = (echo_us % 58) * 10 / 58;

	    // Add a section of code here that changes an index of a variable whenever
	    // the button for cycling modes is pressed
if(state == 0)
	{
	if (top_curr_butt == GPIO_PIN_RESET && top_prev_butt == GPIO_PIN_SET)
	{
		current_mode += 1;
		HAL_Delay(10);
	}
	if (current_mode >= 4)
	{
	    current_mode = 0;
	}
	if (bot_curr_butt == GPIO_PIN_RESET && bot_prev_butt == GPIO_PIN_SET)
	{
		select ^= 1;
		HAL_Delay(10);
	}

	    top_prev_butt = top_curr_butt;
	    bot_prev_butt = bot_curr_butt;

	    sprintf(row1, "Select Mode: ");
	    sprintf(row2, "%-10s ", modes[current_mode]);
//	    sprintf(row2, "%-10s     %u", modes[current_mode],select);

	    row1[16] = '\0';
	    row2[16] = '\0';

	    LCD_Set_Cursor(0, 0);
	    LCD_Print(row1);
	    LCD_Set_Cursor(1, 0);
	    LCD_Print(row2);

	    if (select == 1)
	    {
	    	state = 1;
		    LCD_Set_Cursor(0, 0);
		    LCD_Print("Manual Mode!    ");
		    LCD_Set_Cursor(1, 0);
		    sprintf(row2, "%-16s", "");   // Left-justify empty string padded to 16 chars
		    LCD_Print(row2);
	    }

//	    HAL_Delay(10);
	}
// MOTOR MODE
	if(select == 1 && current_mode == 3)
	{
	    if (top_curr_butt == GPIO_PIN_RESET)        // Forward
	    {
	        HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, GPIO_PIN_SET);
	        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, htim1.Init.Period); // Full speed
	        HAL_GPIO_WritePin(GPIOB, RED_LED_Pin, GPIO_PIN_SET);
	        HAL_GPIO_WritePin(GPIOB, GREEN_LED_Pin, GPIO_PIN_RESET);
	    }
	    else if (bot_curr_butt == GPIO_PIN_RESET)   // Reverse
	    {
	        HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, GPIO_PIN_RESET);
	        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, htim1.Init.Period); // Full speed
	        HAL_GPIO_WritePin(GPIOB, GREEN_LED_Pin, GPIO_PIN_SET);
	        HAL_GPIO_WritePin(GPIOB, RED_LED_Pin, GPIO_PIN_RESET);
	    }
	    else                                         // Stop
	    {
	        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0); // 0% duty = off
	        HAL_GPIO_WritePin(GPIOB, RED_LED_Pin, GPIO_PIN_RESET);
	        HAL_GPIO_WritePin(GPIOB, GREEN_LED_Pin, GPIO_PIN_RESET);
	    }
	}
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
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 63;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, USER_LED_Pin|E_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, TRIG_Pin|D7_Pin|D6_Pin|D5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, RED_LED_Pin|GREEN_LED_Pin|DIR_Pin|RS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_BLUE_USER_BUTTON_Pin */
  GPIO_InitStruct.Pin = B1_BLUE_USER_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_BLUE_USER_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : T_VCP_RX_Pin T_VCP_TX_Pin */
  GPIO_InitStruct.Pin = T_VCP_RX_Pin|T_VCP_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_USART3;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : USER_LED_Pin E_Pin */
  GPIO_InitStruct.Pin = USER_LED_Pin|E_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : TOP_BUTT_Pin BOT_BUTT_Pin */
  GPIO_InitStruct.Pin = TOP_BUTT_Pin|BOT_BUTT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : TRIG_Pin D7_Pin D6_Pin D5_Pin */
  GPIO_InitStruct.Pin = TRIG_Pin|D7_Pin|D6_Pin|D5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : RED_LED_Pin GREEN_LED_Pin DIR_Pin RS_Pin */
  GPIO_InitStruct.Pin = RED_LED_Pin|GREEN_LED_Pin|DIR_Pin|RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : D4_Pin */
  GPIO_InitStruct.Pin = D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(D4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ECHO_Pin */
  GPIO_InitStruct.Pin = ECHO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ECHO_GPIO_Port, &GPIO_InitStruct);

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
