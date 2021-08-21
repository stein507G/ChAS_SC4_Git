/*
 * util.c
 *
 *  Created on: 2015. 4. 8.
 *      Author: chanii
 */
#include "config.h"
//#include "protocol_Macro.h"
//#include "protocol_CMD_Oven.h"
//#include "protocolType_oven.h"
//#include "protocolType_HPLC.h"

//#include "Ch_oven_type.h"


extern volatile uint8_t TimeEvent;
extern uint32_t System_ticks;
uint32_t sysTicks = 0;

//extern Oven_State_t	ovenStt;	// 통신프로토콜에 없는 것

void usr_delay(unsigned int time)
{
	uint32_t i;
	for(i=0;i<time;i++)
	{
		__asm volatile ("nop");
	}
}

// Do not use this function as possible -> performance decrease
void delay_ms(unsigned int time)
{
	uint32_t i;
	for(i=0;i<time;i++)
	{
		usr_delay(2000);
	}
}

unsigned char hex2char_conv(unsigned char ch)
{
  unsigned char ret;
  unsigned char va; // clear High Nibble

  va = ch&0x0F;

  if((va>=0x0)&&(va<=0x9)) ret=(0x30 + va);
  else if(va==0xa) ret=0x41;
  else if(va==0xb) ret=0x42;
  else if(va==0xc) ret=0x43;
  else if(va==0xd) ret=0x44;
  else if(va==0xe) ret=0x45;
  else if(va==0xf) ret=0x46;

  return ret;
}

unsigned char char2hex_conv(unsigned char ch)
{
  unsigned char ret=0;

  if((ch>=0x30)&&(ch<=0x39)) ret=ch&0xF;
  else if(ch==0x41) ret=0x0A;
  else if(ch==0x42) ret=0x0B;
  else if(ch==0x43) ret=0x0C;
  else if(ch==0x44) ret=0x0D;
  else if(ch==0x45) ret=0x0E;
  else if(ch==0x46) ret=0x0F;
  else if(ch==0x61) ret=0x0A;
  else if(ch==0x62) ret=0x0B;
  else if(ch==0x63) ret=0x0C;
  else if(ch==0x64) ret=0x0D;
  else if(ch==0x65) ret=0x0E;
  else if(ch==0x66) ret=0x0F;

  return ret;
}



void PrintLine(void)
{
  printf("-----------------------------------------------------\r\n");
}

// data should have 32 Byte area
void ShowMemory(unsigned char Header, unsigned char * data, unsigned int Length)
{
	unsigned int i;

    if(Header==1) // Display HEX for Length with header
    {
        printf("----------------- Display Memory --------------------\r\n");
        printf("NUM : 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F \r\n");
        PrintLine();

       	for(i=0;i<Length;i++)
        {
            if((i%0x10)==0) printf("%03X : ",(i>>4));
            printf("%02X,",data[i]);
            if((i%0x10)==0xF) printf("\r\n");
        }
       	printf("\r\n");PrintLine();printf("\r\n");
    }
    else if(Header==2) // Display ASCII for Length
    {
        printf("\r\n");
       	for(i=0;i<Length;i++)
        {
            if((i%0x10)==0) printf("%03X : ",(i>>4));
            printf("%02X,",data[i]);
            if((i%0x10)==0xF) printf("\r\n");
        }
        printf("\r\n");
    }
    else
    {
		for(i=0;i<Length;i++)
		{
			printf("%02X,",data[i]);
			if((i%0x10)==0xF) printf("\r\n");
		}
		printf("\r\n");
    }

}


// data should have 32 Byte area
void ShowMemoryWord(uint32_t * data, uint8_t Len)
{
	uint8_t i;
//	dp("Memory contents ....\r\n");
	for(i=0;i<Len;i++)
//	for(i=12;i<Len;i++)
	{
		if((i&0x3)==0) dp("%03X : ",(i>>2));
		dp("%08lX,",data[i]);
		if((i&0x3)==1){}
		if((i&0x3)==2){}
		if((i&0x3)==3) dp("\r\n");
	}
	dp("\r\n");
}

/*
참고로 테스트를 위해 보드의 용도를 Master 와 Slave 로 구분할 필요가 있는데,
0 번지의  FPGA 버전 값을 통해 구분하였다. 
즉 버전 값과 0x1000000 은 논리 곱하여 0 값을 가지면  Master 0 이 아니면 Slave 로 
구분하도록 펌웨어가 설계되어 있다. 
*/
uint8_t PrintFabricVersion(void)
{
	uint32_t ver;
//	ver = FPGA_READ_WORD(0);
	ver = FPGA_READ_WORD(R0_FPGA_VER);

		printf("PrintFabricVersion()\r\n");

	if((ver&0x11000000) == 0x11000000) {
		printf("Fabric Ver : %lX (Rs422 Slave)\r\n", ver);
//		return 3;
		return 1;
	}
	else if(ver&0x01000000)
	{
		printf("Fabric Ver : %lX (Slave)\r\n", ver);
		return 1;
	}
	else
	{
		printf("Fabric Ver : %lX (Master)\r\n", ver);
		return 0;
	}
}

uint8_t Intro(void)
{
	uint8_t MasterSlave;
	
	iprintf("\n=================================================\r\n");
	iprintf("            Youngin Chromass Co., Ltd.\r\n");	
//	dp0("               Application (eNVM) \r\n");
	iprintf("               Chrozen LC Autosampler [YI9650]\r\n");
	iprintf("                        Main Function\r\n");
	iprintf("-------------------------------------------------\r\n");
  iprintf("                     ......... Program Start !!  \r\n");
	iprintf("-------------------------------------------------\r\n");	
//    printf("Build   Time : %s %s\r\n",__DATE__,__TIME__);
  iprintf("          Build Time : ["__DATE__"] ["__TIME__"]\n");		
	iprintf("-------------------------------------------------\r\n");
	iprintf("                               Ver : [" FIRMWARE_VERSION "] \r\n");
	iprintf("=================================================\r\n\n");	
	
	MasterSlave = PrintFabricVersion();
	return MasterSlave;
}



// ryuSF
void get_System_ticks()
{
	sysTicks = System_ticks;
}

void init_System_ticks()
{
	System_ticks = 0;
#if ryuSF	
	timer100ms = 0;
#endif
}

void init_ticks(uint32_t *ticks)
{
	sysTicks = System_ticks;
	*ticks = sysTicks;
}

uint32_t get_Systicks()
{
//	sysTicks = System_ticks;
	return System_ticks;
}

#define RTC_MATCH_INITIAL_VALUE        (uint64_t)0x0100		// 1초 
// RTCCLK --> Crystal Prescaler(RTCCLK / 128) --> 40bit Counter
void init_RTC()
{
#if ryuSF
//uint64_t rtc_match_value = RTC_MATCH_INITIAL_VALUE;
	
  MSS_RTC_init();
  MSS_RTC_configure( MSS_RTC_NO_COUNTER_RESET );
//	MSS_RTC_configure( MSS_RTC_RESET_COUNTER_ON_MATCH );	// 카운트가 0으로 된다.
//  MSS_RTC_set_rtc_count( (uint64_t)0 );
//	MSS_RTC_set_rtc_match(rtc_match_value);
  MSS_RTC_start();
//  MSS_RTC_enable_irq();  /* Enable RTC match interrupt. */
//	set_RtcTime(2016,11,2,18,30,10);
	get_RtcTime();

dp("y=%d M=%d D=%d H=%d M=%d S=%d \n",sysInform.cSysDate.year,sysInform.cSysDate.month,sysInform.cSysDate.date,sysInform.cSysTime.hour,sysInform.cSysTime.minute,sysInform.cSysTime.second);
//dp("tmr=%d %x\n",tmr,tmr);
#endif
}

/*
#define FIRST_YEAR			2001u
#define LAST_YEAR				2099u
#define SECOND_YEAR			86400u
*/
void set_RtcTime(uint32_t year, uint32_t month, uint32_t date, uint32_t hour, uint32_t min, uint32_t sec)
{
#if ryuSF
	unsigned int num_of_days_in_non_leap_year[]={31,28,31,30,31,30,31,31,30,31,30,31};
	unsigned int num_of_days_in_leap_year[]={31,29,31,30,31,30,31,31,30,31,30,31};
	unsigned long time_in_sec =0;
	int isLeapFlag = 0;
	uint32_t noOfLeapYears;
	uint32_t tm_day = 0;

  if( (month>=1)&&(month<=12) && 	((date>= 1)&&(date <=31)) && ((year>= FIRST_YEAR)&&(year<=LAST_YEAR)))
  {
		isLeapFlag = isLeapYear(year);
		tm_day = (year - FIRST_YEAR);// * 365 + noOfLeapYears;		

//		if( isLeapFlag ) 	noOfLeapYears = tm_day/4;
//		else 							noOfLeapYears = tm_day/4 + 1;

		noOfLeapYears = tm_day/4;
		tm_day = tm_day * 365 + noOfLeapYears;
		
		for( int ii = 0; ii < (month -1); ii++ )		{
			if( isLeapFlag ) {
				tm_day += num_of_days_in_leap_year[ii];
			}
			else {
				tm_day += num_of_days_in_non_leap_year[ii];
			}
		}
		tm_day = tm_day + (date - 1);
		time_in_sec = tm_day * 86400u + hour*3600u + min*60u + sec;

dp("time_in_sec -- RTC Secs Set %d %d %d %d %d %d %d\n", tm_day,year,month,hour,min,sec,time_in_sec);
		MSS_RTC_set_seconds_count(time_in_sec);

  }
#endif	
}

void get_RtcTime()
{
#if ryuSF
	uint32_t ii;
	uint64_t tmr;
  unsigned int num_of_days_in_year[]={31,28,31,30,31,30,31,31,30,31,30,31};
	uint32_t  noOfYears, noOfMonths, noOfDays,noOfHours, noOfMins, noOfSecs, residue;
	int isLeapFlag = 0;
	uint32_t year;

	tmr =	(uint32_t)MSS_RTC_get_seconds_count();
//dp("tmr=%d %x\n",tmr,tmr);
	/* Calculating the Number of Days from the Seconds Value */
	noOfDays = (tmr / 86400);
	residue = (tmr % 86400);

	/* Calculating the Number of Hours */
	noOfHours = (residue / 3600);
	noOfSecs = (residue % 3600);

	/* Calculating the Number of Minutes */
	noOfMins = (noOfSecs / 60);
	noOfSecs = (noOfSecs % 60);

	noOfYears = noOfDays /365 ;
	noOfDays = ((noOfDays % 365) - (noOfYears / 4));

	isLeapFlag = isLeapYear(noOfYears + FIRST_YEAR);
	
	if (isLeapFlag)	{
		num_of_days_in_year[1] += 1;
	}
	/*Months */
	for (ii= 0; ii < 12; ii++)
	{
    if (noOfDays >= num_of_days_in_year[ii]) {
    	noOfDays -= num_of_days_in_year[ii];
    }
    else {
    	break;
    }
	}

	sysInform.cSysDate.year = noOfYears + 1;
	noOfYears += FIRST_YEAR;
	noOfDays++;
	noOfMonths = ++ii;
	sysInform.cSysDate.month = noOfMonths;
	sysInform.cSysDate.date = noOfDays;

	sysInform.cSysTime.hour = noOfHours;
	sysInform.cSysTime.minute = noOfMins;
	sysInform.cSysTime.second = noOfSecs;	
	
	tmr =    ( ((noOfYears) << 26 ) |
	           ((noOfMonths) << 22) |
	           ((noOfDays) << 17) |
	           ((noOfHours) << 12) |
	           ((noOfMins) << 6) |
	           ((noOfSecs)) );

//dp("y=%d M=%d D=%d H=%d M=%d S=%d \n",noOfYears,noOfMonths,noOfDays,noOfHours,noOfMins,noOfSecs);
//dp("tmr=%d %x\n",tmr,tmr);

	return tmr;
#endif
}





