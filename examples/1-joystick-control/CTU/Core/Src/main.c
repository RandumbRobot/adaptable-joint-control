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
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_UART4_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

void jci_test_u8(void);
void jci_test_u16(void);
void jci_test_misc(void);

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
  MX_UART4_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */


  jci_test_u8();
  jci_test_u16();
  jci_test_misc();




  PRINT("CTU init\r\n");

	#define MAX_JCI_PACKET_SIZE (3 + 256 * 3 + 1)
    uint8_t packet[MAX_JCI_PACKET_SIZE] = {0};

	jci_t jci_tx = {
			.TRANS = 'S',
			.CHECKSUM_EN = 1,
			.GRAN = 1,
			.PTYPE = 0,
			.PSIZE = 4
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
	int txpacketsize = 0;

	txpacketsize = jci_buildPacket(&jci_tx, txdata, txid, packet);

	char buffer[256];
	sprintf(buffer, "Packet info:\r\n TRANS: %c\r\n CHECKSUM_EN: %i\r\n GRAN: "
			"%i\r\n PTYPE: %i\r\n PACKET SIZE: %i\r\n",
					jci_tx.TRANS,
					jci_tx.CHECKSUM_EN,
					jci_tx.GRAN,
					jci_tx.PTYPE,
					txpacketsize);

	PRINT(buffer);

	HAL_UART_Transmit(&huart4, packet, txpacketsize, HAL_MAX_DELAY);

	//TODO add check to make sure the other board received the 'S' packet
	jci_tx.TRANS= 'C';

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {


	  HAL_Delay(100);

	  txpacketsize = jci_buildPacket(&jci_tx, txdata, txid, packet);

	  sprintf(buffer, "\r\nPacket info:\r\n TRANS: %c\r\n CHECKSUM_EN: %i\r\n GRAN: "
			"%i\r\n PTYPE: %i\r\n PACKET SIZE: %i\r\n",
					jci_tx.TRANS,
					jci_tx.CHECKSUM_EN,
					jci_tx.GRAN,
					jci_tx.PTYPE,
					txpacketsize);
	  PRINT(buffer);
	  for(int i = 0 ; i < jci_tx.PSIZE ; i++){
		  sprintf(buffer, "ID%i: %c    DATA: %i\r\n", i, txid[i], txdata[i]);
		  PRINT(buffer);

		  //update data for next time
		  txdata[i] *= 3;
	  }

	  HAL_UART_Transmit(&huart4, packet, txpacketsize, HAL_MAX_DELAY);

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

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */




void jci_test_misc(void){

	//Every test uses 4 values
	char txid[] = {
		  '0',
		  '1',
		  'r',
		  'o'
	};
	uint8_t rxid[sizeof(txid)/sizeof(char)] = {0};


	//Result variables
	uint32_t txpacketsize = 0;
	int rxpacketsize = 0;

	#define MAX_JCI_PACKET_SIZE (3 + 256 * 3 + 1)
	uint8_t txpacket[MAX_JCI_PACKET_SIZE] = {0};
	uint8_t rxpacket[MAX_JCI_PACKET_SIZE] = {0};

	//********** TEST1 **********//
	//IDs request
	jci_t jci_tx = {
		  .TRANS = 'R',
	};
	jci_t jci_rx;

	txpacketsize = jci_buildPacket(&jci_tx, NULL, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, NULL, rxid, txpacket);


	/*
	 * WITH CUSTOM PTYPE
	 * WITHOUT CHECKSUM
	 */

	//********** TEST1 **********//
	//IDs sent
	jci_tx.TRANS = 'A';
	jci_tx.CHECKSUM_EN = 0;
	jci_tx.PTYPE = 1;
	jci_tx.PSIZE = 4;

	//reset rx IDs
	memset(rxid, 0, sizeof(txid)/sizeof(char));

	txpacketsize = jci_buildPacket(&jci_tx, NULL, txid, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, NULL, rxid, txpacket);



	/*
	 * WITH CUSTOM PTYPE
	 * WITH CHECKSUM
	 */

	//********** TEST1 **********//
	//IDs sent
	jci_tx.TRANS = 'A';
	jci_tx.CHECKSUM_EN = 1;
	jci_tx.PTYPE = 1;
	jci_tx.PSIZE = 4;

	//reset rx IDs
	memset(rxid, 0, sizeof(txid)/sizeof(char));

	txpacketsize = jci_buildPacket(&jci_tx, NULL, txid, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, NULL, rxid, txpacket);


	//********** TEST2 **********//
	//IDs sent
	//introduce error (checksum will fail)
	jci_tx.TRANS = 'A';
	jci_tx.CHECKSUM_EN = 1;
	jci_tx.PTYPE = 1;
	jci_tx.PSIZE = 4;

	//reset rx IDs
	memset(rxid, 0, sizeof(txid)/sizeof(char));

	txpacketsize = jci_buildPacket(&jci_tx, NULL, txid, txpacket);
	txpacket[5] = ' ';
	rxpacketsize = jci_parsePacket(&jci_rx, NULL, rxid, txpacket);





	/*
	 * WITH STANDARD PTYPE
	 * WITHOUT CHECKSUM
	 */

	//********** TEST1 **********//
	//IDs sent
	jci_tx.TRANS = 'A';
	jci_tx.CHECKSUM_EN = 0;
	jci_tx.PTYPE = 0;
	jci_tx.PSIZE = 4;

	//reset rx IDs
	memset(rxid, 0, sizeof(txid)/sizeof(char));

	txpacketsize = jci_buildPacket(&jci_tx, NULL, txid, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, NULL, rxid, txpacket);



	/*
	 * WITH STANDARD PTYPE
	 * WITH CHECKSUM
	 */

	//********** TEST1 **********//
	//IDs sent
	jci_tx.TRANS = 'A';
	jci_tx.CHECKSUM_EN = 1;
	jci_tx.PTYPE = 0;
	jci_tx.PSIZE = 4;

	//reset rx IDs
	memset(rxid, 0, sizeof(txid)/sizeof(char));

	txpacketsize = jci_buildPacket(&jci_tx, NULL, txid, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, NULL, rxid, txpacket);


	//********** TEST2 **********//
	//IDs sent
	//introduce error (checksum will fail)
	jci_tx.TRANS = 'A';
	jci_tx.CHECKSUM_EN = 1;
	jci_tx.PTYPE = 0;
	jci_tx.PSIZE = 4;

	//reset rx IDs
	memset(rxid, 0, sizeof(txid)/sizeof(char));

	txpacketsize = jci_buildPacket(&jci_tx, NULL, txid, txpacket);
	txpacket[3] = ' ';
	rxpacketsize = jci_parsePacket(&jci_rx, NULL, rxid, txpacket);
}


void jci_test_u16(void){

	//Every test uses 4 values
	uint16_t txdata_u16[] = {
		  0,
		  100,
		  1420,
		  10769
	};
	uint16_t rxdata_u16[sizeof(txdata_u16)/sizeof(uint16_t)] = {0};


	char txid[] = {
		  '0',
		  '1',
		  'r',
		  'o'
	};
	uint8_t rxid[sizeof(txid)/sizeof(char)] = {0};


	//Result variables
	uint32_t txpacketsize = 0;
	int rxpacketsize = 0;

	#define MAX_JCI_PACKET_SIZE (3 + 256 * 3 + 1)
	uint8_t txpacket[MAX_JCI_PACKET_SIZE] = {0};
	uint8_t rxpacket[MAX_JCI_PACKET_SIZE] = {0};

	/*
	 * WITHOUT CHECKSUM
	 * WITHOUT GRANULAR CONTROL
	 */

	//********** TEST1 **********//
	//New joint data
	jci_t jci_tx = {
		  .TRANS = 'S',
		  .CHECKSUM_EN = 0,
		  .GRAN = 0,
		  .PTYPE = 1,
		  .PSIZE = 4
	};
	jci_t jci_rx;

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u16, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u16, rxid, txpacket);



	//********** TEST2 **********//
	//Continue operation (first time, last operation was 'S')
	//Need to reuse the same jci_rx for the continue operation.
	jci_tx.TRANS = 'C';

	//new data
	txdata_u16[0] = 3;
	txdata_u16[1] = 10036;
	txdata_u16[2] = 9000;
	txdata_u16[3] = 55;

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u16, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u16, rxid, txpacket);

	//********** TEST3 **********//
	//Continue operation (second time, last operation was 'C')
	//Need to reuse the same jci_rx for the continue operation.

	//new data
	txdata_u16[0] = 10;
	txdata_u16[1] = 12688;
	txdata_u16[2] = 36822;
	txdata_u16[3] = 201;

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u16, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u16, rxid, txpacket);




	/*
	 * WITH CHECKSUM
	 * WITHOUT GRANULAR CONTROL
	 */

	//********** TEST1 **********//
	//New joint data
	jci_tx.TRANS = 'S';
	jci_tx.CHECKSUM_EN = 1;

	//new data
	txdata_u16[0] = 0;
	txdata_u16[1] = 8;
	txdata_u16[2] = 100;
	txdata_u16[3] = 200;

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u16, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u16, rxid, txpacket);



	//********** TEST2 **********//
	//Continue operation (first time, last operation was 'S')
	//Need to reuse the same jci_rx for the continue operation.
	jci_tx.TRANS = 'C';

	//new data
	txdata_u16[0] = 30000;
	txdata_u16[1] = 13610;
	txdata_u16[2] = 9;
	txdata_u16[3] = 559;

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u16, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u16, rxid, txpacket);

	//********** TEST3 **********//
	//Continue operation (second time, last operation was 'C')
	//Need to reuse the same jci_rx for the continue operation.

	//new data
	txdata_u16[0] = 109;
	txdata_u16[1] = 12620;
	txdata_u16[2] = 68;
	txdata_u16[3] = 2012;

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u16, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u16, rxid, txpacket);



	//********** TEST4 **********//
	//Continue operation (second time, last operation was 'C')
	//introduce error (checksum should fail)
	jci_tx.TRANS = 'S';
	jci_tx.CHECKSUM_EN = 1;

	//new data
	txdata_u16[0] = 119;
	txdata_u16[1] = 8;
	txdata_u16[2] = 10220;
	txdata_u16[3] = 200;

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u16, NULL, txpacket);
	txpacket[5] = 0;
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u16, rxid, txpacket);



	/*
	 * WITH CHECKSUM
	 * WITH GRANULAR CONTROL
	 */

	//********** TEST1 **********//
	//New joint data
	jci_tx.TRANS = 'S';
	jci_tx.CHECKSUM_EN = 1;
	jci_tx.GRAN = 1;

	//new data
	txdata_u16[0] = 0;
	txdata_u16[1] = 8;
	txdata_u16[2] = 100;
	txdata_u16[3] = 200;

	//reset rx IDs
	memset(rxid, 0, sizeof(txid)/sizeof(char));

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u16, txid, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u16, rxid, txpacket);



	//********** TEST2 **********//
	//Continue operation (first time, last operation was 'S')
	//Need to reuse the same jci_rx for the continue operation.
	jci_tx.TRANS = 'C';

	//new data
	txdata_u16[0] = 30000;
	txdata_u16[1] = 13610;
	txdata_u16[2] = 9;
	txdata_u16[3] = 559;

	//reset rx IDs
	memset(rxid, 0, sizeof(txid)/sizeof(char));

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u16, txid, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u16, rxid, txpacket);

	//********** TEST3 **********//
	//Continue operation (second time, last operation was 'C')
	//Need to reuse the same jci_rx for the continue operation.

	//new data
	txdata_u16[0] = 109;
	txdata_u16[1] = 12620;
	txdata_u16[2] = 68;
	txdata_u16[3] = 2012;

	//reset rx IDs
	memset(rxid, 0, sizeof(txid)/sizeof(char));

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u16, txid, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u16, rxid, txpacket);



	//********** TEST4 **********//
	//Continue operation (second time, last operation was 'C')
	//introduce error (checksum should fail)
	jci_tx.TRANS = 'S';
	jci_tx.CHECKSUM_EN = 1;

	//new data
	txdata_u16[0] = 119;
	txdata_u16[1] = 8;
	txdata_u16[2] = 10220;
	txdata_u16[3] = 200;

	//reset rx IDs
	memset(rxid, 0, sizeof(txid)/sizeof(char));

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u16, txid, txpacket);
	txpacket[5] = 0;
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u16, rxid, txpacket);

}


void jci_test_u8(void){

	//Every test uses 4 values
	uint8_t txdata_u8[] = {
		  0,
		  8,
		  100,
		  200
	};
	uint8_t rxdata_u8[sizeof(txdata_u8)/sizeof(uint8_t)] = {0};

	char txid[] = {
		  '0',
		  '1',
		  'r',
		  'o'
	};
	uint8_t rxid[sizeof(txid)/sizeof(char)] = {0};

	//Result variables
	uint32_t txpacketsize = 0;
	int rxpacketsize = 0;

	#define MAX_JCI_PACKET_SIZE (3 + 256 * 3 + 1)
	uint8_t txpacket[MAX_JCI_PACKET_SIZE] = {0};
	uint8_t rxpacket[MAX_JCI_PACKET_SIZE] = {0};

	/*
	 * WITHOUT CHECKSUM
	 * WITHOUT GRANULAR CONTROL
	 */

	//********** TEST1 **********//
	//New joint data
	jci_t jci_tx = {
		  .TRANS = 'S',
		  .CHECKSUM_EN = 0,
		  .GRAN = 0,
		  .PTYPE = 0,
		  .PSIZE = 4
	};
	jci_t jci_rx;

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u8, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u8, rxid, txpacket);



	//********** TEST2 **********//
	//Continue operation (first time, last operation was 'S')
	//Need to reuse the same jci_rx for the continue operation.
	jci_tx.TRANS = 'C';

	//new data
	txdata_u8[0] = 3;
	txdata_u8[1] = 136;
	txdata_u8[2] = 9;
	txdata_u8[3] = 55;

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u8, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u8, rxid, txpacket);

	//********** TEST3 **********//
	//Continue operation (second time, last operation was 'C')
	//Need to reuse the same jci_rx for the continue operation.

	//new data
	txdata_u8[0] = 10;
	txdata_u8[1] = 126;
	txdata_u8[2] = 68;
	txdata_u8[3] = 201;

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u8, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u8, rxid, txpacket);




	/*
	 * WITH CHECKSUM
	 * WITHOUT GRANULAR CONTROL
	 */

	//********** TEST1 **********//
	//New joint data
	jci_tx.TRANS = 'S';
	jci_tx.CHECKSUM_EN = 1;

	//new data
	txdata_u8[0] = 0;
	txdata_u8[1] = 8;
	txdata_u8[2] = 100;
	txdata_u8[3] = 200;

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u8, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u8, rxid, txpacket);



	//********** TEST2 **********//
	//Continue operation (first time, last operation was 'S')
	//Need to reuse the same jci_rx for the continue operation.
	jci_tx.TRANS = 'C';

	//new data
	txdata_u8[0] = 3;
	txdata_u8[1] = 136;
	txdata_u8[2] = 9;
	txdata_u8[3] = 55;

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u8, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u8, rxid, txpacket);

	//********** TEST3 **********//
	//Continue operation (second time, last operation was 'C')
	//Need to reuse the same jci_rx for the continue operation.

	//new data
	txdata_u8[0] = 10;
	txdata_u8[1] = 126;
	txdata_u8[2] = 68;
	txdata_u8[3] = 201;

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u8, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u8, rxid, txpacket);



	//********** TEST4 **********//
	//Continue operation (second time, last operation was 'C')
	//introduce error (checksum should fail)
	jci_tx.TRANS = 'S';
	jci_tx.CHECKSUM_EN = 1;

	//new data
	txdata_u8[0] = 1;
	txdata_u8[1] = 8;
	txdata_u8[2] = 100;
	txdata_u8[3] = 200;

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u8, NULL, txpacket);
	txpacket[5] = 0;
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u8, rxid, txpacket);





	/*
	 * WITH CHECKSUM
	 * WITH GRANULAR CONTROL
	 */

	//********** TEST1 **********//
	//New joint data
	jci_tx.TRANS = 'S';
	jci_tx.CHECKSUM_EN = 1;
	jci_tx.GRAN = 1;

	//new data
	txdata_u8[0] = 0;
	txdata_u8[1] = 8;
	txdata_u8[2] = 100;
	txdata_u8[3] = 200;

	//reset rx IDs
	memset(rxid, 0, sizeof(txid)/sizeof(char));

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u8, txid, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u8, rxid, txpacket);



	//********** TEST2 **********//
	//Continue operation (first time, last operation was 'S')
	//Need to reuse the same jci_rx for the continue operation.
	jci_tx.TRANS = 'C';

	//new data
	txdata_u8[0] = 3;
	txdata_u8[1] = 136;
	txdata_u8[2] = 9;
	txdata_u8[3] = 55;

	//reset rx IDs
	memset(rxid, 0, sizeof(txid)/sizeof(char));

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u8, txid, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u8, rxid, txpacket);

	//********** TEST3 **********//
	//Continue operation (second time, last operation was 'C')
	//Need to reuse the same jci_rx for the continue operation.

	//new data
	txdata_u8[0] = 10;
	txdata_u8[1] = 126;
	txdata_u8[2] = 68;
	txdata_u8[3] = 201;

	//reset rx IDs
	memset(rxid, 0, sizeof(txid)/sizeof(char));

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u8, txid, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u8, rxid, txpacket);



	//********** TEST4 **********//
	//Continue operation (second time, last operation was 'C')
	//introduce error (checksum should fail)
	jci_tx.TRANS = 'S';
	jci_tx.CHECKSUM_EN = 1;

	//new data
	txdata_u8[0] = 1;
	txdata_u8[1] = 8;
	txdata_u8[2] = 100;
	txdata_u8[3] = 200;

	//reset rx IDs
	memset(rxid, 0, sizeof(txid)/sizeof(char));

	txpacketsize = jci_buildPacket(&jci_tx, txdata_u8, txid, txpacket);
	txpacket[5] = 0;
	rxpacketsize = jci_parsePacket(&jci_rx, rxdata_u8, rxid, txpacket);

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
