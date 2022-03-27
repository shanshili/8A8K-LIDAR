#ifndef __OLED_H
#define __OLED_H			  	 
#include "8a8k.h"
#include "sys.h"
 
//--------------OLED��������---------------------
#define PAGE_SIZE    8
#define XLevelL		   0x00
#define XLevelH		   0x10
#define YLevel       0xB0
#define	Brightness	 0xFF 
#define WIDTH 	     128
#define HEIGHT 	     64	


//-------------д��������ݶ���-------------------
#define OLED_CMD     0	//д����
#define OLED_DATA    1	//д���� 
   						  
//-----------------OLED�˿ڶ���----------------
sbit OLED_CS = P1^3;    //Ƭѡ�ź�           P13
sbit OLED_DC = P1^2;   //����/��������ź�  P12
sbit OLED_RST = P3^3;   //��λ�ź�          P33

//-----------------OLED�˿ڲ�������---------------- 
#define OLED_CS_Clr()  OLED_CS = 0
#define OLED_CS_Set()  OLED_CS = 1

#define OLED_DC_Clr()  OLED_DC = 0
#define OLED_DC_Set()  OLED_DC = 1
 					   
#define OLED_RST_Clr()  OLED_RST = 0
#define OLED_RST_Set()  OLED_RST = 1

//OLED�����ú���
void OLED_WR_Byte(unsigned dat,unsigned cmd);     							   		    
void OLED_Set_Pos(u8 x, u8 y);
void OLED_Reset(void);	   							   		    
void OLED_Init(void);
void OLED_Set_Pixel(u8 x, u8 y,u8 color);
void OLED_Display(void);
void OLED_Clear(unsigned dat);  

#endif