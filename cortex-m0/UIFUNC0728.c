//
//  test.c
//  
//
//  Created by 吴昊 on 17/5/22.
//
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NUC1xx.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvSPI.h"
#include "Driver\DrvTIMER.h"

#define KEYTMR_OF  100

unsigned int mode = 0;
unsigned int modechangeflag = 0;
unsigned int mode1_freq = 0;     //KHz
unsigned int mode1_freq_b = 0;     //KHz
unsigned int mode1_phasediff = 0;	 //multiplied by 11.25degree
unsigned int init = 0;
unsigned int mode2_freq = 100;	 //KHz
unsigned int mode2_phasediff = 8;	 //multiplied by 11.25degree
unsigned int mode3_freq = 100;
//unsigned int mode3_phasediff=0;
int  phase3_0= 0, phase3_90 = 8;


int32_t para_A_m,para_A_p,para_B_m,para_B_p;
int32_t tmp,tmp_2;
int32_t q,r;

unsigned char stringdegree_00[]="0°     ";
unsigned char stringdegree_01[]="11.25° ";
unsigned char stringdegree_02[]="22.5°  ";
unsigned char stringdegree_03[]="33.75° ";
unsigned char stringdegree_04[]="45°    ";
unsigned char stringdegree_05[]="56.25° ";
unsigned char stringdegree_06[]="67.5°  ";
unsigned char stringdegree_07[]="78.75° ";
unsigned char stringdegree_08[]="90°    ";


struct struct_act
{
    unsigned char num;
    unsigned char *str[20];
    unsigned char x[20],y[20],inverse[20];
} a0,a1,a2,a4,a5,a7;

struct struct_act *act[8];


unsigned char a0_s0[] = "开机", a0_s1[] = "欢迎", a0_s2[] = "按任意键进入";
unsigned char a1_s0[] = "请选择工作模式", a1_s1[] = "双路独立信号源", a1_s2[] = "相差可调信号源", a1_s3[] = "costas";
unsigned char a2_s0[] = "频率", a2_s1[] = "KHz", a2_s2[] = "返回", a2_s3[] = "1", a2_s4[] = "0", a2_s5[] = "0", a2_s6[] = "运行", a2_s7[] = "1", a2_s8[] = "0", a2_s9[] = "0";
//unsigned char *a2_s9;
unsigned char a4_s0[] = "相差可调信号源", a4_s1[] = "频率", a4_s2[] = "KHz", a4_s3[] = "相差", a4_s4[] = "运行", a4_s5[] = "退回", a4_s6[] = "1", a4_s7[] = "0", a4_s8[] = "0";
unsigned char *a4_s9;
unsigned char a5_s0[] = "相差可调信号源", a5_s1[] = "频率", a5_s2[] = "KHz", a5_s3[] = "相差", a5_s4[] = "正在运行", a5_s5[] = "退出", a5_s6[] = "1", a5_s7[] = "0", a5_s8[] = "0", a5_s9[] = "191.25°", a5_s10[] = "定标并返回";
//a5_s11[] = "     ", a5_s12[] = "     ";
unsigned char a7_s0[] = "频率", a7_s1[] = "KHz", a7_s2[] = "返回", a7_s3[] = "1", a7_s4[] = "0", a7_s5[] = "0", a7_s6[] = "运行", a7_s7[]="定标", a7_s8[10], a7_s9[10];

unsigned int ui_state=0;  //状态号

unsigned int key_ENTER_state=0;
unsigned int key_ENTER_prestate=0;
unsigned int ENTER_key_timer=0;
unsigned int key_ENTER_flag=0;
 

unsigned int key_DOWN_state=0;
unsigned int key_DOWN_prestate=0;
unsigned int key_DOWN_timer=0;
unsigned int key_DOWN_flag=0;


unsigned int key_UP_state=0;
unsigned int key_UP_prestate=0;
unsigned int key_UP_timer=0;
unsigned int key_UP_flag=0;


unsigned int key_INCREASE_state=0;
unsigned int key_INCREASE_prestate=0;
unsigned int key_INCREASE_timer=0;
unsigned int key_INCREASE_flag=0;
int degree_counter=8;

unsigned int key_DECREASE_state=0;
unsigned int key_DECREASE_prestate=0;
unsigned int key_DECREASE_timer=0;
unsigned int key_DECREASE_flag=0;

unsigned int key_state=0;
unsigned int key_prestate=0;unsigned int state=0;
unsigned int key_timer=0;
unsigned int key_flag=0;
unsigned char istr1[]="1234  ";




void itodegree(unsigned int phasediff, unsigned char **instrde)
{
    
    switch (phasediff)
    {
        case 0: *instrde = stringdegree_00;break;
        case 1: *instrde = stringdegree_01;break;
        case 2: *instrde = stringdegree_02;break;
        case 3: *instrde = stringdegree_03;break;
        case 4: *instrde = stringdegree_04;break;
        case 5: *instrde = stringdegree_05;break;
        case 6: *instrde = stringdegree_06;break;
        case 7: *instrde = stringdegree_07;break;
        case 8: *instrde = stringdegree_08;break;
            
        default: break;
    }
}


void itoafreq(unsigned int i, unsigned char* istr1,unsigned char* istr2,unsigned char* istr3)
{
    unsigned int j;
    j=i/100;
    istr1[0]='0'+j;istr1[1]='\0';
    i=i-j*100;
    j=i/10;
    istr2[0]='0'+j;istr2[1]='\0';
    i=i-j*10;
    istr3[0]='0'+i;istr3[1]='\0';
}


unsigned int atoifreq(unsigned char* istr1,unsigned char* istr2,unsigned char* istr3)
{
    unsigned int a,b,c,i;
    a=istr1[0]-'0';b=istr2[0]-'0';c=istr3[0]-'0';
    i = a*100+b*10+c;
    return i;
}


//  底板上按钮所用I/O端口初始化
void button_Port_Init(void)
{
    DrvGPIO_Open(E_GPC, 1, E_IO_QUASI);			  // GPC1	 ENTER
    DrvGPIO_Open(E_GPC, 2, E_IO_QUASI);			  // GPC2	 DOWN
    DrvGPIO_Open(E_GPC, 3, E_IO_QUASI);			  // GPC3	 UP
    DrvGPIO_Open(E_GPC, 4, E_IO_QUASI);			  // GPC4	 INCREASE
    DrvGPIO_Open(E_GPC, 5, E_IO_QUASI);			  // GPC5	 DECREASE
    
}


//按钮按下检测
void ENTER_detect(void)
{
    if (DrvGPIO_GetBit(E_GPC,1) == 0) ///////////////////	 ENTER
    {
        key_ENTER_prestate=key_ENTER_state;
        key_ENTER_state=0;
        if (key_ENTER_prestate==1) 	key_ENTER_flag=1;
    }
    else
    {
        key_ENTER_prestate = key_ENTER_state;
        key_ENTER_state=1;
    }
}

void DOWN_detect(void)
{
    if (DrvGPIO_GetBit(E_GPC,2) == 0) ///////////////////	 DOWN
    {
        key_DOWN_prestate=key_DOWN_state;
        key_DOWN_state=0;
        if (key_DOWN_prestate==1) key_DOWN_flag=1;
        
    }
    else
    {
        key_DOWN_prestate = key_DOWN_state;
        key_DOWN_state=1;
    }
    
}

void UP_detect(void)
{
    if (DrvGPIO_GetBit(E_GPC,3) == 0) ///////////////////	 UP
    {
        key_UP_prestate=key_UP_state;
        key_UP_state=0;
        if (key_UP_prestate==1)  key_UP_flag=1;
    }
    else
    {
        key_UP_prestate = key_UP_state;
        key_UP_state=1;
    }
}

void INCREASE_detect(void)
{
    if (DrvGPIO_GetBit(E_GPC,4) == 0) ///////////////////	 INCREASE
    {
        key_INCREASE_prestate=key_INCREASE_state;
        key_INCREASE_state=0;
        if (key_INCREASE_prestate==1)
        {	key_INCREASE_flag=1;	key_INCREASE_timer =0;	}
        else if (key_INCREASE_prestate==0)
        {
            if 	(++key_INCREASE_timer>=KEYTMR_OF)
            { key_INCREASE_flag=1; key_INCREASE_timer=0;}
        }
    }
    else
    {
        key_INCREASE_prestate = key_INCREASE_state;
        key_INCREASE_state=1;
        key_INCREASE_timer=0;
    }
}

void DECREASE_detect(void)
{
    if (DrvGPIO_GetBit(E_GPC,5) == 0) ///////////////////	 DECREASE
    {
        key_DECREASE_prestate=key_DECREASE_state;
        key_DECREASE_state=0;
        if (key_DECREASE_prestate==1)
        {	key_DECREASE_flag=1;	key_DECREASE_timer =0;	}
        else if (key_DECREASE_prestate==0)
        {
            if 	(++key_DECREASE_timer>=KEYTMR_OF)
            { key_DECREASE_flag=1; key_DECREASE_timer=0;}
        }
    }
    else
    {
        key_DECREASE_prestate = key_DECREASE_state;
        key_DECREASE_state=1;
        key_DECREASE_timer=0;
    }
}


void display_ui_act(unsigned int i)
{
    unsigned int j=0;
    clear_screen();
    for (j=0;j<act[i]->num;j++)
    {
        display_GB2312_string(act[i]->x[j],(act[i]->y[j]-1)*8+1,act[i]->str[j],act[i]->inverse[j]);
    }
}


void init_act(void)
{
    itoafreq(mode1_freq, a2_s3, a2_s4, a2_s5);
    itoafreq(mode1_freq_b, a2_s7, a2_s8, a2_s9);		 ////////////////////////////////////////////////////
    itoafreq(mode2_freq, a4_s6, a4_s7, a4_s8);
    itodegree(mode2_phasediff, &a4_s9);
    itoafreq(mode3_freq, a7_s3, a7_s4, a7_s5);
    
    a0.num = 3;
    a0.str[0] = a0_s0; a0.x[0] = 1;  a0.y[0] = 1;  a0.inverse[0] = 0;
    a0.str[1] = a0_s1; a0.x[1] = 3;  a0.y[1] = 1;  a0.inverse[1] = 0;
    a0.str[2] = a0_s2; a0.x[2] = 5;  a0.y[2] = 1;  a0.inverse[2] = 0;						  ///////act0
    act[0] = &a0;
    
    a1.num = 4;                                                                                 ///////act1
    a1.str[0] = a1_s0;	a1.x[0] = 1;	a1.y[0] = 1;	a1.inverse[0] = 0;                      ////choose mode
    a1.str[1] = a1_s1;	a1.x[1] = 3;	a1.y[1] = 1;	a1.inverse[1] = 0;                      ////double way
    a1.str[2] = a1_s2;	a1.x[2] = 5;	a1.y[2] = 1;	a1.inverse[2] = 0;                      ////phase deviation
    a1.str[3] = a1_s3;	a1.x[3] = 7;	a1.y[3] = 1;	a1.inverse[3] = 0;                      ////costas
    act[1] = &a1;
    
    a2.num = 10;                                                                                  //////act2
    a2.str[0] = a2_s0;	a2.x[0] = 1;	a2.y[0] = 1;	a2.inverse[0] = 0;                       //////freq
    a2.str[1] = a2_s1;	a2.x[1] = 1;	a2.y[1] = 11;	a2.inverse[1] = 0;                       //////HZ
    a2.str[2] = a2_s2;  a2.x[2] = 7;	a2.y[2] = 1;	a2.inverse[2] = 0;                       /////RETURN
    a2.str[3] = a2_s3;	a2.x[3] = 1;	a2.y[3] = 6;	a2.inverse[3] = 0;                       ////Bai wei
    a2.str[4] = a2_s4;	a2.x[4] = 1;	a2.y[4] = 7;	a2.inverse[4] = 0;											 ///Shi wei
    a2.str[5] = a2_s5;  a2.x[5] = 1;	a2.y[5] = 8;	a2.inverse[5] = 0;											 ///Ge wei
    a2.str[6] = a2_s6;	a2.x[6] = 7;	a2.y[6] = 13;	a2.inverse[6] = 0;                       ////WORK
    a2.str[7] = a2_s7;	a2.x[7] = 4;	a2.y[7] = 6;	a2.inverse[7] = 0; 
    a2.str[8] = a2_s8;	a2.x[8] = 4;	a2.y[8] = 7;	a2.inverse[8] = 0;
    a2.str[9] = a2_s9;	a2.x[9] = 4;	a2.y[9] = 8;	a2.inverse[9] = 0;  
    act[2] = &a2;
    
    a4.num = 10;
    a4.str[0] = a4_s0;	a4.x[0] = 1;	a4.y[0] = 1;	a4.inverse[0] = 0;
    a4.str[1] = a4_s1;	a4.x[1] = 3;	a4.y[1] = 1;	a4.inverse[1] = 0;
    a4.str[2] = a4_s2;	a4.x[2] = 3;	a4.y[2] = 9;	a4.inverse[2] = 0;
    a4.str[3] = a4_s3;	a4.x[3] = 5;	a4.y[3] = 1;	a4.inverse[3] = 0;
    a4.str[4] = a4_s4;	a4.x[4] = 7;	a4.y[4] = 1;	a4.inverse[4] = 0;
    a4.str[5] = a4_s5;	a4.x[5] = 7;	a4.y[5] = 6;	a4.inverse[5] = 0;
    a4.str[6] = a4_s6;	a4.x[6] = 3;	a4.y[6] = 6;	a4.inverse[6] = 0;
    a4.str[7] = a4_s7;	a4.x[7] = 3;	a4.y[7] = 7;	a4.inverse[7] = 0;
    a4.str[8] = a4_s8;	a4.x[8] = 3;	a4.y[8] = 8;	a4.inverse[8] = 0;
    a4.str[9] = a4_s9;	a4.x[9] = 5;	a4.y[9] = 6;	a4.inverse[9] = 0;               	/////act4
    act[4] = &a4;
    
    a5.num = 13;
    a5.str[0] = a5_s0;	a5.x[0] = 1;	a5.y[0] = 1;	a5.inverse[0] = 0;
    a5.str[1] = a5_s1;	a5.x[1] = 3;	a5.y[1] = 1;	a5.inverse[1] = 0;
    a5.str[2] = a5_s2;	a5.x[2] = 3;	a5.y[2] = 9;	a5.inverse[0] = 0;
    a5.str[3] = a5_s3;	a5.x[3] = 5;	a5.y[3] = 1;	a5.inverse[3] = 0;
    a5.str[4] = a5_s4;	a5.x[4] = 7;	a5.y[4] = 1;	a5.inverse[4] = 0;
    a5.str[5] = a5_s5;	a5.x[5] = 7;	a5.y[5] = 13;	a5.inverse[5] = 0;
    a5.str[6] = a5_s6;	a5.x[6] = 3;	a5.y[6] = 6;	a5.inverse[6] = 0;
    a5.str[7] = a5_s7;	a5.x[7] = 3;	a5.y[7] = 7;	a5.inverse[7] = 0;
    a5.str[8] = a5_s8;	a5.x[8] = 3;	a5.y[8] = 8;	a5.inverse[8] = 0;
    a5.str[9] = a5_s9;	a5.x[9] = 5;	a5.y[9] = 6;	a5.inverse[9] = 0;
    a5.str[10] = a5_s10;	a5.x[10] = 5;	a5.y[10] = 13;	a5.inverse[10] = 0;             ////act5
    
    act[5] = &a5;
    
    a7.num = 10;
    a7.str[0] = a7_s0;	a7.x[0] = 1;	a7.y[0] = 1;	a7.inverse[0] = 0;                 //the same as act1
    a7.str[1] = a7_s1;	a7.x[1] = 1;	a7.y[1] = 11;	a7.inverse[1] = 0;
    a7.str[2] = a7_s2;  a7.x[2] = 7;	a7.y[2] = 1;	a7.inverse[2] = 0;
    a7.str[3] = a7_s3;	a7.x[3] = 1;	a7.y[3] = 6;	a7.inverse[3] = 0;
    a7.str[4] = a7_s4;	a7.x[4] = 1;	a7.y[4] = 7;	a7.inverse[4] = 0;
    a7.str[5] = a7_s5;  a7.x[5] = 1;	a7.y[5] = 8;	a7.inverse[5] = 0;
    a7.str[6] = a7_s6;	a7.x[6] = 7;	a7.y[6] = 13;	a7.inverse[6] = 0;
    a7.str[7] = a7_s7;	a7.x[7] = 4;	a7.y[7] = 1;	a7.inverse[7] = 0;                  ///act7
    a7.str[8] = a7_s8;  a7.x[8] = 4;  a7.y[8] = 6;  a7.inverse[8] = 0;
		a7.str[9] = a7_s9;  a7.x[9] = 4;  a7.y[9] = 11;  a7.inverse[9] = 0;                  ///act7
    act[7] = &a7;
    
    display_ui_act(0);
}


void in_de (unsigned int w, unsigned char* actstring )		//w=1 : increase; w=2 : decrease;  用于phase
{
    if (w==1)
    {
        (*actstring)++;
        if (*actstring>'9') *actstring='0';
    }
    
    if (w==2)
    {
        (*actstring)--;
        if (*actstring<'0') *actstring='9';
    }
    
}

void in_de_f(unsigned int w, unsigned char* actstring)		//w=1 : increase; w=2 : decrease;	 用于frequency
{
    if (w == 1)
    {
        (*actstring)++;
        if (*actstring>'9') *actstring = '0';
    }
    
    if (w == 2)
    {
        (*actstring)--;
        if (*actstring<'0') *actstring = '9';
    }
    
}

void ui_proc0(void)
{
    if(key_ENTER_flag||key_UP_flag||key_DOWN_flag||key_INCREASE_flag||key_DECREASE_flag)
    {
        key_ENTER_flag=0;key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;
        act[1]->inverse[1]=1; display_ui_act(1);
        ui_state=101;
    }
}

void ui_proc101(void)
{
    if(key_UP_flag)
    {
        act[1]->inverse[1]=0; display_GB2312_string(act[1]->x[1],(act[1]->y[1]-1)*8+1,act[1]->str[1],act[1]->inverse[1]);
        act[1]->inverse[3]=1; display_GB2312_string(act[1]->x[3],(act[1]->y[3]-1)*8+1,act[1]->str[3],act[1]->inverse[3]);
        ui_state=103;
    }
    else if (key_DOWN_flag)
    {
        act[1]->inverse[1]=0; display_GB2312_string(act[1]->x[1],(act[1]->y[1]-1)*8+1,act[1]->str[1],act[1]->inverse[1]);
        act[1]->inverse[2]=1; display_GB2312_string(act[1]->x[2],(act[1]->y[2]-1)*8+1,act[1]->str[2],act[1]->inverse[2]);
        ui_state=102;
    }
    else if (key_ENTER_flag)
    {
        act[1]->inverse[1]=0;
        act[2]->inverse[2]=1; display_ui_act(2);
        ui_state=201;
        mode=1;
    }
    
    if(key_ENTER_flag||key_UP_flag||key_DOWN_flag||key_INCREASE_flag||key_DECREASE_flag)
    {
        key_ENTER_flag=0;key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;
    }
}


void ui_proc102(void)
{
    if(key_UP_flag)
    {
        act[1]->inverse[2]=0; display_GB2312_string(act[1]->x[2],(act[1]->y[2]-1)*8+1,act[1]->str[2],act[1]->inverse[2]);
        act[1]->inverse[1]=1; display_GB2312_string(act[1]->x[1],(act[1]->y[1]-1)*8+1,act[1]->str[1],act[1]->inverse[1]);
        ui_state=101;
    }
    else if (key_DOWN_flag)
    {
        act[1]->inverse[2]=0; display_GB2312_string(act[1]->x[2],(act[1]->y[2]-1)*8+1,act[1]->str[2],act[1]->inverse[2]);
        act[1]->inverse[3]=1; display_GB2312_string(act[1]->x[3],(act[1]->y[3]-1)*8+1,act[1]->str[3],act[1]->inverse[3]);
        ui_state=103;
    }
    else if (key_ENTER_flag)
    {
        act[1]->inverse[2]=0;
        itodegree(mode2_phasediff,&a4_s9);a4.str[9]=a4_s9;
        degree_counter=mode2_phasediff;
        act[4]->inverse[6]=1; display_ui_act(4);
        ui_state=406;
        mode=2;
    }
    
    if(key_ENTER_flag||key_UP_flag||key_DOWN_flag||key_INCREASE_flag||key_DECREASE_flag)
    {
        key_ENTER_flag=0;key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;
    }
}

void ui_proc103(void)
{   if(key_UP_flag)
    {
        act[1]->inverse[2]=1;display_GB2312_string(act[1]->x[2],(act[1]->y[2]-1)*8+1,act[1]->str[2],act[1]->inverse[2]);
        act[1]->inverse[3]=0;display_GB2312_string(act[1]->x[3],(act[1]->y[3]-1)*8+1,act[1]->str[3],act[1]->inverse[3]);
        ui_state=102;
    }
    else if(key_DOWN_flag)
    {   act[1]->inverse[1]=1;display_GB2312_string(act[1]->x[1],(act[1]->y[1]-1)*8+1,act[1]->str[1],act[1]->inverse[1]);
        act[1]->inverse[3]=0;display_GB2312_string(act[1]->x[3],(act[1]->y[3]-1)*8+1,act[1]->str[3],act[1]->inverse[3]);
				ui_state=101;
    }
    else if(key_ENTER_flag)      /////////////////////////
    {
        act[1]->inverse[3]=0;
        act[7]->inverse[2]=1; 
			
						tmp=para_A_p;
			
				q=tmp/1000;
			  a7_s8[0]='0'+q;
			  r=tmp-q*1000;
				q=r/100;
				a7_s8[1]='0'+q;
				r=r-q*100;
				q=r/10;
			  a7_s8[2]='0'+q;
			  r=r-q*10;
			  q=r;
			  a7_s8[3]='0'+q;
				a7_s8[4]='\0';
			  
			  a7.str[8]=a7_s8;
			
			  tmp_2=para_B_p;
			
				q=tmp_2/1000;
			  a7_s9[0]='0'+q;
			  r=tmp_2-q*1000;
				q=r/100;
				a7_s9[1]='0'+q;
				r=r-q*100;
				q=r/10;
			  a7_s9[2]='0'+q;
			  r=r-q*10;
			  q=r;
			  a7_s9[3]='0'+q;
				a7_s9[4]='\0';
			  
			  a7.str[9]=a7_s9;
			
			display_ui_act(7);
        ui_state=701;
        mode=3;
    }
    if(key_ENTER_flag||key_UP_flag||key_DOWN_flag||key_INCREASE_flag||key_DECREASE_flag)
    {
        key_ENTER_flag=0;key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;
    }

}


void ui_proc201(void)                           //wait for return
{
    if(key_UP_flag)
    {
        act[2]->inverse[2]=0; display_GB2312_string(act[2]->x[2],(act[2]->y[2]-1)*8+1,act[2]->str[2],act[2]->inverse[2]);
        act[2]->inverse[6]=1; display_GB2312_string(act[2]->x[6],(act[2]->y[6]-1)*8+1,act[2]->str[6],act[2]->inverse[6]);
        ui_state=207;
    }
    else if (key_DOWN_flag)
    {
        act[2]->inverse[2]=0; display_GB2312_string(act[2]->x[2],(act[2]->y[2]-1)*8+1,act[2]->str[2],act[2]->inverse[2]);
        act[2]->inverse[3]=1; display_GB2312_string(act[2]->x[3],(act[2]->y[3]-1)*8+1,act[2]->str[3],act[2]->inverse[3]);
        ui_state=203;
    }
    else if (key_ENTER_flag)
    {
        act[2]->inverse[2]=0;
        act[1]->inverse[1]=1; display_ui_act(1);
        mode=0;
        ui_state=101;
    }
    
    if(key_ENTER_flag||key_UP_flag||key_DOWN_flag||key_INCREASE_flag||key_DECREASE_flag)
    {
        key_ENTER_flag=0;key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;
    }
}


void ui_proc207(void)                           //going to work
{
    if(key_UP_flag)
    {
        act[2]->inverse[9]=1; display_GB2312_string(act[2]->x[9],(act[2]->y[9]-1)*8+1,act[2]->str[9],act[2]->inverse[9]);
        act[2]->inverse[6]=0; display_GB2312_string(act[2]->x[6],(act[2]->y[6]-1)*8+1,act[2]->str[6],act[2]->inverse[6]);
        ui_state=210;
    }
    else if(key_DOWN_flag)
    {
        act[2]->inverse[2]=1; display_GB2312_string(act[2]->x[2],(act[2]->y[2]-1)*8+1,act[2]->str[2],act[2]->inverse[2]);
        act[2]->inverse[6]=0; display_GB2312_string(act[2]->x[6],(act[2]->y[6]-1)*8+1,act[2]->str[6],act[2]->inverse[6]);
        ui_state=201;
    }
    else if(key_ENTER_flag)
    {
        act[2]->str[6] = "退出";
        act[2]->inverse[6] = 1; display_GB2312_string(act[2]->x[6], (act[2]->y[6] - 1) * 8 + 1, act[2]->str[6], act[2]->inverse[6]);
        ui_state = 206;
    }
    
    if (key_ENTER_flag || key_UP_flag || key_DOWN_flag || key_INCREASE_flag || key_DECREASE_flag)
    {
        key_ENTER_flag = 0; key_UP_flag = 0; key_DOWN_flag = 0; key_INCREASE_flag = 0; key_DECREASE_flag = 0;
    }
}

void ui_proc206(void)                       //from work to stop
{   //unsigned char a = *(act[2]->str[3]);
    //unsigned char b = *(act[2]->str[4]);
    //unsigned char c = *(act[2]->str[5]);
    //mode1_freq = (a - '0') * 100 + (b - '0') * 10 + (c - '0');
    itoafreq(mode1_freq,a2_s3,a2_s4,a2_s5);
    itoafreq(mode1_freq_b,a2_s7,a2_s8,a2_s9);
    setup_AD9850(mode1_freq, mode1_freq_b, mode1_phasediff, mode1_phasediff);
    
    if (key_ENTER_flag)
    {
        act[2]->str[6] = "运行";
        act[2]->inverse[6] = 1; display_GB2312_string(act[2]->x[6], (act[2]->y[6] - 1) * 8 + 1, act[2]->str[6], act[2]->inverse[6]);
        ui_state = 207;
    }
    
    if (key_ENTER_flag || key_UP_flag || key_DOWN_flag || key_INCREASE_flag || key_DECREASE_flag)
    {
        key_ENTER_flag = 0; key_UP_flag = 0; key_DOWN_flag = 0; key_INCREASE_flag = 0; key_DECREASE_flag = 0;
    }
}

void ui_proc203(void)                           //change 3position
{
    if (key_UP_flag)
    {
        act[2]->inverse[2] = 1;display_GB2312_string(act[2]->x[2], (act[2]->y[2] - 1) * 8 + 1, act[2]->str[2], act[2]->inverse[2]);
        act[2]->inverse[3] = 0;display_GB2312_string(act[2]->x[3], (act[2]->y[3] - 1) * 8 + 1, act[2]->str[3], act[2]->inverse[3]);
        ui_state = 201;
    }
    else if (key_DOWN_flag)
    {
        act[2]->inverse[3] = 0; display_GB2312_string(act[2]->x[3], (act[2]->y[3] - 1) * 8 + 1, act[2]->str[3], act[2]->inverse[3]);
        act[2]->inverse[4] = 1; display_GB2312_string(act[2]->x[4], (act[2]->y[4] - 1) * 8 + 1, act[2]->str[4], act[2]->inverse[4]);
        ui_state = 204;
    }
    else if (key_INCREASE_flag)
    {
        in_de_f(1, act[2]->str[3]);
        display_GB2312_string(act[2]->x[3], (act[2]->y[3] - 1) * 8 + 1, act[2]->str[3], act[2]->inverse[3]);
        ui_state = 203;
		if (mode1_freq/100==9)
			mode1_freq-=900;
		else
			mode1_freq+=100;
    }
    else if (key_DECREASE_flag)
    {
        in_de_f(2, act[2]->str[3]);
        display_GB2312_string(act[2]->x[3], (act[2]->y[3] - 1) * 8 + 1, act[2]->str[3], act[2]->inverse[3]);
        ui_state = 203;
		if (mode1_freq/100==0)
			mode1_freq+=900;
		else
			mode1_freq-=100;
    }
    
    if (key_ENTER_flag || key_UP_flag || key_DOWN_flag || key_INCREASE_flag || key_DECREASE_flag)
    {
        key_ENTER_flag = 0; key_UP_flag = 0; key_DOWN_flag = 0; key_INCREASE_flag = 0; key_DECREASE_flag = 0;
    }
}

void ui_proc204(void)                           //change 4position
{
    if (key_UP_flag)
    {
        act[2]->inverse[4] = 0; display_GB2312_string(act[2]->x[4], (act[2]->y[4] - 1) * 8 + 1, act[2]->str[4], act[2]->inverse[4]);
        act[2]->inverse[3] = 1; display_GB2312_string(act[2]->x[3], (act[2]->y[3] - 1) * 8 + 1, act[2]->str[3], act[2]->inverse[3]);
        ui_state = 203;
		
    }
    else if (key_DOWN_flag)
    {
        act[2]->inverse[4] = 0; display_GB2312_string(act[2]->x[4], (act[2]->y[4] - 1) * 8 + 1, act[2]->str[4], act[2]->inverse[4]);
        act[2]->inverse[5] = 1; display_GB2312_string(act[2]->x[5], (act[2]->y[5] - 1) * 8 + 1, act[2]->str[5], act[2]->inverse[5]);
        ui_state = 205;
    }
    else if (key_INCREASE_flag)
    {
        in_de_f(1, act[2]->str[4]);		   ///////////////////////////////////////////
        display_GB2312_string(act[2]->x[4], (act[2]->y[4] - 1) * 8 + 1, act[2]->str[4], act[2]->inverse[4]);
        ui_state = 204;
		if (((mode1_freq/10)%10)==9)
			mode1_freq-=90;
		else
			mode1_freq+=10;
    }
    else if (key_DECREASE_flag)
    {
        in_de_f(2, act[2]->str[4]);
        display_GB2312_string(act[2]->x[4], (act[2]->y[4] - 1) * 8 + 1, act[2]->str[4], act[2]->inverse[4]);
        ui_state = 204;
		if (((mode1_freq/10)%10)==0)
			mode1_freq+=90;
		else
			mode1_freq-=10;

    }
    
    if (key_ENTER_flag || key_UP_flag || key_DOWN_flag || key_INCREASE_flag || key_DECREASE_flag)
    {
        key_ENTER_flag = 0; key_UP_flag = 0; key_DOWN_flag = 0; key_INCREASE_flag = 0; key_DECREASE_flag = 0;
    }
}

void ui_proc205(void)
{
    if (key_UP_flag)
    {
        act[2]->inverse[5] = 0; display_GB2312_string(act[2]->x[5], (act[2]->y[5] - 1) * 8 + 1, act[2]->str[5], act[2]->inverse[5]);
        act[2]->inverse[4] = 1; display_GB2312_string(act[2]->x[4], (act[2]->y[4] - 1) * 8 + 1, act[2]->str[4], act[2]->inverse[4]);
        ui_state = 204;
    }
    else if (key_DOWN_flag)
    {
        act[2]->inverse[5] = 0; display_GB2312_string(act[2]->x[5], (act[2]->y[5] - 1) * 8 + 1, act[2]->str[5], act[2]->inverse[5]);
        act[2]->inverse[7] = 1; display_GB2312_string(act[2]->x[7], (act[2]->y[7] - 1) * 8 + 1, act[2]->str[7], act[2]->inverse[7]);
        ui_state = 208;
    }
    else if (key_INCREASE_flag)
    {
        in_de_f(1, act[2]->str[5]);
        display_GB2312_string(act[2]->x[5], (act[2]->y[5] - 1) * 8 + 1, act[2]->str[5], act[2]->inverse[5]);
        ui_state = 205;
		if ((mode1_freq%10)==9)
			mode1_freq-=9;
		else
			mode1_freq+=1;
    }
    else if (key_DECREASE_flag)
    {
        in_de_f(2, act[2]->str[5]);
        display_GB2312_string(act[2]->x[5], (act[2]->y[5] - 1) * 8 + 1, act[2]->str[5], act[2]->inverse[5]);
        ui_state = 205;
		if ((mode1_freq%10)==0)
			mode1_freq+=9;
		else
			mode1_freq-=1;
    }
    
    if (key_ENTER_flag || key_UP_flag || key_DOWN_flag || key_INCREASE_flag || key_DECREASE_flag)
    {
        key_ENTER_flag = 0; key_UP_flag = 0; key_DOWN_flag = 0; key_INCREASE_flag = 0; key_DECREASE_flag = 0;
    }
}

void ui_proc208(void)                           //change 3position
{
    if (key_UP_flag)
    {
        act[2]->inverse[5] = 1;display_GB2312_string(act[2]->x[5], (act[2]->y[5] - 1) * 8 + 1, act[2]->str[5], act[2]->inverse[5]);
        act[2]->inverse[7] = 0;display_GB2312_string(act[2]->x[7], (act[2]->y[7] - 1) * 8 + 1, act[2]->str[7], act[2]->inverse[7]);
        ui_state = 205;
    }
    else if (key_DOWN_flag)
    {
        act[2]->inverse[7] = 0; display_GB2312_string(act[2]->x[7], (act[2]->y[7] - 1) * 8 + 1, act[2]->str[7], act[2]->inverse[7]);
        act[2]->inverse[8] = 1; display_GB2312_string(act[2]->x[8], (act[2]->y[8] - 1) * 8 + 1, act[2]->str[8], act[2]->inverse[8]);
        ui_state = 209;
    }
    else if (key_INCREASE_flag)
    {
        in_de_f(1, act[2]->str[7]);
        display_GB2312_string(act[2]->x[7], (act[2]->y[7] - 1) * 8 + 1, act[2]->str[7], act[2]->inverse[7]);
        ui_state = 208;
		if (mode1_freq_b/100==9)
			mode1_freq_b-=900;
		else
			mode1_freq_b+=100;
    }
    else if (key_DECREASE_flag)
    {
        in_de_f(2, act[2]->str[7]);
        display_GB2312_string(act[2]->x[7], (act[2]->y[7] - 1) * 8 + 1, act[2]->str[7], act[2]->inverse[7]);
        ui_state = 208;
		if (mode1_freq_b/100==0)
			mode1_freq_b+=900;
		else
			mode1_freq_b-=100;
    }
    
    if (key_ENTER_flag || key_UP_flag || key_DOWN_flag || key_INCREASE_flag || key_DECREASE_flag)
    {
        key_ENTER_flag = 0; key_UP_flag = 0; key_DOWN_flag = 0; key_INCREASE_flag = 0; key_DECREASE_flag = 0;
    }
}

void ui_proc209(void)                           //change 4position
{
    if (key_UP_flag)
    {
        act[2]->inverse[8] = 0; display_GB2312_string(act[2]->x[8], (act[2]->y[8] - 1) * 8 + 1, act[2]->str[8], act[2]->inverse[8]);
        act[2]->inverse[7] = 1; display_GB2312_string(act[2]->x[7], (act[2]->y[7] - 1) * 8 + 1, act[2]->str[7], act[2]->inverse[7]);
        ui_state = 208;
		
    }
    else if (key_DOWN_flag)
    {
        act[2]->inverse[8] = 0; display_GB2312_string(act[2]->x[8], (act[2]->y[8] - 1) * 8 + 1, act[2]->str[8], act[2]->inverse[8]);
        act[2]->inverse[9] = 1; display_GB2312_string(act[2]->x[9], (act[2]->y[9] - 1) * 8 + 1, act[2]->str[9], act[2]->inverse[9]);
        ui_state = 210;
    }
    else if (key_INCREASE_flag)
    {
        in_de_f(1, act[2]->str[8]);		   ///////////////////////////////////////////
        display_GB2312_string(act[2]->x[8], (act[2]->y[8] - 1) * 8 + 1, act[2]->str[8], act[2]->inverse[8]);
        ui_state = 209;
		if (((mode1_freq_b/10)%10)==9)
			mode1_freq_b-=90;
		else
			mode1_freq_b+=10;
    }
    else if (key_DECREASE_flag)
    {
        in_de_f(2, act[2]->str[8]);
        display_GB2312_string(act[2]->x[8], (act[2]->y[8] - 1) * 8 + 1, act[2]->str[8], act[2]->inverse[8]);
        ui_state = 209;
		if (((mode1_freq_b/10)%10)==0)
			mode1_freq_b+=90;
		else
			mode1_freq_b-=10;

    }
    
    if (key_ENTER_flag || key_UP_flag || key_DOWN_flag || key_INCREASE_flag || key_DECREASE_flag)
    {
        key_ENTER_flag = 0; key_UP_flag = 0; key_DOWN_flag = 0; key_INCREASE_flag = 0; key_DECREASE_flag = 0;
    }
}

void ui_proc210(void)
{
    if (key_UP_flag)
    {
        act[2]->inverse[9] = 0; display_GB2312_string(act[2]->x[9], (act[2]->y[9] - 1) * 8 + 1, act[2]->str[9], act[2]->inverse[9]);
        act[2]->inverse[8] = 1; display_GB2312_string(act[2]->x[8], (act[2]->y[8] - 1) * 8 + 1, act[2]->str[8], act[2]->inverse[8]);
        ui_state = 209;
    }
    else if (key_DOWN_flag)
    {
        act[2]->inverse[9] = 0; display_GB2312_string(act[2]->x[9], (act[2]->y[9] - 1) * 8 + 1, act[2]->str[9], act[2]->inverse[9]);
        act[2]->inverse[6] = 1; display_GB2312_string(act[2]->x[6], (act[2]->y[6] - 1) * 8 + 1, act[2]->str[6], act[2]->inverse[6]);
        ui_state = 207;
    }
    else if (key_INCREASE_flag)
    {
        in_de_f(1, act[2]->str[9]);
        display_GB2312_string(act[2]->x[9], (act[2]->y[9] - 1) * 8 + 1, act[2]->str[9], act[2]->inverse[9]);
        ui_state = 210;
		if ((mode1_freq_b%10)==9)
			mode1_freq_b-=9;
		else
			mode1_freq_b+=1;
    }
    else if (key_DECREASE_flag)
    {
        in_de_f(2, act[2]->str[9]);
        display_GB2312_string(act[2]->x[9], (act[2]->y[9] - 1) * 8 + 1, act[2]->str[9], act[2]->inverse[9]);
        ui_state = 210;
		if ((mode1_freq_b%10)==0)
			mode1_freq_b+=9;
		else
			mode1_freq_b-=1;
    }
    
    if (key_ENTER_flag || key_UP_flag || key_DOWN_flag || key_INCREASE_flag || key_DECREASE_flag)
    {
        key_ENTER_flag = 0; key_UP_flag = 0; key_DOWN_flag = 0; key_INCREASE_flag = 0; key_DECREASE_flag = 0;
    }
}


void ui_proc404(void)
{
    if(key_UP_flag)
    {
        act[4]->inverse[4]=0; display_GB2312_string(act[4]->x[4],(act[4]->y[4]-1)*8+1,act[4]->str[4],act[4]->inverse[4]);
        act[4]->inverse[9]=1; display_GB2312_string(act[4]->x[9],(act[4]->y[9]-1)*8+1,act[4]->str[9],act[4]->inverse[9]);
        ui_state=409;
    }
    else if (key_DOWN_flag)
    {
        act[4]->inverse[4]=0; display_GB2312_string(act[4]->x[4],(act[4]->y[4]-1)*8+1,act[4]->str[4],act[4]->inverse[4]);
        act[4]->inverse[5]=1; display_GB2312_string(act[4]->x[5],(act[4]->y[5]-1)*8+1,act[4]->str[5],act[4]->inverse[5]);
        ui_state=405;
    }
    else if (key_ENTER_flag)
    {
        act[4]->inverse[4]=0;
        act[5]->inverse[5]=1;
        act[5]->str[6]=act[4]->str[6];act[5]->str[7]=act[4]->str[7];act[5]->str[8]=act[4]->str[8];
        mode2_freq = atoifreq(act[4]->str[6],act[4]->str[7],act[4]->str[8]);
        mode2_phasediff = degree_counter;
        act[5]->str[9]=act[4]->str[9];
        mode=2;modechangeflag=2;
        display_ui_act(5);
        ui_state=505;
    }
    
    if(key_ENTER_flag||key_UP_flag||key_DOWN_flag||key_INCREASE_flag||key_DECREASE_flag)
    {
        key_ENTER_flag=0;key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;
    }
}


void ui_proc405(void)
{
    if(key_UP_flag)
    {
        act[4]->inverse[5]=0; display_GB2312_string(act[4]->x[5],(act[4]->y[5]-1)*8+1,act[4]->str[5],act[4]->inverse[5]);
        act[4]->inverse[4]=1; display_GB2312_string(act[4]->x[4],(act[4]->y[4]-1)*8+1,act[4]->str[4],act[4]->inverse[4]);
        ui_state=404;
    }
    else if (key_DOWN_flag)
    {
        act[4]->inverse[5]=0; display_GB2312_string(act[4]->x[5],(act[4]->y[5]-1)*8+1,act[4]->str[5],act[4]->inverse[5]);
        act[4]->inverse[6]=1; display_GB2312_string(act[4]->x[6],(act[4]->y[6]-1)*8+1,act[4]->str[6],act[4]->inverse[6]);
        ui_state=406;
    }
    else if (key_ENTER_flag)
    {
        act[4]->inverse[5]=0;
        act[1]->inverse[2]=1; display_ui_act(1);
        itoafreq(mode2_freq,a4_s6,a4_s7,a4_s8);
        itodegree(mode2_phasediff,&a4_s9);
        mode=0;
        ui_state=102;
    }
    
    if(key_ENTER_flag||key_UP_flag||key_DOWN_flag||key_INCREASE_flag||key_DECREASE_flag)
    {
        key_ENTER_flag=0;key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;
    }
}


void ui_proc406(void)
{
    if(key_UP_flag)
    {
        act[4]->inverse[6]=0; display_GB2312_string(act[4]->x[6],(act[4]->y[6]-1)*8+1,act[4]->str[6],act[4]->inverse[6]);
        act[4]->inverse[5]=1; display_GB2312_string(act[4]->x[5],(act[4]->y[5]-1)*8+1,act[4]->str[5],act[4]->inverse[5]);
        ui_state=405;
    }
    else if (key_DOWN_flag)
    {
        act[4]->inverse[6]=0; display_GB2312_string(act[4]->x[6],(act[4]->y[6]-1)*8+1,act[4]->str[6],act[4]->inverse[6]);
        act[4]->inverse[7]=1; display_GB2312_string(act[4]->x[7],(act[4]->y[7]-1)*8+1,act[4]->str[7],act[4]->inverse[7]);
        ui_state=407;
    }
    else if (key_INCREASE_flag)
    {
        in_de(1,act[4]->str[6]);
        display_GB2312_string(act[4]->x[6],(act[4]->y[6]-1)*8+1,act[4]->str[6],act[4]->inverse[6]);
        ui_state=406;
		if (mode2_freq/100==9)
			mode2_freq-=900;
		else
			mode2_freq+=100;
    }
    else if (key_DECREASE_flag)
    {
        in_de(2,act[4]->str[6]);
        display_GB2312_string(act[4]->x[6],(act[4]->y[6]-1)*8+1,act[4]->str[6],act[4]->inverse[6]);
        ui_state=406;
		if (mode2_freq/100==0)
			mode2_freq+=900;
		else
			mode2_freq-=100;
		
    }
    
    if(key_ENTER_flag||key_UP_flag||key_DOWN_flag||key_INCREASE_flag||key_DECREASE_flag)
    {
        key_ENTER_flag=0;key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;
    }
}

void ui_proc407(void)
{
    if(key_UP_flag)
    {
        act[4]->inverse[7]=0; display_GB2312_string(act[4]->x[7],(act[4]->y[7]-1)*8+1,act[4]->str[7],act[4]->inverse[7]);
        act[4]->inverse[6]=1; display_GB2312_string(act[4]->x[6],(act[4]->y[6]-1)*8+1,act[4]->str[6],act[4]->inverse[6]);
        ui_state=406;
    }
    else if (key_DOWN_flag)
    {
        act[4]->inverse[7]=0; display_GB2312_string(act[4]->x[7],(act[4]->y[7]-1)*8+1,act[4]->str[7],act[4]->inverse[7]);
        act[4]->inverse[8]=1; display_GB2312_string(act[4]->x[8],(act[4]->y[8]-1)*8+1,act[4]->str[8],act[4]->inverse[8]);
        ui_state=408;
    }
    else if (key_INCREASE_flag)
    {
        in_de(1,act[4]->str[7]);
        display_GB2312_string(act[4]->x[7],(act[4]->y[7]-1)*8+1,act[4]->str[7],act[4]->inverse[7]);
        ui_state=407;
		if (((mode2_freq/10)%10)==9)
			mode2_freq-=90;
		else
			mode2_freq+=10;
    }
    else if (key_DECREASE_flag)
    {
        in_de(2,act[4]->str[7]);
        display_GB2312_string(act[4]->x[7],(act[4]->y[7]-1)*8+1,act[4]->str[7],act[4]->inverse[7]);
        ui_state=407;
		if (((mode2_freq/10)%10)==0)
			mode2_freq+=90;
		else
			mode2_freq-=10;
    }
    
    if(key_ENTER_flag||key_UP_flag||key_DOWN_flag||key_INCREASE_flag||key_DECREASE_flag)
    {
        key_ENTER_flag=0;key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;
    }
}


void ui_proc408(void)
{
    if(key_UP_flag)
    {
        act[4]->inverse[8]=0; display_GB2312_string(act[4]->x[8],(act[4]->y[8]-1)*8+1,act[4]->str[8],act[4]->inverse[8]);
        act[4]->inverse[7]=1; display_GB2312_string(act[4]->x[7],(act[4]->y[7]-1)*8+1,act[4]->str[7],act[4]->inverse[7]);
        ui_state=407;
    }
    else if (key_DOWN_flag)
    {
        act[4]->inverse[8]=0; display_GB2312_string(act[4]->x[8],(act[4]->y[8]-1)*8+1,act[4]->str[8],act[4]->inverse[8]);
        act[4]->inverse[9]=1; display_GB2312_string(act[4]->x[9],(act[4]->y[9]-1)*8+1,act[4]->str[9],act[4]->inverse[9]);
        ui_state=409;
    }
    else if (key_INCREASE_flag)
    {
        in_de(1,act[4]->str[8]);
        display_GB2312_string(act[4]->x[8],(act[4]->y[8]-1)*8+1,act[4]->str[8],act[4]->inverse[8]);
        ui_state=408;
		if ((mode2_freq%10)==9)
			mode2_freq-=9;
		else
			mode2_freq+=1;
    }
    else if (key_DECREASE_flag)
    {
        in_de(2,act[4]->str[8]);
        display_GB2312_string(act[4]->x[8],(act[4]->y[8]-1)*8+1,act[4]->str[8],act[4]->inverse[8]);
        ui_state=408;
		if ((mode2_freq%10)==0)
			mode2_freq+=9;
		else
			mode2_freq-=1;
    }
    
    if(key_ENTER_flag||key_UP_flag||key_DOWN_flag||key_INCREASE_flag||key_DECREASE_flag)
    {
        key_ENTER_flag=0;key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;
    }
}

void ui_proc701(void)                           //wait for return
{
    if(key_UP_flag)
    {
        act[7]->inverse[2]=0; display_GB2312_string(act[7]->x[2],(act[2]->y[2]-1)*8+1,act[7]->str[2],act[7]->inverse[2]);
        act[7]->inverse[3]=1; display_GB2312_string(act[7]->x[3],(act[2]->y[3]-1)*8+1,act[7]->str[3],act[7]->inverse[3]);
        ui_state=703;
    }
    else if (key_DOWN_flag)
    {
        act[7]->inverse[2]=0; display_GB2312_string(act[7]->x[2],(act[2]->y[2]-1)*8+1,act[7]->str[2],act[7]->inverse[2]);
        act[7]->inverse[6]=1; display_GB2312_string(act[7]->x[6],(act[2]->y[6]-1)*8+1,act[7]->str[6],act[7]->inverse[6]);
        ui_state=707;
    }
    else if (key_ENTER_flag)
    {
        act[7]->inverse[2]=0;
        act[1]->inverse[1]=1; display_ui_act(1);
        mode=0;
        ui_state=101;
    }
    
    if(key_ENTER_flag||key_UP_flag||key_DOWN_flag||key_INCREASE_flag||key_DECREASE_flag)
    {
        key_ENTER_flag=0;key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;
    }
}


void ui_proc707(void)                           //going to work
{
    if(key_UP_flag)
    {
        act[7]->inverse[2]=1; display_GB2312_string(act[7]->x[2],(act[7]->y[2]-1)*8+1,act[7]->str[2],act[7]->inverse[2]);
        act[7]->inverse[6]=0; display_GB2312_string(act[7]->x[6],(act[7]->y[6]-1)*8+1,act[7]->str[6],act[7]->inverse[6]);
        ui_state=701;
    }
    else if(key_DOWN_flag)
    {
        act[7]->inverse[5]=1; display_GB2312_string(act[7]->x[5],(act[7]->y[5]-1)*8+1,act[7]->str[5],act[7]->inverse[5]);
        act[7]->inverse[6]=0; display_GB2312_string(act[7]->x[6],(act[7]->y[6]-1)*8+1,act[7]->str[6],act[7]->inverse[6]);
        ui_state=705;
    }
    else if(key_ENTER_flag)
    {
        act[7]->str[6] = "退出";
        act[7]->inverse[6] = 1; display_GB2312_string(act[7]->x[6], (act[7]->y[6] - 1) * 8 + 1, act[7]->str[6], act[7]->inverse[6]);
        ui_state = 706;
    }
    
    if (key_ENTER_flag || key_UP_flag || key_DOWN_flag || key_INCREASE_flag || key_DECREASE_flag)
    {
        key_ENTER_flag = 0; key_UP_flag = 0; key_DOWN_flag = 0; key_INCREASE_flag = 0; key_DECREASE_flag = 0;
    }
}

void ui_proc706(void)                       //from work to stop
{   unsigned char a = *(act[7]->str[3]);
    unsigned char b = *(act[7]->str[4]);
    unsigned char c = *(act[7]->str[5]);
    //mode1_freq = (a - '0') * 100 + (b - '0') * 10 + (c - '0');
   // mode3_freq = atoifreq(a,b,c);
		itoafreq(mode3_freq,a7_s3,a7_s4,a7_s5);
    //setup_AD9850(mode3_freq, mode3_freq, phase3_0, phase3_90);
    
    if (key_ENTER_flag)
    {
        act[7]->str[6] = "运行";
        act[7]->inverse[6] = 1; display_GB2312_string(act[7]->x[6], (act[7]->y[6] - 1) * 8 + 1, act[7]->str[6], act[7]->inverse[6]);
        ui_state = 707;
        /*act[7]->inverse[6] = 1;
        display_ui_act(7);
        ui_state = 707;
        act[7]->str[6] = "运行";*/
    }
    
    if (key_ENTER_flag || key_UP_flag || key_DOWN_flag || key_INCREASE_flag || key_DECREASE_flag)
    {
        key_ENTER_flag = 0; key_UP_flag = 0; key_DOWN_flag = 0; key_INCREASE_flag = 0; key_DECREASE_flag = 0;
    }
}

void ui_proc703(void)                           //change 3position
{
    if (key_UP_flag)
    {
        act[7]->inverse[4] = 1;display_GB2312_string(act[7]->x[4], (act[7]->y[4] - 1) * 8 + 1, act[7]->str[4], act[7]->inverse[4]);
        act[7]->inverse[3] = 0;display_GB2312_string(act[7]->x[3], (act[7]->y[3] - 1) * 8 + 1, act[7]->str[3], act[7]->inverse[3]);
        ui_state = 704;
    }
    else if (key_DOWN_flag)
    {
        act[7]->inverse[3] = 0; display_GB2312_string(act[7]->x[3], (act[7]->y[3] - 1) * 8 + 1, act[7]->str[3], act[7]->inverse[3]);
        act[7]->inverse[2] = 1; display_GB2312_string(act[7]->x[2], (act[7]->y[2] - 1) * 8 + 1, act[7]->str[2], act[7]->inverse[2]);
        ui_state = 701;
    }
    else if (key_INCREASE_flag)
    {
        in_de_f(1, act[7]->str[3]);
        display_GB2312_string(act[7]->x[3], (act[7]->y[3] - 1) * 8 + 1, act[7]->str[3], act[7]->inverse[3]);
        ui_state = 703;
		if (mode3_freq/100==9)
			mode3_freq-=900;
		else
			mode1_freq+=100;
    }
    else if (key_DECREASE_flag)
    {
        in_de_f(2, act[7]->str[3]);
        display_GB2312_string(act[7]->x[3], (act[7]->y[3] - 1) * 8 + 1, act[7]->str[3], act[7]->inverse[3]);
        ui_state = 703;
		if (mode3_freq/100==0)
			mode3_freq+=900;
		else
			mode1_freq-=100;
    }
    
    if (key_ENTER_flag || key_UP_flag || key_DOWN_flag || key_INCREASE_flag || key_DECREASE_flag)
    {
        key_ENTER_flag = 0; key_UP_flag = 0; key_DOWN_flag = 0; key_INCREASE_flag = 0; key_DECREASE_flag = 0;
    }
}

void ui_proc704(void)                           //change 4position
{
    if (key_UP_flag)
    {
        act[7]->inverse[4] = 0; display_GB2312_string(act[7]->x[4], (act[7]->y[4] - 1) * 8 + 1, act[7]->str[4], act[7]->inverse[4]);
        act[7]->inverse[5] = 1; display_GB2312_string(act[7]->x[5], (act[7]->y[5] - 1) * 8 + 1, act[7]->str[5], act[7]->inverse[5]);
        ui_state = 705;
    }
    else if (key_DOWN_flag)
    {
        act[7]->inverse[4] = 0; display_GB2312_string(act[7]->x[4], (act[7]->y[4] - 1) * 8 + 1, act[7]->str[4], act[7]->inverse[4]);
        act[7]->inverse[3] = 1; display_GB2312_string(act[7]->x[3], (act[7]->y[3] - 1) * 8 + 1, act[7]->str[3], act[7]->inverse[3]);
        ui_state = 703;
    }
    else if (key_INCREASE_flag)
    {
        in_de_f(1, act[7]->str[4]);		   ///////////////////////////////////////////
        display_GB2312_string(act[7]->x[4], (act[7]->y[4] - 1) * 8 + 1, act[7]->str[4], act[7]->inverse[4]);
        ui_state = 704;
		if (((mode3_freq/10)%10)==9)
			mode3_freq-=90;
		else
			mode3_freq+=10;
    }
    else if (key_DECREASE_flag)
    {
        in_de_f(2, act[7]->str[4]);
        display_GB2312_string(act[7]->x[4], (act[7]->y[4] - 1) * 8 + 1, act[7]->str[4], act[7]->inverse[4]);
        ui_state = 704;
		if (((mode3_freq/10)%10)==0)
			mode3_freq+=90;
		else
			mode3_freq-=10;
    }
    
    if (key_ENTER_flag || key_UP_flag || key_DOWN_flag || key_INCREASE_flag || key_DECREASE_flag)
    {
        key_ENTER_flag = 0; key_UP_flag = 0; key_DOWN_flag = 0; key_INCREASE_flag = 0; key_DECREASE_flag = 0;
    }
}

void ui_proc705(void)
{
    if (key_UP_flag)
    {
        act[7]->inverse[5] = 0; display_GB2312_string(act[7]->x[5], (act[7]->y[5] - 1) * 8 + 1, act[7]->str[5], act[7]->inverse[5]);
        act[7]->inverse[6] = 1; display_GB2312_string(act[7]->x[6], (act[7]->y[6] - 1) * 8 + 1, act[7]->str[6], act[7]->inverse[6]);
        ui_state = 707;
    }
    else if (key_DOWN_flag)
    {
        act[7]->inverse[5] = 0; display_GB2312_string(act[7]->x[5], (act[7]->y[5] - 1) * 8 + 1, act[7]->str[5], act[7]->inverse[5]);
        act[7]->inverse[4] = 1; display_GB2312_string(act[7]->x[4], (act[7]->y[4] - 1) * 8 + 1, act[7]->str[4], act[7]->inverse[4]);
        ui_state = 704;
    }
    else if (key_INCREASE_flag)
    {
        in_de_f(1, act[7]->str[5]);
        display_GB2312_string(act[7]->x[5], (act[7]->y[5] - 1) * 8 + 1, act[7]->str[5], act[7]->inverse[5]);
        ui_state = 705;
		if ((mode3_freq%10)==9)
			mode3_freq-=9;
		else
			mode3_freq+=1;
    }
    else if (key_DECREASE_flag)
    {
        in_de_f(2, act[7]->str[5]);
        display_GB2312_string(act[7]->x[5], (act[7]->y[5] - 1) * 8 + 1, act[7]->str[5], act[7]->inverse[5]);
        ui_state = 705;
		if ((mode3_freq%10)==0)
			mode3_freq+=9;
		else
			mode3_freq-=1;
    }
    
    if (key_ENTER_flag || key_UP_flag || key_DOWN_flag || key_INCREASE_flag || key_DECREASE_flag)
    {
        key_ENTER_flag = 0; key_UP_flag = 0; key_DOWN_flag = 0; key_INCREASE_flag = 0; key_DECREASE_flag = 0;
    }
}



void in_de_degree(int degree_counter)
{
    switch (degree_counter)
    {
        case 0: act[4]->str[9]= stringdegree_00;break;
        case 1: act[4]->str[9]= stringdegree_01;break;
        case 2: act[4]->str[9]= stringdegree_02;break;
        case 3: act[4]->str[9]= stringdegree_03;break;
        case 4: act[4]->str[9]= stringdegree_04;break;
        case 5: act[4]->str[9]= stringdegree_05;break;
        case 6: act[4]->str[9]= stringdegree_06;break;
        case 7: act[4]->str[9]= stringdegree_07;break;
        case 8: act[4]->str[9]= stringdegree_08;break;
            
        default: break;
    }
}


void ui_proc409(void)
{
    if(key_UP_flag)
    {
        act[4]->inverse[9]=0; display_GB2312_string(act[4]->x[9],(act[4]->y[9]-1)*8+1,act[4]->str[9],act[4]->inverse[9]);
        act[4]->inverse[8]=1; display_GB2312_string(act[4]->x[8],(act[4]->y[8]-1)*8+1,act[4]->str[8],act[4]->inverse[8]);
        ui_state=408;
    }
    else if (key_DOWN_flag)
    {
        act[4]->inverse[9]=0; display_GB2312_string(act[4]->x[9],(act[4]->y[9]-1)*8+1,act[4]->str[9],act[4]->inverse[9]);
        act[4]->inverse[4]=1; display_GB2312_string(act[4]->x[4],(act[4]->y[4]-1)*8+1,act[4]->str[4],act[4]->inverse[4]);
        ui_state=404;
    }
    else if (key_INCREASE_flag)
    {
        degree_counter++;
        if (degree_counter>8) degree_counter=0;
        in_de_degree(degree_counter);
        display_GB2312_string(act[4]->x[9],(act[4]->y[9]-1)*8+1,act[4]->str[9],act[4]->inverse[9]);
        ui_state=409;
		if (mode2_phasediff==8)
			mode2_phasediff=0;
		else
			mode2_phasediff+=1;
    }
    else if (key_DECREASE_flag)
    {
        degree_counter--;
        if (degree_counter<0) degree_counter=8;
        in_de_degree(degree_counter);
        display_GB2312_string(act[4]->x[9],(act[4]->y[9]-1)*8+1,act[4]->str[9],act[4]->inverse[9]);
        ui_state=409;
		if (mode2_phasediff==0)
			mode2_phasediff=8;
		else
			mode2_phasediff-=1;
    }
    
    if(key_ENTER_flag||key_UP_flag||key_DOWN_flag||key_INCREASE_flag||key_DECREASE_flag)
    {
        key_ENTER_flag=0;key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;
    }
}


void ui_proc505(void)
{
    if (key_ENTER_flag)
    {
        act[5]->inverse[5]=0;
        act[1]->inverse[2]=1; display_ui_act(1);
        mode=0;
        ui_state=102;
    }
    
    if(key_ENTER_flag||key_UP_flag||key_DOWN_flag||key_INCREASE_flag||key_DECREASE_flag)
    {
        key_ENTER_flag=0;key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;
    }
}


void ui_state_proc(unsigned int ui_state)
{
    switch (ui_state)
    {
        case 0: ui_proc0(); break;
        case 101: ui_proc101();break;
        case 102: ui_proc102();break;
        case 103: ui_proc103();break;
        case 201: ui_proc201();break;
        case 203: ui_proc203();break;
        case 204: ui_proc204();break;
        case 205: ui_proc205();break;
        case 206: ui_proc206();break;
        case 207: ui_proc207();break;
        case 208: ui_proc208();break;
        case 209: ui_proc209();break;
        case 210: ui_proc210();break;
        case 404: ui_proc404();break;
        case 405: ui_proc405();break;
        case 406: ui_proc406();break;
        case 407: ui_proc407();break;
        case 408: ui_proc408();break;
        case 409: ui_proc409();break;
        case 505: ui_proc505();break;
        case 701: ui_proc701();break;
        case 703: ui_proc703();break;
        case 704: ui_proc704();break;
        case 705: ui_proc705();break;
        case 706: ui_proc706();break;
        case 707: ui_proc707();break;
            
        default: break;
    }
}
