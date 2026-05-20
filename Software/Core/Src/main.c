#include "main.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include <stdio.h>

#define buttonPin GPIO_PIN_9
#define powerPin GPIO_PIN_8

uint32_t AD_RES_SUM = 0;
uint16_t AD_RES = 5;      //0 - 4095
const double Vadc = 3.28;
const double Rsense = 0.05;
const double Gain = 20.0;


double Vout = 0;
double Iload = 0;
char output[10] = "abc";

I2C_HandleTypeDef hi2c1;
ADC_HandleTypeDef hadc1;
const uint16_t powerStateX = 52;
const uint16_t powerStateY = 7;
const uint16_t ampereMeterX = 30;
const uint16_t ampereMeterY = 20;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC1_Init(void);
void changePowerState();
void shortDetected();
void updateAmpereMeter();


uint32_t timeStart;

int main(void)
{
  
  HAL_Init();
  
  SystemClock_Config();
  
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  ssd1306_Init(); // initialise the display 
  
  ssd1306_SetCursor(powerStateX,powerStateY); // goto 10, 10
  ssd1306_WriteString("ON ", Font_7x10, White);
  ssd1306_UpdateScreen();   
  HAL_GPIO_WritePin(GPIOB, powerPin, GPIO_PIN_SET);
  
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADC_Start_IT(&hadc1);
  timeStart = HAL_GetTick();
  while (1)
  {
    if (HAL_GPIO_ReadPin(GPIOB, buttonPin) == GPIO_PIN_RESET) changePowerState();
    updateAmpereMeter();
    
    }
      
  
  }
  



void updateAmpereMeter(){
  Vout = (AD_RES * Vadc) / 4095.0;
  Iload = Vout / (Gain * Rsense);
  if (Iload > 1.6) shortDetected();
  if ((HAL_GetTick() - timeStart) >= 50) {
    snprintf(output, sizeof(output), "%.3f", Iload);
    ssd1306_SetCursor(ampereMeterX,ampereMeterY);
    ssd1306_WriteString(output, Font_11x18, White);
    ssd1306_UpdateScreen();
    timeStart = HAL_GetTick();
  }
}
void shortDetected(){
  snprintf(output, sizeof(output), "%.3f", Iload);
  ssd1306_SetCursor(ampereMeterX,ampereMeterY);
  ssd1306_WriteString(output, Font_11x18, White);
  ssd1306_UpdateScreen();
  changePowerState();
  uint16_t color = 1;
  uint32_t timeStart = HAL_GetTick();
  while(1){
    ssd1306_SetCursor(10,50);
    if (!HAL_GPIO_ReadPin(GPIOB, buttonPin)){
      ssd1306_WriteString("      ", Font_7x10, White);
      ssd1306_UpdateScreen();
      changePowerState();
      break;
    }
    if ((HAL_GetTick() - timeStart) >= 500){
      ssd1306_WriteString("short", Font_7x10, color);
      ssd1306_UpdateScreen();
      if (color == 1) color = 0;
      else color = 1;
      timeStart = HAL_GetTick();
    }

  }
}

void changePowerState(){
  while (HAL_GPIO_ReadPin(GPIOB, buttonPin) == 0);
  HAL_GPIO_TogglePin(GPIOB, powerPin);
  ssd1306_SetCursor(powerStateX,powerStateY);
  if (HAL_GPIO_ReadPin(GPIOB, powerPin) == 0) ssd1306_WriteString("OFF", Font_7x10, White);
  else ssd1306_WriteString("ON ", Font_7x10, White);
  ssd1306_UpdateScreen();
}

uint16_t shortCounter = 0;
uint16_t shortCounterBoundary = 10;

/*
void checkForShort(uint16_t ADC_res){
  Vout = (ADC_res*3.3)/4095;
  Iload = ((Vout-Vref)/Gain)/0.1;
  if (Iload < 0) Iload*=-1;
  if (Iload > 1.7) shortCounter +=1;
  if (shortCounter >= shortCounterBoundary){
    changePowerState();
    shortCounter = 0;
  }
}
*/
uint16_t counter = 0;
const uint16_t boundary = 64;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    //checkForShort(HAL_ADC_GetValue(&hadc1));
    // Read & Update The ADC Result
    AD_RES_SUM += HAL_ADC_GetValue(&hadc1);
    counter+=1;
    if (counter >= boundary){
      AD_RES = AD_RES_SUM/boundary;
      counter = 0; 
      AD_RES_SUM = 0;
    }
    
}

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */



void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB1 PB2 PB10 PB11
                           PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
