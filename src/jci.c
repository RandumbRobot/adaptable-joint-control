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


#define MAX_PSIZE       256
#define HEADER_SIZE_B   3       //Header size in bytes

//Transaction types
#define NEW_JOINT_DATA      'S'     //New joint data transaction
#define CONT_JOINT_DATA     'C'     //Continue joint data transaction
#define REQUEST_JOINT_ID    'R'     //Request joint IDs and types
#define SEND_JOINT_ID       'A'     //Send joint IDs and types

//Header masks
#define CHECKSUM_EN_MASK    0b1 
#define GRAN_MASK           0b10
#define PTYPE_MASK          0b100


/**
  * @brief  Sends the joint control data using JCI format.
  * @note   Enough memory should be provided for packet, else things will be overwritten.
  * 
  * @param  jci         Pointer to struct containing the information about the packet to send.
  * @param  data        Pointer to joint control data
  * @param  id_list     Pointer to list of IDs to use if grain control is enabled.
  * @param  packet      Pointer to buffer where to store the constructed packet.
  * 
  * @retval Error value from the tx function. If the tx function was not given to the jci handle, -1 is returned.
  */
int jci_sendJointData(jci_t* jci, uint8_t* data, uint8_t* id_list, uint8_t* packet){

    uint32_t packet_size;
    int err;

    if(jci->tx == NULL){
        err = -1;
    }else{
        packet_size = jci_buildPacket(jci, data, id_list, packet);
        err = jci->tx(packet, packet_size);
    }

    return err;
}


/**
  * @brief  Build a packet in the JCI format.
  * @note   Enough memory should be provided for a full packet. Else the behavior is undefined.
  * 
  * @param  jci         Pointer to struct containing the information about the packet to send.
  * @param  data        Pointer to joint control data. Used if transaction type is 'S' or 'C'
  * @param  id_list     Pointer to list of IDs. Used if transaction type is 'S' or 'C' or 'A'
  * @param  packet      Pointer to buffer where to store the constructed packet.
  * 
  * @retval Error value:
  *         -If the transaction type is illegal, 0 is returned 
  *         -If no error occured, packet size in bytes is returned
  */
uint32_t jci_buildPacket(jci_t* jci, uint8_t* data, uint8_t* id_list, uint8_t* packet){

    int err;
    uint32_t packet_size;

    err = 0;

    switch (jci->TRANS)
    {
    case NEW_JOINT_DATA:
        //Payload data
        //Copy data
        memcpy(packet + HEADER_SIZE_B, data, jci->PSIZE * (1+jci->PTYPE));
        //Copy IDs if granular control (0 bytes memcpy otherwise)
        memcpy(packet + HEADER_SIZE_B + (jci->PSIZE*(1+jci->PTYPE)), id_list, jci->GRAN*jci->PSIZE);
        break;
    
    case CONT_JOINT_DATA:
        //Payload data
        //Copy data
        memcpy(packet + (HEADER_SIZE_B - 2), data, jci->PSIZE * (1+jci->PTYPE));
        //Copy IDs if granular control (0 bytes memcpy otherwise)
        memcpy(packet + (HEADER_SIZE_B - 2) + (jci->PSIZE*(1+jci->PTYPE)), id_list, jci->GRAN*jci->PSIZE);
        break;
    
    case REQUEST_JOINT_ID:
        //do nothing
        break;
    
    case SEND_JOINT_ID:
        //Payload
        //Copy joint IDs data
        memcpy(packet + HEADER_SIZE_B, id_list, jci->PSIZE);
        break;
    
    default:
        err = -1; //illegal transaction type
        break;
    }


    if(err == 0){
        packet_size = jci_buildHeader(jci, packet);
    }else{
        packet_size = 0;
    }

    return packet_size;
}


/**
  * @brief  Build the header for a JCI packet.
  * @note   This method is faster since it assumes that packet buffer
  *         already contains the payload data.
  *         The user must allocate enough memory around the data buffer
  *         to make sure no problem occurs. The data should be located at an offset of
  *         2 bytes in the allocated buffer (the first two bytes are used for the header).
  *         The data shall contain both the joint control data and the ID list if applicable.
  *         
  * @note   This method can be use for a no copy method of building 
  *         the packet if the packet buffer is used directly.
  * 
  * @param  jci         Pointer to struct containing the information about the packet to send.
  * @param  packet      Pointer to packet buffer. The address of the start of the
  *                     packet shall be provided, not the start of the payload data.
  * 
  * @retval packet size in bytes. If 0 is returned, it means it was an illegal transaction type.
  */
uint32_t jci_buildHeader(jci_t* jci, uint8_t* packet){
    
    uint16_t payload_size;
    uint16_t packet_size;
    uint8_t checksum;

    
    switch (jci->TRANS)
    {
    case NEW_JOINT_DATA:
        //Calculate packet size
        payload_size = jci->PSIZE*(1+jci->PTYPE)*(1+jci->GRAN);
        packet_size = HEADER_SIZE_B + payload_size + jci->CHECKSUM_EN;

        //Packet header
        packet[0] = jci->TRANS;
        packet[1] = (jci->PTYPE << 2) | (jci->GRAN << 1) | jci->CHECKSUM_EN;
        packet[2] = jci->PSIZE;

        //Packet checksum
        if(jci->CHECKSUM_EN)
        {
            checksum = 0;
            for (int i = 0; i < (HEADER_SIZE_B + payload_size) ; i++) 
            {
                checksum += packet[i];
            }
            packet[HEADER_SIZE_B + payload_size] = checksum;
        }
        break;
    
    case CONT_JOINT_DATA:
        //Calculate packet size
        payload_size = jci->PSIZE*(1+jci->PTYPE)*(1+jci->GRAN);
        packet_size = (HEADER_SIZE_B - 2) + payload_size + jci->CHECKSUM_EN;

        //Packet header
        packet[0] = jci->TRANS;
        //packet[1] is removed
        //packet[2] is removed

        //Packet checksum
        if(jci->CHECKSUM_EN)
        {
            checksum = 0;
            for (int i = 0; i < ((HEADER_SIZE_B - 2) + payload_size) ; i++) 
            {
                checksum += packet[i];
            }
            packet[(HEADER_SIZE_B - 2) + payload_size] = checksum;
        }
        break;
    
    case REQUEST_JOINT_ID:
        //Calculate packet size
        payload_size = 0;
        packet_size = 3;

        //Packet header
        packet[0] = 'R';
        packet[1] = 'E';
        packet[2] = 'Q';

        //Packet checksum
        //no checksum
        break;
    
    case SEND_JOINT_ID:
        //Calculate packet size
        payload_size = jci->PSIZE;
        packet_size = HEADER_SIZE_B + payload_size + jci->CHECKSUM_EN;

        //Packet header
        packet[0] = jci->TRANS;
        packet[1] = (jci->PTYPE << 2) | (0 << 1) | jci->CHECKSUM_EN;
        packet[2] = jci->PSIZE;

        //Packet checksum
        if(jci->CHECKSUM_EN)
        {
            checksum = 0;
            for (int i = 0; i < (HEADER_SIZE_B + payload_size) ; i++) 
            {
                checksum += packet[i];
            }
            packet[HEADER_SIZE_B + payload_size] = checksum;
        }
        break;
    
    default:
        packet_size = 0; //illegal transaction type
        break;
    }

    return packet_size;
}


/**
  * @brief  Finds a JCI packet in a buffer of data.
  * @note   None.
  * 
  * @param data        Pointer to buffer to search a packet in.
  * @param size        Number of bytes to check .
  * @param trans       Pointer to the transaction type that was found. A NULL character is given
  *                     if nothing is found.
  * 
  * 
  * @retval If the packet is found, it returns the address of the start of the packet. 
  *         Otherwise, it simply returns the same address as data.
  */
uint8_t* jci_findPacket(uint8_t* data, uint32_t size, uint8_t* trans){

    uint8_t currchar;
    uint8_t* addr;

    //initialize the return value
    addr = data;

    for(int i = size ; i < (size - 2) ; i++){

        currchar = data[i];
        if( (currchar == NEW_JOINT_DATA) ||
            (currchar == CONT_JOINT_DATA) ||
            (currchar == SEND_JOINT_ID)){

            //Check if header data is valid
            if(data[i+1] > 0b1000){
                continue;
            }

            *trans = currchar;

            addr = data + i;
            break;
        }
        else if(currchar == REQUEST_JOINT_ID){
            
            //Check if header data is valid
            if(data[i+1] != 'E'){
                continue;
            }

            *trans = currchar;

            addr = data + i;
            break;
        }
    }

    *trans = NULL;

    return addr;
}


/**
  * @brief  Parses a JCI packet.
  * @note   The packet is assumed to contain all the data.
  *         Enough memory should be provided for a full payload. Else the behavior is undefined.

  * @param jci         Pointer to struct to store the information about the packet received.
  * @param data        Pointer to buffer where to store joint data.
  * @param id_list     Pointer to buffer where to store the list of IDs if applicable.
  * @param packet      Pointer to packet to parse.
  * 
  * @retval Error result: -4 if illegal transaction type
  *                       -3 if 'R' packet is invalid
  *                       -2 if 'C' trans received but jci is invalid.
  *                       -1 if checksum error
  *                        packet size if no error.
  */
int jci_parsePacket(jci_t* jci, uint8_t* data, uint8_t* id_list, uint8_t* packet){

    int packet_size;
    int err;

    packet_size = jci_parseHeader(jci, packet);

    if(packet_size >= 0){ //if no error
        //Checksum test passed, can copy data over.
        //Copy data
        memcpy(data, packet + HEADER_SIZE_B, jci->PSIZE * (1+jci->PTYPE));
        //Copy IDs if grain control (0 bytes memcpy otherwise)
        memcpy(id_list, packet + HEADER_SIZE_B + (jci->PSIZE*jci->GRAIN), jci->GRAIN*(jci->PSIZE * (1+jci->PTYPE)));
    }




    //Checksum test passed, can copy data over.
    switch (jci->TRANS)
    {
    case NEW_JOINT_DATA:
        //Copy data
        memcpy(data, packet + HEADER_SIZE_B, jci->PSIZE * (1+jci->PTYPE));
        //Copy IDs if grain control (0 bytes memcpy otherwise)
        memcpy(id_list, packet + HEADER_SIZE_B + (jci->PSIZE*(1+jci->PTYPE)), jci->GRAN*jci->PSIZE);
        break;
    
    case CONT_JOINT_DATA:
        //Copy data
        memcpy(data, packet + (HEADER_SIZE_B - 2), jci->PSIZE * (1+jci->PTYPE));
        //Copy IDs if granular control (0 bytes memcpy otherwise)
        memcpy(id_list, packet + (HEADER_SIZE_B - 2) + (jci->PSIZE*(1+jci->PTYPE)), jci->GRAN*jci->PSIZE);
        break;

    case REQUEST_JOINT_ID:
        //do nothing
        break;
    
    case SEND_JOINT_ID:
        //Payload
        //Copy joint IDs data
        memcpy(id_list, packet + HEADER_SIZE_B, jci->PSIZE);
        break;
    
    default:
        err = -1; //illegal transaction type
        break;
    }


    if(err == 0){
        packet_size = jci_parseHeader(jci, packet);
    }else{
        packet_size = 0;
    }

    return packet_size;
}


/**
  * @brief  Parses a JCI packet to get its header.
  * @note   The packet is assumed to contain all the data. This method can be use for a 
  *         no copy method of parsing the packet if the packet buffer is used
  *         directly. Note that this function expects a JCI with pre-filled data
  *         in the case where a 'C' transaction is received, otherwise an error (-2)
  *         is returned. It checks the TRANS part of the header to check if it is
  *         either 'C' or 'S', otherwise the -2 error is thrown.
  * 
  * @param jci         Pointer to struct to store the information about the packet received.
  * @param packet      Pointer to packet to parse.
  * 
  * 
  * @retval Error result: -4 if illegal transaction type
  *                       -3 if 'R' packet is invalid
  *                       -2 if 'C' trans received but jci is invalid.
  *                       -1 if checksum error
  *                        packet size if no error.
  */
int jci_parseHeader(jci_t* jci, uint8_t* packet){

    char prev_trans;
    uint8_t header;
    uint16_t payload_size;
    int packet_size;
    uint8_t checksum;

    //Save the transaction type for later verification
    prev_trans = jci->TRANS;

    //Transaction type
    jci->TRANS = packet[0];

    
    switch (jci->TRANS)
    {
    case NEW_JOINT_DATA:

        //Packet header
        header = packet[1];
        jci->CHECKSUM_EN = header & CHECKSUM_EN_MASK;
        jci->GRAN = header & GRAN_MASK;
        jci->PTYPE = header & PTYPE_MASK;
        jci->PSIZE = packet[2];

        //Packet and payload sizes
        payload_size = jci->PSIZE*(1+jci->PTYPE)*(1+jci->GRAN);
        packet_size = HEADER_SIZE_B + payload_size + jci->CHECKSUM_EN;

        //Packet checksum
        if(jci->CHECKSUM_EN)
        {
            jci->CHECKSUM = packet[HEADER_SIZE_B + payload_size];

            //Calculate checksum
            checksum = 0;
            for (int i = 0 ; i < (HEADER_SIZE_B + payload_size) ; i++) 
            {
                checksum += packet[i];
            }

            //Verify checksum
            if (jci->CHECKSUM != checksum){
                packet_size = -1;
                break;
            }
        }
        break;
    
    case CONT_JOINT_DATA:

        //Packet header
        //reusing the same JCI header from last 'S' transaction reveived.
        //Check for valid JCI
        if((prev_trans != CONT_JOINT_DATA) && (prev_trans != NEW_JOINT_DATA)){
            packet_size = -2;
            break;
        }

        //Packet and payload sizes
        payload_size = jci->PSIZE*(1+jci->PTYPE)*(1+jci->GRAN);
        packet_size = (HEADER_SIZE_B - 2) + payload_size + jci->CHECKSUM_EN;


        //Packet checksum
        if(jci->CHECKSUM_EN)
        {
            jci->CHECKSUM = packet[(HEADER_SIZE_B - 2) + payload_size];

            //Calculate checksum
            checksum = 0;
            for (int i = 0 ; i < ((HEADER_SIZE_B - 2) + payload_size) ; i++) 
            {
                checksum += packet[i];
            }

            //Verify checksum
            if (jci->CHECKSUM != checksum){
                packet_size = -1;
                break;
            }
        }
        break;
    
    case REQUEST_JOINT_ID:

        //Packet header
        //Validate packet (only need to valiate packet[2] since packet[0] and packet[1]
        //were previously validated)
        if(packet[2] != 'Q'){
            packet_size = -3;
            break;
        }

        //Packet and payload sizes
        payload_size = 0;
        packet_size = 3;

        //Packet checksum
        //no checksum
        break;
    
    case SEND_JOINT_ID:

        //Packet header
        header = packet[1];
        jci->CHECKSUM_EN = header & CHECKSUM_EN_MASK;
        jci->GRAN = header & GRAN_MASK;
        jci->PTYPE = header & PTYPE_MASK;
        jci->PSIZE = packet[2];

        //Packet and payload sizes
        payload_size = jci->PSIZE;
        packet_size = HEADER_SIZE_B + payload_size + jci->CHECKSUM_EN;

        //Packet checksum
        if(jci->CHECKSUM_EN)
        {
            jci->CHECKSUM = packet[HEADER_SIZE_B + payload_size];

            //Calculate checksum
            checksum = 0;
            for (int i = 0 ; i < (HEADER_SIZE_B + payload_size) ; i++) 
            {
                checksum += packet[i];
            }

            //Verify checksum
            if (jci->CHECKSUM != checksum){
                packet_size = -1;
                break;
            }
        }
        break;
    
    default:
        packet_size = -4; //illegal transaction type
        break;
    }

    return packet_size;
}