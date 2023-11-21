/*
 * cli.h
 *
 *  Created on: Nov 21, 2023
 *      Author: Jacoby
 */

#ifndef INC_CLI_H_
#define INC_CLI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "jci.h"

//Control functions
void cli_StartPlayback(void);
void cli_StopPlayback(void);


//Display functions
void ui_showPacket(jci_t header, uint8_t* data, uint8_t* id_list);



#ifdef __cplusplus
}
#endif


#endif /* INC_CLI_H_ */
