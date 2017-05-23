/////////////////////////////////////////////////
///////2015年4月1日   DEMO精简版      ///////////
////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NUC1xx.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvSPI.h"
#include "Driver\DrvADC.h"
#include "Driver\DrvTIMER.h"
#include "AD9850FUNC0903.c"
#include "LCDFUNC0723.c"
#include "UIFUNC0728.c"

//////////////////////////////
// 常量定义  伪指令定义    f //
//////////////////////////////

// 1s软件定时器溢出值，100个5ms
#define V_T1s	100		 //0.5s定时器

//伪指令  
#define A10LED_H    DrvGPIO_SetBit(E_GPA,10)	//M0板上led off
#define A10LED_L    DrvGPIO_ClrBit(E_GPA,10)    //M0板上led on
#define PD11_H    DrvGPIO_SetBit(E_GPD,11)	    //中断时间测试
#define PD11_L    DrvGPIO_ClrBit(E_GPD,11)	    //中断时间测试


//////////////////////////////
//			变量定义        //
//////////////////////////////

// 1s软件定时器计数
unsigned int clock1s=0;
// 1s软件定时器溢出标志
unsigned int clock1s_flag=0;
unsigned int times10s=0;
unsigned int led=0;
long int scaling_ADC=100000;
long int tmp_adc=0;								
int32_t scaling_lA=0;						
int32_t scaling_allA=0;	
int32_t scaling_sA=1000000000;
int32_t scaling_lB=0;					
int32_t scaling_allB=0;
int32_t scaling_sB=1000000000;
int32_t adc_A=0;
int32_t adc_B=0;
int32_t para_A_m=0,para_A_p=0,para_B_m=0,para_B_p=0;
int flag=0;
int change=1;
int flag_test=1;
unsigned int frequency=100;
int sub1,sub2;
	
int phase_cur1=0;
int phase_cur2=24;
void itoa(int i, unsigned char* istr)
{
	unsigned int j;
	j=i/1000;
	istr[0]='0'+j;
	i=i-j*1000;
	j=i/100;
	istr[1]='0'+j;
	i=i-j*100;
	j=i/10;
	istr[2]='0'+j;
	i=i-j*10;
	istr[3]='0'+i;
	istr[4]='\0';
}


//  I/O端口初始化
void Port_Init(void)
{
		LCD_Port_Init();								   //LCD接口初始化
		button_Port_Init();								   //LCD板上按键初始化

		DrvGPIO_Open(E_GPA, 10, E_IO_OUTPUT);			  //LED
		DrvGPIO_Open(E_GPD, 11, E_IO_OUTPUT);			   //中断时间测试管脚
}

 
// Timer0_A0 interrupt service routine
void AdcSingleCycleModeTest();
 void Timer0_Callback (void)
{
	PD11_L;	 //中断时间测试

	ENTER_detect();
	DOWN_detect();
	UP_detect();
	INCREASE_detect();
	DECREASE_detect();
 
////// 1秒钟软定时器计数 /////////////////////////////////
	if (++clock1s>=V_T1s)
	{
		clock1s_flag = 1; //当1秒到时，溢出标志置1
		clock1s = 0;
	}

		PD11_H;	//中断时间测试
	
	
}

//TIMER0 initialize -
// desired value: 5ms
void Timer0_Init(void)
{
	DrvTIMER_Init();//初始化timer
	DrvTIMER_Open(E_TMR0,200,E_PERIODIC_MODE);//设置定时器timer0,定时器tick每秒200次 ,5ms
	DrvTIMER_SetTimerEvent(E_TMR0,1,(TIMER_CALLBACK) Timer0_Callback,0); //安装定时处理事件到timer0
	
}

void Init_Devices(void)
{	  
	SYSCLK->APBCLK.WDT_EN =0;//Disable WDT clock source

	DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1);	 // Enable the external 12MHz oscillator oscillation
	DrvSYS_SelectHCLKSource(0);	 // HCLK clock source. 0: external 12MHz; 4:internal 22MHz RC oscillator
	DrvSYS_SetClockDivider(E_SYS_HCLK_DIV, 11); // HCLK clock frequency = 12M/(11+1)=1M

	DrvADC_Open(ADC_SINGLE_END, ADC_SINGLE_CYCLE_OP, 192, INTERNAL_HCLK, 0);  //使用ADCchannel 6&7；ADCClock = 1M/(0+1)  =1M

	Port_Init();             //初始化I/O口
    Timer0_Init();          //初始化定时器0
	
	DrvTIMER_Start(E_TMR0);

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t SingleEndInput_ScanOpModeChannelSelect(uint8_t * u8ChannelSelBitwise, uint8_t * pu8ActiveChannel)
{
    uint8_t u8ChannelCount, u8ActiveChannelCount, u8Option;
    E_DRVGPIO_FUNC aeADCPinFunc[]={ E_FUNC_ADC0, E_FUNC_ADC1, E_FUNC_ADC2, E_FUNC_ADC3,
                                    E_FUNC_ADC4, E_FUNC_ADC5, E_FUNC_ADC6, E_FUNC_ADC7 };

    u8ActiveChannelCount = 0;
    *u8ChannelSelBitwise = 0;
    for(u8ChannelCount=6; u8ChannelCount<8; u8ChannelCount++)
    {
        while(1)
        {
            u8Option = 'y';
            if( (u8Option=='y') || (u8Option=='Y') )
            {
                *u8ChannelSelBitwise = *u8ChannelSelBitwise | (1<<u8ChannelCount);
                *(pu8ActiveChannel + u8ActiveChannelCount) = u8ChannelCount;
                u8ActiveChannelCount++;
                /* Disable the digital input path */
                DrvGPIO_DisableDigitalInputBit(E_GPA, u8ChannelCount);
                /* Configure the corresponding ADC analog input pin */
                DrvGPIO_InitFunction(aeADCPinFunc[u8ChannelCount]);
                break;
            }
        }
    }
    return u8ActiveChannelCount;
}
void AdcContScanModeTest()
{
    E_ADC_INPUT_MODE InputMode;
    uint8_t u8ChannelSelBitwise=0, u8ActiveChannelNum=0, u8ChannelNum, u8ChannelCount;
    int32_t i32ConversionData;
    uint8_t au8ActiveChannel[8];
    
   // printf("\n=== Continuous scan mode test ===\n");
    /* Set the ADC operation mode as continuous scan mode */
    DrvADC_SetADCOperationMode(ADC_CONTINUOUS_OP);
    

    InputMode = ADC_SINGLE_END;
    /* Set the ADC input mode */
    DrvADC_SetADCInputMode(InputMode);
    

    /* Select the ADC channels */
    u8ActiveChannelNum = SingleEndInput_ScanOpModeChannelSelect(&u8ChannelSelBitwise, au8ActiveChannel);
    /* Check the active channel number */
    if(u8ActiveChannelNum==0)
    {
    //    printf("No channel was selected.\n");
    //    printf("--- Exit continuous scan mode test ---\n");
        return ;
    }
    
    /* Set the ADC channel */
    DrvADC_SetADCChannel(u8ChannelSelBitwise);
    
    /* start A/D conversion */
    DrvADC_StartConvert();

    for (tmp_adc=0;tmp_adc<scaling_ADC;tmp_adc++){
    /* Wait conversion done */
        while(DrvADC_IsConversionDone()==FALSE);
        
        for(u8ChannelCount=0; u8ChannelCount<u8ActiveChannelNum; u8ChannelCount++)
        {
            u8ChannelNum = au8ActiveChannel[u8ChannelCount];
            i32ConversionData = DrvADC_GetConversionData(u8ChannelNum);
            //printf("Conversion result of channel %d: 0x%X (%d)\n\n", u8ChannelNum, i32ConversionData, i32ConversionData);
        	  if (u8ChannelCount==0){
								if (i32ConversionData>scaling_lA)	scaling_lA = i32ConversionData;
								if (i32ConversionData<scaling_sA)	scaling_sA = i32ConversionData;
								/*scaling_allA += i32ConversionData;*/
								
						}
						if (u8ChannelCount==1){
								if (i32ConversionData>scaling_lB)	scaling_lB = i32ConversionData;
								if (i32ConversionData<scaling_sB)	scaling_sB = i32ConversionData;
								/*scaling_allB += i32ConversionData;*/
						}
				}
				
		}
	para_A_m= (scaling_lA-scaling_sA)/2;
	para_A_p= (scaling_lA+scaling_sA)/2;   
	para_B_m= (scaling_lB-scaling_sB)/2;
	para_B_p= (scaling_lB+scaling_sB)/2;
	/*para_A_p = scaling_allA/scaling_ADC;
	para_B_p = scaling_allB/scaling_ADC;*/
    /* Stop A/D conversion */
    DrvADC_StopConvert();
    
    /* Clear the ADC interrupt flag */
    _DRVADC_CLEAR_ADC_INT_FLAG();
}
void AdcSingleCycleModeTest()
{
		int change1=0;
		int change2=0;
    E_ADC_INPUT_MODE InputMode;
    uint8_t u8ChannelSelBitwise=0, u8ActiveChannelNum=0, u8ChannelNum, u8ChannelCount;
    int32_t i32ConversionData;
    uint8_t au8ActiveChannel[8];
   // printf("\n=== Single cycle scan mode test ===\n");
   /* Set the ADC operation mode as single cycle scan mode */
	DrvADC_SetADCOperationMode(ADC_SINGLE_CYCLE_OP);
   

    InputMode = ADC_SINGLE_END;

    /* Set the ADC input mode */
    DrvADC_SetADCInputMode(InputMode);
    

    /* Select the ADC channels */
    u8ActiveChannelNum = SingleEndInput_ScanOpModeChannelSelect(&u8ChannelSelBitwise, au8ActiveChannel);
    /* Check the active channel number */
    if(u8ActiveChannelNum==0)
    {
       // printf("No channel was selected.\n");
       // printf("--- Exit single cycle scan mode test ---\n");
        return ;
    }
    
    /* Set the ADC channel */
    DrvADC_SetADCChannel(u8ChannelSelBitwise);
    
    /* start A/D conversion */
    DrvADC_StartConvert();
    
    /* Wait conversion done */
    while(DrvADC_IsConversionDone()==FALSE);
    
    for(u8ChannelCount=0; u8ChannelCount<u8ActiveChannelNum; u8ChannelCount++)
    {
        u8ChannelNum = au8ActiveChannel[u8ChannelCount];
        i32ConversionData = DrvADC_GetConversionData(u8ChannelNum);
        //printf("Conversion result of channel %d: 0x%X (%d)\n\n", u8ChannelNum, i32ConversionData, i32ConversionData);
    	if (u8ChannelCount==0){
			adc_A=i32ConversionData;
//			if (adc_A-para_A_p==0) change=0;
//			else if (adc_A>para_A_p) change=1;
//			else change=-1;
//			sub1=adc_A-para_A_p;
				change1=adc_A-para_A_p;
		}
		else if (u8ChannelCount==1) {
			adc_B=i32ConversionData;
//			if (adc_B-para_B_p==0 ) change=change*0;
//			else if (adc_B>para_B_p) change=1*change;
//			else change=-1*change;
//			sub2=adc_B-para_B_p;
			change2=adc_B-para_B_p;
		}	
		change=change1*change2;
		flag=1;
	}      
}

int main(void)
{
	int abs_change;
	int step;
//	sprintf(a0_s0,"%d",tmm);
//	sprintf(a0_s1,"%d",tmm);
	Init_Devices();
	DrvSYS_Delay(2000);
	times10s =2000;
	Initial_lcd();
	init_act();


	ad9850_Port_Init();
	ad9850_reset();
	DrvTIMER_EnableInt(E_TMR0);
	ui_state=0;
	flag=1;
	ui_state=0;

	
//	a0_s0[0]='a';
	setup_AD9850(frequency,frequency,phase_cur1,phase_cur2);
	AdcContScanModeTest();
// 主循环，本例中，在Timer0_A0中断服务程序未被执行的空余时间里，处理机在以下程序中不断循环
	while(1)
	{
//	 	sprintf(a0_s0,"%d",scaling_lA);
//		sprintf(a0_s1,"%d",scaling_sA);
//		sprintf(a0_s0,"%d",para_A_p);
//		sprintf(a0_s1,"%d",para_B_p);
		//sprintf(a0_s0,"%d",change);
		//sprintf(a0_s1,"%d",sub2);
		ui_state_proc(ui_state);
		//AdcSingleCycleModeTest();
	   	//	sprintf(a0_s1,"%d",change);
		// setup_AD9850,phase_cur1 represents the first sin wave,
		//i.e. phase_cur1 is the leading phase
		//ui_state=0;
		if (ui_state==706)
		{
			AdcSingleCycleModeTest();
			
			step=1;
			
			
			//sprintf(a0_s1,"%d",change);
			if (change>0)
		 	{
			 	phase_cur1-=step;
			 	if (phase_cur1<0)
				phase_cur1+=32;
				flag_test=1;
		 	}
		 	else if (change<0)
		 	{
				phase_cur1+=step;
			 	if (phase_cur1>31)
				phase_cur1-=32;
				flag_test=1;
		 	}
		 
			phase_cur2=phase_cur1+24;
		 	if (phase_cur2>31)
				phase_cur2-=32;
		 
			setup_AD9850(mode3_freq,mode3_freq,phase_cur1,phase_cur2);
		 
		 	//change=0;
		 	flag=0;
	 	
			  
      ////////////////////////////////////////////////////////////////////

		
		}

		if (ui_state==206)
		{
			setup_AD9850(mode1_freq,mode1_freq_b,0,0);
		}

		if (ui_state==505)
		{
			setup_AD9850(mode2_freq,mode2_freq,0,mode2_phasediff);
		}

		if (clock1s_flag==1 && flag_test==1)   // ?ì2é1???¨ê±ê?·?μ?
		{
			clock1s_flag=0;

			if(led==0) { A10LED_L;led=1;}
			else       { A10LED_H;led=0;}
		}
	}		 	 
}
