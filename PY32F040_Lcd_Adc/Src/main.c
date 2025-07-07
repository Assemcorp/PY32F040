/*******************************************************************************
 * @file    main.c
 * @author  MCU Application Team
 * @author  AssemCorp - Emre Karabek - Field Application Engineer
 * @date    2025-June
 * @brief   Main program body
 ******************************************************************************
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
#include "lcd.h"
/* Private define ------------------------------------------------------------*/
#define TRUE 1
#define FALSE 0
#define RESET 0
#define ADC_SAMPLE_RATE 100
/* Private variables ---------------------------------------------------------*/
uint8_t running = TRUE;
uint16_t mainMenu;
uint8_t counter;
ADC_HandleTypeDef AdcHandle;
uint16_t aADCxConvertedData;
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_GpioConfig(void);
void APP_SystemClockConfig(void);
void APP_AdcConfig(void);
void APP_GpioConfig_Led(GPIO_TypeDef *port, uint16_t pin);
void APP_GpioConfig_Button(GPIO_TypeDef *port, uint16_t pin);
void APP_GpioConfig_Input(GPIO_TypeDef *port, uint16_t pin, uint32_t mode, uint32_t pull);
void LED_On(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void LED_Off(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
uint8_t Button_Read(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
/**
 * @brief  Main program.
 * @retval int
 */
int main(void)
{
  /* Reset all peripherals, initialize the Systick. */
  HAL_Init();
  APP_SystemClockConfig();
  APP_AdcConfig();
  /* Initialize GPIO */
  APP_GpioConfig_Led(GPIOA, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_4 | GPIO_PIN_5);
  APP_GpioConfig_Button(GPIOA, GPIO_PIN_7 | GPIO_PIN_6);
  APP_GpioConfig_Button(GPIOB, GPIO_PIN_0 | GPIO_PIN_11);
  APP_GpioConfig_Input(GPIOB, GPIO_PIN_1, GPIO_MODE_ANALOG, GPIO_NOPULL);

  // --- Initialize LCD and Print ---
  LCD_Init();
  LCD_SetCursor(firstLine, firstCol);
  LCD_Print("   AssemCorp  ");
  LCD_SetCursor(secondLine, firstCol);
  LCD_Print("Puya DevKit 2025");
  // -------------------------------

  static uint32_t delay_ms = 250; // Initial delay time

  while (1)
  {
    static uint32_t button_last_tick = 0;
    uint32_t button_debounce_delay = 200;
    uint32_t now = HAL_GetTick();
    HAL_ADC_Start_IT(&AdcHandle);

    if ((now - button_last_tick) > button_debounce_delay)
    {
      if (!(Button_Read(GPIOB, GPIO_PIN_0))) // X Button
      {
        running = FALSE;
        LCD_SetCursor(secondLine, firstCol);
        LCD_Print("Button 1    STOP");
        button_last_tick = now;
        mainMenu = 0; // Reset main menu
      }
      else if (!(Button_Read(GPIOB, GPIO_PIN_11))) // OK Button
      {
        running = TRUE;
        LCD_SetCursor(secondLine, firstCol);
        LCD_Print("Button 2     RUN");
        button_last_tick = now;
        mainMenu = 0; // Reset main menu
      }
      else if (!(Button_Read(GPIOA, GPIO_PIN_7))) // + Button
      {
        running = TRUE;
        if (delay_ms > 10) // Minimum limit
          delay_ms -= 10;  // Increase speed (toggle faster)
        LCD_SetCursor(secondLine, firstCol);
        LCD_Print("Speed: ");
        LCD_PrintNumber(delay_ms);
        LCD_Print(" ms   ");
        button_last_tick = now;
        mainMenu = 0; // Reset main menu
      }
      else if (!(Button_Read(GPIOA, GPIO_PIN_6))) // - Button
      {
        running = TRUE;
        if (delay_ms < 1000) // Maximum limit
          delay_ms += 10;    // Decrease speed (toggle slower)
        LCD_SetCursor(secondLine, firstCol);
        LCD_Print("Speed: ");
        LCD_PrintNumber(delay_ms);
        LCD_Print(" ms   ");
        button_last_tick = now;
        mainMenu = 0; // Reset main menu
      }
    }

    mainMenu++;
    if (mainMenu == 65000)
    {
      static uint32_t adc_sum = 0;
      static uint8_t adc_count = 0;
      static uint16_t adc_avg = 0;

      // Her döngüde ADC değerini topla ve sayacı artır
      adc_sum += aADCxConvertedData;
      adc_count++;

      if (adc_count >= ADC_SAMPLE_RATE)
      {
        adc_avg = adc_sum / ADC_SAMPLE_RATE;
        LCD_SetCursor(secondLine, firstCol);
        LCD_Print("ADC AVG: ");
        LCD_PrintNumber(adc_avg);
        LCD_Print("     ");
        adc_sum = 0;
        adc_count = 0;
      }
      mainMenu = 64999;
    }

    static uint32_t last_tick = 0;
    static uint8_t led_state = 0;

    uint32_t current_tick = HAL_GetTick();
    counter++;

    if (running)
    {
      // Toggle LEDs at adjustable speed
      if ((current_tick - last_tick) >= delay_ms)
      {
        last_tick = current_tick;
        led_state = !led_state;
        if (led_state)
        {
          HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
        }
      }
    }
    else
    {
      // Short wait for faster button control
      // Turn off LEDs
      LED_Off(GPIOA, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
    }
  }
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  aADCxConvertedData = hadc->Instance->DR;
}
/**
 * @brief  Error executing function.
 * @param  None
 * @retval None
 */
void APP_ErrorHandler(void)
{
  while (1)
  {
  }
}
/**
 * @brief  Configures LED pins as output.
 */
void APP_GpioConfig_Led(GPIO_TypeDef *port, uint16_t pin)
{
  if (port == GPIOB)
    __HAL_RCC_GPIOB_CLK_ENABLE();
  if (port == GPIOA)
    __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(port, &GPIO_InitStruct);
}

/**
 * @brief  Configures button pins as input.
 */
void APP_GpioConfig_Button(GPIO_TypeDef *port, uint16_t pin)
{
  if (port == GPIOB)
    __HAL_RCC_GPIOB_CLK_ENABLE();
  if (port == GPIOA)
    __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void APP_GpioConfig_Input(GPIO_TypeDef *port, uint16_t pin, uint32_t mode, uint32_t pull)
{
  if (port == GPIOB)
    __HAL_RCC_GPIOB_CLK_ENABLE();
  if (port == GPIOA)
    __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = pin;
  GPIO_InitStruct.Mode = mode;
  GPIO_InitStruct.Pull = pull;
  HAL_GPIO_Init(port, &GPIO_InitStruct);
}

/**
 * @brief  Turns on the LED(s).
 */
void LED_On(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
}

/**
 * @brief  Turns off the LED(s).
 */
void LED_Off(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
}
/**
 * @brief  System clock configuration function.
 * @param  None
 * @retval None
 */
void APP_SystemClockConfig(void)
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
 * @brief  ADC configuration function.
 * @param  None
 * @retval None
 */
void APP_AdcConfig()
{
  ADC_ChannelConfTypeDef sConfig = {0};
  RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInit = {0};

  /* Enable ADC clock */
  __HAL_RCC_ADC_CLK_ENABLE();

  RCC_PeriphCLKInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  RCC_PeriphCLKInit.ADCClockSelection = RCC_ADCCLKSOURCE_PCLK_DIV4;
  HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInit);

  AdcHandle.Instance = ADC1;

  AdcHandle.Init.Resolution = ADC_RESOLUTION_12B;
  AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  AdcHandle.Init.ScanConvMode = ADC_SCAN_DISABLE;
  AdcHandle.Init.ContinuousConvMode = DISABLE;
  AdcHandle.Init.NbrOfConversion = 1;
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;
  AdcHandle.Init.NbrOfDiscConversion = 1;
  AdcHandle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  if (HAL_ADC_Init(&AdcHandle) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  sConfig.Channel = ADC_CHANNEL_9; /* Select ADC channel 9 (PB1) */
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
  if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
  {
    APP_ErrorHandler();
  }
  if (HAL_ADCEx_Calibration_Start(&AdcHandle) != HAL_OK)
  {
    APP_ErrorHandler();
  }
}
/**
 * @brief  Reads the button state.
 * @retval 1 if pressed, 0 otherwise
 */
uint8_t Button_Read(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
  return (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_SET) ? 1 : 0;
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
  /* Users can add their own printing information as needed,
     for example: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* Infinite loop */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
