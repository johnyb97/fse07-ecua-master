

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H
#define __CAN_H


#include "stm32f10x.h"

#define CAN1_BAUDRATE   500

void Init_CAN_Interfaces(void);
int CAN1_TransmitMessage(uint32_t StdID, const uint8_t *data, uint8_t length);





#endif /*__CAN_H */

/* END OF FILE */
