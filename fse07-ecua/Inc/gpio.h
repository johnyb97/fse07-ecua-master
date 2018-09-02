
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPIO_H
#define __GPIO_H

/* Outputs */

#define AIR1_GPIO_PORT          GPIOA
#define AIR1_GPIO_PIN           GPIO_Pin_9

#define AIR2_GPIO_PORT          GPIOB
#define AIR2_GPIO_PIN           GPIO_Pin_15

#define PRECHARGE_GPIO_PORT     GPIOC
#define PRECHARGE_GPIO_PIN      GPIO_Pin_13

#define DCDC_HVRELAY_GPIO_PORT  GPIOC
#define DCDC_HVRELAY_GPIO_PIN   GPIO_Pin_14

#define DCDC_FAN_DISABLE_PORT   GPIOB
#define DCDC_FAN_DISABLE_PIN    GPIO_Pin_5

#define DCDC_GLV_DISABLE_PORT   GPIOB
#define DCDC_GLV_DISABLE_PIN    GPIO_Pin_0

#define AMS_OK_GPIO_PORT        GPIOB
#define AMS_OK_GPIO_PIN         GPIO_Pin_13

#define SPI1_NSS_GPIO_PORT      GPIOA
#define SPI1_NSS_GPIO_PIN       GPIO_Pin_4

#define BMS_WKUP_GPIO_PORT      GPIOB
#define BMS_WKUP_GPIO_PIN       GPIO_Pin_2

#define BMS_RX_GPIO_PORT        GPIOB
#define BMS_RX_GPIO_PIN         GPIO_Pin_11

/* Inputs */

#define CAN1RX_GPIO_PORT        GPIOA
#define CAN1RX_GPIO_PIN         GPIO_Pin_11

#define FAN_PGOOD_GPIO_PORT     GPIOB
#define FAN_PGOOD_GPIO_PIN      GPIO_Pin_9

#define AIR1_OPEN_GPIO_PORT     GPIOA
#define AIR1_OPEN_GPIO_PIN      GPIO_Pin_15

#define AIR2_OPEN_GPIO_PORT     GPIOA
#define AIR2_OPEN_GPIO_PIN      GPIO_Pin_10

#define SDC_END_SENSE_GPIO_PORT GPIOB
#define SDC_END_SENSE_GPIO_PIN  GPIO_Pin_3

#define SDC_HVIL_GPIO_PORT      GPIOA
#define SDC_HVIL_GPIO_PIN       GPIO_Pin_8

#define IMD_OK_GPIO_PORT        GPIOB
#define IMD_OK_GPIO_PIN         GPIO_Pin_14

#define NLATCH_GPIO_PORT        GPIOB
#define NLATCH_GPIO_PIN         GPIO_Pin_12

#define PRECHARGE_OPEN_GPIO_PORT GPIOB
#define PRECHARGE_OPEN_GPIO_PIN  GPIO_Pin_4

#define SPI1_MISO_GPIO_PORT     GPIOA
#define SPI1_MISO_GPIO_PIN      GPIO_Pin_6

#define BMS_FAULT_GPIO_PORT		GPIOB
#define BMS_FAULT_GPIO_PIN		GPIO_Pin_1

/* Outputs with AF */

#define CAN1TX_GPIO_PORT        GPIOA
#define CAN1TX_GPIO_PIN         GPIO_Pin_12

#define FAN_PWM_GPIO_PORT       GPIOB
#define FAN_PWM_GPIO_PIN        GPIO_Pin_6

#define SPI1_MOSI_GPIO_PORT     GPIOA
#define SPI1_MOSI_GPIO_PIN      GPIO_Pin_7

#define SPI1_SCK_GPIO_PORT      GPIOA
#define SPI1_SCK_GPIO_PIN       GPIO_Pin_5

#define BMS_TX_GPIO_PORT        GPIOB
#define BMS_TX_GPIO_PIN         GPIO_Pin_10

/*GPIO Analog inputs */

/* GPIO Macros */

#define AIR1_Close()        AIR1_GPIO_PORT->BSRR = AIR1_GPIO_PIN
#define AIR1_Open()         AIR1_GPIO_PORT->BRR  = AIR1_GPIO_PIN

#define AIR2_Close()        AIR2_GPIO_PORT->BSRR = AIR2_GPIO_PIN
#define AIR2_Open()         AIR2_GPIO_PORT->BRR  = AIR2_GPIO_PIN

#define PRECHARGE_On()      PRECHARGE_GPIO_PORT->BSRR = PRECHARGE_GPIO_PIN
#define PRECHARGE_Off()     PRECHARGE_GPIO_PORT->BRR  = PRECHARGE_GPIO_PIN

#define DCDC_HVRELAY_On()   DCDC_HVRELAY_GPIO_PORT->BSRR = DCDC_HVRELAY_GPIO_PIN
#define DCDC_HVRELAY_Off()  DCDC_HVRELAY_GPIO_PORT->BRR  = DCDC_HVRELAY_GPIO_PIN

#define DCDC_FAN_Enable()   DCDC_FAN_DISABLE_PORT->BRR  = DCDC_FAN_DISABLE_PIN
#define DCDC_FAN_Disable()  DCDC_FAN_DISABLE_PORT->BSRR = DCDC_FAN_DISABLE_PIN

#define DCDC_GLV_Enable()   DCDC_GLV_DISABLE_PORT->BRR  = DCDC_GLV_DISABLE_PIN
#define DCDC_GLV_Disable()  DCDC_GLV_DISABLE_PORT->BSRR = DCDC_GLV_DISABLE_PIN

#define SPI1_NSS_High()     SPI1_NSS_GPIO_PORT->BSRR = SPI1_NSS_GPIO_PIN
#define SPI1_NSS_Low()      SPI1_NSS_GPIO_PORT->BRR = SPI1_NSS_GPIO_PIN

#define AMS_Set_OK()        AMS_OK_GPIO_PORT->BSRR = AMS_OK_GPIO_PIN
#define AMS_Set_Fail()      AMS_OK_GPIO_PORT->BRR = AMS_OK_GPIO_PIN

/*----*/

#define is_FAN_Pwr_OK           (!(FAN_PGOOD_GPIO_PORT->IDR & FAN_PGOOD_GPIO_PIN))

#define is_AIR1_Open            (AIR1_OPEN_GPIO_PORT->IDR & AIR1_OPEN_GPIO_PIN)
#define is_AIR2_Open            (AIR2_OPEN_GPIO_PORT->IDR & AIR2_OPEN_GPIO_PIN)
#define is_PRECHARGE_Open       (PRECHARGE_OPEN_GPIO_PORT->IDR & PRECHARGE_OPEN_GPIO_PIN)

#define is_IMD_OK               (IMD_OK_GPIO_PORT->IDR & IMD_OK_GPIO_PIN)

#define is_SDC_END_Present      (SDC_END_SENSE_GPIO_PORT->IDR & SDC_END_SENSE_GPIO_PIN)
#define is_SDC_HVIL_Present     (SDC_HVIL_GPIO_PORT->IDR & SDC_HVIL_GPIO_PIN)
#define is_SDC_Err_Latched      (!(NLATCH_GPIO_PORT->IDR & NLATCH_GPIO_PIN))





void Init_GPIO(void);





#endif /*__GPIO_H */

/* END OF FILE */
