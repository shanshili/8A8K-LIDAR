/*
串口2、3测试
蜜汁问题导致串口2、3发送数据一直错误
最后还是直接引用例程解决
（抱拳）例程yyds
*/

#include "8a8k.h"
#include "sys.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>

#define FOSC 24000000UL 
#define BRT (65536 - FOSC / 115200 / 4) 
u32 Receivenum = 123456;
u8 Receive;  
u8 Recive_table[]=0;
u8 flag = 0;
u8 save = 0;
bit busy; 
char wptr; 
char rptr; 
char buffer[16]; 

//*******************************--MQTT message--***********************************//

code const u8 MessageConnect[115] = {
		0x10,0x70,0x00,0x04,0x4D,0x51,0x54,0x54,0x04,0xC2,0x00,0x64,0x00,0x28,0x32,
	  0x32,0x30,0x33,0x31,0x31,0x7c,0x73,0x65,0x63,0x75,0x72,0x65,0x6d,0x6f,0x64,
		0x65,0x3d,0x33,0x2c,0x73,0x69,0x67,0x6e,0x6d,0x65,0x74,0x68,0x6f,0x64,0x3d,
		0x68,0x6d,0x61,0x63,0x73,0x68,0x61,0x31,0x7c,0x00,0x10,0x65,0x73,0x70,0x31,
		0x26,0x61,0x31,0x30,0x75,0x58,0x33,0x6e,0x6e,0x30,0x53,0x5a,0x00,0x28,0x65,
		0x61,0x36,0x64,0x32,0x36,0x37,0x33,0x32,0x31,0x66,0x34,0x36,0x62,0x36,0x33,
		0x65,0x62,0x34,0x61,0x35,0x38,0x37,0x38,0x63,0x65,0x30,0x38,0x33,0x31,0x34,
		0x66,0x62,0x30,0x66,0x66,0x37,0x36,0x31,0x30
};

code const u8 MessageSubscribeTopic[57] = {
    0x82,0x34,0x00,0x0A,0x00,0x2f,0x2f,0x73,0x79,0x73,0x2f,0x61,0x31,0x30,0x75,
		0x58,0x33,0x6e,0x6e,0x30,0x53,0x5a,0x2f,0x65,0x73,0x70,0x31,0x2f,0x74,0x68,
		0x69,0x6e,0x67,0x2f,0x65,0x76,0x65,0x6e,0x74,0x2f,0x70,0x72,0x6f,0x70,0x65,
		0x72,0x74,0x79,0x2f,0x70,0x6f,0x73,0x74,0x00
};

//***********************************--Uart init--***************************************//

void Uart1_Init(void)  //串口1 模式1  8位数据  115200bps@24.000MHz  定时器1  RxD P3.0 TxD P3.1
{
		SCON = 0x50;		//8位数据,可变波特率
		AUXR |= 0x40;		//定时器1时钟为Fosc,即1T
		AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
		TMOD &= 0x0F;		//设定定时器1为16位自动重装方式
		TL1 = 0xCC;		//设定定时初值
		TH1 = 0xFF;		//设定定时初值
		ET1 = 0;		//禁止定时器1中断
	  EA = 1;
	  ES = 1;
		TR1 = 1;		//启动定时器1
}

/*
void Uart2_Init(void)		//115200bps@24.000MHz 定时器2  RxD P1.0 TxD P1.1
{
//	  P_SW2 = 0X00;
		S2CON = 0x50;		//8位数据,可变波特率
		AUXR |= 0x04;		//定时器时钟1T模式
		T2L = 0xCC;		//设置定时初始值
		T2H = 0xFF;		//设置定时初始值
		AUXR |= 0x10;	//定时器2开始计时
//	  IE2 |= 0x01; 
}
*/
void Uart2_Init() 
{ 
		S2CON = 0x10; 
		T2L = BRT; 
		T2H = BRT >> 8; 
		AUXR = 0x14;
		wptr = 0x00; 
		rptr = 0x00; 
		busy = 0; 
} 
/*
void Uart3_Init(void)		//115200bps@24.000MHz  定时器3  RxD P0.0 TxD P0.1
{	 
	  P0M0 = 0X03;
	  P0M1 = 0X00;
//	  P0PU = 0Xff;
//	 P0M1 &= 0xFA;   P0M0 &= 0xFA;                   //设置P0.0  P0.2为准双向口  
//    P0M1 &= 0xF5;   P0M0 |= 0x0A;                   //设置P0.1  P0.3为推挽输出  
//	  P_SW2 = 0X80;	
		S3CON = 0x10;		//8位数据,可变波特率
		S3CON |= 0x40;		//串口3选择定时器3为波特率发生器
		T4T3M |= 0x02;		//定时器时钟1T模式
		T3L = 0xCC;		//设置定时初始值
		T3H = 0xFF;		//设置定时初始值
		T4T3M |= 0x08;		//定时器3开始计时
	  IE2 |= 0x08; 
}
*/
//********************************************************************************************//


void Send_Uart1(u8 value) 
{  
		ES=0;                 
		TI=0;                 
		SBUF=value;           
		while(TI==0);          
		TI=0;                 
		ES=1;                
}

/*
void Send_Uart2(u8 value) 
{  
		IE2 &= ~0x01;                 
		S2CON &= ~0X02;                 
		S2BUF=value;           
		while(S2CON & 0Xff== 0x50);          
		S2CON &= ~0X02;                
		IE2 |= 0x01;                
}
*/
void Send_Uart2(char dat) 
{ 
		while (busy); 
		busy = 1; 
		S2BUF = dat; 
} 

/*
void Send_Uart3(u8 value) 
{  
		IE2 &= 0xf7;                 
		S3CON &= 0Xfd;                 
		S3BUF=value;           
		while(S3CON & 0XFF == 0x50);          
		S3CON &= 0Xfd;                 
		IE2 |= 0x08;                
}
*/
/*
void toJson(char *oristr, const char *str, int value)
{
    char distance[] = "000";
    strcat(oristr, "\"");  // {"id":"1234567","version":"1.0","params":{"
    strcat(oristr, str);   // {"id":"1234567","version":"1.0","params":{"temp
    strcat(oristr, "\":"); // {"id":"1234567","version":"1.0","params":{"temp"：
    if (value < 0)
    {
        distance[0] = '-';   //如果小于0，加负号
    }
    else
    {
        distance[0] = ' ';
    }
    distance[1] = value / 10 + '0';
    distance[2] = value % 10 + '0';
    strcat(oristr, distance); // {"id":"1234567","version":"1.0","params":{"temp"：温度值
    strcat(oristr, ",");  // {"id":"1234567","version":"1.0","params":{"temp"：温度值，
}
*/
//**************************************--esp8266--*****************************************//

//Commend Send
void ESP8266_Set(u8 *puf) 	
{    
//		Clear_Table();//每此发送命令前清空前一次接收到的回复
	  while(*puf!='\0')                    
		{   
				Send_Uart2(*puf);               
				puf++;    
		}            
//		Send_Uart1('\r');                       
//		Send_Uart1('\n');		
}

/*
//Esp Initialize
void esp_int()
{	    
		ESP8266_Set("AT+RST");
	  Delay1000ms();	  Delay1000ms();	  Delay1000ms();
	  ESP8266_Set("ATE0");
	  Delay100ms();	
		ESP8266_Set("AT+CWMODE_CUR=1");//station pattern
		Delay100ms();	
		ESP8266_Set("AT+CIPMUX=0");//Enabling single connection
		Delay100ms();			
	  ESP8266_Set("AT+CWJAP_CUR=\"316316\",\"@@@@@@@@\"");//Connect to the ap
		delay_s(6);
}

//Connect to IOT platform and Subscribe
void esp_IOT()
{
	  unsigned char i;
	  ESP8266_Set("AT+CIPSTART=\"TCP\",\"a1UUBhveKMv.iot-as-mqtt.cn-shanghai.aliyuncs.com\",1883");
		Delay800ms();
		ESP8266_Set("AT+CIPMODE=1");//使能透传
		Delay100ms();
		ESP8266_Set("AT+CIPSEND");//开始透传
		i = 0;	
		while (i < 115)
		{
			 Send_Uart2(MessageConnect[i++]);		    
		}
		i=0;
		Delay100ms();
		P02 = 0;  //连接成功亮一次灯		
		while (i < 57)
		{
				 Send_Uart2(MessageSubscribeTopic[i++]);		    
		}
		i=0;
		P02 = 1;  //订阅结束灭灯
}

//IOT publish
void IOT_Publish_distance(u32 distance)
{
    // example: { "id":"1234567", "version":"1.0", "params":{"distance":34567}, "method":"thing.event.property.post" }
    unsigned int i,j=0;
    unsigned char id[8] = "1234567";//姑且先写死
    unsigned char str[300] = {0x30};
    unsigned char content[200] = "{\"id\":\""; // 7nums
    //unsigned char contentnum = 7;              // id from 8th
    unsigned char totalnum = 0; // TOPIC+CONTENT
    unsigned char enlargedbit = 0;
    // Random id
//    Adc_Init();  //adc获得随机数
//    for (i = 0; i < 8; i++)
//    {
//        id[i] = '0' + (getRandom() % 10);
//    }
    strcat(content, id);  //连接content 和 id ， 即  {"id":"1234567
    // Payload
    strcat(content, "\",\"version\":\"1.0\",\"params\":{");   //  {"id":"1234567","version":"1.0","params":{
    toJson(content, "distance", distance);                                                       
    content[strlen(content) - 1] = '}';                       //  {"id":"1234567","version":"1.0","params":{"distance": 4 }
    strcat(content, ",\"method\":\"thing.event.property.set\"}");
    // Length
    totalnum = 52 + strlen(content);  //50 ：length  of topic 
		str[1] = totalnum + 0x00;
		str[2] = 0X01;
		enlargedbit = 1;
		str[2 + enlargedbit ] = 0x00;
		str[3 + enlargedbit ] = 0x32; //topic 的长度
    //TOPIC    /没加topic的长度
    for (i = 0; i < 50; i++)
    {
        str[4 + enlargedbit + i] = MessageSubscribeTopic[6+i];
    }
    //CONTENT
    for (i = 0; i < strlen(content); i++)
    {
        str[54 + enlargedbit + i] = content[i];
    }
//    Esp8266_Send(str, 2 + enlargedbit + totalnum);
		while(j<(4 + enlargedbit + totalnum))
		{
				Send_Uart2(str[j++]);
		}
}
*/	
//*****************************************--main--******************************************//

void main()
{  
//	  Uart1_Init();
	  Uart2_Init();
	  IE2 = 0x01; 
    EA = 1; 
//	  esp_int(); 
    while(1)
		{			
			      ESP8266_Set("test");
            delay_s(2);			
//					esp_IOT();
//					IOT_Publish_distance(Receivenum);	
//					Delay800ms();
//					Send_Uart3(0xE0); //连续发送，无法断开
//					Send_Uart3(0x00);	
//					ESP8266_Set("+++");
//					delay_s(2); 		    
//					ESP8266_Set("AT+CIPMODE=0");											
  	}
}


void Uart2Isr() interrupt 8 
{ 
			 if (S2CON & 0x02) 
			 { 
						 S2CON &= ~0x02; 
						 busy = 0; 
			 } 
			 if (S2CON & 0x01) 
			 { 
						 S2CON &= ~0x01; 
						 buffer[wptr++] = S2BUF; 
						 wptr &= 0x0f; 
			 } 
} 
/*
void Uart2_Interrupt() interrupt 8         
{   
      IE2 &= 0xfe;  
			if( S2CON & 0x01 )  
		  {  	
						S2CON &= 0XFE;   
						Receive=S2BUF,                                    
						Recive_table[save]=Receive;				
						if(Recive_table[save]=='K'|| Recive_table[save]=='R')
						{                     
										flag = 0;
										save = 0;
						}                                      
			}   
			else S2CON &= 0Xfd ; 
      IE2 |= 0x01; 		
}
*/
/*
void Uart3Isr() interrupt 17 
{ 
		if (S3CON & 0x02) 
		{ 		
			    S3CON &= 0Xfd; 
		} 
		if (S3CON & 0x01) 
		{ 
					S3CON &= ~0x01; 
					Receive=S3BUF,                                    
					Recive_table[save]=Receive;			
					if(Recive_table[save]=='K'|| Recive_table[save]=='R')
					{                     
						      flag = 0;
						      while(flag<save)
									{
												Send_Uart1(Recive_table[flag++]);
									}
									save = 0;
					}    
					else save++; 
		} 
} 
*/