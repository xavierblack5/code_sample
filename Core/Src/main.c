// LAB7:  SPI Xavier Black
//
//
// General-purpose timer cookbook for STM32 microcontrollers:
//   https://www.st.com/resource/en/application_note/dm00236305-generalpurpose-timer-cookbook-for-stm32-microcontrollers-stmicroelectronics.pdf
//	Setup for 10-segment LED
//               CN9: Bottom Left
//                 +------------+
//                 |    ....    |
//                      ....GND | ground
//                 |    ....PE2 | 9
//  			   |     -- PE4 | 8
//  			   |     -- PE5 | 7
//                 |     -- PE6 | 6
//   			   |     -- PE3 | 5
//                 |     -- PF8	| 4
//    			   |     -- PF7 | 3
//  			   |     -- PF9 | 2
//               0 | PG0 -- PG1 | 1
//                 +------------+
///////////////////////////////////////////////////////////////////
#include "main.h"
#include <string.h>
//Global structures and variables
I80IT8951DevInfo gstI80DevInfo;
uint8_t* gpFrameBuf;
//uint8_t FrameBuf[59520];
uint32_t gulImgBufAddr; //IT8951 Image buffer address

//////////////////////////////////
/*         Main Function        */
//////////////////////////////////
int main(void) {

	//Push another invalid to get data
    setClks();     // Clocks are ready to use, 16Mhz for system
    RGBLEDinit();    //  LEDs are ready to us
    SPIinit();
//    uint16_t addr = 0x0208;
//    uint16_t data = 0x6235;
//    mem_write(0x0004, 0x0001);
//    mem_write(addr, data);
//    uint16_t d = mem_read(addr);
    IT8951Display1bppExample();
//    IT8951Display1bppExample2();
  IT8951Display1bppExample3();

    while(1){

    }
}

/////////////////////
// SPI Functions
/////////////////////

void SPIinit() {
 	RCC->AHB2ENR|= 0x1<<4; //Enable clock to GPIOE
	RCC->AHB2ENR|= 0x3<<2; //Enable clock to GPIOC and D
	GPIOC->MODER &= ~(1<<2);
	GPIOC->MODER |= 1<<3;		// Set PC1 to AF
	GPIOC->AFR[0] |= (0x3<<4);	//AF3 for SPI2_MOSI
	GPIOD->MODER &= ~(1<<6);
	GPIOD->MODER |= 1<<7;		// Set PD3 to AF
	GPIOD->AFR[0] |= (0x5<<12);	//AF5 for SPI2_MISO
	GPIOD->MODER &= ~(1<<1);
	GPIOD->MODER |= 1;		//Set PD0 to output for CS
	GPIOD->MODER &= ~(1<<2);
	GPIOD->MODER |= 1<<3;		// Set PD1 to AF
	GPIOD->AFR[0] |= (0x5<<4); //AF5 for SPI1_SCK

	RCC->APB1ENR1|= 0x1<<14; //Enable clock to SPI2
	SPI2->CR1 |= 0x1<<2 | 0x1<<9; //Put in master mode and software slave management and prescaler for BR /8
	SPI2->CR2 |= 0xF<<8 | 0x1<<2; //Data size for transfers and 16bit for RXFiFo event

	// Setup GPIO PE4 for HRDY
	GPIOE->MODER &= ~(0x3<<8); //Clear the two bits for pin PE4 to input
-

	SPI2->CR1 |= 0x1<<6; //Enable the SPI
}

void mem_write(uint16_t addr, uint16_t data) {

	uint16_t opCode = 0x0011;	//Op code for writing to a memory address


	CMDwrite(opCode);
	Datawrite(addr);
	Datawrite(data);

}

void GetIT8951SystemInfo(void* pBuf)
{
	uint16_t* pusWord = (uint16_t*)pBuf;
	I80IT8951DevInfo* pstDevInfo;

	CMDwrite(USDEF_I80_CMD_GET_DEV_INFO);

	//Burst Read Request for SPI interface only
	BurstDataRead(pusWord, sizeof(I80IT8951DevInfo));//Polling HRDY for each words(2-bytes) if possible


	//Show Device information of IT8951
	pstDevInfo = (I80IT8951DevInfo*)pBuf;
	printf("Panel(W,H) = (%d,%d)\r\n",
	pstDevInfo->usPanelW, pstDevInfo->usPanelH );
	printf("Image Buffer Address = %X\r\n",
	pstDevInfo->usImgBufAddrL | (pstDevInfo->usImgBufAddrH << 16));
	//Show Firmware and LUT Version
	printf("FW Version = %s\r\n", (uint8_t*)pstDevInfo->usFWVersion);
	printf("LUT Version = %s\r\n", (uint8_t*)pstDevInfo->usLUTVersion);
}

void HostInit()
{
	//Get Device Info
	GetIT8951SystemInfo(&gstI80DevInfo);
	//Host Frame Buffer allocation
	gpFrameBuf = malloc((gstI80DevInfo.usPanelW * gstI80DevInfo.usPanelH)/4);
	//Get Image Buffer Address of IT8951
	gulImgBufAddr = gstI80DevInfo.usImgBufAddrL | (gstI80DevInfo.usImgBufAddrH << 16);

	//Set to Enable I80 Packed mode
	mem_write(0x0004, 0x0001);
}

void IT8951WaitForDisplayReady()
{
	//Check IT8951 Register LUTAFSR => NonZero �V Busy, 0 - Free
	while(mem_read(LUTAFSR));
}

void IT8951DisplayArea(uint16_t usX, uint16_t usY, uint16_t usW, uint16_t usH, uint16_t usDpyMode)
{
	//Send I80 Display Command (User defined command of IT8951)
	CMDwrite(USDEF_I80_CMD_DPY_AREA); //0x0034
	//Write arguments
	Datawrite(usX);
	Datawrite(usY);
	Datawrite(usW);
	Datawrite(usH);
	Datawrite(usDpyMode);
}

void IT8951SetImgBufBaseAddr(uint16_t ulImgBufAddr)
{
	uint16_t usWordH = (uint16_t)((ulImgBufAddr >> 16) & 0x0000FFFF);
	uint16_t usWordL = (uint16_t)( ulImgBufAddr & 0x0000FFFF);
	//Write LISAR Reg
	mem_write(LISAR + 2 ,usWordH);
	mem_write(LISAR ,usWordL);
}

void IT8951LoadImgAreaStart(IT8951LdImgInfo* pstLdImgInfo ,IT8951AreaImgInfo* pstAreaImgInfo)
{
	uint16_t usArg[5];
    //Setting Argument for Load image start
    usArg[0] = (pstLdImgInfo->usEndianType << 8 )
    |(pstLdImgInfo->usPixelFormat << 4)
    |(pstLdImgInfo->usRotate);
    usArg[1] = pstAreaImgInfo->usX;
    usArg[2] = pstAreaImgInfo->usY;
    usArg[3] = pstAreaImgInfo->usWidth;
    usArg[4] = pstAreaImgInfo->usHeight;
    //Send Cmd and Args
    LCDSendCmdArg(IT8951_TCON_LD_IMG_AREA , usArg , 5);
}

void IT8951LoadImgEnd(void)
{
    CMDwrite(IT8951_TCON_LD_IMG_END);
}

void LCDSendCmdArg(uint16_t usCmdCode,uint16_t* pArg, uint16_t usNumArg)
{
	 uint16_t i;
     //Send Cmd code
     CMDwrite(usCmdCode);
     //Send Data
     for(i=0;i<usNumArg;i++)
     {
    	 CMDwrite(pArg[i]);
     }
}
//extern const unsigned char pond[];
extern const unsigned char BSU[];
void IT8951HostAreaPackedPixelWrite(IT8951LdImgInfo* pstLdImgInfo,IT8951AreaImgInfo* pstAreaImgInfo)
{
	uint16_t i,j;
	//Source buffer address of Host
	uint16_t* pusFrameBuf = (uint16_t*)pstLdImgInfo->ulStartFBAddr;



	//Set Image buffer(IT8951) Base address
	IT8951SetImgBufBaseAddr(pstLdImgInfo->ulImgBufBaseAddr);
	//Send Load Image start Cmd
	IT8951LoadImgAreaStart(pstLdImgInfo , pstAreaImgInfo);
	printf("IT8951HostAreaPackedPixelWrite01\r\n");
	//Host Write Data
	for(j=0;j< pstAreaImgInfo->usHeight;j++)
		{
			 for(i=0;i< pstAreaImgInfo->usWidth/2;i++)
				{
						//Write a Word(2-Bytes) for each time
				 	 	Datawrite(*pusFrameBuf);
						pusFrameBuf++;
				}
		}
	printf("IT8951HostAreaPackedPixelWrite02\r\n");
	//Send Load Img End Command
	IT8951LoadImgEnd();
}


void IT8951Load1bppImage(uint8_t* p1bppImgBuf, uint16_t usX, uint16_t usY, uint16_t usW, uint16_t usH)
{
	  IT8951LdImgInfo stLdImgInfo;
    IT8951AreaImgInfo stAreaImgInfo;

    //Setting Load image information
    stLdImgInfo.ulStartFBAddr    = (uint32_t) p1bppImgBuf;
    stLdImgInfo.usEndianType     = IT8951_LDIMG_L_ENDIAN;
    stLdImgInfo.usPixelFormat    = IT8951_8BPP; //we use 8bpp because IT8951 does not support 1bpp mode for load image�Aso we use Load 8bpp mode ,but the transfer size needs to be reduced to Size/8
    stLdImgInfo.usRotate         = IT8951_ROTATE_0;
    stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;
    //Set Load Area
    stAreaImgInfo.usX      = usX/8;
    stAreaImgInfo.usY      = usY;
    stAreaImgInfo.usWidth  = usW/8;//1bpp, Changing Transfer size setting to 1/8X of 8bpp mode
    stAreaImgInfo.usHeight = usH;
    printf("IT8951HostAreaPackedPixelWrite [wait]\n\r");
    //Load Image from Host to IT8951 Image Buffer
    IT8951HostAreaPackedPixelWrite(&stLdImgInfo, &stAreaImgInfo);//Display function 2
}

void IT8951DisplayArea1bpp(uint16_t usX, uint16_t usY, uint16_t usW, uint16_t usH, uint16_t usDpyMode, uint16_t ucBGGrayVal, uint16_t ucFGGrayVal)
{
    //Set Display mode to 1 bpp mode - Set 0x18001138 Bit[18](0x1800113A Bit[2])to 1
    mem_write(UP1SR+2, mem_read(UP1SR+2) | (1<<2));

    //Set BitMap color table 0 and 1 , => Set Register[0x18001250]:
    //Bit[7:0]: ForeGround Color(G0~G15)  for 1
    //Bit[15:8]:Background Color(G0~G15)  for 0
    mem_write(BGVR, (ucBGGrayVal<<8) | ucFGGrayVal);

    //Display
    IT8951DisplayArea( usX, usY, usW, usH, usDpyMode);
    IT8951WaitForDisplayReady();

    //Restore to normal mode
    mem_write(UP1SR+2, mem_read(UP1SR+2) & ~(1<<2));
}


void IT8951Display1bppExample()
{
    IT8951AreaImgInfo stAreaImgInfo;

    //Host Initial
    HostInit(); //Test Function 1
    //Prepare image
    //Write pixel 0x00(Black) to Frame Buffer
    //or you can create your image pattern here..
    memset(gpFrameBuf, 0x00, (gstI80DevInfo.usPanelW * gstI80DevInfo.usPanelH)/8);//Host Frame Buffer(Source)

    //Check TCon is free ? Wait TCon Ready (optional)
    IT8951WaitForDisplayReady();

    //Load Image and Display
    //Set Load Area
    stAreaImgInfo.usX      = 0;
    stAreaImgInfo.usY      = 0;
    stAreaImgInfo.usWidth  = gstI80DevInfo.usPanelW;
    stAreaImgInfo.usHeight = gstI80DevInfo.usPanelH;
    //Load Image from Host to IT8951 Image Buffer
    IT8951Load1bppImage(gpFrameBuf, stAreaImgInfo.usX, stAreaImgInfo.usY, stAreaImgInfo.usWidth, stAreaImgInfo.usHeight);//Display function 4, Arg

    //Display Area - (x,y,w,h) with mode 2 for Gray Scale
    //e.g. if we want to set b0(Background color) for Black-0x00 , Set b1(Foreground) for White-0xFF
    IT8951DisplayArea1bpp(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 0, 0x00, 0xFF);
}


void IT8951Display1bppExample2()
{
   IT8951AreaImgInfo stAreaImgInfo;

    //Host Initial
//    HostInit(); //Test Function 1
    //Prepare image
    //Write pixel 0x00(Black) to Frame Buffer
    //or you can create your image pattern here..
    memset(gpFrameBuf, 0x00, (gstI80DevInfo.usPanelW * gstI80DevInfo.usPanelH)/8);//Host Frame Buffer(Source)

    //Check TCon is free ? Wait TCon Ready (optional)
    IT8951WaitForDisplayReady();

    //Load Image and Display
    //Set Load Area
    stAreaImgInfo.usX      = 0;
    stAreaImgInfo.usY      = 0;
    stAreaImgInfo.usWidth  = gstI80DevInfo.usPanelW;
    stAreaImgInfo.usHeight = gstI80DevInfo.usPanelH;
    //Load Image from Host to IT8951 Image Buffer
    IT8951Load1bppImage(gpFrameBuf, stAreaImgInfo.usX, stAreaImgInfo.usY, stAreaImgInfo.usWidth, stAreaImgInfo.usHeight);//Display function 4, Arg

    //Display Area - (x,y,w,h) with mode 2 for Gray Scale
    //e.g. if we want to set b0(Background color) for Black-0x00 , Set b1(Foreground) for White-0xFF
    IT8951DisplayArea1bpp(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 2, 0x00, 0xFF);
}

void IT8951Display1bppExample3()
{
   IT8951AreaImgInfo stAreaImgInfo;

    //Host Initial
//    HostInit(); //Test Function 1
    //Prepare image
    //Write pixel 0x00(Black) to Frame Buffer
    //or you can create your image pattern here..
	uint32_t i;

	for (i = 0;i < ((gstI80DevInfo.usPanelW*gstI80DevInfo.usPanelH)/8);i++)
	{
//		gpFrameBuf[i] = pic[i];					//9.7inch e-paper 1200*825
		gpFrameBuf[i] = BSU[i]; 	//6inch e-paper    800*600
	}

    //Check TCon is free ? Wait TCon Ready (optional)
    IT8951WaitForDisplayReady();

    //Load Image and Display
    //Set Load Area
    stAreaImgInfo.usX      = 0;
    stAreaImgInfo.usY      = 0;
    stAreaImgInfo.usWidth  = gstI80DevInfo.usPanelW;
    stAreaImgInfo.usHeight = gstI80DevInfo.usPanelH;
    //Load Image from Host to IT8951 Image Buffer
    IT8951Load1bppImage(gpFrameBuf, stAreaImgInfo.usX, stAreaImgInfo.usY, stAreaImgInfo.usWidth, stAreaImgInfo.usHeight);//Display function 4, Arg

    //Display Area - (x,y,w,h) with mode 2 for Gray Scale
    //e.g. if we want to set b0(Background color) for Black-0x00 , Set b1(Foreground) for White-0xFF
    IT8951DisplayArea1bpp(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 2, 0x00, 0xFF);
}

void IT8951DisplayExample()
{
	IT8951LdImgInfo stLdImgInfo;
	IT8951AreaImgInfo stAreaImgInfo;

	//Host Initial
	HostInit(); //Test Function 1
	//Prepare image
	//Write pixel 0xF0(White) to Frame Buffer
	memset(gpFrameBuf, 0xF0, (gstI80DevInfo.usPanelW * gstI80DevInfo.usPanelH)/2);

	//Check TCon is free ? Wait TCon Ready (optional)
	IT8951WaitForDisplayReady();

//	//--------------------------------------------------------------------------------------------
//	//      initial display - Display white only
//	//--------------------------------------------------------------------------------------------
//	//Load Image and Display
//	//Setting Load image information
	stLdImgInfo.ulStartFBAddr    = (uint32_t)gpFrameBuf;
	stLdImgInfo.usEndianType     = IT8951_LDIMG_L_ENDIAN;
	stLdImgInfo.usPixelFormat    = IT8951_8BPP;
	stLdImgInfo.usRotate         = IT8951_ROTATE_0;
	stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;
	//Set Load Area
	stAreaImgInfo.usX      = 0;
	stAreaImgInfo.usY      = 0;
	stAreaImgInfo.usWidth  = gstI80DevInfo.usPanelW;
	stAreaImgInfo.usHeight = gstI80DevInfo.usPanelH;

	printf("IT8951DisplayExample 01\r\n");
	//Load Image from Host to IT8951 Image Buffer
	IT8951HostAreaPackedPixelWrite(&stLdImgInfo, &stAreaImgInfo);//Display function 2
	printf("IT8951DisplayExample 02\r\n");
//	Display Area �V (x,y,w,h) with mode 0 for initial White to clear Panel
	IT8951DisplayArea(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 0);
	printf("IT8951DisplayExample 03\r\n");
//	//--------------------------------------------------------------------------------------------
//	//      Regular display - Display Any Gray colors with Mode 2 or others
//	//--------------------------------------------------------------------------------------------
//	//Preparing buffer to All black (8 bpp image)
//	//or you can create your image pattern here..
	memset(gpFrameBuf, 0x00, gstI80DevInfo.usPanelW * gstI80DevInfo.usPanelH);

	IT8951WaitForDisplayReady();

	//Setting Load image information
	stLdImgInfo.ulStartFBAddr    = (uint32_t)gpFrameBuf;
	stLdImgInfo.usEndianType     = IT8951_LDIMG_L_ENDIAN;
	stLdImgInfo.usPixelFormat    = IT8951_8BPP;
	stLdImgInfo.usRotate         = IT8951_ROTATE_0;
	stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;
	//Set Load Area
	stAreaImgInfo.usX      = 0;
	stAreaImgInfo.usY      = 0;
	stAreaImgInfo.usWidth  = gstI80DevInfo.usPanelW;
	stAreaImgInfo.usHeight = gstI80DevInfo.usPanelH;
//
//	//Load Image from Host to IT8951 Image Buffer
	IT8951HostAreaPackedPixelWrite(&stLdImgInfo, &stAreaImgInfo);//Display function 2
//	//Display Area �V (x,y,w,h) with mode 2 for fast gray clear mode - depends on current waveform
	IT8951DisplayArea(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 2);
}


void IT8951DisplayExample3()
{
	IT8951LdImgInfo stLdImgInfo;
	IT8951AreaImgInfo stAreaImgInfo;
	uint32_t i;

	for (i = 0;i < (gstI80DevInfo.usPanelW*gstI80DevInfo.usPanelH)/4;i++)
	{
//		gpFrameBuf[i] = pic[i];					//9.7inch e-paper 1200*825
		gpFrameBuf[i] = BSU[i]; 	//6inch e-paper    800*600
	}

	IT8951WaitForDisplayReady();

	//Setting Load image information
//	stLdImgInfo.ulStartFBAddr    = (uint32_t)gpFrameBuf;
	stLdImgInfo.usEndianType     = IT8951_LDIMG_L_ENDIAN;
	stLdImgInfo.usPixelFormat    = IT8951_2BPP;
	stLdImgInfo.usRotate         = IT8951_ROTATE_0;
	stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;
	//Set Load Area
	stAreaImgInfo.usX      = 0;
	stAreaImgInfo.usY      = 0;
	stAreaImgInfo.usWidth  = gstI80DevInfo.usPanelW/4;
	stAreaImgInfo.usHeight = gstI80DevInfo.usPanelH;

	//Load Image from Host to IT8951 Image Buffer
	IT8951HostAreaPackedPixelWrite(&stLdImgInfo, &stAreaImgInfo);//Display function 2
	//Display Area �V (x,y,w,h) with mode 2 for fast gray clear mode - depends on current waveform
	IT8951DisplayArea(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 2);
}

uint16_t mem_read(uint16_t addr) {

	uint16_t opCode = 0x0010;	//Op code for writing to a memory address
//	uint8_t opCode2 = 0x10;
//	uint16_t invalid = 0xFFFF;	//byte to send to receive whatever is in the peripherals register
//	uint8_t addr1 = addr>>8;
//	uint8_t addr2 = addr & 0xFF;
	uint16_t data1;

	CMDwrite(opCode);
	Datawrite(addr);
	data1 = Dataread();
	return data1;
}


void CMDwrite(uint16_t cmd) {
	SPI2->CR1  &= ~(0x1<<8);  //Low on CS bit
	GPIOD->ODR ^= 1;	//Low on CS bit
	while(!(GPIOE->IDR & (0x1<<4))); //Busy status on I80 HRDY
	SPIsend(cPreamble); //Send preamble for command
	while(!(GPIOE->IDR & (0x1<<4))); //Busy status on I80 HRDY
	SPIsend(cmd);
	delay_nop();
	while(SPI2->SR & 0x1<<7);	//Wait until the SPI is not busy to set CS high and disable the SPI
	SPI2->CR1  |= 0x1<<8; //High on CS bit
	GPIOD->ODR ^= 1;	//High on CS bit
}

void Datawrite(uint16_t data) {
	SPI2->CR1  &= ~(0x1<<8);  //Low on CS bit
	GPIOD->ODR ^= 1;	//Low on CS bit
	while(!(GPIOE->IDR & (0x1<<4))); //Busy status on I80 HRDY
	SPIsend(wPreamble); //Send preamble for command
	while(!(GPIOE->IDR & (0x1<<4))); //Busy status on I80 HRDY
	SPIsend(data);
	delay_nop();
	while(SPI2->SR & 0x1<<7);	//Wait until the SPI is not busy to set CS high and disable the SPI
	SPI2->CR1  |= 0x1<<8; //High on CS bit
	GPIOD->ODR ^= 1;	//High on CS bit
}

void BurstDataWrite(uint16_t* pwBuf, uint16_t ulSizeWordCnt)
{
	uint32_t i;

	SPI2->CR1  &= ~(0x1<<8);  //Low on CS bit
	GPIOD->ODR ^= 1;	//Low on CS bit

	while(!(GPIOE->IDR & (0x1<<4))); //Busy status on I80 HRDY
	SPIsend(wPreamble); //Send preamble for command

	while(!(GPIOE->IDR & (0x1<<4))); //Busy status on I80 HRDY
	//Send Data
	for(i=0;i<ulSizeWordCnt;i++)
	{
		SPIsend(pwBuf[i]);
	}

	delay_nop();

	while(SPI2->SR & 0x1<<7);	//Wait until the SPI is not busy to set CS high and disable the SPI
	SPI2->CR1  |= 0x1<<8; //High on CS bit
	GPIOD->ODR ^= 1;	//High on CS bit
}

uint16_t Dataread() {
	uint16_t data;
	SPI2->CR1  &= ~(0x1<<8);  //Low on CS bit
	GPIOD->ODR ^= 1;	//Low on CS bit
	while(!(GPIOE->IDR & (0x1<<4))); //Busy status on I80 HRDY
	SPIsend(rPreamble); //Send preamble for command
	while(!(GPIOE->IDR & (0x1<<4))); //Busy status on I80 HRDY
	SPIread();//Dummy
	while(!(GPIOE->IDR & (0x1<<4))); //Busy status on I80 HRDY
	data = SPIread();
	while(SPI2->SR & 0x1<<7);	//Wait until the SPI is not busy to set CS high and disable the SPI
	SPI2->CR1  |= 0x1<<8; //High on CS bit
	GPIOD->ODR ^= 1;	//High on CS bit
	return data;
}

void BurstDataRead(uint16_t* pwBuf, uint16_t ulSizeWordCnt)
{
	uint32_t i;

	SPI2->CR1  &= ~(0x1<<8);  //Low on CS bit
	GPIOD->ODR ^= 1;	//Low on CS bit

	//Send Preamble before reading data
	while(!(GPIOE->IDR & (0x1<<4))); //Busy status on I80 HRDY
	SPIsend(rPreamble); //Send preamble for command

	while(!(GPIOE->IDR & (0x1<<4))); //Busy status on I80 HRDY
	SPIread();//Dummy

	for(i=0;i<ulSizeWordCnt;i++)
	{
		while(!(GPIOE->IDR & (0x1<<4))); //Busy status on I80 HRDY
		//Read Data
		pwBuf[i]= SPIread();
	}
	while(SPI2->SR & 0x1<<7);	//Wait until the SPI is not busy to set CS high and disable the SPI
	SPI2->CR1  |= 0x1<<8; //High on CS bit
	GPIOD->ODR ^= 1;	//High on CS bit
}

void SPIsend(uint16_t data){
	uint16_t word;
	while(!(SPI2->SR & 0x1<<1));
	SPI2->DR = data; //Send op-code for read id
	while(!(SPI2->SR & 0x1));
	word = SPI2->DR;
}

uint16_t SPIread() {
	uint16_t invalid = 0xFFFF;
	uint16_t word;
	while(!(SPI2->SR & 0x1<<1));
	SPI2->DR = invalid; //Send op-code for read id
	while(!(SPI2->SR & 0x1));
	word = SPI2->DR;
	return word;
}


/////////////////////
// Helping Functions
/////////////////////

void delay_ms(uint32_t val){
    // Using SysTick Timer:
    //        A delay function that can stall CPU 1msec to 100 sec, depending onval.
    //
    // useful link: https://www.youtube.com/watch?v=aLCUDv_fgoU
    //
    // The concept here is to make a delay block using SysTick timer for a delay of1 msec.
    // The 1 msec delay will be inside a for loop that will loop for val timeswhich will
    // result in a delay of as short as 1msec (for val=1) and as long as 1msec*0xffff_ffff (4,294,967.295 sec)
    // Here are the steps to set the SysTick to delay 1 msec
    //   1- Set the load register to achieve 1msec. Note that you have two optionsto source your
    //      timer clock. Ohe counterne is to use the HSI clock of 16MHz while the other touse 16MHz/8.
    //   2- Set t current value to 0 so that the counter start
    //   3- Enable the counter and the bit to select which clock you want to use
    // Now the counter is counting down once it reaches 0, a flag in the controlregister
    // will be set -- use that flag.
    SysTick->LOAD = 2000-1;   /* reload with number of clocks per millisecond (use N-1)*/
    SysTick->VAL = 0;          /* clear current value register */
    SysTick->CTRL = 0x1;       /* Enable the timer,  CLKSOURCE= 0 (/8), 1 HCLK */
    for (uint32_t i=0; i<val; i++){
            while((SysTick->CTRL & 0x10000) == 0){
            }; /* wait until the COUNTFLAG is set */
        }
    SysTick->CTRL = 0;      /* Stop the timer (Enable = 0) */
}

void RGBLEDinit(){
// Enable clock going to GPIOA, GPIOB, GPIOC
RCC->AHB2ENR|= 0x7;
// Set up the mode
GPIOA->MODER |= 1<<18; // setting bit 18 for PA9
GPIOA->MODER &= ~(1<<19);
GPIOB->MODER |= 1<<14; // setting bit 14 for PB7
GPIOB->MODER &= ~(1<<15);
GPIOC->MODER |= 1<<14; // setting bit 14 for PC7
GPIOC->MODER &= ~(1<<15);

}
void setClks(){
RCC->APB1ENR1 |=1<<28;   // Enable the power interface clock by setting the PWRENbits
RCC->APB1ENR2 |=0x1;     // Enable LPUART1EN clock
RCC->CCIPR1   |=0x800;   // 01 for HSI16 to be used for LPUART1
RCC->CCIPR1   &= ~(0x400);
RCC->CFGR     |=0x1;     // Use HSI16 as SYSCLK
RCC->CR       |=0x161;   // MSI clock enable; MSI=4 MHz; HSI16 clock enable
}


void delay_nop()
{
	uint16_t i;
	for (i=0;i<1000;i++);
}
