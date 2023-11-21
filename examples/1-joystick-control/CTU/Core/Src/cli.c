/*
 * cli.c
 *
 *  Created on: Nov 21, 2023
 *      Author: Jacoby
 */


#include "cli.h"
#include "usart.h"
#include "string.h"

#define PRINT(s) HAL_UART_Transmit(&huart1, s, strlen(s), HAL_MAX_DELAY)


/**
  * @brief  Receives a start playback request.
  * @note   A playback may already have been started when this function is called,
  * 		in which case the request is ignored.
  *
  * @param  None
  *
  * @retval None
  */
void cli_StartPlayback(void){
	//TODO
}






/**
  * @brief  Receives a stop playback request.
  * @note   If no playback is currently underway, the request is simply ignored.
  *
  * @param  None
  *
  * @retval None
  */
void cli_StopPlayback(void){
	//TODO
}




/**
  * @brief  Shows the content of a JCI packet (header + data + IDs)
  * @note   None
  *
  * @param  header		JCI packet header
  * @param	data		Data of the JCI packet
  * @param	id_list		ID lists of the JCI packet
  *
  * @retval None
  */
void ui_showPacket(jci_t header, uint8_t* data, uint8_t* id_list){

	char buffer[256];

	/* Get inspiration from this
	sprintf(buffer, "Packet info:\r\n TRANS: %c\r\n CHECKSUM_EN: %i\r\n GRAN: "
		"%i\r\n PTYPE: %i\r\n PACKET SIZE: %i\r\n",
				jci_tx.TRANS,
				jci_tx.CHECKSUM_EN,
				jci_tx.GRAN,
				jci_tx.PTYPE,
				txpacketsize);
	*/

	PRINT(buffer);
}
