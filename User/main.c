#include "sys.h"
#include "delay.h"
#include "timer.h"
#include "adc.h"
#include "usart1.h"
#include "usart3.h"
#include "dht11.h"
#include "MQ2.h"
#include "MQ4.h"
#include "lcd.h"
#include "gpio.h"
#include "Picture.h"
#include "esp8266.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define FLASH_SAVE_ADDR  ((u32)0x0800FF00)  				//设置FLASH 保存地址(必须为偶数)

u8 temperature=0;  //温度
u8 temp_max = 40;  //温度上限
u8 humidity=0;     //湿度
u8 humi_max = 75;  //湿度上限
u16 NaturalGas=0;           //天然气
u16 gas_max = 500;      //天然气上限
u16 smoke_ppm=0;       //烟雾浓度
u16 smoke_max=800;     //烟雾上限
u8 PWM = 5;           //用于控制舵机
u8 key_num = 0;  //获取按键值的变量
u8 command=0x00;   // 接收语音识别的指令
u8 MessageID=0;  //需要播报的语音ID
u8 play_time=0;   
u16 timeCount1 = 0;

bool set_flag = 0;  //设置标志
bool SendFlag = 0; //发送数据标志
bool InitDispalyFlag = 0;//初始显示标志
bool shuaxin = 1;  //刷新标志
bool mode = 0;     //模式变量
char display[16];
bool beep_flag=0; //蜂鸣器标志

//STM32 Flash写入数据函数
void STM32_FlashWriteData(void)  
{
		u16 temp_buf[5];
	  u8 add = 0;
	  
	  /* 将设置的参数以及上限下限值存储在单片机FLASH */
    temp_buf[add ++] = temp_max;           /* 存储温度上限 */
	  temp_buf[add ++] = humi_max;           /* 存储湿度上限 */
	  temp_buf[add ++] = smoke_max;          /* 存储烟雾上限 */
	  temp_buf[add ++] = gas_max;            /* 存储天然气上限 */
	
	  STMFLASH_Write(FLASH_SAVE_ADDR + 0xC0,temp_buf,4); //存入数据
		delay_ms(10);
}

//STM32 Flash读出数据函数
void STM32_FlashReadData(void)  
{
		u16 temp_buf[5];
	  u8 add = 0;
	  
	  /* 将存储的数据读出 */
	  STM32F10x_Read(FLASH_SAVE_ADDR + 0xC0,temp_buf,4); //读出数据
	  temp_max = temp_buf[add ++];    /* 读出温度上限 */
	  humi_max = temp_buf[add ++];    /* 读出湿度上限 */
	  smoke_max = temp_buf[add ++];    /* 读出湿度上限 */
	  gas_max = temp_buf[add ++];    /* 读出湿度上限 */
		delay_ms(10);
}

// 检查是否是新的单片机，是的话需要存储初始值，否的话直接读出存储的值
void STM32_FlashCheck(void)  
{
	  u8 comper_str[6];
		
	  STM32F10x_Read(FLASH_SAVE_ADDR + 0xB0,(u16*)comper_str,5);
	  comper_str[5] = '\0';
	  if(strstr((char *)comper_str,"FDYDZ") == NULL)  //新的单片机
		{
			 STMFLASH_Write(FLASH_SAVE_ADDR + 0xB0,(u16*)"FDYDZ",5); //写入“FDYDZ”，方便下次校验
			 delay_ms(50);
			 STM32_FlashWriteData(); //写入初始数据
	  }
		STM32_FlashReadData();     //每次开机读出存储的数据
		delay_ms(100);
}

void display_mode(void) //显示模式
{
		if(mode == 0){Gui_DrawFont_GBK12(52, 101, YELLOW, COFFEE,(u8 *)"自动");Gui_DrawFont_GBK12(52, 115,YELLOW, COFFEE,(u8 *)"模式");}
		else         {Gui_DrawFont_GBK12(52, 101, YELLOW, COFFEE,(u8 *)"手动");Gui_DrawFont_GBK12(52, 115,YELLOW, COFFEE,(u8 *)"模式");}
}

void display_switch(void) //显示开关状态
{
	  if(PWM==5)Gui_DrawFont_GBK12(105, 101,VIOLETRED,GREEN,(u8 *) "关");   else Gui_DrawFont_GBK12(105, 101,VIOLETRED,YELLOW,(u8 *) "开"); //显示舵机的开关状态
	  if(RELAY3==0)Gui_DrawFont_GBK12(105, 116,VIOLETRED,GREEN,(u8 *) "关");else Gui_DrawFont_GBK12(105, 116,VIOLETRED,YELLOW,(u8 *) "开"); //显示水泵的开关状态
		if(RELAY2==0)Gui_DrawFont_GBK12(147, 101,VIOLETRED,GREEN,(u8 *) "关");else Gui_DrawFont_GBK12(147, 101,VIOLETRED,YELLOW,(u8 *) "开"); //显示风扇的开关状态
	  if(RELAY1==0)Gui_DrawFont_GBK12(147, 116,VIOLETRED,GREEN,(u8 *) "关");else Gui_DrawFont_GBK12(147, 116,VIOLETRED,YELLOW,(u8 *) "开"); //显示排气的开关状态
}

void DisplayInitInter(void) //显示初始界面
{
	   Lcd_Clear(WHITE);   //清屏，屏幕填充为白色
	  
	   LCD_Fill(0,0,160,98,VIOLETRED);  //区域填充颜色
   	 LCD_Fill(3,3,94,96,WHITE);      //区域填充颜色
	   LCD_Fill(100,3,156,96,WHITE);    //区域填充颜色
	   LCD_Fill(51,100,75,114,COFFEE);    //区域填充颜色
	   LCD_Fill(51,112,75,128,COFFEE);    //区域填充颜色
	
	   display_mode(); //显示模式
	   
     LCD_ShowPicture(2,3,30,30,gImage_temp);      //显示温度图标
	   LCD_ShowPicture(2,31,30,30,gImage_humi);     //显示湿度图标
	   LCD_ShowPicture(2,62,33,33,gImage_smoke);    //显示烟雾图标
	   
	   LCD_ShowPicture(106,3,42,42,gImage_gas);  //显示天然气图标
	   
	   LCD_ShowPicture(0,100,30,30,gImage_huoyan); //显示火焰图标
	
	   /* 显示中文 */
     Gui_DrawFont_GBK12(34, 12,  GRAY2, WHITE,(u8 *) "温度");
	   Gui_DrawFont_GBK12(34, 40,  GRAY2, WHITE,(u8 *) "湿度");
	   Gui_DrawFont_GBK12(38, 61,  GRAY2, WHITE,(u8 *) "烟雾");
	   
     LCD_Fill(78,100,102,113,VIOLETRED);  //区域填充颜色 
	   LCD_Fill(78,115,102,128,VIOLETRED); //区域填充颜色

     LCD_Fill(120,100,144,113,VIOLETRED);  //区域填充颜色 
	   LCD_Fill(120,115,144,128,VIOLETRED); //区域填充颜色

     /* 显示直线框 */
	   Gui_DrawLine(102,100,117,100,VIOLETRED);
		 Gui_DrawLine(117,100,117,113,VIOLETRED);
		 Gui_DrawLine(102,113,117,113,VIOLETRED);

     /* 显示直线框 */
	   Gui_DrawLine(102,115,117,115,VIOLETRED);
		 Gui_DrawLine(117,115,117,128,VIOLETRED);
		 Gui_DrawLine(102,128,117,128,VIOLETRED);

      /* 显示直线框 */
	   Gui_DrawLine(144,100,159,100,VIOLETRED);
		 Gui_DrawLine(159,100,159,113,VIOLETRED);
		 Gui_DrawLine(144,113,159,113,VIOLETRED);
		 
		 /* 显示直线框 */
	   Gui_DrawLine(144,115,159,115,VIOLETRED);
		 Gui_DrawLine(159,115,159,128,VIOLETRED);
		 Gui_DrawLine(144,128,159,128,VIOLETRED);

     Gui_DrawFont_GBK12(79, 101,  WHITE, VIOLETRED,(u8 *) "舵机");
	   Gui_DrawFont_GBK12(79, 116,  WHITE, VIOLETRED,(u8 *) "水泵");
     Gui_DrawFont_GBK12(121, 101,  WHITE, VIOLETRED,(u8 *) "风扇");
	   Gui_DrawFont_GBK12(121, 116,  WHITE, VIOLETRED,(u8 *) "排气");

     LCD_ShowString(61,61,(u8 *)"<ppm>",GRAY2,WHITE,12);

     LCD_ShowString(102,59,(u8 *)" <    > ",GRAY2,WHITE,12);
     LCD_ShowString(117,57,(u8 *)"ppm",GRAY2,WHITE,12);
		 
		 Gui_DrawFont_GBK12(110, 48, GRAY2, WHITE,(u8 *)"天然气");
     LCD_Fill(48,99,48,129,VIOLETRED);  //区域填充颜色
		 LCD_Fill(2,60,94,60,VIOLETRED);  //区域填充颜色
}

u8 keyscan(void)   //获取按键值
{
	  u8 key = 0;
	
		if(KEY1 == 0)   //第一个按键按下返回键值1
		{
		  	delay_ms(1); 
			  if(KEY1 == 0)
				{
					  while(KEY1 == 0);
						key = 1;
				}
		}
		if(KEY2 == 0)  //第二个按键按下返回键值2
		{
		  	if(!set_flag && mode)delay_ms(1);
			  else                delay_ms(80); 
			  if(KEY2 == 0)
				{
					  if(!set_flag && mode)
						{
								while(KEY2 == 0);
						}
						key = 2;
				}
		}
		if(KEY3 == 0) //第三个按键按下返回键值3
		{
		  	if(!set_flag && mode)delay_ms(1);
			  else                delay_ms(80); 
			  if(KEY3 == 0)
				{
					  if(!set_flag && mode)
						{
								while(KEY3 == 0);
						}
						key = 3;
				}
		}
		if(KEY4 == 0) //第四个按键按下返回键值4
		{
		  	delay_ms(1);
			  if(KEY4 == 0)
				{
					  while(KEY4 == 0);
						key = 4;
				}
		}
		if(KEY5 == 0) //第五个按键按下返回键值5
		{
		  	delay_ms(1);
			  if(KEY5 == 0)
				{
					  while(KEY5 == 0);
						key = 5;
				}
		}
		return key;
}

//接收手机端设置的参数
bool ReceiveSetValue_1(char *uart_buf,char *str,u16 *data) 
{
		u16  setValue=0;
	  char *buf = str;
	  char *str1=0,i;
	  char setvalue[6]={0};
	
		if(strstr(uart_buf,buf) == NULL) //没有接收到数据
		{
				return  1;
		}
	  else
		{
				str1 = strstr(uart_buf,buf);  //接收到数据
				
				while(*str1 < '0' || *str1 > '9')    //判断是不是0到9有效数字
				{
						str1 = str1 + 1;
						delay_ms(10);
				}
				i = 0;
				while(*str1 >= '0' && *str1 <= '9')        //判断是不是0到9有效数字
				{
						setvalue[i] = *str1;
						i ++; str1 ++;
						if(*str1 == ',')break;            //换行符，直接退出while循环
						delay_ms(10);
				}
				setvalue[i] = '\0';            //加上结尾符
				setValue = atoi(setvalue);
				*data = setValue;
		}	
		return  0;
}

void UsartSendReceiveData(void) //串口发送和接收数据，用于和手机APP通信
{
		unsigned char *dataPtr = NULL;
	  u16  int_value=0;
	  char SEND_BUF[200];
	
	  dataPtr = ESP8266_GetIPD(1);   //接收数据

		if(dataPtr != NULL)
		{
			  /*   设置温度上限的指令   */
			  if((ReceiveSetValue_1((char *)dataPtr,"temp_max:",&int_value))==0)
				{
					  beep_flag=1;
					  BEEP = 1;
					  delay_ms(200);  /*   蜂鸣器响一声   */
					  BEEP = 0;
					  beep_flag=0;
					
					  if(int_value < 99)
						{
								temp_max = int_value;
							  if(set_flag==1)InitDispalyFlag = 1;
						}
				}
			  /*   设置湿度上限的指令   */
			  if((ReceiveSetValue_1((char *)dataPtr,"humi_max:",&int_value))==0)
				{
					  if(int_value < 99)
						{
								humi_max = int_value;
							  if(set_flag==1)InitDispalyFlag = 1;
						}
				}
				/*   设置烟雾上限的指令   */
			  if((ReceiveSetValue_1((char *)dataPtr,"smoke_max:",&int_value))==0)
				{
					  if(int_value < 9999)
						{
								smoke_max = int_value;
							  if(set_flag==1)InitDispalyFlag = 1;
						}
				}
					
				/*   设置天然气上限的指令   */
			  if((ReceiveSetValue_1((char *)dataPtr,"gas_max:",&int_value))==0)
				{
					  if(int_value < 9999)
						{
								gas_max = int_value;
							  if(set_flag==1)InitDispalyFlag = 1;
						}
						STM32_FlashWriteData(); // 存储设置的值
				}
				
				if(strstr((char *)dataPtr,"manual")!=NULL)  //接收到手动模式的指令
				{
					  beep_flag=1;
					  BEEP = 1;
					  delay_ms(200);  /*   蜂鸣器响一声   */
					  BEEP = 0;
					  beep_flag=0;
					
					  mode = 1;
					  if(set_flag == 0) 
						{
								display_mode(); //显示模式
						}
						else
						{
								InitDispalyFlag = 1;
						}
						SendFlag = 0;		
						timeCount1 = 0;
				}
				if(strstr((char *)dataPtr,"auto")!=NULL)  //接收到自动模式的指令
				{
					  beep_flag=1;
					  BEEP = 1; 
					  delay_ms(200);  /*   蜂鸣器响一声   */
					  BEEP = 0;
					  beep_flag=0;
					
					  mode = 0;

						if(set_flag == 0)
						{
								display_mode(); //显示模式
						}
						else
						{
								InitDispalyFlag = 1;
						}
						SendFlag = 0;		
						timeCount1 = 0;
				}
				
				if(strstr((char *)dataPtr,"motor_on")!=NULL) //接收到控制舵机的指令
				{
						if(mode == 1)PWM = 15;
						if(set_flag == 0)
						{
								display_switch(); //显示开关状态
						}
						SendFlag = 0;		
						timeCount1 = 0;
				}
				if(strstr((char *)dataPtr,"motor_off")!=NULL) //接收到控制舵机的指令
				{
						if(mode == 1)PWM = 5;
						if(set_flag == 0)
						{
								display_switch(); //显示开关状态
						}
						SendFlag = 0;		
						timeCount1 = 0;
				}
				
				if(strstr((char *)dataPtr,"pump_on")!=NULL) //接收到控制水泵开启的指令
				{
						if(mode == 1)RELAY3  = 1;
						if(set_flag == 0)
						{
								display_switch(); //显示开关状态
						}
						SendFlag = 0;		
						timeCount1 = 0;
				}
				if(strstr((char *)dataPtr,"pump_off")!=NULL)//接收到控制水泵关闭的指令
				{
						if(mode == 1)RELAY3  = 0;
						if(set_flag == 0)
						{
								display_switch(); //显示开关状态
						}
						SendFlag = 0;		
						timeCount1 = 0;
				}
				if(strstr((char *)dataPtr,"vent_on")!=NULL)//接收到控制排气开启的指令
				{
						if(mode == 1)RELAY1  = 1;
						if(set_flag == 0)
						{
								display_switch(); //显示开关状态
						}
						SendFlag = 0;		
						timeCount1 = 0;
				}
				if(strstr((char *)dataPtr,"vent_off")!=NULL)//接收到控制排气关闭的指令
				{
						if(mode == 1)RELAY1  = 0;
						if(set_flag == 0)
						{
								display_switch(); //显示开关状态
						}
						SendFlag = 0;		
						timeCount1 = 0;
				}
				if(strstr((char *)dataPtr,"fan_on")!=NULL)//接收到控制风扇开启的指令
				{
						if(mode == 1)RELAY2  = 1;
						if(set_flag == 0)
						{
								display_switch(); //显示开关状态
						}
						SendFlag = 0;		
						timeCount1 = 0;
				}
				if(strstr((char *)dataPtr,"fan_off")!=NULL)//接收到控制风扇关闭的指令
				{
						if(mode == 1)RELAY2  = 0;
						if(set_flag == 0)
						{
								display_switch(); //显示开关状态
						}
						SendFlag = 0;		
						timeCount1 = 0;
				}
				
				ESP8266_Clear();									//清空缓存
		}
		if(SendFlag == 1)    //1秒钟上传一次数据
		{
			  SendFlag = 0;		
			   
				UsartSendBufClear(sizeof(SEND_BUF),SEND_BUF,WIFIADDR);  //清除发送缓存
				sprintf(SEND_BUF+strlen("SEND"),"temp:%d#,humi:%d#,gas:%d#,smoke:%d#",temperature,humidity,NaturalGas,smoke_ppm);/*发送数据装载*/
			  if(FLAME==0)strcat(SEND_BUF,",flame"); //有火
			  if(temperature>=temp_max)strcat(SEND_BUF,",temp_warn"); /* 如果温度超过上限，发送异常标志，APP上温度值会显示红色 */
        if(humidity>=humi_max)strcat(SEND_BUF,",humi_warn");    /* 如果湿度低于下限，发送异常标志，APP上湿度值会显示红色 */
			  if(NaturalGas>=gas_max)strcat(SEND_BUF,",gas_warn");           /* 如果天然气超过上限，发送异常标志，APP上天然气值会显示红色 */
			  if(smoke_ppm>=smoke_max)strcat(SEND_BUF,",smoke_warn");           /* 如果烟雾超过上限，发送异常标志，APP上烟雾值会显示红色 */
			 
				if(PWM==15)strcat(SEND_BUF,",swit1_on");else strcat(SEND_BUF,",swit1_off");  //舵机的开关状态
				if(RELAY3==1)strcat(SEND_BUF,",swit2_on");else strcat(SEND_BUF,",swit2_off");  //水泵的开关状态
				if(RELAY2==1)strcat(SEND_BUF,",swit3_on");else strcat(SEND_BUF,",swit3_off");  //风扇的开关状态
				if(RELAY1==1)strcat(SEND_BUF,",swit4_on");else strcat(SEND_BUF,",swit4_off");  //排气的开关状态
        
			  ESP8266_SendData((u8 *)SEND_BUF, strlen(SEND_BUF));  //将数据上传至APP端
			  ESP8266_Clear();
		}
}

/**************************************************************************************************************
					                      以下为液晶菜单界面显示部分
***************************************************************************************************************/

#define x            2    //设置的第一行起始位置x
#define y            21   //设置的第一行起始位置y
#define f_size       16   //字体大小
#define interval     2    //每行间隔的像素

int display_menu5(void)//显示选择模式界面
{
		Lcd_Clear(WHITE);   //清屏，屏幕填充为白色
	  LCD_Fill(0,0,160,18,RED);
	  Gui_DrawFont_GBK16(48, 2, WHITE, RED, (u8 *)"选择模式");
	  InitDispalyFlag = 1;
		while(1)
		{
			  UsartSendReceiveData();			 //串口发送和接收数据，用于和手机APP通信
				key_num = keyscan();  //获取按键值
				if(key_num == 2)    //第二个按键按下，是加
				{
						mode=0;
						InitDispalyFlag = 1;
				}
				if(key_num == 3)   //第三个按键按下，是减
				{
					  mode=1;
					  InitDispalyFlag = 1;
				}
				if(key_num == 4)  //返回上一级
				{
						Lcd_Clear(WHITE);   //清屏，屏幕填充为白色
						LCD_Fill(0,0,160,18,RED);
						Gui_DrawFont_GBK16(40, 2, WHITE, RED, (u8 *)"设置主菜单");
						InitDispalyFlag = 1;
						key_num = 0;
						return 0;
				}
				if(InitDispalyFlag)
				{
					  if(mode == 0)
						{
								Gui_DrawFont_GBK16(48, 64, BLACK, WHITE, (u8 *)"自动模式");
						}
						else
						{
								Gui_DrawFont_GBK16(48, 64, BLACK, WHITE, (u8 *)"手动模式");
						}
					  InitDispalyFlag = 0;
				}
		}
}

int display_menu4(void)//显示设置天然气界面
{
		Lcd_Clear(WHITE);   //清屏，屏幕填充为白色
	  LCD_Fill(0,0,160,18,RED);
	  Gui_DrawFont_GBK16(40, 2, WHITE, RED, (u8 *)"设置天然气");
	  Gui_DrawFont_GBK12(103, 68,BLACK,WHITE,(u8 *)"ppm"); // 显示ppm
	  InitDispalyFlag = 1;
		while(1)
		{
			  UsartSendReceiveData();			 //串口发送和接收数据，用于和手机APP通信
				key_num = keyscan();  //获取按键值
				if(key_num == 2)    //第二个按键按下，是加
				{
						if(gas_max<9999)gas_max++;
						InitDispalyFlag = 1;
				}
				if(key_num == 3)   //第三个按键按下，是减
				{
					  if(gas_max>0)gas_max--;
					  InitDispalyFlag = 1;
				}
				if(key_num == 4)  //返回上一级
				{
						Lcd_Clear(WHITE);   //清屏，屏幕填充为白色
						LCD_Fill(0,0,160,18,RED);
						Gui_DrawFont_GBK16(40, 2, WHITE, RED, (u8 *)"设置主菜单");
						InitDispalyFlag = 1;
						key_num = 0;
						return 0;
				}
				if(InitDispalyFlag)
				{
					  sprintf((char *)display,"%04d",gas_max);
						LCD_ShowString(52,60,(u8 *)display,BLACK,WHITE,24); /*  显示天然气设置值  */
					  InitDispalyFlag = 0;
				}
		}
}

int display_menu3(void)//显示设置烟雾界面
{
		Lcd_Clear(WHITE);   //清屏，屏幕填充为白色
	  LCD_Fill(0,0,160,18,RED);
	  Gui_DrawFont_GBK16(48, 2, WHITE, RED, (u8 *)"设置烟雾");
	  Gui_DrawFont_GBK12(103, 68,BLACK,WHITE,(u8 *)"ppm"); // 显示ppm
	  InitDispalyFlag = 1;
		while(1)
		{
			  UsartSendReceiveData();			 //串口发送和接收数据，用于和手机APP通信
				key_num = keyscan();  //获取按键值
				if(key_num == 2)    //第二个按键按下，是加
				{
						if(smoke_max<9999)smoke_max++;
						InitDispalyFlag = 1;
				}
				if(key_num == 3)   //第三个按键按下，是减
				{
					  if(smoke_max>0)smoke_max--;
					  InitDispalyFlag = 1;
				}
				if(key_num == 4)  //返回上一级
				{
						Lcd_Clear(WHITE);   //清屏，屏幕填充为白色
						LCD_Fill(0,0,160,18,RED);
						Gui_DrawFont_GBK16(40, 2, WHITE, RED, (u8 *)"设置主菜单");
						InitDispalyFlag = 1;
						key_num = 0;
						return 0;
				}
				if(InitDispalyFlag)
				{
					  sprintf((char *)display,"%04d",smoke_max);
						LCD_ShowString(52,60,(u8 *)display,BLACK,WHITE,24); /*  显示烟雾设置值  */
					  InitDispalyFlag = 0;
				}
		}
}

int display_menu2(void)//显示设置湿度界面
{
		Lcd_Clear(WHITE);   //清屏，屏幕填充为白色
	  LCD_Fill(0,0,160,18,RED);
	  Gui_DrawFont_GBK16(48, 2, WHITE, RED, (u8 *)"设置湿度");
	  Gui_DrawFont_GBK12(88, 68,BLACK,WHITE,(u8 *)"%"); // 显示百分号
	  InitDispalyFlag = 1;
		while(1)
		{
			  UsartSendReceiveData();			 //串口发送和接收数据，用于和手机APP通信
				key_num = keyscan();  //获取按键值
				if(key_num == 2)    //第二个按键按下，是加
				{
						if(humi_max<99)humi_max++;
						InitDispalyFlag = 1;
				}
				if(key_num == 3)   //第三个按键按下，是减
				{
					  if(humi_max>0)humi_max--;
					  InitDispalyFlag = 1;
				}
				if(key_num == 4)  //返回上一级
				{
						Lcd_Clear(WHITE);   //清屏，屏幕填充为白色
						LCD_Fill(0,0,160,18,RED);
						Gui_DrawFont_GBK16(40, 2, WHITE, RED, (u8 *)"设置主菜单");
						InitDispalyFlag = 1;
						key_num = 0;
						return 0;
				}
				if(InitDispalyFlag)
				{
					  sprintf((char *)display,"%02d",humi_max);
						LCD_ShowString(60,60,(u8 *)display,BLACK,WHITE,24); /*  显示湿度设置值  */
					  InitDispalyFlag = 0;
				}
		}
}

int display_menu1(void)//显示设置温度界面
{
		Lcd_Clear(WHITE);   //清屏，屏幕填充为白色
	  LCD_Fill(0,0,160,18,RED);
	  Gui_DrawFont_GBK16(48, 2, WHITE, RED, (u8 *)"设置温度");
	  Gui_DrawFont_GBK12(88, 68,BLACK,WHITE,(u8 *)"℃"); // 显示摄氏度
	  InitDispalyFlag = 1;
		while(1)
		{
			  UsartSendReceiveData();			 //串口发送和接收数据，用于和手机APP通信
				key_num = keyscan();  //获取按键值
				if(key_num == 2)    //第二个按键按下，是加
				{
						if(temp_max<99)temp_max++;
						InitDispalyFlag = 1;
				}
				if(key_num == 3)   //第三个按键按下，是减
				{
					  if(temp_max>0)temp_max--;
					  InitDispalyFlag = 1;
				}
				if(key_num == 4)  //返回上一级
				{
						Lcd_Clear(WHITE);   //清屏，屏幕填充为白色
						LCD_Fill(0,0,160,18,RED);
						Gui_DrawFont_GBK16(40, 2, WHITE, RED, (u8 *)"设置主菜单");
						InitDispalyFlag = 1;
						key_num = 0;
						return 0;
				}
				if(InitDispalyFlag)
				{
					  sprintf((char *)display,"%02d",temp_max);
						LCD_ShowString(60,60,(u8 *)display,BLACK,WHITE,24); /*  显示温度设置值  */
					  InitDispalyFlag = 0;
				}
		}
}

int display_main_menu(void) //显示主菜单界面
{	  
	  unsigned char index=1,i;
	  bool  up_flag  = 0;  //上翻标志
	  bool down_flag = 0;  //下翻标志
	  unsigned char  row = 5;   //每页最多显示的行数
	  u16 color[5][2];//颜色数组
	
struct info {
    char *str; 
};
struct  info  display_[] = {
				{"1.设置温度"},    /*  显示的字符界面 */
				{"2.设置湿度"},
				{"3.设置烟雾"},
				{"4.设置天然气"},
				{"5.选择模式"},
};

	  set_flag = 1;
		Lcd_Clear(WHITE);   //清屏，屏幕填充为白色
	  LCD_Fill(0,0,160,18,RED);
	  Gui_DrawFont_GBK16(40, 2, WHITE, RED,(u8 *)"设置主菜单");
	  InitDispalyFlag = 1;
		while(1)
		{
			  UsartSendReceiveData();			 //串口发送和接收数据，用于和手机APP通信
				key_num = keyscan();  //获取按键值
			  if(key_num == 1)     //第一个按键按下，是确定
				{
						if(index==1)display_menu1();  //选择进入温度设置界面
            if(index==2)display_menu2();  //选择进入湿度设置界面
					  if(index==3)display_menu3();  //选择进入烟雾设置界面
					  if(index==4)display_menu4();  //选择进入天然气设置界面
					  if(index==5)display_menu5();  //选择进入选择模式界面
				}
				if(key_num == 2)    //第二个按键按下，是下翻
				{
						index ++;
					  down_flag = 1;
					  if(index>row)
						{
								index=1;
						}
				}
				if(key_num == 3)   //第三个按键按下，是上翻
				{
					  index --;
					  up_flag  = 1;
					  if(index==0)
					  {
							  index=row;
						}
				}
				if(key_num == 4) //退出设置
				{
					  DisplayInitInter(); //数据显示界面
					
						STM32_FlashWriteData(); //退出设置以后，先存储下设置的数据
					  set_flag = 0; //设置标志清零
					  return 0;
				}
				if(down_flag || up_flag || InitDispalyFlag)
				{
						/*  初始化字体颜色和字体背景颜色  */
					  for(i = 0;i < row; i ++)
					  {
								color[i][0]=BLACK;
							  color[i][1]=WHITE;
						}
					  
						/*  当按上行或者下行的按键时字体颜色和字体背景颜色需要发生变化  */
						color[index-1][0]=WHITE;color[index-1][1]=BLUE;
						
						/* 选中选项的颜色和未选择选中选项的颜色处理 */
						/*  未选中的颜色处理  */
						/*  每次下翻时，上一行需要显示白色  */
						if(down_flag==1)
						{
								if(index%row==1)LCD_Fill(0,y + (row-1)*(f_size + interval),X_MAX_PIXEL,(y + (row-1)*(f_size + interval))+f_size,WHITE); 
								else 
								{		
								  	LCD_Fill(0,y + (index-1-1)*(f_size + interval),X_MAX_PIXEL,((y + f_size) + (index-1-1)*f_size)+(index-1)*interval,WHITE);	
								}
						}
						/*  每次上翻时，下一行需要显示白色  */
						if(up_flag==1)
						{
							  if(index==row)
								{
									LCD_Fill(0,y + (index-row)*(f_size + interval),X_MAX_PIXEL,(y + (index-row)*(f_size + interval))+f_size,WHITE); 
								}
							  else
							  {
								 LCD_Fill(0,y + (index)*(f_size + interval),X_MAX_PIXEL,((y+f_size) + (index)*f_size)+(index)*interval,WHITE);	 
							  }
						}
						
						/*  选中的颜色处理  */
						LCD_Fill(0,y + (index-1)*(f_size + interval),X_MAX_PIXEL,((y+f_size) + (index-1)*f_size)+(index-1)*interval,BLUE); //显示选中行颜色
						
						if(InitDispalyFlag)
						{
								/* 显示主信息 */
								for(i = 0;i < row; i ++) 
								{
										Gui_DrawFont_GBK16(x, y + i*(f_size + interval), color[i][0], color[i][1], (u8 *)display_[i].str);    //显示第一页的信息
								}
						}
						else
						{
								/* 当每次上翻或者下翻时，不需要刷新每行的显示，这样容易导致刷新变慢。 只需要显示刷新相连的两行字符即可*/
								if(down_flag == 1)   //当下翻时，只需要刷新当前选中的这一行和上一行
								{
										if(index%row==1)  //第一行
										{
												Gui_DrawFont_GBK16(x, y + (index-1)*(f_size + interval), color[index-1][0], color[index-1][1], (u8 *)display_[index-1].str);
											  Gui_DrawFont_GBK16(x, y + (row-1)*(f_size + interval), color[row-1][0], color[row-1][1], (u8 *)display_[row-1].str);
										}
										else
										{
												Gui_DrawFont_GBK16(x, y + (index-1-1)*(f_size + interval), color[index-1-1][0], color[index-1-1][1], (u8 *)display_[index-1-1].str);
												Gui_DrawFont_GBK16(x, y + (index-1)*(f_size + interval),   color[index-1][0]  , color[index-1][1]  , (u8 *)display_[index-1].str);
										}
								}
								if(up_flag==1) //当上翻时，只需要刷新当前选中的这一行和下一行
								{
										if(index==row)  //第最后一行
										{
											  Gui_DrawFont_GBK16(x, y + (index-row)*(f_size + interval), color[index-row][0], color[index-row][1], (u8 *)display_[index-row].str);
												Gui_DrawFont_GBK16(x, y + (index-1)*(f_size + interval), color[index-1][0], color[index-1][1], (u8 *)display_[index-1].str);
										}
										else
										{
												Gui_DrawFont_GBK16(x, y + (index-1)*(f_size + interval), color[index-1][0], color[index-1][1], (u8 *)display_[index-1].str);
												Gui_DrawFont_GBK16(x, y + index*(f_size + interval), color[index][0], color[index][1], (u8 *)display_[index].str);
										}
								}
					  }
						down_flag = 0;  //下翻标志清零
						up_flag   = 0;  //上翻标志清零
						InitDispalyFlag = 0;
				}
		}
}

/*****************************************************************************************************************************
					                                              结束
******************************************************************************************************************************/

void Get_MQ4_PPM(void)   //读取天然气浓度值
{
	 NaturalGas = MQ4_readPpm();         //读取天然气ppm值
	 NaturalGas = (NaturalGas > 9999) ? 9999 : NaturalGas; //最大到9999
}

void Get_MQ2_PPM(void)   //读取烟雾值
{
	 smoke_ppm = MQ2_readPpm();              //读取烟雾ppm值
	 smoke_ppm = (smoke_ppm > 9999) ? 9999 : smoke_ppm; //最大到9999
}

void SpeechRecognitionHandle(void) // 语音识别处理函数
{  
		if(command != 0)
		{
				delay_ms(10);
				switch(command)   //处理语音识别指令
				{
						case(0x01): mode = 0; if(set_flag==0){display_mode();}else InitDispalyFlag = 1; break;    //自动模式
						case(0x02): mode = 1; if(set_flag==0){display_mode();}else InitDispalyFlag = 1; break;	  //手动模式
						/* 播报当前温湿度 */
						case(0x0B):   play_time = 12;  //大概的播报时间，防止播报期间数据发生变化
								          USART3_Sned_Char(0xAA);   /* 数据头 */
													USART3_Sned_Char(0x55);   
													USART3_Sned_Char(1);      /* 消息号 */
						              USART3_Sned_Char(temperature);/* 播报的温度 */
						              USART3_Sned_Char(humidity);   /* 播报的湿度 */
						              USART3_Sned_Char((u8)(smoke_ppm>>0));
						              USART3_Sned_Char((u8)(smoke_ppm>>8));
						              USART3_Sned_Char((u8)(smoke_ppm>>16));  
						              USART3_Sned_Char((u8)(smoke_ppm>>24));/* 播报的烟雾值 */ 
						              USART3_Sned_Char((u8)(NaturalGas>>0));
						              USART3_Sned_Char((u8)(NaturalGas>>8));
						              USART3_Sned_Char((u8)(NaturalGas>>16));  
						              USART3_Sned_Char((u8)(NaturalGas>>24));/* 播报的天然气浓度值 */ 
						              USART3_Sned_Char(0x55);
						              USART3_Sned_Char(0xAA);   /* 结尾字节 */
													break;
						default: break;
				}
				if(command>=0x03 && command<=0x0A)
				{
						mode = 1; // 切换到手动模式
            if(set_flag==0){
							display_mode();
						}else{
  						InitDispalyFlag = 1;
						}
						switch(command)
						{
						  	case(0x03):  PWM = 15;  break; // 打开柜门
							  case(0x04):  PWM = 5;   break; // 关闭柜门
							  case(0x05):  RELAY3=1;  break; // 开启水泵
							  case(0x06):  RELAY3=0;  break; // 关闭水泵
							  case(0x07):  RELAY2=1;  break; // 开启风扇
							  case(0x08):  RELAY2=0;  break; // 关闭风扇
							  case(0x09):  RELAY1=1;  break; // 开启排气
							  case(0x0A):  RELAY1=0;  break; // 关闭排气
							  default: break;
						}
				}
				command=0x00;
		}
}

int main(void)
{	
	  u8  HandlePlayFlag=0x00;//处理播报标志
	
		delay_init();	    //延时函数初始化	  
	  NVIC_Configuration();
		delay_ms(500);       //上电瞬间加入一定延时在初始化
	  KEY_GPIO_Init();    //按键初始化
	  Lcd_Init();         //LCD液晶初始化
	  KEY_GPIO_Init();        //按键引脚初始化
	  Lcd_Clear(BLACK);   //清屏，屏幕填充为黑色
	  STM32_FlashCheck(); 
	  /* 显示加载中 */
	  Gui_DrawFont_GBK16(48,50,VIOLETRED,BLACK,(u8 *)"Loading...");
	  ESP8266_Init();     //ESP8266初始化
	  delay_ms(1000);
	  USART3_Init(9600);
	  Adc_Init();
	  DHT11_Init();   // DHT11温湿度初始化
	  do
		{
				DHT11_Read_Data(&temperature,&humidity);
			  delay_ms(500);
		}while(temperature==0 && humidity==0);
	  DisplayInitInter();  //显示初始界面
		TIM2_Init(99,71);   //定时器初始化，定时100us
		TIM3_Init(999,719); //定时器初始化，定时10ms
		//Tout = ((arr+1)*(psc+1))/Tclk ; 
		//Tclk:定时器输入频率(单位MHZ)
		//Tout:定时器溢出时间(单位us)
		while(1)
		{  
			  key_num = keyscan();     //按键扫描
			  
				if(mode == 1)   //在手动模式下
				{
					  if(key_num == 2)
						{
							  if(PWM==5)PWM=15;
							  else if(PWM==15)PWM=5;//手动开启关闭舵机
							  display_switch(); //显示开关状态
						}
						if(key_num == 3)
						{
								RELAY3 = !RELAY3; //手动开启关闭水泵
							  display_switch(); //显示开关状态
						}
						if(key_num == 4)
						{
								RELAY1 = !RELAY1;  //手动开启关闭风扇
							  display_switch(); //显示开关状态
						}
						if(key_num == 5)
						{
								RELAY2 = !RELAY2; //手动开启关闭排气
							  display_switch(); //显示开关状态
						}
				}
				if(key_num == 1)
				{
						display_main_menu();  //显示主菜单
				}
				SpeechRecognitionHandle(); // 语音识别处理函数、
				
				if(shuaxin == 1 && play_time==0)  //延时一段时间读取温度值
				{
					  shuaxin = 0;
					
					  DHT11_Read_Data(&temperature,&humidity);  //读取温湿度
					  Get_MQ2_PPM(); //读取烟雾浓度
					  Get_MQ4_PPM();//读取天然气浓度
					  
					  if(FLAME==0) //检测到有火
						{
								Gui_DrawFont_GBK12(34, 102,RED, WHITE,(u8 *)"有");
								Gui_DrawFont_GBK12(34, 115,RED, WHITE,(u8 *)"火");
							  if(!(HandlePlayFlag&0x10))
								{
										HandlePlayFlag|=0x10;
										MessageID = 2; /* 播报消息号2“警告，检测到火焰！” */
								}
						}
						else
						{
								Gui_DrawFont_GBK12(34, 102,GREE2, WHITE,(u8 *)"无");
								Gui_DrawFont_GBK12(34, 115,GREE2, WHITE,(u8 *)"火");
							  HandlePlayFlag&=0xEF;
						}
					
						sprintf(display,"%02d",temperature);
						if(temperature>=temp_max)
						{
								LCD_ShowString(57,6,(u8 *)display,RED,WHITE,24);   //温度超过上限，字体显示红色
								Gui_DrawFont_GBK12(83, 9,RED,WHITE,(u8 *)"℃");    // 显示摄氏度
							  if(!(HandlePlayFlag&0x01))
								{
										HandlePlayFlag|=0x01;
										MessageID = 3;/* 播报消息号3“请注意，当前温度过高！” */
								}
						}
						else
						{
								LCD_ShowString(57,6,(u8 *)display,GREE2,WHITE,24);    // 显示温度
								Gui_DrawFont_GBK12(83, 9,GREE2,WHITE,(u8 *)"℃");    // 显示摄氏度
							  HandlePlayFlag&=0xFE;
						}

						sprintf(display,"%02d",humidity);
						if(humidity>=humi_max)  /* 如果湿度高于上限，湿度值会显示红色 */
						{
								LCD_ShowString(57,34,(u8 *)display,RED,WHITE,24);  //显示湿度值
							  Gui_DrawFont_GBK12(84, 38,RED,WHITE,(u8 *)"%");    // 显示百分比
							  if(!(HandlePlayFlag&0x02))
								{
										HandlePlayFlag|=0x02;
										MessageID = 4; /* 播报消息号4“请注意，当前湿度过高！” */
								}
						}
						else
						{
								LCD_ShowString(57,34,(u8 *)display,GREE2,WHITE,24);       // 显示湿度
							  Gui_DrawFont_GBK12(84, 38,GREE2,WHITE,(u8 *)"%");    // 显示百分比
							  HandlePlayFlag&=0xFD;
						}
						
						sprintf(display,"%04d",smoke_ppm);
						if(smoke_ppm>=smoke_max)   /* 如果烟雾高于上限，烟雾值会显示红色 */
						{
								LCD_ShowString(38,73,(u8 *)display,RED,WHITE,24);    //显示烟雾值
							  if(!(HandlePlayFlag&0x04))
								{
										HandlePlayFlag|=0x04;
										MessageID = 5; /* 播报消息号5“请注意，当前烟雾浓度过高！” */
								}
						}
						else
						{
								LCD_ShowString(38,73,(u8 *)display,GREE2,WHITE,24);  // 显示烟雾
							  HandlePlayFlag&=0xFB;
						}
						
						sprintf(display,"%04d",NaturalGas);
						if(NaturalGas>=gas_max)     /* 如果天然气高于上限，天然气值会显示红色 */
						{
							  LCD_ShowString(103,70,(u8 *)display,RED,WHITE,24);
							  if(!(HandlePlayFlag&0x08))
								{
										HandlePlayFlag|=0x08;
										MessageID = 6; /* 播报消息号6“请注意，当前天然气浓度过高！” */
								}
						}
						else
						{
								LCD_ShowString(103,70,(u8 *)display,GREE2,WHITE,24);
							  HandlePlayFlag&=0xF7;
						}
						SendFlag= 1; // 发送数据标志置1
				}
				if(mode == 0) //在自动模式下
				{
						if(temperature>=temp_max||humidity>=humi_max) // 温湿度高，开启风扇
						{
								RELAY2 = 1;  // 开启风扇
						}
						else
						{
								RELAY2 = 0; // 关闭风扇
						}
						if(NaturalGas>=gas_max)//天然气超标
						{
								RELAY1 = 1;  // 开启风扇排气
							  PWM = 15; //开启舵机
						}
						else
						{
								RELAY1 = 0;  // 关闭风扇排气
							  PWM = 5; //关闭舵机
						}
						if(smoke_ppm>=smoke_max||FLAME==0) //烟雾超标或者有火，开启水泵
						{
								RELAY3 = 1; // 开启水泵
						}
						else
						{
								RELAY3 = 0; // 关闭水泵
						}
						
				}
				display_switch(); //显示开关状态
				if(MessageID != 0) //有语音要播报时，发送要播报的消息号到语音识别模块
				{
					  USART3_Sned_Char(0xAA);   /* 数据头 */
						USART3_Sned_Char(0x55);   
						USART3_Sned_Char(MessageID);      /* 消息号 */
						USART3_Sned_Char(0x55);
						USART3_Sned_Char(0xAA);   /* 结尾字节 */
						MessageID = 0;
				}
				UsartSendReceiveData(); //串口发送和接收数据，用于和手机APP通信
		}	
}

void USART3_IRQHandler(void)                
{
     if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  
     {   
				 command = USART_ReceiveData(USART3);//接收模块的数据
     }

     if(USART_GetFlagStatus(USART3,USART_FLAG_ORE) == SET)
     {
         USART_ClearFlag(USART3,USART_FLAG_ORE);
     }
     USART_ClearITPendingBit(USART3, USART_IT_RXNE);
}

void TIM2_IRQHandler(void)//定时器2中断服务程序，用于舵机驱动
{ 
	  static u16 timeCount1 = 0;
	
		if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
		{ 
				TIM_ClearITPendingBit(TIM2, TIM_IT_Update); //清除中断标志位  

			  timeCount1++;
				if(timeCount1<=PWM)MOTOR=1; else MOTOR=0;
				if(timeCount1>=200)//20ms一周期
				{
						timeCount1 = 0;
				}
	  }
}

void TIM3_IRQHandler(void)//定时器3中断服务程序，用于记录时间
{ 
	  static u16 timeCount2 = 0;
	  static u16 timeCount3 = 0;
	
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
		{ 
				TIM_ClearITPendingBit(TIM3, TIM_IT_Update); //清除中断标志位  

			  timeCount1 ++;
				if(timeCount1 >= 80) //800ms
				{
						timeCount1 = 0;
					  shuaxin = 1; // 刷新数据标志置1
				}
				timeCount2 ++;
			  if(timeCount2 >= 100)//1000ms
				{
						timeCount2 = 0;

					  if(play_time>0)play_time--;
				}
				timeCount3 ++;
				if(timeCount3 >= 20)//200ms
				{
						timeCount3 = 0;
					  if(FLAME==0)
						{
								BEEP = 1;  //检测到火焰，蜂鸣器一直响
						}
						else
						{
								if(temperature>=temp_max||humidity>=humi_max||NaturalGas>=gas_max||smoke_ppm>=smoke_max)//温湿度烟雾天然气超标，蜂鸣器响
								{
										BEEP = ~BEEP;
								}
								else
								{
										if(beep_flag==0)BEEP = 0;
								}
					  }
				}
				
	  }
}


