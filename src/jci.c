

#include "jci.h"


#define MAX_PSIZE       256
#define JCI_START_CHAR      'S'
#define HEADER_SIZE_B       3       //Header size in bytes

//Header masks
#define CHECKSUM_EN_MASK    0b1 
#define GRAIN_MASK          0b10
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

    if(jci->tx == NULL){
        return -1;
    }

    packet_size = jci_buildPacket(jci, data, id_list, packet);

    return jci->tx(packet, packet_size);
}





/**
  * @brief  Build a packet in the JCI format.
  * @note   Enough memory should be provided for a full packet. Else the behavior is undefined.
  * 
  * @param  jci         Pointer to struct containing the information about the packet to send.
  * @param  data        Pointer to joint control data
  * @param  id_list     Pointer to list of IDs to use if grain control is enabled.
  * @param  packet      Pointer to buffer where to store the constructed packet.
  * 
  * @retval packet size in bytes
  */
uint32_t jci_buildPacket(jci_t* jci, uint8_t* data, uint8_t* id_list, uint8_t* packet){

    //Packet data
    //Copy data
    memcpy(packet + HEADER_SIZE_B, data, jci->PSIZE * (1+jci->PTYPE));
    //Copy IDs if grain control (0 bytes memcpy otherwise)
    memcpy(packet + HEADER_SIZE_B + (jci->PSIZE*jci->GRAIN), id_list, jci->GRAIN*(jci->PSIZE * (1+jci->PTYPE)));

    return jci_buildHeader(jci, packet);
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
  * @retval packet size in bytes
  */
uint32_t jci_buildHeader(jci_t* jci, uint8_t* packet){
    
    uint16_t payload_size;
    uint16_t packet_size;
    uint8_t checksum;

    //Calculate packet size
    //Given by:
    //-three bytes for header
    //-(PSIZE*(1+PTYPE))*(1+GRAIN) bytes for payload
    //-CHECKSUM_EN bytes for checksum
    payload_size = jci->PSIZE*(1+jci->PTYPE)*(1+jci->GRAIN);
    packet_size = HEADER_SIZE_B + payload_size + jci->CHECKSUM_EN;

    //Packet header
    packet[0] = jci->START;
    packet[1] = (jci->PTYPE << 2) | (jci->GRAIN << 1) | jci->CHECKSUM_EN;
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

    return packet_size;
}







/**
  * @brief  Finds a JCI packet in a buffer of data.
  * @note   None.
  * 
  * @param data        Pointer to buffer to search a packet in.
  * @param size        Number of bytes to check .
  * @param found       Pointer to flag to indicate if a start of packet has been found.
  * 
  * 
  * @retval If the packet is found, it returns the address of the start of the packet. 
  *         Otherwise, it simply returns the same address as data.
  */
uint8_t* jci_findPacket(uint8_t* data, uint32_t size, uint8_t* found){

    for(int i = size ; i < (size - 2) ; i++){
        if(data[i] == JCI_START_CHAR){

            //Check if header data is valid
            if(data[i+1] > 0b1000){
                continue;
            }

            *found = 1;

            return data + i;
        }
    }

    return data;
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
  * @retval Result of the checksum checking. -1 if error, 0 if no error.
  */
int jci_parsePacket(jci_t* jci, uint8_t* data, uint8_t* id_list, uint8_t* packet){

    if(jci_parseHeader(jci, packet)){
        return -1;
    }

    //Checksum test passed, can copy data over.
    //Copy data
    memcpy(data, packet + HEADER_SIZE_B, jci->PSIZE * (1+jci->PTYPE));
    //Copy IDs if grain control (0 bytes memcpy otherwise)
    memcpy(id_list, packet + HEADER_SIZE_B + (jci->PSIZE*jci->GRAIN), jci->GRAIN*(jci->PSIZE * (1+jci->PTYPE)));

    return 0;
}








/**
  * @brief  Parses a JCI packet to get its header.
  * @note   The packet is assumed to contain all the data.
  *         This method can be use for a no copy method of 
  *         parsing the packet if the packet buffer is used
  *         directly.
  * 
  * @param jci         Pointer to struct to store the information about the packet received.
  * @param packet      Pointer to packet to parse.
  * 
  * 
  * @retval Result of the checksum checking. -1 if error, 0 if no error.
  */
int jci_parseHeader(jci_t* jci, uint8_t* packet){

    uint8_t header;
    uint16_t payload_size;
    uint16_t packet_size;
    uint8_t checksum;

    //Packet header
    header = packet[1];
    jci->CHECKSUM_EN = header & CHECKSUM_EN_MASK;
    jci->GRAIN = header & GRAIN_MASK;
    jci->PTYPE = header & PTYPE_MASK;
    jci->PSIZE = packet[2];

    //Packet and payload sizes
    payload_size = jci->PSIZE*(1+jci->PTYPE)*(1+jci->GRAIN);
    packet_size = HEADER_SIZE_B + payload_size + jci->CHECKSUM_EN;

    //Packet checksum
    if(jci->CHECKSUM_EN)
    {
        jci->CHECKSUM = packet[HEADER_SIZE_B + payload_size];

        //Calculate checksum
        checksum = 0;
        for (int i = 0; i < (HEADER_SIZE_B + payload_size) ; i++) 
        {
            checksum += packet[i];
        }

        //Verify checksum
        if (jci->CHECKSUM != checksum){
            return -1;
        }
    }

    return 0;
}