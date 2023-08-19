/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l5xx_hal.h"
#include "stm32l5xx_ll_ucpd.h"
#include "stm32l5xx_ll_bus.h"
#include "stm32l5xx_ll_cortex.h"
#include "stm32l5xx_ll_rcc.h"
#include "stm32l5xx_ll_system.h"
#include "stm32l5xx_ll_utils.h"
#include "stm32l5xx_ll_pwr.h"
#include "stm32l5xx_ll_gpio.h"
#include "stm32l5xx_ll_dma.h"

#include "stm32l5xx_ll_exti.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define USER_BUTTON_Pin GPIO_PIN_13
#define USER_BUTTON_GPIO_Port GPIOC
#define VBUS_SENSE_Pin GPIO_PIN_2
#define VBUS_SENSE_GPIO_Port GPIOC
#define UCPD_FLT_Pin GPIO_PIN_14
#define UCPD_FLT_GPIO_Port GPIOB
#define ST_LINK_VCP_TX_Pin GPIO_PIN_7
#define ST_LINK_VCP_TX_GPIO_Port GPIOG
#define ST_LINK_VCP_RX_Pin GPIO_PIN_8
#define ST_LINK_VCP_RX_GPIO_Port GPIOG
#define LED_GREEN_Pin GPIO_PIN_7
#define LED_GREEN_GPIO_Port GPIOC
#define LED_RED_Pin GPIO_PIN_9
#define LED_RED_GPIO_Port GPIOA
#define UCPD_DBN_Pin GPIO_PIN_5
#define UCPD_DBN_GPIO_Port GPIOB
#define LED_BLUE_Pin GPIO_PIN_7
#define LED_BLUE_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
//prototype of structure
//structure prototype 1
typedef struct IT8951LdImgInfo
{
	uint16_t usEndianType; //little or Big Endian
	uint16_t usPixelFormat; //bpp
	uint16_t usRotate; //Rotate mode
	uint32_t ulStartFBAddr; //Start address of source Frame buffer
	uint32_t ulImgBufBaseAddr;//Base address of target image buffer

}IT8951LdImgInfo;

//structure prototype 2
typedef struct IT8951AreaImgInfo
{
	uint16_t usX;
	uint16_t usY;
	uint16_t usWidth;
	uint16_t usHeight;

}IT8951AreaImgInfo;

typedef struct
{
	uint16_t usPanelW;
	uint16_t usPanelH;
	uint16_t usImgBufAddrL;
	uint16_t usImgBufAddrH;
	uint16_t usFWVersion[8]; 	//16 Bytes String
	uint16_t usLUTVersion[8]; 	//16 Bytes String
}I80IT8951DevInfo;

//Built in I80 Command Code
#define IT8951_TCON_SYS_RUN      0x0001
#define IT8951_TCON_STANDBY      0x0002
#define IT8951_TCON_SLEEP        0x0003
#define IT8951_TCON_REG_RD       0x0010
#define IT8951_TCON_REG_WR       0x0011
#define IT8951_TCON_MEM_BST_RD_T 0x0012
#define IT8951_TCON_MEM_BST_RD_S 0x0013
#define IT8951_TCON_MEM_BST_WR   0x0014
#define IT8951_TCON_MEM_BST_END  0x0015
#define IT8951_TCON_LD_IMG       0x0020
#define IT8951_TCON_LD_IMG_AREA  0x0021
#define IT8951_TCON_LD_IMG_END   0x0022

//I80 User defined command code
#define USDEF_I80_CMD_DPY_AREA     0x0034
#define USDEF_I80_CMD_GET_DEV_INFO 0x0302
#define USDEF_I80_CMD_DPY_BUF_AREA 0x0037
//Panel
#define IT8951_PANEL_WIDTH   1024 //it Get Device information
#define IT8951_PANEL_HEIGHT   758

//Rotate mode
#define IT8951_ROTATE_0     0
#define IT8951_ROTATE_90    1
#define IT8951_ROTATE_180   2
#define IT8951_ROTATE_270   3

//Pixel mode , BPP - Bit per Pixel
#define IT8951_2BPP   0
#define IT8951_3BPP   1
#define IT8951_4BPP   2
#define IT8951_8BPP   3

//Waveform Mode
#define IT8951_MODE_0   0
#define IT8951_MODE_1   1
#define IT8951_MODE_2   2
#define IT8951_MODE_3   3
#define IT8951_MODE_4   4

//Endian Type
#define IT8951_LDIMG_L_ENDIAN   0
#define IT8951_LDIMG_B_ENDIAN   1

//Auto LUT
#define IT8951_DIS_AUTO_LUT   0
#define IT8951_EN_AUTO_LUT    1

//LUT Engine Status
#define IT8951_ALL_LUTE_BUSY 0xFFFF

//-----------------------------------------------------------------------
// IT8951 TCon Registers defines
//-----------------------------------------------------------------------
//Register Base Address
#define DISPLAY_REG_BASE 0x1000               //Register RW access for I80 only

//Base Address of Basic LUT Registers
#define LUT0EWHR  (DISPLAY_REG_BASE + 0x00)   //LUT0 Engine Width Height Reg
#define LUT0XYR   (DISPLAY_REG_BASE + 0x40)   //LUT0 XY Reg
#define LUT0BADDR (DISPLAY_REG_BASE + 0x80)   //LUT0 Base Address Reg
#define LUT0MFN   (DISPLAY_REG_BASE + 0xC0)   //LUT0 Mode and Frame number Reg
#define LUT01AF   (DISPLAY_REG_BASE + 0x114)  //LUT0 and LUT1 Active Flag Reg

//Update Parameter Setting Register
#define UP0SR (DISPLAY_REG_BASE + 0x134)      //Update Parameter0 Setting Reg

#define UP1SR     (DISPLAY_REG_BASE + 0x138)  //Update Parameter1 Setting Reg
#define LUT0ABFRV (DISPLAY_REG_BASE + 0x13C)  //LUT0 Alpha blend and Fill rectangle Value
#define UPBBADDR  (DISPLAY_REG_BASE + 0x17C)  //Update Buffer Base Address
#define LUT0IMXY  (DISPLAY_REG_BASE + 0x180)  //LUT0 Image buffer X/Y offset Reg
#define LUTAFSR   (DISPLAY_REG_BASE + 0x224)  //LUT Status Reg (status of All LUT Engines)

#define BGVR      (DISPLAY_REG_BASE + 0x250)  //Bitmap (1bpp) image color table


//-------Memory Converter Registers----------------
#define MCSR_BASE_ADDR 0x0200
#define MCSR (MCSR_BASE_ADDR  + 0x0000)
#define LISAR (MCSR_BASE_ADDR + 0x0008)

#define cPreamble 0x6000
#define wPreamble 0x0000
#define rPreamble 0x1000

void setClks();
void RLEDinit();
void RLEDtoggle();
void RGBLEDinit();
void SPIinit();
void delay_ms(uint32_t val);
void SPIsend(uint16_t data);
uint16_t SPIread();
void mem_write(uint16_t addr, uint16_t byte);
uint16_t mem_read(uint16_t addr);                // read a byte from provided address
uint32_t mem_id();
void mem_bwrite(uint32_t addr, int num, char bytes_buffer[]);	// write <num> of bytes starting at address
void mem_bread(uint32_t addr, int num, char bytes_buffer[]);	// read num bytes from addr and store on bytes_buffer
void delay_nop();
void CMDwrite(uint16_t cmd);
void Datawrite(uint16_t data);
void BurstDataWrite(uint16_t* pwBuf, uint16_t ulSizeWordCnt);
uint16_t Dataread();
void BurstDataRead(uint16_t* pwBuf, uint16_t ulSizeWordCnt);
void GetIT8951SystemInfo(void* pBuf);
void HostInit();
void IT8951DisplayExample();
void IT8951WaitForDisplayReady();
void IT8951HostAreaPackedPixelWrite(IT8951LdImgInfo* pstLdImgInfo,IT8951AreaImgInfo* pstAreaImgInfo);
void IT8951SetImgBufBaseAddr(uint16_t ulImgBufAddr);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
