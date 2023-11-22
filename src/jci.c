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



const char jci_std_id_table[] = {
    0, //Index joint
    1, //Middle finger joint
    2, //Annular joint
    3  //Pinky joint
};


#define MAX_PSIZE       256
#define HEADER_SIZE_B   3       //Header size in bytes

//Transaction types
#define JCI_START_TRANS_TYPE    'S'     //New joint data transaction
#define JCI_CONT_TRANS_TYPE     'C'     //Continue joint data transaction
#define JCI_REQUEST_TRANS_TYPE  'R'     //Request joint IDs and types
#define JCI_ACK_TRANS_TYPE      'A'     //Send joint IDs and types

//Header offsets
#define CHECKSUM_EN_OFFSET  0
#define GRAN_OFFSET         1
#define PTYPE_OFFSET        2
#define SOURCE_OFFSET       3
#define CONT_OFFSET         4

//Header masks
#define CHECKSUM_EN_MASK    (0b1 << CHECKSUM_EN_OFFSET)
#define GRAN_MASK           (0b1 << GRAN_OFFSET)
#define PTYPE_MASK          (0b1 << PTYPE_OFFSET)
#define SOURCE_MASK         (0b1 << SOURCE_OFFSET)
#define CONT_MASK           (0b1 << CONT_OFFSET)


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
    case JCI_START_TRANS_TYPE:
        //Payload data
        //Copy data
        memcpy(packet + HEADER_SIZE_B, data, jci->PSIZE * (1+jci->PTYPE));
        //Copy IDs if granular control (0 bytes memcpy otherwise)
        memcpy(packet + HEADER_SIZE_B + (jci->PSIZE*(1+jci->PTYPE)), id_list, jci->GRAN*jci->PSIZE);
        break;
    
    case JCI_CONT_TRANS_TYPE:

        //Check if currently in 'C' transaction flow. Return error otherwise.
        //NOTE: this should be handled by the jci_parseHeader function anyway)
        if(jci->CONTACCEPT != 1){
            packet_size = 0;
            break;
        }

        //Payload data
        //Copy data
        memcpy(packet + (HEADER_SIZE_B - 2), data, jci->CONTPSIZE * (1+jci->CONTPTYPE));
        //Copy IDs if granular control (0 bytes memcpy otherwise)
        //IDs are agreed upon already, not sent.
        break;
    
    case JCI_REQUEST_TRANS_TYPE:
        //do nothing
        break;
    
    case JCI_ACK_TRANS_TYPE:
        //Payload
        //force correct PSIZE for TX
        if(jci->GRAN == 1){ //if answer for 'R'
            jci->PSIZE = jci->PSIZE*jci->PTYPE; //set to 0 if standard IDs
        }else{ //if answer for 'S'
            jci->PSIZE = jci->PSIZE*jci->CONT; //set to 0 if declined
        }
        //Copy joint IDs data (no copy when PTYPE is 0. PSIZE is set to 0 when CONT is declined for 'S' transaction)
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
  * @retval packet size in bytes. If 0 is returned, it means it was an illegal transaction type or that
  *         a 'C' transaction was attempted but no 'C' flow was occuring.
  */
uint32_t jci_buildHeader(jci_t* jci, uint8_t* packet){
    
    uint16_t payload_size;
    uint16_t packet_size;
    uint8_t checksum;

    
    switch (jci->TRANS)
    {
    case JCI_START_TRANS_TYPE:
        //Calculate packet size
        payload_size = jci->PSIZE*(1 + jci->PTYPE + jci->GRAN);
        packet_size = HEADER_SIZE_B + payload_size + jci->CHECKSUM_EN;

        //Packet header
        packet[0] = jci->TRANS;
        packet[1] = (jci->CHECKSUM_EN   <<  CHECKSUM_EN_OFFSET) |
                    (jci->GRAN          <<  GRAN_OFFSET)        |
                    (jci->PTYPE         <<  PTYPE_OFFSET)       |
                    (jci->SOURCE        <<  SOURCE_OFFSET)      |
                    (jci->CONT          <<  CONT_OFFSET);
                    //last bits are set to 0
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
            jci->CHECKSUM = checksum;
        }


        if(jci->CONT == 1){ //save C-flow request
            //Disable C-flow (new request)
            //NOTE: this way the TX won't send C-flow requests by accident
            jci->CONTACCEPT = 0;

            //Save TX C-flow state to compare with RX (data sender side)
            jci->CONTCHECKSUM_EN = jci->CHECKSUM_EN;
            jci->CONTPTYPE = jci->PTYPE;
            jci->CONTPSIZE = jci->PSIZE;
        }

        break;
    
    case JCI_CONT_TRANS_TYPE:
        //Make sure a C-flow is enabled for TX
        if(jci->CONTACCEPT != 1){
            packet_size = 0;
            break;
        }

        //Calculate packet size
        payload_size = jci->CONTPSIZE*(1 + jci->CONTPTYPE);
        packet_size = (HEADER_SIZE_B - 2) + payload_size + jci->CONTCHECKSUM_EN;

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
            jci->CHECKSUM = checksum;
        }
        break;
    
    case JCI_REQUEST_TRANS_TYPE:
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
    
    case JCI_ACK_TRANS_TYPE:
        //Answer specific header
        if(jci->GRAN == 0){ //answer to 'S' packet

            //Enable/Disable 'C' flow for RX (data receiver side)
            //jci->CONTACCEPT = jci->CONT; TODO use confirm function

            if(jci->CONT == 1){ //if accepted
                //Save TX C-flow context to compare to RX (data receiver side)
                //(fields agreed upon)
                jci->CONTCHECKSUM_EN = jci->CHECKSUM_EN;
                jci->CONTPTYPE = jci->PTYPE;
                jci->CONTPSIZE = jci->PSIZE;
            }else{ //if declined
                jci->PTYPE = 0;
                jci->PSIZE = 0;
            }
        }
        else{ //answer to 'R' packet
            jci->PSIZE = jci->PSIZE * jci->PTYPE; //set to 0 if standard ID (if PTYPE == 0)
            jci->CONT = 0; //unused
        }

        //Calculate packet size
        payload_size = jci->PSIZE;
        packet_size = HEADER_SIZE_B + payload_size + jci->CHECKSUM_EN;

        //Packet header
        packet[0] = jci->TRANS;
        packet[1] = (jci->CHECKSUM_EN   <<  CHECKSUM_EN_OFFSET) |
                    (jci->GRAN          <<  GRAN_OFFSET)        |
                    (jci->PTYPE         <<  PTYPE_OFFSET)       |
                    (jci->SOURCE        <<  SOURCE_OFFSET)      |
                    (jci->CONT          <<  CONT_OFFSET);
                    //last bits are set to 0
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
            jci->CHECKSUM = checksum;
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
  * @param trans       Pointer to the transaction type that was found. A ' ' character is given
  *                     if nothing is found.
  * 
  * 
  * @retval If the packet is found, it returns the address of the start of the packet. 
  *         Otherwise, it simply returns the same address as data.
  */
uint8_t* jci_findPacket(uint8_t* data, uint32_t size, char* trans){

    uint8_t currchar;
    uint8_t* addr;

    //initialize the return value
    addr = data;

    //not found be default
    *trans = ' ';

    for(int i = 0 ; i < (size - 2) ; i++){

        currchar = data[i];
        if( (currchar == JCI_START_TRANS_TYPE) ||
            (currchar == JCI_ACK_TRANS_TYPE)){

            //Check if header data is valid
            if(data[i+1] >= 0b100000){ //header must be smaller
                continue;
            }

            *trans = currchar;

            addr = data + i;
            break;
        }
        else if(currchar == JCI_REQUEST_TRANS_TYPE){
            
            //Check if header data is valid
            if(data[i+1] != 'E'){
                continue;
            }

            *trans = currchar;

            addr = data + i;
            break;
        }
        else if(currchar == JCI_CONT_TRANS_TYPE){

        	//cannot partially validate, checksum recommended

            *trans = currchar;

            addr = data + i;
            break;
        }
    }

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
  *                       -2 if C-flow packet received but not in C-flow.
  *                       -1 if checksum error
  *                        packet size if no error.
  */
int jci_parsePacket(jci_t* jci, uint8_t* data, uint8_t* id_list, uint8_t* packet){

    int packet_size;

    packet_size = jci_parseHeader(jci, packet);

    if(packet_size >= 0){ //if no error

        //Checksum test passed, can copy data over.
        switch (jci->TRANS)
        {
        case JCI_START_TRANS_TYPE:
            //Copy data
            memcpy(data, packet + HEADER_SIZE_B, jci->PSIZE * (1+jci->PTYPE));
            //Copy IDs if granular control (0 bytes memcpy otherwise)
            memcpy(id_list, packet + HEADER_SIZE_B + (jci->PSIZE*(1+jci->PTYPE)), jci->GRAN*jci->PSIZE);
            break;

        case JCI_CONT_TRANS_TYPE:
            //Check if currently in 'C' transaction flow. Return error otherwise.
            //NOTE: this should be handled by the jci_parseHeader function anyway)
            if(jci->CONTACCEPT != 1){
                packet_size = -2;
                break;
            }

            //Copy data
            memcpy(data, packet + (HEADER_SIZE_B - 2), jci->CONTPSIZE * (1+jci->CONTPTYPE));
            //Copy IDs if granular control (0 bytes memcpy otherwise)
            //IDs are agreed upon already, not sent.
            break;

        case JCI_REQUEST_TRANS_TYPE:
            //do nothing
            break;

        case JCI_ACK_TRANS_TYPE:
            //Payload
            //Copy joint IDs data (no copy when PTYPE is 0. PSIZE is set to 0 when CONT is declined for 'S' transaction)
            memcpy(id_list, packet + HEADER_SIZE_B, jci->PSIZE);
            break;

        default:
            packet_size = -1; //illegal transaction type
            break;
        }
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
  *                    If a 'C' transaction can occur, make sure to pass the same
  *                    RX jci_t handle from the latest 'S' transaction since its header is used
  *                    and there is verification.
  *                    
  * @param packet      Pointer to packet to parse.
  * 
  * 
  * @retval Error result: -4 if illegal transaction type
  *                       -3 if 'R' packet is invalid
  *                       -2 if 'C' trans received but not in 'C' flow.
  *                       -1 if checksum error
  *                        packet size if no error.
  */
int jci_parseHeader(jci_t* jci, uint8_t* packet){

    char prev_trans;
    uint8_t prev_cont;
    uint8_t header;
    uint16_t payload_size;
    int packet_size;
    uint8_t checksum;

    //Save the transaction type for later transaction verification
    prev_trans = jci->TRANS;
    prev_cont = jci->CONT;

    //Transaction type
    jci->TRANS = packet[0];

    
    switch (jci->TRANS)
    {
    case JCI_START_TRANS_TYPE:

        //Packet header
        header = packet[1];
        jci->CHECKSUM_EN = (header & CHECKSUM_EN_MASK)  >> CHECKSUM_EN_OFFSET;
        jci->GRAN =        (header & GRAN_MASK)         >> GRAN_OFFSET;
        jci->PTYPE =       (header & PTYPE_MASK)        >> PTYPE_OFFSET;
        jci->SOURCE =      (header & SOURCE_MASK)       >> SOURCE_OFFSET;
        jci->CONT =        (header & CONT_MASK)         >> CONT_OFFSET;
        jci->PSIZE = packet[2];

        //Packet and payload sizes
        payload_size = jci->PSIZE*(1 + jci->PTYPE + jci->GRAN);
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

        if(jci->CONT == 1){
            //Save RX C-flow state to compare to TX (data receiver side)
            jci->CONTCHECKSUM_EN = jci->CHECKSUM_EN;
            jci->CONTPTYPE = jci->PTYPE;
            jci->CONTPSIZE = jci->PSIZE;
        }
        break;
    
    case JCI_CONT_TRANS_TYPE:
        //Packet header
        //Reusing the same JCI header from last 'S' transaction reveived and agreed upon.
        //Check if currently in 'C' transaction flow. Return error otherwise.
        if(jci->CONTACCEPT != 1){
            packet_size = -2;
            break;
        }

        //use 'C' flow RX saved state

        //Packet and payload sizes
        payload_size = jci->CONTPSIZE*(1 + jci->CONTPTYPE);
        packet_size = (HEADER_SIZE_B - 2) + payload_size + jci->CONTCHECKSUM_EN;


        //Packet checksum
        if(jci->CONTCHECKSUM_EN)
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
    
    case JCI_REQUEST_TRANS_TYPE:

        //Packet header
        //Validate packet (only need to validate packet[2] since packet[0] and packet[1]
        //were previously validated using "jci_findPacket")
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
    
    case JCI_ACK_TRANS_TYPE:

        //Packet header
        header = packet[1];
        jci->CHECKSUM_EN = (header & CHECKSUM_EN_MASK)  >> CHECKSUM_EN_OFFSET;
        jci->GRAN =        (header & GRAN_MASK)         >> GRAN_OFFSET;
        jci->PTYPE =       (header & PTYPE_MASK)        >> PTYPE_OFFSET;
        jci->SOURCE =      (header & SOURCE_MASK)       >> SOURCE_OFFSET;
        jci->CONT =        (header & CONT_MASK)         >> CONT_OFFSET;
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

        //Answer specific transaction
        if(jci->GRAN == 0){ //Response to 'S' transaction

            if(jci->CONT == 1){ //if accepted
                //Save RX C-flow state to compare with TX (data sender side)
                jci->CONTCHECKSUM_EN = jci->CHECKSUM_EN;
                jci->CONTPTYPE = jci->PTYPE;
                jci->CONTPSIZE = jci->PSIZE;
            }

            //else do nothing
        }
        else{ //Response to 'R' transaction
            //do nothing, simply receive data
        }
        break;
    
    default:
        packet_size = -4; //illegal transaction type
        break;
    }

    return packet_size;
}




/**
  * @brief  Compares a JCI TX packet and a JCI RX packet to confirm wheter or not C-flow can
  *         occur.
  * @note   This function shall only be used by the data sender of the C-flow transactions. The data
  *         receiver enables C-flow as soon as it sees a request for C-flow and accepts it. If the 
  *         data sender side changes the C-flow, it won't be able to send C-flow transactions
  *         until the receiver side has confirmed to have received it.
  * 
  * @param jci_tx      Pointer to a JCI struct for TX JCI handle
  * @param jci_rx      Pointer to a JCI struct for RX JCI handle
  * 
  * @retval Error result: -1 if error when comparing C-flow state for TX and RX
  *                        0 if C-flow declined (or if no response received)
  *                        1 if C-flow accepted
  */
int jci_confirmCFlow(jci_t* jci_tx, char* tx_id, jci_t* jci_rx, char* rx_id){

    int res;

    res = 0; //declined by default

    //Check if RX port received an acceptance response packet for a C-flow request (data sender side)
    //OR
    //Check if TX port sent a response packet to accept C-flow request (data receiver side)
    if( ((jci_rx->TRANS == JCI_ACK_TRANS_TYPE) && (jci_rx->GRAN == 0) && (jci_rx->CONT == 1)) ||
        ((jci_tx->TRANS == JCI_ACK_TRANS_TYPE) && (jci_tx->GRAN == 0) && (jci_tx->CONT == 1))){

        //Verify that the fields agreed upon are correct
        if( (jci_tx->CONTCHECKSUM_EN != jci_rx->CONTCHECKSUM_EN)    ||
            (jci_tx->CONTPTYPE       != jci_rx->CONTPTYPE)          ||
            (jci_tx->CONTPSIZE       != jci_rx->CONTPSIZE) ){

            //Error result
            res = -1;
        }
        else{ //fields agreed upon are correct

            //Verify that the IDs are the same
            for(int i = 0 ; i < jci_tx->CONTPSIZE ; i++){
                if(tx_id[i] != rx_id[i]){

                    //Error result
                    res = -1;
                    break;
                }
            }

            if(res == 0){ //if no error
                //Enable C-flow for TX and RX
                jci_tx->CONTACCEPT = 1;
                jci_rx->CONTACCEPT = 1;
                res = 1;
            }
        }
    }

    if(res < 1){
        //Disable C-flow for TX and RX
    	jci_tx->CONTACCEPT = 0;
    	jci_rx->CONTACCEPT = 0;
    	res = 0;
    }


    return res;
}
