/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "jci.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define PRINT(s) HAL_UART_Transmit(&huart1, s, strlen(s), HAL_MAX_DELAY)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */


/****** JCI protocol ******/

#define MAX_JCI_PACKET_SIZE (3 + 256 * 3 + 1)

//DATA SENDER TX
uint8_t txpacket[MAX_JCI_PACKET_SIZE] = {0};

volatile jci_t jci_tx = {
		.TRANS = 'S',
		.CHECKSUM_EN = 1,
		.GRAN = 1,
		.PTYPE = 0,
		.PSIZE = 4,
		.SOURCE = 0,
		.CONT = 1
};
uint8_t txdata[4] = {
		4,
		5,
		90,
		156
};
uint8_t txid[4] = {
		'0',
		'1',
		'y',
		'p'
};

//DATA SENDER RX
uint8_t rxpacket[MAX_JCI_PACKET_SIZE] = {0};
volatile jci_t jci_rx;
uint8_t rxdata[4] = {0};
uint8_t rxid[4] = {0};
volatile uint8_t rxflag = 0;




/****** State Logic ******/
typedef enum{
	STOPPED,
	PLAYBACK,
	REALTIME
}state_e;

volatile uint8_t state = STOPPED;	//state flag
volatile uint8_t recording = 0; 	//flag to indicate if recording


/****** Joystick ******/

//DMA storing address, array length of 4:
//Joy1_X, Joy1_Y, Joy2_X, Joy2_Y
volatile uint8_t joysticksVal[4];
volatile uint8_t conversionFlag = 0; //flag wait for conversion
char buff[200];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
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
  MX_DMA_Init();
  MX_UART4_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */



  //JCI packets
  int txpacketsize = 0;
  uint8_t cflow = 0;

  //Playback recording
  uint32_t timediff = 0;
  uint8_t flash_buffer[2*MAX_JCI_PACKET_SIZE] = {0};

  	PRINT("CTU init\r\n");


    //Start RX
    HAL_UARTEx_ReceiveToIdle_IT(&huart4, rxpacket, MAX_JCI_PACKET_SIZE);


    //Initial time sampling
    timediff = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	//TODO CLI and state logic (stopped, playback, real-time)

	  switch (state)
	  {
	  	  case STOPPED:
	  			//_WFI(); //TODO for interrupt/event (low-power)
	  		  //TODO are we changing mode or just random interrupt?
	  	  case PLAYBACK:
	  		  //TODO play back what's in the Flash



	  	  case REALTIME:

			  //Joystick control values
			  jci_getPot();


			  //JCI packet
			  if(jci_tx.CONTACCEPT){
				  if(cflow == 0){
					  PRINT("ENTERING C-flow\r\n");
					  cflow = 1;
				  }
				  //C-flow obtained
				  jci_tx.TRANS = 'C';
			  }else{
				  cflow = 0;
				  //Back to S packets
				  jci_tx.TRANS = 'S';
			  }
			  txpacketsize = jci_buildPacket(&jci_tx, joysticksVal, txid, txpacket);
			  if(txpacketsize == 0){
				  PRINT("ERROR: build packet\r\n");
			  }else{
				  HAL_UART_Transmit(&huart4, txpacket, txpacketsize, HAL_MAX_DELAY);
			  }


			  //Rewrite over previous TX packet shown using "\r"
			  PRINT("\r");
			  ui_showPacket(jci_tx, joysticksVal, txid, "\r");

			  //Check for RX packet
			  if(rxflag){
				  rxflag = 0;

				  PRINT("\r\nSDU PACKET RECEIVED:\r\n");
				  ui_showPacket(jci_rx, rxdata, rxid);
				  PRINT("\r\n");

				  //Wait for next packet
				  HAL_UARTEx_ReceiveToIdle_IT(&huart4, rxpacket, MAX_JCI_PACKET_SIZE);
			  }


			  //CLI current data and IDs for joystick values
			  //TODO cleaner CLI above
			  sprintf(buff, "Value of Joystick 1 - X: %d\r\nValue of Joystick 1 - Y: %d\r\nValue of Joystick 2 - X: %d\r\nValue of Joystick 2 - Y: %d\r\n",
			  	joysticksVal[0],joysticksVal[1],joysticksVal[2],joysticksVal[3]);
			  PRINT(buff);



			  //Playback recording
			  if(recording){ //start or continue recording
				  //Record to Flash with time passed from previous sample
				  //to reproduce the delays accurately

				  //Copy packet data
				  memcpy(flash_buffer, txpacket, txpacketsize);

				  //Copy delay
				  timediff = HAL_GetTick() - timediff;
				  memcpy(flash_buffer + txpacketsize, &timediff, 4);
				  timediff = HAL_GetTick();

				  //Store into Flash
				  //TODO

				  //TODO check if full, in which case stop recording and indicate
				  //that it's full
				  //if(full){
				  //  recording = ~recording;
				  //  ui_flashFull()
				  //}

			  }else{ //stop recording

			  }

			  HAL_Delay(10);
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	if(GPIO_Pin == BUTTON_INT_Pin){

		//Toggle the recording state
		recording = ~recording;

	}

}


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){


	if(huart == &huart4){

		char trans = ' ';
		uint8_t* addr;

		char buffer[256];
		int rxpacketsize = 0;

		//Check for JCI packet
		addr = jci_findPacket(rxpacket, Size, &trans);
		if(trans != ' '){

			//Parse packet
			rxpacketsize = jci_parsePacket(&jci_rx, rxdata, rxid, addr);

			//Check for C-flow
			jci_confirmCFlow(&jci_tx, txid, &jci_rx, rxid);

			//Indicate that packet is received
			rxflag = 1;
		}

	}

}




//function to update the joystick values
void jci_getPot(void) {

	//calibrate ADC
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED); //single ended reference if we look at the figures of the ADC

	//start ADC DMA
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)joysticksVal, 4);

	while(conversionFlag == 0) {
		//wait for conversion to end
	}

	//clear flag
	conversionFlag = 0;

	//update values of txdata TODO
//	for (int i = 0; i < 4; i++) {
//		txdata_u16[i] = joysticksVal[i];
//	}

//	sprintf(buff, "Value of Joystick 1 - X: %d\r\nValue of Joystick 1 - Y: %d\r\nValue of Joystick 2 - X: %d\r\nValue of Joystick 2 - Y: %d\r\n",
//			joysticksVal[0],joysticksVal[1],joysticksVal[2],joysticksVal[3]);
//	PRINT(buff);


	//stop ADC DMA (not sure if needed)
	//HAL_ADC_Stop_DMA(&hadc1);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {

	if (hadc == &hadc1) {
	//send 1 when conversion flag is done
	conversionFlag = 1;
	}
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
