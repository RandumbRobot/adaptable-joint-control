

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
    uint8_t START;
    uint8_t CHECKSUM_EN;
    uint8_t GRAIN;
    uint8_t PTYPE;
    uint8_t PSIZE;

    //Payload
    void* PAYLOAD; //UNUSED

    //Checksum
    uint8_t CHECKSUM;

}jci_t;


//TX
uint32_t jci_buildPacket(jci_t* jci, uint8_t* data, uint8_t* id_list, uint8_t* packet);
uint32_t jci_buildHeader(jci_t* jci, uint8_t* packet);

//RX
uint8_t* jci_findPacket(uint8_t* data, uint32_t size);
int jci_parsePacket(jci_t* jci, uint8_t* data, uint8_t* id_list, uint8_t* packet);
int jci_parseHeader(jci_t* jci, uint8_t* packet);






/***** JCI STANDARD IDs *****/

//Standard IDs
const char jci_std_id_table[] = {
    0, //Index joint
    1, //Middle finger joint
    2, //Annular joint
    3  //Pinky joint
};