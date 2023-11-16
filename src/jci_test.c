/*
BSD 3-Clause License

Copyright (c) 2023, Jacoby Roy

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "jci.h"
#include "string.h"
#include "stdio.h"

#define MAX_JCI_PACKET_SIZE (3 + 256 * 3 + 1)
#define THROW_ERROR do{printf("ERROR AT LINE: %i", __LINE__);while (1);}while(1);

/**
  * @brief  Compares the data at two addresses and checks if they are the same. 
  * 
  * @retval 1 if the same, 0 otherwise
  */
static int check_data(uint8_t* data1, uint8_t* data2, uint32_t size){

	uint8_t res;

	res = 1;
	for(int i = 0 ; i < size ; i++){
		if(data1[i] != data2[i]){
			res = 0;
			break;
		}
	}

	return res;
}


/**
  * @brief  This test does a C-flow connection and explore its edge cases.
  * 		PART1: 
  * 		This tests has the first board attempting to start a 'C' connection
  * 		with the second board. The second board IGNORES the first attempt, the 
  * 		first board then tries to send a C-flow packet (error shall be 
  * 		reported by the library), the second board then DECLINES
  * 		a second attempt to connect, the first board then tries AGAIN to send a 
  * 		C-flow packet (error shall be reported by the library), then sends a 
  * 		third requests and accepted by the second board. 
  * 		PART2: 
  * 		Two 'C' packets are then sent, when a random 'S' packet after the first
  * 		(with CONT disabled). Then, another C-flow request is made by the first
  * 		board, reconfirmed by second board and finally a last C-flow packet is
  * 		sent for the new C-flow.
  * 		Checksum is enabled and Granular control too.
  */
void jci_cont_connect(void){

	int res;

	printf("TEST: jci_cont_connect\r\n");
	
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

	uint8_t txpacket[MAX_JCI_PACKET_SIZE] = {0};
	uint8_t rxpacket[MAX_JCI_PACKET_SIZE] = {0};

	//********** PART1 **********//

	//DATA SENDER
	jci_t send_jci_tx = {
		.TRANS = 'S',
		.CHECKSUM_EN = 1,
		.GRAN = 1,
		.PTYPE = 0,
		.SOURCE = 0,
		.CONT = 1,
		.PSIZE = 4
	};
	jci_t send_jci_rx;

	//DATA RECEIVER
	jci_t recv_jci_tx = {
		.TRANS = 'A',
		.CHECKSUM_EN = 1,
		.GRAN = 0,
		.PTYPE = 0,
		.SOURCE = 0,
		.CONT = 0,
		.PSIZE = 4
	};
	jci_t recv_jci_rx;

	//First attempt -> ignored
	txpacketsize = jci_buildPacket(&send_jci_tx, txdata_u8, txid, txpacket);
	rxpacketsize = jci_parsePacket(&recv_jci_rx, rxdata_u8, rxid, txpacket);
	//Compare data
	if(!check_data(txdata_u8, rxdata_u8, send_jci_tx.PSIZE)){
		THROW_ERROR
	}
	//Check if fails
	res = jci_confirmCFlow(&send_jci_tx, txid, &send_jci_rx, rxid);
	if(res != 0){
		THROW_ERROR
	}
	//Data sender attemps to send C-flow data
	send_jci_tx.TRANS = 'C';
	txpacketsize = jci_buildPacket(&send_jci_tx, txdata_u8, txid, txpacket);
	if(txpacketsize != 0){
		THROW_ERROR
	}



	//Second attempt -> declined
	recv_jci_tx.CONT = 0;
	txpacketsize = jci_buildPacket(&recv_jci_tx, NULL, NULL, rxpacket);
	rxpacketsize = jci_parsePacket(&send_jci_rx, NULL, NULL, rxpacket);
	//Check if fails
	res = jci_confirmCFlow(&send_jci_tx, txid, &send_jci_rx, rxid);
	if(res != 0){
		THROW_ERROR
	}
	//Data sender attemps to send C-flow data
	send_jci_tx.TRANS = 'C';
	txpacketsize = jci_buildPacket(&send_jci_tx, txdata_u8, txid, txpacket);
	if(txpacketsize != 0){
		THROW_ERROR
	}



	//Third attempt -> connect
	//reset tx IDs (for data sender)
	memset(txid, 0, sizeof(txid)/sizeof(char));
	//set the fields to the agreed upon values
	recv_jci_tx.CHECKSUM_EN = recv_jci_rx.CHECKSUM_EN;
	recv_jci_tx.PTYPE = recv_jci_rx.PTYPE;
	recv_jci_tx.PSIZE = recv_jci_rx.PSIZE;

	//rxid ARE THE AGREE UPON IDs (the ones received)
	//transaction
	recv_jci_tx.CONT = 1;
	txpacketsize = jci_buildPacket(&recv_jci_tx, NULL, rxid, rxpacket);
	rxpacketsize = jci_parsePacket(&send_jci_rx, NULL, txid, rxpacket);
	//Check if fails on data sender
	res = jci_confirmCFlow(&send_jci_tx, txid, &send_jci_rx, rxid);
	if(res != 1){
		THROW_ERROR
	}
	//Check if fails on data receiver
	res = jci_confirmCFlow(&recv_jci_tx, txid, &recv_jci_rx, rxid);
	if(res != 1){
		THROW_ERROR
	}


	//********** PART2 **********//

	//Data sender attemps to send C-flow data
	send_jci_tx.TRANS = 'C';
	txpacketsize = jci_buildPacket(&send_jci_tx, txdata_u8, txid, txpacket);
	if(txpacketsize == 0){
		THROW_ERROR
	}

	//reset data
	memset(rxdata_u8, 0, sizeof(rxdata_u8)/sizeof(char));

	rxpacketsize = jci_parsePacket(&recv_jci_rx, rxdata_u8, NULL, txpacket);
	//Compare data
	if(!check_data(txdata_u8, rxdata_u8, send_jci_tx.PSIZE)){
		THROW_ERROR
	}



	//Data sender attemps to send "S" transaction
	send_jci_tx.TRANS = 'S';
	send_jci_tx.CONT = 0;
	txpacketsize = jci_buildPacket(&send_jci_tx, txdata_u8, txid, txpacket);
	if(txpacketsize == 0){
		THROW_ERROR
	}
	//reset rx IDs and data
	memset(rxid, 0, sizeof(rxid)/sizeof(char));
	memset(rxdata_u8, 0, sizeof(rxdata_u8)/sizeof(char));

	rxpacketsize = jci_parsePacket(&recv_jci_rx, rxdata_u8, rxid, txpacket);
	//Compare data
	if(!check_data(txdata_u8, rxdata_u8, send_jci_tx.PSIZE)){
		THROW_ERROR
	}
	//Compare IDs
	if(!check_data(txid, rxid, send_jci_tx.PSIZE)){
		THROW_ERROR
	}



	//Data sender attemps to send C-flow data
	send_jci_tx.TRANS = 'C';
	//CONT field ignored
	txpacketsize = jci_buildPacket(&send_jci_tx, txdata_u8, txid, txpacket);
	if(txpacketsize == 0){
		THROW_ERROR
	}

	//reset data
	memset(rxdata_u8, 0, sizeof(rxdata_u8)/sizeof(char));

	rxpacketsize = jci_parsePacket(&recv_jci_rx, rxdata_u8, NULL, txpacket);
	//Compare data
	if(!check_data(txdata_u8, rxdata_u8, send_jci_tx.PSIZE)){
		THROW_ERROR
	}

	printf("TEST PASSED\r\n");

}


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

	txpacketsize = jci_buildPacket(&jci_tx, (uint8_t*) txdata_u16, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, (uint8_t*) rxdata_u16, rxid, txpacket);



	//********** TEST2 **********//
	//Continue operation (first time, last operation was 'S')
	//Need to reuse the same jci_rx for the continue operation.
	jci_tx.TRANS = 'C';

	//new data
	txdata_u16[0] = 3;
	txdata_u16[1] = 10036;
	txdata_u16[2] = 9000;
	txdata_u16[3] = 55;

	txpacketsize = jci_buildPacket(&jci_tx, (uint8_t*) txdata_u16, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, (uint8_t*) rxdata_u16, rxid, txpacket);

	//********** TEST3 **********//
	//Continue operation (second time, last operation was 'C')
	//Need to reuse the same jci_rx for the continue operation.

	//new data
	txdata_u16[0] = 10;
	txdata_u16[1] = 12688;
	txdata_u16[2] = 36822;
	txdata_u16[3] = 201;

	txpacketsize = jci_buildPacket(&jci_tx, (uint8_t*) txdata_u16, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, (uint8_t*) rxdata_u16, rxid, txpacket);




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

	txpacketsize = jci_buildPacket(&jci_tx, (uint8_t*) txdata_u16, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, (uint8_t*) rxdata_u16, rxid, txpacket);



	//********** TEST2 **********//
	//Continue operation (first time, last operation was 'S')
	//Need to reuse the same jci_rx for the continue operation.
	jci_tx.TRANS = 'C';

	//new data
	txdata_u16[0] = 30000;
	txdata_u16[1] = 13610;
	txdata_u16[2] = 9;
	txdata_u16[3] = 559;

	txpacketsize = jci_buildPacket(&jci_tx, (uint8_t*) txdata_u16, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, (uint8_t*) rxdata_u16, rxid, txpacket);

	//********** TEST3 **********//
	//Continue operation (second time, last operation was 'C')
	//Need to reuse the same jci_rx for the continue operation.

	//new data
	txdata_u16[0] = 109;
	txdata_u16[1] = 12620;
	txdata_u16[2] = 68;
	txdata_u16[3] = 2012;

	txpacketsize = jci_buildPacket(&jci_tx, (uint8_t*) txdata_u16, NULL, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, (uint8_t*) rxdata_u16, rxid, txpacket);



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

	txpacketsize = jci_buildPacket(&jci_tx, (uint8_t*) txdata_u16, NULL, txpacket);
	txpacket[5] = 0;
	rxpacketsize = jci_parsePacket(&jci_rx, (uint8_t*) rxdata_u16, rxid, txpacket);



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

	txpacketsize = jci_buildPacket(&jci_tx, (uint8_t*) txdata_u16, txid, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, (uint8_t*) rxdata_u16, rxid, txpacket);



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

	txpacketsize = jci_buildPacket(&jci_tx, (uint8_t*) txdata_u16, txid, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, (uint8_t*) rxdata_u16, rxid, txpacket);

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

	txpacketsize = jci_buildPacket(&jci_tx, (uint8_t*) txdata_u16, txid, txpacket);
	rxpacketsize = jci_parsePacket(&jci_rx, (uint8_t*) rxdata_u16, rxid, txpacket);



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

	txpacketsize = jci_buildPacket(&jci_tx, (uint8_t*) txdata_u16, txid, txpacket);
	txpacket[5] = 0;
	rxpacketsize = jci_parsePacket(&jci_rx, (uint8_t*) rxdata_u16, rxid, txpacket);

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
