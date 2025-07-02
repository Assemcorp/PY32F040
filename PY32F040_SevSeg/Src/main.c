  /*******************************************************************************
    * @file    main.c
    * @author  MCU Application Team
    * @author  AssemCorp - Emre Karabek - Field Application Engineer
    * @date    2025-June
    * @brief   Main program body
    *******************************************************************************/
  #include "main.h"

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

  // Segment map (for common cathode)
  const uint8_t SEGMENT_MAP[10] = {
      0x3F, // 0: a, b, c, d, e, f
      0x06, // 1: b, c
      0x5B, // 2: a, b, d, e, g
      0x4F, // 3: a, b, c, d, g
      0x66, // 4: b, c, f, g
      0x6D, // 5: a, c, d, f, g
      0x7D, // 6: a, c, d, e, f, g
      0x07, // 7: a, b, c
      0x7F, // 8: a, b, c, d, e, f, g
      0x6F  // 9: a, b, c, d, f, g
  };

  void SystemClock_Config(void);
  void GPIO_Init(void);
  void DisplayNumber(uint8_t number);

  void LED_Init(void);
  void LED_Sequence(void);
  void DisplayDigit(uint8_t digit);
  void Button_Init(void);
  void GPIO_PWM_A9_Init(void);
  void MX_TIM1_Init(void);
  void MX_USART1_UART_Init(void);
  void GPIO_UART1_TX_Init(void);

  uint8_t LedBlink = 0;
  uint8_t counter_running = 1; // Counter is running at start
  UART_HandleTypeDef huart1; // <-- Global definition
  TIM_HandleTypeDef htim1;

  int main(void)
  {
      HAL_Init();
      SystemClock_Config();
      GPIO_Init();
      LED_Init();
      Button_Init();
      GPIO_PWM_A9_Init();
      MX_TIM1_Init();
      GPIO_UART1_TX_Init();
      MX_USART1_UART_Init();

      uint8_t number = 0;
      const char msg[] = "AssemCorp\r\n";
      
      while (1)
      {
          // Button control
          if (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_STOP_PIN) == GPIO_PIN_RESET)
              counter_running = FALSE; // Stop counter
          if (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_START_PIN) == GPIO_PIN_RESET)
              counter_running = TRUE; // Resume counter

          DisplayNumber(number);

          if (counter_running)
              number = (number + 1) % 100; // Loop between 0-99
                  
          LedBlink++;
          
          if (LedBlink==10) HAL_GPIO_WritePin(LED_PORT_B, GPIO_PIN_11, GPIO_PIN_SET);
          if (LedBlink==20) HAL_GPIO_WritePin(LED_PORT_B, GPIO_PIN_12, GPIO_PIN_SET);
          if (LedBlink==30) HAL_GPIO_WritePin(LED_PORT_B, GPIO_PIN_13, GPIO_PIN_SET);
          if (LedBlink==40) HAL_GPIO_WritePin(LED_PORT_B, GPIO_PIN_14, GPIO_PIN_SET);
          if (LedBlink==50) HAL_GPIO_WritePin(LED_PORT_B, GPIO_PIN_15, GPIO_PIN_SET);
          if (LedBlink==60) HAL_GPIO_WritePin(LED_PORT_A, GPIO_PIN_8, GPIO_PIN_SET);
          if (LedBlink==70) {    
              HAL_GPIO_WritePin(LED_PORT_B, LED_PINS_B, GPIO_PIN_RESET);
              HAL_GPIO_WritePin(LED_PORT_A, LED_PIN_A8, GPIO_PIN_RESET);
              LedBlink=0;    
              HAL_UART_Transmit(&huart1, (uint8_t*)msg, sizeof(msg)-1, HAL_MAX_DELAY);
          }
          
          
      }
  }

  /**
    * @brief  Initializes GPIO pins.
    * @retval None
    */
  void GPIO_Init(void)
  {
      GPIO_InitTypeDef GPIO_InitStruct = {0};

      // Enable GPIOA and GPIOB clocks
      __HAL_RCC_GPIOA_CLK_ENABLE();
      __HAL_RCC_GPIOB_CLK_ENABLE();

      // Set segment pins as output
      GPIO_InitStruct.Pin = SEG_A_PIN | SEG_F_PIN | SEG_G_PIN;
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

  void DisplayDigit(uint8_t digit)
  {
      uint8_t segments = SEGMENT_MAP[digit];

      // Set segments
      HAL_GPIO_WritePin(SEG_PORT_A, SEG_A_PIN, (segments & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(SEG_PORT_B, SEG_B_PIN, (segments & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(SEG_PORT_B, SEG_C_PIN, (segments & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(SEG_PORT_B, SEG_D_PIN, (segments & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(SEG_PORT_B, SEG_E_PIN, (segments & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(SEG_PORT_A, SEG_F_PIN, (segments & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(SEG_PORT_A, SEG_G_PIN, (segments & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }

  /**
    * @brief  Displays a two-digit number on the 7-segment display.
    * @param  number: Number to display (0-99).
    * @retval None
    */
   void DisplayNumber(uint8_t number)
   {
       uint8_t digit1 = number / 10; // Tens digit
       uint8_t digit2 = number % 10; // Ones digit
   
       for (int i = 0; i < 10; i++) // Total 10 cycles for both digits
       {
           // Show digit 1
           HAL_GPIO_WritePin(DIGIT_PORT, DIGIT1_PIN, GPIO_PIN_RESET); // Digit 1 active
           HAL_GPIO_WritePin(DIGIT_PORT, DIGIT2_PIN, GPIO_PIN_SET);   // Digit 2 inactive
           uint8_t segments = SEGMENT_MAP[digit1];
           HAL_GPIO_WritePin(SEG_PORT_A, SEG_A_PIN, (segments & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
           HAL_GPIO_WritePin(SEG_PORT_B, SEG_B_PIN, (segments & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
           HAL_GPIO_WritePin(SEG_PORT_B, SEG_C_PIN, (segments & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
           HAL_GPIO_WritePin(SEG_PORT_B, SEG_D_PIN, (segments & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
           HAL_GPIO_WritePin(SEG_PORT_B, SEG_E_PIN, (segments & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
           HAL_GPIO_WritePin(SEG_PORT_A, SEG_F_PIN, (segments & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
           HAL_GPIO_WritePin(SEG_PORT_A, SEG_G_PIN, (segments & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
           HAL_Delay(1); // Small delay (1 ms)
   
           // Show digit 2
           HAL_GPIO_WritePin(DIGIT_PORT, DIGIT1_PIN, GPIO_PIN_SET);   // Digit 1 inactive
           HAL_GPIO_WritePin(DIGIT_PORT, DIGIT2_PIN, GPIO_PIN_RESET); // Digit 2 active
           segments = SEGMENT_MAP[digit2];
           HAL_GPIO_WritePin(SEG_PORT_A, SEG_A_PIN, (segments & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
           HAL_GPIO_WritePin(SEG_PORT_B, SEG_B_PIN, (segments & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
           HAL_GPIO_WritePin(SEG_PORT_B, SEG_C_PIN, (segments & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
           HAL_GPIO_WritePin(SEG_PORT_B, SEG_D_PIN, (segments & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
           HAL_GPIO_WritePin(SEG_PORT_B, SEG_E_PIN, (segments & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
           HAL_GPIO_WritePin(SEG_PORT_A, SEG_F_PIN, (segments & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
           HAL_GPIO_WritePin(SEG_PORT_A, SEG_G_PIN, (segments & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
           HAL_Delay(1); // Small delay (1 ms)
       }
   }

  /**
    * @brief  Configures the system clock.
    * @retval None
    */
  void SystemClock_Config(void)
  {
      RCC_OscInitTypeDef RCC_OscInitStruct = {0};
      RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

      // Enable HSI Oscillator
      RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
      RCC_OscInitStruct.HSIState = RCC_HSI_ON;
      RCC_OscInitStruct.HSICalibrationValue = 16;
      RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
      if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
      {
          while (1); // Infinite loop on error
      }

      // Configure CPU, AHB and APB clocks
      RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
      RCC_ClkInitStruct.SYSCLKSource = 0;
      RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
      RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

      if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
      {
          while (1); // Infinite loop on error
      }
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

      // Set LED pin (A8) on GPIOA as output
      GPIO_InitStruct.Pin = LED_PIN_A8;
      HAL_GPIO_Init(LED_PORT_A, &GPIO_InitStruct);
  }

  void Button_Init(void)
  {
      GPIO_InitTypeDef GPIO_InitStruct = {0};
      // Set button pins as input (with pull-up)
      GPIO_InitStruct.Pin = BUTTON_STOP_PIN | BUTTON_START_PIN;
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      HAL_GPIO_Init(BUTTON_PORT, &GPIO_InitStruct);
  }

  void MX_TIM1_Init(void)
  {
      TIM_OC_InitTypeDef sConfigOC = {0};

      __HAL_RCC_TIM1_CLK_ENABLE(); // <-- ADD THIS
      
      // TIM1 basic configuration
      htim1.Instance = TIM1;
      htim1.Init.Prescaler = 1; // 64 MHz / (63+1) = 1 MHz timer clock
      htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
      htim1.Init.Period = 1120;   // 1 MHz / (999+1) = 1 kHz PWM
      htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
      htim1.Init.RepetitionCounter = 0;
      htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
      HAL_TIM_PWM_Init(&htim1);

      // PWM output configuration
      sConfigOC.OCMode = TIM_OCMODE_PWM1;
      sConfigOC.Pulse = 560; // 50% duty cycle
      sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
      sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
      HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2);

      // Start PWM
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  }

  void GPIO_PWM_A9_Init(void)
  {
      GPIO_InitTypeDef GPIO_InitStruct = {0};
      __HAL_RCC_GPIOA_CLK_ENABLE();

      // Set A9 as alternate function (TIM1_CH2)
      GPIO_InitStruct.Pin = GPIO_PIN_9;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }

  void MX_USART1_UART_Init(void)
  {
      __HAL_RCC_USART1_CLK_ENABLE(); // <-- ADD THIS

      huart1.Instance = USART1;
      huart1.Init.BaudRate = 9600;
      huart1.Init.WordLength = UART_WORDLENGTH_8B;
      huart1.Init.StopBits = UART_STOPBITS_1;
      huart1.Init.Parity = UART_PARITY_NONE;
      huart1.Init.Mode = UART_MODE_TX_RX;
      huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
      huart1.Init.OverSampling = UART_OVERSAMPLING_16;
      HAL_UART_Init(&huart1);
  }

  void GPIO_UART1_TX_Init(void)
  {
      GPIO_InitTypeDef GPIO_InitStruct = {0};
      __HAL_RCC_GPIOB_CLK_ENABLE();

      // PB8 -> USART1_TX (AF1)
      GPIO_InitStruct.Pin = GPIO_PIN_8;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      GPIO_InitStruct.Alternate = GPIO_AF1_USART1;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }