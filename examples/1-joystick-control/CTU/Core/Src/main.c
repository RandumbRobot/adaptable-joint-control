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
#include "octospi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "jci.h"

#include "stm32l4s5i_iot01_qspi.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define PRINT(s) HAL_UART_Transmit(&huart1, s, strlen(s), HAL_MAX_DELAY)
#define UIPRINT(s) HAL_UART_Transmit_DMA(&huart1, s, strlen(s))

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
	REALTIME,
	PLAYBACK
}state_e;

volatile uint8_t state = REALTIME;	//state flag
volatile uint8_t recording = 0; 	//flag to indicate if recording
volatile uint8_t full = 0;


/****** Joystick ******/

//DMA storing address, array length of 4:
//Joy1_X, Joy1_Y, Joy2_X, Joy2_Y
volatile uint8_t joysticksVal[4];
volatile uint8_t conversionFlag = 0; //flag wait for conversion
char buff[200];



/****** Flash ******/

#define FLASH_BLOCK_SIZE 1280000
volatile uint8_t* flash_write_ptr = 0;
volatile uint8_t* flash_read_ptr = 0;



/****** UI ******/
void ui_printTxData(jci_t jci, uint8_t* data, uint8_t* ids, uint8_t state, uint8_t rec);
#define MAX_UI_DELAY 100
uint32_t ui_timediff;

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
  MX_OCTOSPI1_Init();
  /* USER CODE BEGIN 2 */



  //JCI packets
  int txpacketsize = 0;
  uint8_t cflow = 0;

  //Playback recording
  uint32_t timediff = 0;
  uint8_t flash_buffer[2*MAX_JCI_PACKET_SIZE] = {0};

  	PRINT("\r\n/***** CTU init *****/\r\n");


    //Start RX
    HAL_UARTEx_ReceiveToIdle_IT(&huart4, rxpacket, MAX_JCI_PACKET_SIZE);


    //Initial time sampling
    timediff = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {


	  /***** STATE CONTROL AND DATA ACQUISITION *****/
	  switch (state)
	  {
	  	  case STOPPED:

	  		  UIPRINT("\rSTOPPED                                                                       ");
	  		  //Put to sleep
	  		  HAL_SuspendTick();
	  		  HAL_PWR_EnableSleepOnExit();
	  		  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON,PWR_SLEEPENTRY_WFI);

	  		  //Only comes out to main if change of state
	  		  HAL_ResumeTick();

	  		  //Reset time difference to current tick
	  		  timediff = HAL_GetTick();

	  		  //Reset recording state
	  		  recording = 0;
	  		  full = 0;
	  		  flash_write_ptr = 0;

	  		  break;

	  	  case PLAYBACK:

	  		  if(flash_read_ptr < flash_write_ptr){ //if not done reading

	  			//Read back joystick data
	  			BSP_QSPI_Read(flash_buffer, flash_read_ptr, 8);

	  			//Update flash pointer
	  			flash_read_ptr += 8;

	  			//Copy pot data
	  			memcpy(joysticksVal, flash_buffer, 4);

	  			//Copy delay
	  			memcpy(&timediff, flash_buffer + 4, 4);


	  		  }else{
	  			  //Playback done
	  			  recording = 2;
	  		  }
	  		  break;



	  	  case REALTIME:

			  //Joystick control values
			  jci_getPot();

			  break;
	  }


	  /***** JCI TX/RX *****/
	  //JCI tx packet
	  if(jci_tx.CONTACCEPT){
		  if(cflow == 0){
			  PRINT("\r\nENTERING C-flow\r\n");
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
		  PRINT("\r\nERROR: build packet\r\n");
	  }else{
		  HAL_UART_Transmit(&huart4, txpacket, txpacketsize, HAL_MAX_DELAY);
	  }

//	  //Check for JCI RX packet
//	  if(rxflag){
//		  rxflag = 0;
//
//		  ui_printRxData(jci_rx, rxdata, rxid);
//
//		  //Wait for next packet
//		  HAL_UARTEx_ReceiveToIdle_IT(&huart4, rxpacket, MAX_JCI_PACKET_SIZE);
//	  }


	  /***** RECORDING *****/
	  //Playback recording
	  if(recording == 1){ //start or continue recording
		  //Record to Flash with time passed from previous sample
		  //to reproduce the delays accurately

		  //Copy pot data
		  memcpy(flash_buffer, joysticksVal, 4);

		  //Copy delay
		  timediff = HAL_GetTick() - timediff;
		  memcpy(flash_buffer + 4, &timediff, 4);
		  timediff = HAL_GetTick();

		  //Check if full, in which case stop recording and indicate
		  //that it's full

		  //Store into Flash
		  BSP_QSPI_Write(flash_buffer, flash_write_ptr, 8);

		  //Update flash pointer
		  flash_write_ptr += 8;

		  if(flash_write_ptr > FLASH_BLOCK_SIZE){
			  full = 1;
			  recording = 2;
		  }

	  }

	  HAL_Delay(10);


	  /***** UI *****/
	  if((HAL_GetTick() - ui_timediff) > MAX_UI_DELAY){
		  ui_timediff = HAL_GetTick();

		  //CLI current data and IDs for joystick values
		  ui_printTxData(jci_tx, joysticksVal, txid, state, recording);

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


uint32_t but_debounce = 0;
uint32_t joy_debounce = 0;
#define MIN_DEBOUNCE_TIME 100

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	if(GPIO_Pin == BUTTON_INT_Pin){

		if(((HAL_GetTick() - but_debounce) > MIN_DEBOUNCE_TIME) || (state == STOPPED)){

			but_debounce = HAL_GetTick();

			//State change
			state++;
			if(state > PLAYBACK){
				state = STOPPED;
			}

			if(state == PLAYBACK){
				//Reset Flash read logic
				full = 0;
				flash_read_ptr = 0;
				recording = 0;

			}

			//Wake up to new state
			HAL_PWR_DisableSleepOnExit();

		}

	}

	//Can only record if realtime
	if(GPIO_Pin == JOYSTICK_INT_Pin && (state == REALTIME)){

		if((HAL_GetTick() - joy_debounce) > MIN_DEBOUNCE_TIME){

			joy_debounce = HAL_GetTick();

			//Toggle the recording state
			recording = !recording;

			//If recording, reset Flash state
			if(recording == 1){

				//Erase Flash Block (only using first one)
				BSP_QSPI_Erase_Block(0);

				//Reset recording logic
				full = 0;
				flash_write_ptr = 0;
			}

		}

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




char buffer[256] = {0};

void ui_printTxData(jci_t jci, uint8_t* data, uint8_t* ids, uint8_t state, uint8_t rec) {
    int offset = 0;



    // Print carriage return to overwrite the old output
    offset += sprintf(buffer + offset, "\r");


    //Print the state
    switch (state) {
        case PLAYBACK:
            offset += sprintf(buffer + offset, "PLAYBACK");

            if(rec == 2){
            	offset += sprintf(buffer + offset, "\t DONE                                                       ");
            	UIPRINT(buffer);
            	return;
            }
            break;
        case REALTIME:
            offset += sprintf(buffer + offset, "REALTIME");

            if(rec == 2){
            	offset += sprintf(buffer + offset, "\t FULL ");
            }
            else if(rec == 1){
            	offset += sprintf(buffer + offset, "\t REC  ");
            }else{
            	offset += sprintf(buffer + offset, "\t     ");
            }
            break;
        case STOPPED:
        	return;
        default:
            offset += sprintf(buffer + offset, "UNKNOWN");
            break;
    }


    // Loop through each ID and data pair
    for (int i = 0; i < jci.PSIZE; i++) {
      offset += sprintf(buffer + offset, "ID");
        offset += sprintf(buffer + offset, "<%c>: ", ids[i]);

        // Check PTYPE to determine data type
        if (jci.PTYPE) {
            // uint16_t
            uint16_t* uint16_data = (uint16_t*)&data[i * 2];
            offset += sprintf(buffer + offset, "%03d", *uint16_data);
        } else {
            // uint8_t
            offset += sprintf(buffer + offset, "%03d", data[i]);
        }

        // Add a separator if it's not the last pair
        if (i < jci.PSIZE - 1) {
            offset += sprintf(buffer + offset, "   ");
        }
    }

    // Print the complete string
    UIPRINT(buffer);
}




void ui_printRxData(jci_t jci, uint8_t* data, uint8_t* ids) {
    char buffer[256];
    int offset = 0;

    offset += sprintf(buffer + offset, "\r\nPACKET RECEIVED: ");


    // Loop through each ID and data pair
    for (int i = 0; i < jci.PSIZE; i++) {
      offset += sprintf(buffer + offset, "ID");
        offset += sprintf(buffer + offset, "<%c>: ", ids[i]);

        // Check PTYPE to determine data type
        if (jci.PTYPE) {
            // uint16_t
            uint16_t* uint16_data = (uint16_t*)&data[i * 2];
            offset += sprintf(buffer + offset, "%03d", *uint16_data);
        } else {
            // uint8_t
            offset += sprintf(buffer + offset, "%03d", data[i]);
        }

        // Add a separator if it's not the last pair
        if (i < jci.PSIZE - 1) {
            offset += sprintf(buffer + offset, "   ");
        }
    }

    offset += sprintf(buffer + offset, "\r\n");

    // Print the complete string
    UIPRINT(buffer);
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
