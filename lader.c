/*
此版为旧版，已更新为<laderReceive>
*/
/*
串口1 打印未换算和换算后的数据 R 3.0   T 3.1
串口2 lader接收                R 1.0   T 1.1
串口3 esp8266联网              R 0.0   T 0.1
传角度数据                     R 0.2   T 0.3
完成联网,一直发送3D数据

*协议帧
——————————————————————————————————————
| 帧开始  设备地址  寄存器命令  寄存器地址  数据（mm）  校验结果  帧结束
|——————————————————————————————————————
| ~（1B）    1B        1B         2B         2B         2B     \r\n
——————————————————————————————————————
*OLED
   VCC         DC 5V/3.3V    
   GND           GND        
   D1            P15 
   D0            P14   	         
   RES           P33          
   DC            P12          
   CS            P13                                                                
*/
#include "8a8k.h"
#include "sys.h"
#include "delay.h"
#include "oled.h"
#include "gui.h"
#include "test.h"
#include <stdio.h>
#include <string.h>
bit busy3;
bit busy4;

u8 Receive;  
u8 Recive_table[]=0;
u8 receivedata[4]=0;
u16 Receivenum;
u8 flag = 0;
u8 save = 0;

u8 Receive3;  
u8 Recive_table3[]=0;
u8 flag3 = 0;
u8 save3 = 0;

u8 Receive4;  
u8 Recive_table4[4]=0;
u8 receivedata4[4]=0;
u8 flag4 = 0;
u8 save4 = 0;
u8 length = 0;



//*******************************--MQTT message--***********************************//
code const u8 MessageConnect[114] = {
		0x10,0x70,0x00,0x04,0x4D,0x51,0x54,0x54,0x04,0xC2,0x00,0x64,0x00,0x28,0x32,
	  0x32,0x30,0x33,0x31,0x31,0x7c,0x73,0x65,0x63,0x75,0x72,0x65,0x6d,0x6f,0x64,
		0x65,0x3d,0x33,0x2c,0x73,0x69,0x67,0x6e,0x6d,0x65,0x74,0x68,0x6f,0x64,0x3d,
		0x68,0x6d,0x61,0x63,0x73,0x68,0x61,0x31,0x7c,0x00,0x10,0x65,0x73,0x70,0x31,
		0x26,0x61,0x31,0x30,0x75,0x58,0x33,0x6e,0x6e,0x30,0x53,0x5a,0x00,0x28,0x65,
		0x61,0x36,0x64,0x32,0x36,0x37,0x33,0x32,0x31,0x66,0x34,0x36,0x62,0x36,0x33,
		0x65,0x62,0x34,0x61,0x35,0x38,0x37,0x38,0x63,0x65,0x30,0x38,0x33,0x31,0x34,
		0x66,0x62,0x30,0x66,0x66,0x37,0x36,0x31,0x30
};
code const u8 MessageSubscribeTopic[54] = {
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
void Uart2_Init(void) //115200bps@24.000MHz 定时器2  RxD P4.0 TxD P4.2
{ 
	P_SW2 = 0X01;
	S2CON = 0x50; 
	AUXR |= 0x04;		//定时器时钟1T模式
	T2L = 0xcc; 
	T2H = 0xff;   //初值要放在定时器设置之后（否则会接收00）
	AUXR |= 0x10;	//定时器2开始计时
	IE2 |= 0x01;	
}

void Uart3_Init(void) //115200bps@24.000MHz 定时器3  RxD P0.0 TxD P0.1
{ 
	S3CON = 0x50; 
	T4T3M |= 0x0a;
	T4T3M &= 0xfa;
	T3L = 0xcc; 
	T3H = 0xff; 
	busy3 = 0; 
	IE2 |= 0x08; 	
}

void Uart4_Init(void) //115200bps@24.000MHz 定时器4  RxD P0.2 TxD P0.3
{ 
	S4CON = 0x50; 
	T4L = 0xcc; 
	T4H = 0xff; 
	T4T3M |= 0xa0;
	T4T3M &= 0xaf;
	busy4 = 0; 
	IE2 |= 0x10; 
}
//********************************--Uart Send--*************************************************//
void Send_Uart1(u8 value) //串口打印发送
{  
		ES=0;                 
		TI=0;                 
		SBUF=value;           
		while(TI==0);          
		TI=0;                 
		ES=1;                
}
void Send_Uart3(char dat) //ESP8266 发送
{ 
		while (busy3);//如果busy是1就空转 
		busy3 = 1; 
		S3BUF = dat; 
} 
void Send_Uart4(char dat) 
{ 
		while (busy4); 
		busy4 = 1; 
		S4BUF = dat; 
} 
//***************************************************************************************//
void Clear_Table1(void)  //ladir 接收
{
		memset(Recive_table, 0, sizeof(Recive_table));//把数组txt清零
}
void Clear_Table3(void)  //esp接收
{
		memset(Recive_table3, 0, sizeof(Recive_table3));//把数组txt清零
}
/* 返回ch字符在sign数组中的序号 */
unsigned int AsciiToNum(char ch)
{
	  unsigned int num;
    num = ch-48;
    return  num;
}
/* 十六进制字符串转换为十进制数 *///协议转换为测量长度
unsigned int hexToDec(char *source)
{
	  unsigned char i;
	  unsigned int Dec[4]=0;
	  unsigned int sum = 0;
    for(i=0;i<4;i++)
	  {
				 Dec[i]=AsciiToNum(source[i]);
		}
		sum = (Dec[0]*4096)+(Dec[1]*256)+(Dec[2]*16)+Dec[3]	;	
    return sum;
}
/*
void SendAngle(unsigned char *receivedata4)
{
	int i,len,j;
	unsigned char angle[2][10]=0;
	for(i=0;i<2;i++)
	{
		len = sprintf(angle[i], "%u ", receivedata4[i]);//sprintf将8位2进制转换成十进制字符串，把其放入数组中，并输出字符串的长度
		for(j = 0; j < len; j++)
		{
			Send_Uart1(angle[i][j]);//发送数组txt
		}
		GUI_ShowString((30+(i*30)),4,angle[i],16,1);//显示完整协议帧
		memset(angle[i], 0, sizeof(angle[i]));//把数组txt清零
	}
}
*/
void toJson(char *oristr, const char *str, unsigned int value) //注意更改value数据类型
{
    char distance[] = "00000";
    strcat(oristr, "\"");  // {"id":"1234567","version":"1.0","params":{"
    strcat(oristr, str);   // {"id":"1234567","version":"1.0","params":{"temp
    strcat(oristr, "\":"); // {"id":"1234567","version":"1.0","params":{"temp"：
    distance[0] = value / 10000 + '0';
    distance[1] = (value / 1000)%10 + '0';
		distance[2] = (value / 100)%10 + '0';
		distance[3] = (value / 10)%10 + '0';
		distance[4] = value % 10 + '0';
    strcat(oristr, distance); // {"id":"1234567","version":"1.0","params":{"temp"：温度值
    strcat(oristr, ",");  // {"id":"1234567","version":"1.0","params":{"temp"：温度值，
}
//**************************************--esp8266--*****************************************//
//Commend Send
void ESP8266_Set(u8 *puf) 	
{    
		Clear_Table3();//每此发送命令前清空前一次接收到的回复
	  while(*puf!='\0')                    
		{   
				Send_Uart3(*puf);               
				puf++;    
		}            
		Send_Uart3('\r');                       
		Send_Uart3('\n');		
}
//Esp Initialize
void esp_int()
{	    
		ESP8266_Set("AT+RST");
	  Delay1000ms();	  Delay1000ms();	  Delay1000ms();
	  Clear_Table3();
	  ESP8266_Set("ATE0");
	  Delay100ms();	
		ESP8266_Set("AT+CWMODE_CUR=1");//station pattern
		Delay100ms();	
		ESP8266_Set("AT+CIPMUX=0");//Enabling single connection
		Delay100ms();			
	  ESP8266_Set("AT+CWJAP_CUR=\"316316\",\"@@@@@@@@\"");//Connect to the ap
		delay_s(4);
}
//Connect to IOT platform and Subscribe
void esp_IOT()
{
	  unsigned char sendnum;
	  ESP8266_Set("AT+CIPSTART=\"TCP\",\"a10uX3nn0SZ.iot-as-mqtt.cn-shanghai.aliyuncs.com\",1883"); //别忘了改！！
		Delay800ms();
		ESP8266_Set("AT+CIPMODE=1");//使能透传
		Delay100ms();
		ESP8266_Set("AT+CIPSEND");//开始透传
		Clear_Table3();//发送报文前清空接收
		sendnum = 0;	
		while (sendnum < 114)
		{
			 Send_Uart3(MessageConnect[sendnum++]);		    
		}
		sendnum=0;
		Delay100ms();	
		while (sendnum < 54)
		{
				 Send_Uart3(MessageSubscribeTopic[sendnum++]);		    
		}
		sendnum =0;
}
//IOT publish
void IOT_Publish_distance(u16 distance)
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
    totalnum = 49 + strlen(content);  //47 ：length  of topic 
		str[1] = totalnum + 0x00;
		str[2] = 0X01;
		enlargedbit = 1;
		str[2 + enlargedbit ] = 0x00;
		str[3 + enlargedbit ] = 0x2f; //topic 的长度
    //TOPIC    /没加topic的长度
    for (i = 0; i < 47 ; i++)
    {
        str[4 + enlargedbit + i] = MessageSubscribeTopic[6+i];
    }
    //CONTENT
    for (i = 0; i < strlen(content); i++)
    {
        str[51 + enlargedbit + i] = content[i]; // 4 + length  of topic =51
    }
 //    Esp8266_Send(str, 2 + enlargedbit + totalnum);
		while(j<(4 + enlargedbit + totalnum))
		{
				Send_Uart3(str[j++]);
		}
}
//IOT publish
void IOT_Publish_3Dpoint(u16 distance,u16 angle1,u16 angle2)
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
    strcat(content, "\",\"version\":\"1.0\",\"params\":{\"3Dpoint\":{");   //  {"id":"1234567","version":"1.0","params":{"3Dpoint":{
    toJson(content, "distance", distance);//  {"id":"1234567","version":"1.0","params":{"3Dpoint":{"distance":1234
		strcat(content, ",");//  {"id":"1234567","version":"1.0","params":{"3Dpoint":{"distance":1234,
		toJson(content, "angle1", angle1);//  {"id":"1234567","version":"1.0","params":{"3Dpoint":{"distance":1234,"angle1":1
		strcat(content, ",");//  {"id":"1234567","version":"1.0","params":{"3Dpoint":{"distance":1234,"angle1":1,
		toJson(content, "angle2", angle2);//  {"id":"1234567","version":"1.0","params":{"3Dpoint":{"distance":1234,"angle1":2
    content[strlen(content) - 1] = '}'; //  {"id":"1234567","version":"1.0","params":{"3Dpoint":{"distance":1234,"angle1":2}                      //  {"id":"1234567","version":"1.0","params":{"distance": 4 }
    strcat(content, ",\"method\":\"thing.event.property.set\"}");
    // Length
    totalnum = 49 + strlen(content);  //47 ：length  of topic 
		str[1] = totalnum + 0x00;
		str[2] = 0X01;
		enlargedbit = 1;
		str[2 + enlargedbit ] = 0x00;
		str[3 + enlargedbit ] = 0x2f; //topic 的长度
    //TOPIC    /没加topic的长度
    for (i = 0; i < 47 ; i++)
    {
        str[4 + enlargedbit + i] = MessageSubscribeTopic[6+i];
    }
    //CONTENT
    for (i = 0; i < strlen(content); i++)
    {
        str[51 + enlargedbit + i] = content[i]; // 4 + length  of topic =51
    }
 //    Esp8266_Send(str, 2 + enlargedbit + totalnum);
		while(j<(4 + enlargedbit + totalnum))
		{
				Send_Uart3(str[j++]);
		}
}
//*****************************************--oled--******************************************//
void OLED(void)
{
 	  OLED_Init();
    Delay800ms();
	  OLED_Clear(0);  
	  TEST_MainPage();
	  Delay800ms();
   	OLED_Clear(0);
    Test_Color();
	  Delay800ms();
}
//*****************************************--main--******************************************//
void main()
{  
	unsigned char i;
	OLED();
	Uart1_Init();
	Uart2_Init();
    Uart3_Init();
	Uart4_Init();

 //	  esp_int(); 
   	OLED_Clear(0);  
 //	  esp_IOT();
    while(1)
	{		
		if((flag == 1)&&(flag4 == 1))
		{
			for(i=0;i<length;i++)
			{
				receivedata4[i] = Recive_table4[i];
			}
			//SendAngle(receivedata4);
			for(i=0;i<4;i++)
			{
				receivedata[i] =  Recive_table[9+i]; //取出四位数字
			}
			Receivenum = hexToDec(receivedata);  //四位数字转换为十进制							
//			GUI_ShowString(0,0,Recive_table,16,1);//显示完整协议帧
			GUI_ShowNum(0,6,Receivenum,5,16,1);//显示换算后的长度					
 /*
			问题，数据获取有折损，在处理数据过程中进入中断获得的数据会被漏掉
 */							
			// while(i<4)
			// {
			// 	Send_Uart1(receivedata[i++]);//串口发送未换算的长度
			// }	
 //				Send_Uart1(' ');
 			// for(i=0;i<4;i++)
			// {
 			// 	Send_Uart1(receivedata4[i]);
			// }

			//SendAngle(receivedata4);
			i=0;						
			flag = 0;
			Clear_Table1();
 //				IOT_Publish_distance(Receivenum);	
 //				IOT_Publish_3Dpoint(Receivenum,Recive_table4,ReceiveAngle1)//Recive_table4:angle1
 	        IE2 |= 0x10;
		}
  	}
}

//***************************************--Interrupt--******************************************//
 /*
 void Uart1_Interrupt()  4         
 {     
	    ES = 0;  
			if( RI == 1 )  
		  {  	
						RI=0;   
						Receive=SBUF;                                      
						Recive_table[save]=Receive;			
				    if(Recive_table[save]=='\n')
						{                     
										save = 0;
							      flag = 1;
						}    
						else save++;                                       
			}   
			else TI=0;  
      ES = 1;			
 }
 */

void Uart2Isr() interrupt 8  //lader接收中断
{  
	  IE2 &= ~0x01;
		if (S2CON & 0x01) 
		{ 
			S2CON &= ~0x01;			
			Receive=S2BUF;                                      
			Recive_table[save]=Receive;
			if(Recive_table[save]=='\n')
			{                     
				save = 0;
				flag = 1;
			}    
			else save++; 
		}
		else S2CON &= ~0x02 ;	
    IE2 |= 0x01;		
} 
 /*
 void Uart3Isr() interrupt 17  //8266接收中断
 { 
	  IE2 &= ~0x08;
		if (S3CON & 0x02) 
		{ 
					S3CON &= ~0x02; 
					busy3 = 0; 
		} 
		if(S3CON & 0x01) 
		{ 
					S3CON &= ~0x01;
					Receive3=S3BUF;                                      
					Recive_table3[save3]=Receive3;
					if(Recive_table3[save3]=='\n'||Recive_table3[save3]=='K')
					{                                                                                     	
									save3 = 0;
									flag3 = 1;     
					}    
					else save3++; 			
		}
		IE2 |= 0x08;		
 } 
 */
void Uart4Isr() interrupt 18  //获得角度数据
{ 
	IE2 &= ~0x10;
	if (S4CON & 0x02) 
	{ 
		S4CON &= ~0x02; 
		busy4 = 0; 
	} 
	if (S4CON & 0x01) 
	{ 
		S4CON &= ~0x01; 
		Receive4=S4BUF;
		Recive_table4[save4] = Receive4;
		if(Recive_table4[save4] == '\n')
		{             
			length = save4;           
			// for(flag4=0;flag4<length;flag4++)
			// {
			//     Send_Uart1(Recive_table4[flag4]); 
			// }   
			// Send_Uart1('\r');
			// Send_Uart1('\n');                                                              	
			save4 = 0;
			flag4 = 1;
			IE2 &= ~0x10;						
		}    
		else save4++;		
	}
	IE2 |= 0x10;
} 