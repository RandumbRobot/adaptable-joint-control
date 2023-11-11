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

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void jci_test_u8(void);
void jci_test_u16(void);

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
  /* USER CODE BEGIN 2 */


  jci_test_u8();
  jci_test_u16();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
