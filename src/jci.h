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


/***** JCI INTERFACE *****/


//Transmit and receive operations
typedef int (*jci_transmit)(uint8_t* data, uint32_t size);
typedef int (*jci_receive)(uint8_t* data, uint32_t size);


//Packet struct
typedef struct{

    /* Operations */
    jci_transmit tx; //UNUSED
    jci_receive rx; //UNUSED

    /* Packet */
    //Header
    uint8_t TRANS;
    uint8_t CHECKSUM_EN;
    uint8_t GRAN;
    uint8_t PTYPE;
    uint8_t PSIZE;

    //Payload
    //void* PAYLOAD; //UNUSED

    //Checksum
    uint8_t CHECKSUM;

}jci_t;


//TX
uint32_t jci_buildPacket(jci_t* jci, uint8_t* data, uint8_t* id_list, uint8_t* packet);
uint32_t jci_buildHeader(jci_t* jci, uint8_t* packet);

//RX
uint8_t* jci_findPacket(uint8_t* data, uint32_t size, uint8_t* trans);
int jci_parsePacket(jci_t* jci, uint8_t* data, uint8_t* id_list, uint8_t* packet);
int jci_parseHeader(jci_t* jci, uint8_t* packet);






/***** JCI STANDARD IDs *****/

//Standard IDs
extern const char jci_std_id_table[];

#endif //JCI_H_
