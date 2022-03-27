#ifndef _SPI_H_
#define _SPI_H_
#include "8a8k.h"
#include "sys.h"


//�����Գ���ʹ�õ������ģ��SPI�ӿ�����
//SPI���������Ŷ����ʱ�����Ŷ��嶼���������޸�
sbit OLED_MOSI = P1^5;   //OLED��SPIд�������� D1
sbit OLED_CLK = P1^4;    //OLED��SPIʱ������   D0

#define OLED_MOSI_Clr()  OLED_MOSI = 0
#define OLED_MOSI_Set()  OLED_MOSI = 1 

#define OLED_CLK_Clr()   OLED_CLK = 0
#define OLED_CLK_Set()   OLED_CLK = 1

//-------------spi��������-----------------/
void SPI_WriteByte(u8 byte);

#endif
