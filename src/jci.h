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


#ifndef JCI_H_
#define JCI_H_

#include "stdint.h"

//TODO current problem is it assumes we are using the same packet for both TX and RX for C-flow confirmation, which is bad
//put the C-flow control at the RX level only. TX need to manually verify with an RX packet if confirmation is received.

/***** JCI INTERFACE *****/


//Transmit and receive operations
typedef int (*jci_transmit)(uint8_t* data, uint32_t size);
typedef int (*jci_receive)(uint8_t* data, uint32_t size);


//Transaction struct
typedef struct{

   /* Operations */
   jci_transmit tx; //UNUSED
   jci_receive rx; //UNUSED

   /* Packet */
   //Header
   uint8_t TRANS;         //Transaction type
   uint8_t CHECKSUM_EN;   //Enable/Disable Checksum
   uint8_t GRAN;          //Granular data (specific joint IDs)
   uint8_t PTYPE;         //Type of data (transaction type dependent)
   uint8_t SOURCE;        //Packet source
   uint8_t CONT;          //'C' transaction type request (transaction type dependent)
   uint8_t PSIZE;         //Payload size

   //Payload
   //void* PAYLOAD; //UNUSED

   //Checksum
   uint8_t CHECKSUM;


   /* C-flow State Control */
   //These variables are used to keep track of C-flow state
   //accross multiple packets received, since the structs fields
   //are overwritten.
   //A C-flow can be interrupted by any other type of packet
   //(except an 'S' packet with CONT field set to '1' since this is
   //a request for a new C-flow) and then continue thanks this state
   //control.
   //All private variables.
   uint8_t CONTACCEPT;      //Internal control variables to check if a 'C' flow
                            //is currently underway.
   uint8_t CONTPTYPE;       //'C' flow PTYPE
   uint8_t CONTCHECKSUM_EN; //'C' flow CHECKSUM_EN
   uint8_t CONTPSIZE;       //'C' flow PSIZE

}jci_t;


//TX
uint32_t jci_buildPacket(jci_t* jci, uint8_t* data, uint8_t* id_list, uint8_t* packet);
uint32_t jci_buildHeader(jci_t* jci, uint8_t* packet);

//RX
uint8_t* jci_findPacket(uint8_t* data, uint32_t size, char* trans);
int jci_parsePacket(jci_t* jci, uint8_t* data, uint8_t* id_list, uint8_t* packet);
int jci_parseHeader(jci_t* jci, uint8_t* packet);

//C-flow control
int jci_confirmCFlow(jci_t* jci_tx, char* tx_id, jci_t* jci_rx, char* rx_id);






/***** JCI STANDARD IDs *****/

//Standard IDs
extern const char jci_std_id_table[];

#endif //JCI_H_
