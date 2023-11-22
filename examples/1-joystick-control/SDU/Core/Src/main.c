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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "jci.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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




volatile bool new_data = false;
#define SERVO_COUNT 4
uint8_t servo_Ids[SERVO_COUNT] = {'0','1','y','p'}; //TODO: change IDS
TIM_HandleTypeDef* tim_ids[SERVO_COUNT] = {&htim2,&htim2,&htim2,&htim3};
//TODO: This is the wrong datatype but i don't know what to use
uint32_t tim_channels[SERVO_COUNT] = {TIM_CHANNEL_1,TIM_CHANNEL_3,TIM_CHANNEL_4,TIM_CHANNEL_1};
#define MIN_SERVO_POSITION  30 //should be 50 using spec sheet, but 30 is closer to the true value when testing
#define MAX_SERVO_POSITION 125
volatile uint16_t servo_positions[SERVO_COUNT] = {50,50,50,50}; //50 is defined as default position, can be changed





/****** JCI protocol ******/

#define MAX_JCI_PACKET_SIZE (3 + 256 * 3 + 1)

//DATA RECEIVER TX
uint8_t txpacket[MAX_JCI_PACKET_SIZE] = {0};

volatile jci_t jci_tx = {
		.TRANS = 'A',
		.CHECKSUM_EN = 1,
		.GRAN = 1,
		.PTYPE = 0,
		.PSIZE = 4,
		.SOURCE = 0,
		.CONT = 1
};

uint8_t txdata[SERVO_COUNT] = {0};
uint8_t txid[SERVO_COUNT] = {
		'0',
		'1',
		'y',
		'p'
};

//DATA RECEIVER RX
uint8_t rxpacket[MAX_JCI_PACKET_SIZE] = {0};
volatile jci_t jci_rx;
uint8_t rxdata[SERVO_COUNT] = {0};
uint8_t rxid[SERVO_COUNT] = {0};

volatile uint8_t num_agreed_id = 0;
uint8_t agreeid[SERVO_COUNT] = {0};



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
  MX_TIM2_Init();
  MX_UART4_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */


  PRINT("SDU Init\r\n");

  //Servo PWM timers
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);


  //Start RX
  HAL_UARTEx_ReceiveToIdle_IT(&huart4, rxpacket, MAX_JCI_PACKET_SIZE);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	PRINT("Entering Sleep\r\n");
	HAL_SuspendTick();
	HAL_PWR_EnableSleepOnExit();
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON,PWR_SLEEPENTRY_WFI);

	//This code should not be reached
	HAL_ResumeTick();
	PRINT("ERROR: Exited Sleep in MAIN\r\n");

	//	while(!new_data){}



//	if(new_data){
//		new_data = false;
//		//Loop through servos and update values
//		for(int i=0;i<SERVO_COUNT;i++){
//			if(__HAL_TIM_GET_COMPARE(tim_ids[i],tim_channels[i]) != servo_positions[i]){
//				__HAL_TIM_SET_COMPARE(tim_ids[i],tim_channels[i],servo_positions[i]);
//			}
//		}
//	}

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


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){

	if(huart == &huart4){

		uint8_t id_found;

		char trans = ' ';
		uint8_t* addr;

		char buffer[256];
		int rxpacketsize = 0;
		int txpacketsize = 0;

		//Check for JCI packet
		addr = jci_findPacket(rxpacket, Size, &trans);
		if(trans != ' '){

			//Parse packet
			rxpacketsize = jci_parsePacket(&jci_rx, rxdata, rxid, addr);


			if(rxpacketsize < 0){
				PRINT("INVALID PACKET RECEIVED\r\n");
			}
			else{ //valid packet


				if(jci_rx.TRANS == 'C'){ //receive C-flow data
					//NOTE: illegal 'C' packet are handled using the

					PRINT(buffer);
					for(int i = 0 ; i < jci_rx.PSIZE ; i++){
						sprintf(buffer, "ID%i: %c    DATA: %i\r\n", i, rxid[i], rxdata[i]);
						PRINT(buffer);
					}
					if(!new_data){
						//Set servo values
						for(int i = 0 ; i < num_agreed_id ; i++){
							for(int j=0; j < SERVO_COUNT; j++){
								if(agreeid[i]==servo_Ids[j]){
									servo_positions[j] = MIN_SERVO_POSITION + (uint16_t)(((float)rxdata[i])*((MAX_SERVO_POSITION-MIN_SERVO_POSITION))/255.0); //scale between min and max
								}
							}
						}
						new_data = true; //flag main process
					}

				}
				else if((jci_rx.TRANS == 'S') && (jci_rx.CONT == 1)){ //if request for C-flow, send ACK

					PRINT("C-flow REQUESTED\r\n");

					//Check the IDs requested for control to make sure they are valid
					jci_tx.CONT = 1; //accept by default
					for(int i = 0; i < jci_rx.PSIZE ; i++){
						id_found = 0;
						for(int j = 0; j < SERVO_COUNT ; j++){
							if(servo_Ids[j] == rxid[i]){

								id_found = 1;

								//update agreed ID list
								agreeid[num_agreed_id++] = servo_Ids[j];

								//update joint value if found
								servo_positions[j] = MIN_SERVO_POSITION + (uint16_t)(((float)rxdata[i])*((MAX_SERVO_POSITION-MIN_SERVO_POSITION))/255.0); //scale between min and max
								break;
							}
						}
						if(id_found == 0){
							PRINT("INVALID ID REQUESTED, REFUSING C-flow\r\n");
							jci_tx.CONT = 0; //refuse because invalid ID
							//continue cause there might be other valid IDs with data
						}
					}

					//Confirm the parameters (send back to agree)
					jci_tx.CHECKSUM_EN = jci_rx.CHECKSUM_EN;
					jci_tx.PTYPE = jci_rx.PTYPE;
					jci_tx.PSIZE = jci_rx.PSIZE;

					//Send the ACK
					jci_tx.TRANS = 'A';
					jci_tx.GRAN = 0; //response to 'S'

					txpacketsize = jci_buildPacket(&jci_tx, NULL, txid, txpacket);

					HAL_UART_Transmit(&huart4, txpacket, txpacketsize, HAL_MAX_DELAY);
					//Check for C-flow
					jci_confirmCFlow(&jci_tx, txid, &jci_rx, rxid);

				}
				else if(jci_rx.TRANS == 'S'){ //check if normal data

					if(!new_data){
						//Set servo values
						for(int i = 0 ; i < jci_rx.PSIZE ; i++){
							for(int j=0; j < SERVO_COUNT; j++){
								if(rxid[i]==servo_Ids[j]){
									servo_positions[j] = MIN_SERVO_POSITION + (uint16_t)(((float)rxdata[i])*((MAX_SERVO_POSITION-MIN_SERVO_POSITION))/255.0); //scale between min and max
								}
							}
						}
						new_data = true; //flag main process
					}
				}
			}
			for(int i = 0 ; i < jci_rx.PSIZE ; i++){
				sprintf(buffer, "ID%i: %c    DATA: %i\r\n", i, rxid[i], rxdata[i]);
				PRINT(buffer);
			}
		}

		//Update servo positions if new data is received
		if(new_data){
			for(int i=0;i<SERVO_COUNT;i++){
				if(__HAL_TIM_GET_COMPARE(tim_ids[i],tim_channels[i]) != servo_positions[i]){
					__HAL_TIM_SET_COMPARE(tim_ids[i],tim_channels[i],servo_positions[i]);
				}
			}
			new_data = false;
		}

		//Wait for next packet
		HAL_UARTEx_ReceiveToIdle_IT(&huart4, rxpacket, MAX_JCI_PACKET_SIZE);

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
