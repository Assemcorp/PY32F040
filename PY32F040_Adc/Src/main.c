/*******************************************************************************
 * @file    main.c
 * @author  MCU Application Team
 * @author  AssemCorp - Emre Karabek - Field Application Engineer
 * @date    2025-June
 * @brief   Main program body
 *******************************************************************************/
/*
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 Puya Semiconductor Co.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by Puya under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private define ------------------------------------------------------------*/
// Segment pin definitions
#define SEG_A_PIN GPIO_PIN_5
#define SEG_B_PIN GPIO_PIN_0
#define SEG_C_PIN GPIO_PIN_1
#define SEG_D_PIN GPIO_PIN_10
#define SEG_E_PIN GPIO_PIN_2
#define SEG_F_PIN GPIO_PIN_6
#define SEG_G_PIN GPIO_PIN_3

#define DIGIT1_PIN GPIO_PIN_15
#define DIGIT2_PIN GPIO_PIN_7

#define SEG_PORT_A GPIOA
#define SEG_PORT_B GPIOB
#define DIGIT_PORT GPIOA

#define LED_PINS_B (GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15)
#define LED_PIN_A8 GPIO_PIN_8
#define LED_PORT_B GPIOB
#define LED_PORT_A GPIOA

#define BUTTON_STOP_PIN GPIO_PIN_1
#define BUTTON_START_PIN GPIO_PIN_2
#define BUTTON_PORT GPIOA

#define FALSE 0
#define TRUE 1
/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef AdcHandle;
uint16_t aADCxConvertedData;
TIM_HandleTypeDef TimHandle;
TIM_MasterConfigTypeDef sMasterConfig;

// Take segment pins into an array in order (a-g)
const uint16_t seg_pins[7] = {
  SEG_A_PIN, SEG_B_PIN, SEG_C_PIN, SEG_D_PIN, SEG_E_PIN, SEG_F_PIN, SEG_G_PIN};
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_SystemClockConfig(void);
static void APP_AdcConfig(void);
void Led_Bar(void);
void Display_2Digit(uint8_t);

void LED_Init(void);
/**
 * @brief  Main program.
 * @retval int
 */
int main(void)
{

  HAL_Init();
  APP_SystemClockConfig();
  APP_AdcConfig();
  LED_Init();

  while (1)
  {
  /* ADC starts conversion and enables interruption */
  HAL_ADC_Start_IT(&AdcHandle);
  HAL_Delay(100);
  Led_Bar();
  }
}

/**
 * @brief  ADC Conversion Callback.
 * @param  hadc：ADC handle
 * @retval None
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  aADCxConvertedData = hadc->Instance->DR;
}

/**
 * @brief  ADC configuration function.
 * @param  None
 * @retval None
 */
static void APP_AdcConfig()
{
  ADC_ChannelConfTypeDef sConfig = {0};
  RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInit = {0};

  /* Enable ADC clock */
  __HAL_RCC_ADC_CLK_ENABLE();

  RCC_PeriphCLKInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  RCC_PeriphCLKInit.ADCClockSelection = RCC_ADCCLKSOURCE_PCLK_DIV4;
  HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInit);

  AdcHandle.Instance = ADC1;

  AdcHandle.Init.Resolution = ADC_RESOLUTION_12B;       /* 12-bit resolution for converted data */
  AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;       /* Right-alignment for converted data */
  AdcHandle.Init.ScanConvMode = ADC_SCAN_DISABLE;       /* Scan mode off */
  AdcHandle.Init.ContinuousConvMode = DISABLE;          /* Single mode */
  AdcHandle.Init.NbrOfConversion = 1;                   /* Number of conversion channels 1 */
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;       /* Discontinuous mode not enabled */
  AdcHandle.Init.NbrOfDiscConversion = 1;               /* Discontinuous mode short sequence length is 1 */
  AdcHandle.Init.ExternalTrigConv = ADC_SOFTWARE_START; /* software trigger */
  /* ADC initialization */
  if (HAL_ADC_Init(&AdcHandle) != HAL_OK)
  {
  APP_ErrorHandler();
  }

  sConfig.Channel = ADC_CHANNEL_0; /* Select ADC channel 0 */
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
  /* ADC channel configuration */
  if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
  {
  APP_ErrorHandler();
  }
  /* ADC calibration */
  if (HAL_ADCEx_Calibration_Start(&AdcHandle) != HAL_OK)
  {
  APP_ErrorHandler();
  }

  /*  if(HAL_ADCEx_Calibration_GetStatus(&AdcHandle) != HAL_ADCCALIBOK)   */
  /*  {                                                                   */
  /*    APP_ErrorHandler();                                               */
  /*  }                                                                   */
}
/**
 * @brief  Initializes LED pins.
 * @retval None
 */
void LED_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // Enable GPIOB clock
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // Enable GPIOA clock
  __HAL_RCC_GPIOA_CLK_ENABLE();

  // Set LED pins on GPIOB as output
  GPIO_InitStruct.Pin = LED_PINS_B;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_PORT_B, &GPIO_InitStruct);

  // Set segment pins as output
  GPIO_InitStruct.Pin = SEG_A_PIN | SEG_F_PIN | SEG_G_PIN | LED_PIN_A8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SEG_PORT_A, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SEG_B_PIN | SEG_C_PIN | SEG_D_PIN | SEG_E_PIN;
  HAL_GPIO_Init(SEG_PORT_B, &GPIO_InitStruct);

  // Set digit pins as output
  GPIO_InitStruct.Pin = DIGIT1_PIN | DIGIT2_PIN;
  HAL_GPIO_Init(DIGIT_PORT, &GPIO_InitStruct);
}
void Led_Bar(void)
{
  uint8_t led_count = aADCxConvertedData / 650;
  if (led_count > 6)
  led_count = 6;

  // Take LED pins into an array
  uint16_t led_pins[6] = {
    GPIO_PIN_11, // LED1 (GPIOB)
    GPIO_PIN_12, // LED2 (GPIOB)
    GPIO_PIN_13, // LED3 (GPIOB)
    GPIO_PIN_14, // LED4 (GPIOB)
    GPIO_PIN_15, // LED5 (GPIOB)
    GPIO_PIN_8   // LED6 (GPIOA)
  };
  // First, turn off all LEDs
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
  for (uint8_t i = 0; i < led_count; i++)
  {
  if (i < 5)
  {
    HAL_GPIO_WritePin(GPIOB, led_pins[i], GPIO_PIN_SET);
  }
  else
  {
    HAL_GPIO_WritePin(GPIOA, led_pins[i], GPIO_PIN_SET);
  }
  }
}
/**
 * @brief  System clock configuration function.
 * @param  None
 * @retval None
 */
static void APP_SystemClockConfig(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /* Oscillator configuration */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE; /* Select oscillator HSE, HSI, LSI, LSE */
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                                                                                              /* Enable HSI */
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                                                                                              /* HSI 1 frequency division */
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_8MHz;                                                                      /* Configure HSI clock at 8MHz */
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;                                                                                             /* Disable HSE */
  /*RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;*/
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF; /* Disable LSI */
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF; /* Disable LSE */
  /*RCC_OscInitStruct.LSEDriver = RCC_LSEDRIVE_MEDIUM;*/
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF; /* Disable PLL */
  /*RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_NONE;*/
  /*RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;*/
  /* Configure oscillator */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
  APP_ErrorHandler();
  }

  /* Clock source configuration */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1; /* Choose to configure clock HCLK, SYSCLK, PCLK1 */
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSISYS;                                      /* Select HSI as the system clock */
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                                             /* AHB clock 1 division */
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;                                              /* APB clock 1 division */
  /* Configure clock source */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
  APP_ErrorHandler();
  }
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
void APP_ErrorHandler(void)
{
  while (1)
  {
  }
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
  /* User can add his own implementation to report the file name and line number,
   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
