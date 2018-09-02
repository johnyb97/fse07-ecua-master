
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MCP3302_H
#define __MCP3302_H

#include "stm32f10x.h"

#define  MCP3302_SINGLE       0x0C0000
#define  MCP3302_DIFFERENTIAL 0x080000

#define  MCP3302_CHANNEL_0    0x000000
#define  MCP3302_CHANNEL_1    0x008000
#define  MCP3302_CHANNEL_2    0x010000
#define  MCP3302_CHANNEL_3    0x018000



void MCP3302_Init(void);
uint16_t Measure_HV_In(void);
uint16_t Measure_HV_Out(void);



#endif /*__CAN_H */

/* END OF FILE */
