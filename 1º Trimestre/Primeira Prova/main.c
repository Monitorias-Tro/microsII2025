/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/*
 	Each LED has a corresponding struct with its counter, kickoff (decreasing counter start
	value) which is the key to set the LEDs frequency, pin to which it's connected and a name
	string for the UI on the serial terminal.
*/
typedef struct {
	int counter;
	int kickoff;
	uint16_t pin;
	char name[9];
} LEDTimer_t;

/*
	TIM11 was set to 1041Hz, so T = 960,7us
	For the amount of pulses to obtain a desired toggle period, divide it by 960,7us

	16Hz LED -> 32Hz toggle period -> kickoff = 32
	13Hz LED -> 26Hz toggle period -> kickoff = 40
	5Hz LED -> 10Hz toggle period -> kickoff = 104
	2Hz LED -> 4Hz toggle period -> kickoff = 260
 */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim10;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
int ctrl=0, BS_index = 0;	//Control value and index of the LEDTimer array (for LED selection):
char ctrl_char = ctrl + '0';  //printable control variable
char rx_data[2];	//Rx string
LEDTimer_t LEDTimer[3];	//Red is 0, Yellow is 1 and Green is 2 (I miss Python dicts :(

char ctrlUpdateMsg[] = {"\n\rControle = "};
char selectLEDUpdateMsg[] = {"\n\rLED selecionado = "};
char invalidLEDSelectionMsg[] = {"\n\rLED invalido"};
char invalidFrequencySelectionMsg[] = {"\n\rFrequencia invalida"};
char newLine[] = {"\n\r"};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM10_Init(void);
/* USER CODE BEGIN PFP */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void setColourNames();
int BFIsPressed();
int BSIsPressed();
int toKickoff(int control);
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
	//initialize all LEDs as deactivated
	for(int i=0; i<3; i++){
		LEDTimer[i].counter = -1;
		LEDTimer[i].kickoff = -1;
	}

	//set each colour to its pin
	LEDTimer[0].pin = RedLED_Pin;
	LEDTimer[1].pin = YellowLED_Pin;
	LEDTimer[2].pin = GreenLED_Pin;

	setColourNames();	//set colour names (bruh)
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
  MX_USART2_UART_Init();
  MX_TIM10_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim10);
  HAL_UART_Receive_IT(&huart2, &rx_data, 2);

  //print initial state
  HAL_UART_Transmit(&huart2, (uint8_t*)ctrlUpdateMsg, strlen(ctrlUpdateMsg), HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart2, &ctrl_char, 1, HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart2, (uint8_t*)selectLEDUpdateMsg, strlen(selectLEDUpdateMsg), HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart2, (uint8_t*)LEDTimer[BS_index].name, strlen(LEDTimer[BS_index].name), HAL_MAX_DELAY);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(BFIsPressed()){
		  ctrl++;
		  if(ctrl>4)	ctrl = 0;
		  ctrl_char = ctrl + '0';

		  //print new ctrl value
		  HAL_UART_Transmit(&huart2, (uint8_t*)ctrlUpdateMsg, strlen(ctrlUpdateMsg), HAL_MAX_DELAY);
		  HAL_UART_Transmit(&huart2, &ctrl_char, 1, HAL_MAX_DELAY);

		  LEDTimer[BS_index].kickoff = toKickoff(ctrl);				//update kickoff value
		  LEDTimer[BS_index].counter = LEDTimer[BS_index].kickoff;	//reset counter
	  }

	  if(BSIsPressed()){
		  BS_index++;
		  if(BS_index>2)	BS_index = 0;

		  //print new selected LED
		  HAL_UART_Transmit(&huart2, (uint8_t*)selectLEDUpdateMsg, strlen(selectLEDUpdateMsg), HAL_MAX_DELAY);
		  HAL_UART_Transmit(&huart2, (uint8_t*)LEDTimer[BS_index].name, strlen(LEDTimer[BS_index].name), HAL_MAX_DELAY);
	  }
    /* USER CODE END WHILE */

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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
}

/**
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = PSC_1040Hz;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = ARR;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GreenLED_Pin|YellowLED_Pin|RedLED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GreenLED_Pin YellowLED_Pin RedLED_Pin */
  GPIO_InitStruct.Pin = GreenLED_Pin|YellowLED_Pin|RedLED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : BF_Pin BS_Pin */
  GPIO_InitStruct.Pin = BF_Pin|BS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if (huart->Instance == USART2){
		//echo
		HAL_UART_Transmit(&huart2, (uint8_t*)newLine, strlen(newLine), HAL_MAX_DELAY);
		HAL_UART_Transmit(&huart2, (uint8_t*)rx_data, strlen(rx_data), HAL_MAX_DELAY);

		switch(rx_data[0]){
		case 'r':
			LEDTimer[0].kickoff = toKickoff((int)rx_data[1]);
			break;
		case 'y':
			LEDTimer[1].kickoff = toKickoff((int)rx_data[1]);
			break;
		case 'g':
			LEDTimer[2].kickoff = toKickoff((int)rx_data[1]);
			break;
		default:
			//transmit invalid LED message
			HAL_UART_Transmit(&huart2, (uint8_t*)invalidLEDSelectionMsg, strlen(invalidLEDSelectionMsg), HAL_MAX_DELAY);
			break;
		}

		HAL_UART_Receive_IT(&huart2, &rx_data, 2);	//restart reception of next bytes
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == TIM10){
		for(int i=0; i<3; i++){
			//if LED is active
			if(LEDTimer[i].kickoff > -1){
				//decrement LED counter
				LEDTimer[i].counter--;
				//if the timer is complete, toggle pin and reset counter
				if(LEDTimer[i].counter<=0){
					LEDTimer[i].counter = LEDTimer[i].kickoff;
					HAL_GPIO_TogglePin(GPIOC, LEDTimer[i].pin);
				}
			}

			//turn off otherwise
			else	HAL_GPIO_WritePin(GPIOC, LEDTimer[i].pin, RESET);
		}
	}
}

int BFIsPressed(){
	int debounce = 1000;
	int return_value;

	//when BS be read as "SET" for 1000 times in a row, return that BS is pressed
	while(1){
		if(HAL_GPIO_ReadPin(GPIOA, BF_Pin)){
			debounce--;
		}
		else{
			return_value=0;
			break;
		}

		if(debounce==0){
			return_value = 1;
			break;
		}
	}

	while(HAL_GPIO_ReadPin(GPIOA, BF_Pin));	//wait release of the button to exit the function
	return return_value;
}

int BSIsPressed(){
	int debounce = 1000;
	int return_value;

	//when BS be read as "SET" for 1000 times in a row, return that BS is pressed
	while(1){
		if(HAL_GPIO_ReadPin(GPIOA, BS_Pin)){
			debounce--;
		}
		else{
			return_value=0;
			break;
		}

		if(debounce==0){
			return_value = 1;
			break;
		}
	}

	while(HAL_GPIO_ReadPin(GPIOA, BS_Pin));	//wait release of the button to exit the function
	return return_value;
}

int toKickoff(int control){
	int freq;

	//translate ctrl command to corresponding frequency
	switch(control){
	case 1:
		freq = 2;
		break;
	case 2:
		freq = 5;
		break;
	case 3:
		freq = 13;
		break;
	case 4:
		freq = 16;
		break;
	default:
		freq = -1;
		//transmit invalid frequency message
		HAL_UART_Transmit(&huart2, (uint8_t*)invalidFrequencySelectionMsg, strlen(invalidFrequencySelectionMsg), HAL_MAX_DELAY);
		break;
	}

	/*
	 Reminder:

	 16Hz LED -> 32Hz timer -> kickoff = 32
	 13Hz LED -> 26Hz timer -> kickoff = 40
	 5Hz LED -> 10Hz timer -> kickoff = 104
	 2Hz LED -> 4Hz timer -> kickoff = 260
	 */

	//translate frequency to the amount of clock pulses to toggle pin (kickoff)
	switch(freq){
	case 16:
		return 32;
	case 13:
		return 40;
	case 5:
		return 104;
	case 2:
		return 260;
	default:
		return -1;
	}
}

void setColourNames(){
	LEDTimer[0].name[0] = 'V';
	LEDTimer[0].name[1] = 'e';
	LEDTimer[0].name[2] = 'r';
	LEDTimer[0].name[3] = 'm';
	LEDTimer[0].name[4] = 'e';
	LEDTimer[0].name[5] = 'l';
	LEDTimer[0].name[6] = 'h';
	LEDTimer[0].name[7] = 'o';
	LEDTimer[0].name[8] = '\0';	//assign last character so "strlen()" knows the size of each colour name

	LEDTimer[1].name[0] = 'A';
	LEDTimer[1].name[1] = 'm';
	LEDTimer[1].name[2] = 'a';
	LEDTimer[1].name[3] = 'r';
	LEDTimer[1].name[4] = 'e';
	LEDTimer[1].name[5] = 'l';
	LEDTimer[1].name[6] = 'o';
	LEDTimer[1].name[7] = '\0';

	LEDTimer[2].name[0] = 'V';
	LEDTimer[2].name[1] = 'e';
	LEDTimer[2].name[2] = 'r';
	LEDTimer[2].name[3] = 'd';
	LEDTimer[2].name[4] = 'e';
	LEDTimer[2].name[5] = '\0';
}
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
