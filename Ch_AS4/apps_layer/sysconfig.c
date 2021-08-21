/*
 * sysconfig.c
 *
 *  Created on: 2015. 4. 14.
 *      Author: chanii
 */
#include "sys_config_mss_clocks.h"
#include "config.h"

#include "EEPROM.h"
//#include "protocolType_oven.h"
//#include "protocolType_HPLC.h"

//#include "Ch_oven_type.h"
//#include "fpga_ChAS.h"
#include "conf_eth.h"

//#include "Motor_Ctrl.h"

#include "Led_task.h"


#define WDOG_SYSREG_CR_ENABLE_MASK      0x00000001u


extern void uart_tx_handler(mss_uart_instance_t * this_uart);
extern void mss_uart1_rx_handler(mss_uart_instance_t * this_uart);

#define PWM_PRESCALE    32		// 35Mhz / 254 / 32 --> 4.3K
#define PWM_PERIOD      254 // Full값을 255로 정하기 위해서  // 256	

extern ETHERNET_CONFIG_t ethernetConfig;
//extern Oven_State_t	ovenStt;	// 통신프로토콜에 없는 것

extern SYSTEM_INFORM_t sysInform;

//extern pwm_instance_t the_pwm1;

//extern char ethAddr[6];
extern LCAS_DEV_SET_t devSet;


#define ETH_ADDR_LEN 6
#define IP_ADDR_LEN 4

// ryu
unsigned char my_ip[IP_ADDR_LEN] =	{
	ETHERNET_CONF_IPADDR0, 
	ETHERNET_CONF_IPADDR1,
	ETHERNET_CONF_IPADDR2,
	ETHERNET_CONF_IPADDR3
};
unsigned char my_mac[ETH_ADDR_LEN] = {
	ETHERNET_CONF_ETHADDR0,
	ETHERNET_CONF_ETHADDR1,
	ETHERNET_CONF_ETHADDR2,
	ETHERNET_CONF_ETHADDR3,
	ETHERNET_CONF_ETHADDR4,
//	ETHERNET_CONF_ETHADDR4
	ETHERNET_CONF_ETHADDR5
};
unsigned char my_netMask[IP_ADDR_LEN]	= { 
	ETHERNET_CONF_NET_MASK0,
	ETHERNET_CONF_NET_MASK1,
	ETHERNET_CONF_NET_MASK2,
	ETHERNET_CONF_NET_MASK3
};
unsigned char my_gateway[IP_ADDR_LEN] = { 
	ETHERNET_CONF_GATEWAY_ADDR0,
	ETHERNET_CONF_GATEWAY_ADDR1,
	ETHERNET_CONF_GATEWAY_ADDR2,
	ETHERNET_CONF_GATEWAY_ADDR3
};




void cpu_init(void)
{
	uint32_t timer1_load_value;

	  /* Turn off the watchdog */
	SYSREG->WDOG_CR = 0;
	
//=============================================================================================
// Configure UART0 for Debug.
//---------------------------------------------
// Newlib_stubs.c     
//#define MICROSEMI_STDIO_BAUD_RATE  MSS_UART_460800_BAUD     
//---------------------------------------------
// Register the function that will be called on UART0 receive interrupts.
// 인터럽트를 사용하고자 할 때는 다음 심볼을 지워야한다.
// Property-->Setting-->Symbols--> MICROSEMI_STDIO_THRU_MMUART0 //ACTEL_STDIO_THRU_UART  
// MICROSEMI_STDIO_THRU_MMUART0
//---------------------------------------------
#if 1
  MSS_UART_init(&g_mss_uart0,
//  							MSS_UART_460800_BAUD,
  							MSS_UART_115200_BAUD,
                MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

//	MSS_UART_set_rx_handler(&g_mss_uart0, mss_uart0_rx_handler,MSS_UART_FIFO_SINGLE_BYTE /* MSS_UART_FIFO_FOUR_BYTES */);
//=========================================================================
#endif

//=============================================================================================
// Configure UART1
//---------------------------------------------
#if 0
	MSS_UART_init(&g_mss_uart1,
								MSS_UART_460800_BAUD, // MSS_UART_230400_BAUD , // //MSS_UART_115200_BAUD,
                MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);
	MSS_UART_set_rx_handler(&g_mss_uart1, mss_uart1_rx_handler,MSS_UART_FIFO_SINGLE_BYTE /* MSS_UART_FIFO_FOUR_BYTES */);

//	MSS_UART_enable_irq(&g_mss_uart1,(MSS_UART_RBF_IRQ));
//	MSS_UART_set_tx_handler(&g_mss_uart1, uart_tx_handler);
//	MSS_UART_set_tx_handler(gp_my_uart, uart_tx_handler);	
//=========================================================================
#endif

//=============================================================================================
// Configure Timer1.
//---------------------------------------------
	timer1_load_value = MSS_SYS_APB_0_CLK_FREQ / 1000;	// 1ms Timer Value
	MSS_TIM1_init( MSS_TIMER_PERIODIC_MODE );
	MSS_TIM1_load_immediate( timer1_load_value );
	MSS_TIM1_start();
	MSS_TIM1_enable_irq();
//=========================================================================

  SYSREG->WDOG_CR = WDOG_SYSREG_CR_ENABLE_MASK;
  /* Watchdog initialization. */
  MSS_WD_init();
  MSS_WD_enable_timeout_irq();

//	NVIC_SetPriority(EthernetMAC_IRQn, 5u);
//	NVIC_SetPriority(EthernetMAC_IRQn, 0u);	// RYU_TCP			
//	NVIC_SetPriority(EthernetMAC_IRQn, 1u);	// RYU_TCP			

}

#if 0
void cpu_init_Main(void)
{

	NVIC_SetPriority(EthernetMAC_IRQn, 5u);
//	NVIC_SetPriority(EthernetMAC_IRQn, 0u);	// RYU_TCP			

//=============================================================================================
// Configure I2C for RTC(DS1308)
//---------------------------------------------
	//MSS_I2C_init( I2C_MASTER, MASTER_SER_ADDR, MSS_I2C_PCLK_DIV_160 );
//=========================================================================

//=============================================================================================
// Configure SPI for FRAM,  etc
//---------------------------------------------
//	spi_sd_init_Main();
//=========================================================================
}
#endif

void InterruptEnable_Main(void)
{

// 우선순위 5 : 패킷을 놓치는 경우가 생긴다.  - printf 문을 제거하니 증상 사라짐. - orignal
// 우선순위 1 : 패킷 안 놓친다.
// 우선순위01 : RYU_TCP EEPROM 저장시 문제가 발생(멈춤)

// 적당한 우선순위를 찾아야 
// The default priority is 0 for every interrupt. 
// This is the highest possible priority.
	NVIC_SetPriority(EthernetMAC_IRQn, 5u);		// 패킷을 놓치는 경우가 생긴다.  - printf 문을 제거하니 증상 사라짐.
//	XXX NVIC_SetPriority(EthernetMAC_IRQn, 0u);	// RYU_TCP EEPROM 저장시 문제가 발생(멈춤)
//	NVIC_SetPriority(EthernetMAC_IRQn, 1u);	// RYU_TCP

// ADC Auto Sampling interrupt enable ----------
    NVIC_EnableIRQ( FabricIrq1_IRQn );
    NVIC_ClearPendingIRQ( FabricIrq1_IRQn );
// START_IN : remote
    NVIC_EnableIRQ( FabricIrq2_IRQn );
    NVIC_ClearPendingIRQ( FabricIrq2_IRQn );

// RS422 Rx intr
//	NVIC_EnableIRQ( FabricIrq0_IRQn );
//	NVIC_ClearPendingIRQ( FabricIrq0_IRQn );

// (Master) rs422 Auto Completion interrupt enable ----------
//	NVIC_EnableIRQ( FabricIrq3_IRQn );
//	NVIC_ClearPendingIRQ( FabricIrq3_IRQn );

#if 0
    NVIC_EnableIRQ( FabricIrq4_IRQn );
    NVIC_ClearPendingIRQ( FabricIrq4_IRQn );

    NVIC_EnableIRQ( FabricIrq5_IRQn );
    NVIC_ClearPendingIRQ( FabricIrq5_IRQn );

// Key In
    NVIC_EnableIRQ( FabricIrq6_IRQn );
    NVIC_ClearPendingIRQ( FabricIrq6_IRQn );

// RTC
    NVIC_EnableIRQ( FabricIrq7_IRQn );
    NVIC_ClearPendingIRQ( FabricIrq7_IRQn );

    NVIC_EnableIRQ( FabricIrq8_IRQn );
    NVIC_ClearPendingIRQ( FabricIrq8_IRQn );
#endif		
}

#define POWERDOWN_SET_VALUE		1
#define SLEEP_SET_VALUE				0

#define MOTOR_POWERDOWN_MODE_SET_SAM1	(1 << 0)
#define MOTOR_POWERDOWN_MODE_SET_SAM2	(1 << 1)
#define MOTOR_POWERDOWN_MODE_SET_M3	(1 << 2)
#define MOTOR_POWERDOWN_MODE_SET_M4	(1 << 3)
#define MOTOR_POWERDOWN_MODE_SET_M5	(1 << 4)
#define MOTOR_POWERDOWN_MODE_SET_M6	(1 << 5)

#define MOTOR_SLEEP_MODE_SET_SAM1	(0 << 0)
#define MOTOR_SLEEP_MODE_SET_SAM2	(0 << 1)
#define MOTOR_SLEEP_MODE_SET_M3	(0 << 2)
#define MOTOR_SLEEP_MODE_SET_M4	(0 << 3)
#define MOTOR_SLEEP_MODE_SET_M5	(0 << 4)
#define MOTOR_SLEEP_MODE_SET_M6	(0 << 5)

void init_Control_port() // init_CoreGpio();
{
	unsigned int motorPowerModeData = 0x0;

#if 0		// CoreGpio사용 
// 현재는 FPGA에서 in/out을 고정으로 사용하고 있다.
// GPIO_init()를 사용하면 초기화 설정이 된다.
//GPIO_init( &g_gpio,	COREGPIO_BASE_ADDR, GPIO_APB_32_BITS_BUS );  <== 포트 초기화 됨 
 		g_gpio.base_addr = COREGPIO_APBBIF_0_BASE_ADDR;
    g_gpio.apb_bus_width = GPIO_APB_32_BITS_BUS;
#endif

// 주의 bit 순서 
// [27:22] ==> [Inj:Val:Syr:X:Y:Z] 
#if MOTOR_POWER_DOWN_MODE
//		Core_gpio_pattern = 0x0000bf00; 	
// 20200217
//		Core_gpio_pattern = 0x0e40bf00; 	// [Inj:Val:Syr:-:-:Z] 
		motorPowerModeData = (motorPowerModeData
												| MOTOR_POWERDOWN_MODE_SET_SAM1 
												| MOTOR_POWERDOWN_MODE_SET_SAM2 
												| MOTOR_POWERDOWN_MODE_SET_M3
												| MOTOR_POWERDOWN_MODE_SET_M4
												| MOTOR_POWERDOWN_MODE_SET_M5
												| MOTOR_POWERDOWN_MODE_SET_M6);							
#else
//		Core_gpio_pattern = 0x0000bf00;
		motorPowerModeData = (motorPowerModeData
												| MOTOR_SLEEP_MODE_SET_SAM1 
												| MOTOR_SLEEP_MODE_SET_SAM2 
												| MOTOR_SLEEP_MODE_SET_M3
												| MOTOR_SLEEP_MODE_SET_M4
												| MOTOR_SLEEP_MODE_SET_M5
												| MOTOR_SLEEP_MODE_SET_M6);												
#endif

		FPGA_WRITE_WORD(W4_MOTOR_POW_CON, motorPowerModeData);
//		*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern);

//readyOnOff
//remoteAction(
		REMOTE_READY_OUT_CONTACT_OPEN;

		REMOTE_START_OUT_CONTACT_OPEN;

// POWER BTN 설정 
		CPU_PWR_CON_HIGH;
		FPGA_WRITE_WORD(W2_POWER_BTN_TIME, (ST_ON_BTN_DET_TIME_DEFAULT | (ST_OFF_SEQUENCE_TIME_DEFAULT << 16)));	// 버튼 누름 시간 , 버튼 인지 후 Off 시간
//		FPGA_WRITE_WORD(W2_POWER_BTN_TIME, ST_ON_BTN_DET_TIME_DEFAULT | (300 << 16));	// 버튼 누름 시간 , 버튼 인지 후 Off 시간 300 x 10msec
// LED 설정
		InitialLED();
		
		BUZZER_CON_OFF;

//BUZZER_CON_ON;

EXT_LED_CON_OFF;


/*		
 		g_gpio1.base_addr = COREGPIO_APBBIF_1_BASE_ADDR;
    g_gpio1.apb_bus_width = GPIO_APB_32_BITS_BUS;
*/

// ===================================================================================================
// Example
// ===================================================================================================
#if 0
	gpio_in = HAL_get_32bit_reg( this_gpio->base_addr, GPIO_IN );
	(*((uint32_t volatile *)(COREGPIO_APBBIF_0_BASE_ADDR + GPIO_OUT_REG_OFFSET)) = (Core_gpio_pattern & 0xfffffffe));	    
	Core_gpioInput = (*((uint32_t volatile *)(COREGPIO_APBBIF_0_IN_REG)));
#endif

}

void init_system()
{
  /*gcsvc.MasterSlave = */	
	Intro();

	init_Control_port(); // init_CoreGpio();
//  init_RTC();

	static uint8_t msen_switch[5];
	static uint8_t rotory_switch[5];
	
	PCF8574_ROTARY_init();
// 한 바이트만 읽으면 되지만 처음에 에러발생한 경우가..  
	I2C_PCF8574_read(PCF8574_ROTARY_SLAVE_ADDRESS, rotory_switch, 3);

// 로터리 스위치 번호가 잘못 되어서 
	devSet.IP_switch = 7 - ( (rotory_switch[2] & 0x02) << 1 | (rotory_switch[2] & 0x04) >> 1 | (rotory_switch[2] & 0x08)>> 3);
	dp("devSet.IP_switch = %x\n\r",devSet.IP_switch);

//	PCF8574_TRAY_SEN_init();
//	I2C_PCF8574_read(PCF8574_TRAY_SEN_SLAVE_ADDRESS, msen_switch, sizeof(msen_switch));
	I2C_PCF8574_read(PCF8574_TRAY_SEN_SLAVE_ADDRESS, msen_switch, 3);

	devSet.msen_switch = (msen_switch[2] & 0x3f);
	dp("devSet.msen_switch = %x\n\r",devSet.msen_switch);





#if 0
//	uint8_t rtc_data;
//	uint8_t sec,min,hour;
	uint8_t rtc_time[10];
		
//	RTC_BQ32002DR_init();

	rtcRead_byte(RTC_SECONDS_REG_ADDR,&rtc_data);
	sec = rtc_data & 0x0f;
	sec += (rtc_data & 0x7f >> 4) * 10;
	dfp("RTC_SECONDS_REG_ADDR = %x\n\r",rtc_data);	

//	RTC_BQ32002DR_init();
	rtcRead_byte(RTC_MINUTES_REG_ADDR,&rtc_data);
	min = rtc_data;	
	dfp("RTC_MINUTES_REG_ADDR = %x\n\r",rtc_data);	

//	RTC_BQ32002DR_init();
	rtcRead_byte(RTC_CENT_HOURS_REG_ADDR,&rtc_data);
	dfp("RTC_CENT_HOURS_REG_ADDR = %x\n\r",rtc_data);	
	hour = 
//	RTC_BQ32002DR_init();
	rtcRead_byte(RTC_YEARS_REG_ADDR,&rtc_data);
	dfp("RTC_YEARS_REG_ADDR = %x\n\r",rtc_data);	

	dfp("RTC_YEARS_REG_ADDR = %x\n\r",rtc_data);	

	rtcRead_time(RTC_SECONDS_REG_ADDR,rtc_time);
	for(int i=0 ; i<10; i++) {
		dfp("rtc_time[%d] = %x\n\r",i, rtc_time[i]);	
	}
#endif

initSystemInform();

	rtcTime_Macaddr_Init();

//	EEPROM_init();			
}

void Init_rtcTime()
{
#if 0
	uint8_t rtc_time[10];

	rtc_time[0] = RTC_SECONDS_REG_ADDR;		// add
	sysInform.cSysTime.second =	rtc_time[1] = 0x30;	// sec
	sysInform.cSysTime.minute = rtc_time[2] = 0x15;	// min
	sysInform.cSysTime.hour = rtc_time[3] = 0x15;	// hour
	rtc_time[4] = 0x04;	// day => 1 : Sunday
	sysInform.cSysDate.date = rtc_time[5] = 0x16;	// date
	sysInform.cSysDate.month = rtc_time[6] = 0x02;	// month
	sysInform.cSysDate.year = rtc_time[7] = 0x21;	// year

	rtcWrite_time(rtc_time);
#endif

/*
uint8_t r[6];
int n,i;

n  = 100; 
srand(n);
for(i=0;i<6;i++){
	r[i] = rand();
	iprintf("rand = %d %x \n",n,r[i]);
}

n = 101;
srand(n);
for(i=0;i<6;i++){
	r[i] = rand();
	iprintf("rand = %d %x \n",n,r[i]);
}

n = 100;
srand(n);
for(i=0;i<6;i++){
	r[i] = rand();
	iprintf("rand = %d %x \n",n,r[i]);
}

n = 100;
srand(n);
for(i=0;i<6;i++){
	r[i] = rand();
	iprintf("rand = %d %x \n",n,r[i]);
}
*/
}

int rtcTime_Macaddr_Init()
{
	unsigned char saveFlag;
	uint8_t rtc_time[10];
	
	eepromRead_Data((RTC_INIT_SAVE_ADDR), 1, &(saveFlag));
	if(saveFlag != DATA_SAVE_FLAG) {
		rtc_time[0] = RTC_SECONDS_REG_ADDR;		// add
sysInform.cSysTime.second =	rtc_time[1] = 0x11;	// sec
sysInform.cSysTime.minute = rtc_time[2] = 0x11;	// min
sysInform.cSysTime.hour = rtc_time[3] = 0x11;	// hour
		rtc_time[4] = 0x04;	// day
sysInform.cSysDate.date = rtc_time[5] = 0x11;	// date
sysInform.cSysDate.month = rtc_time[6] = 0x11;	// month
sysInform.cSysDate.year = rtc_time[7] = 0x20;	// year
		
		rtcWrite_time(rtc_time);

//		eepromRead_Data((RTC_INIT_SAVE_ADDR), 1, &(saveFlag));
		saveFlag = DATA_SAVE_FLAG;
		eepromWrite_Data((RTC_INIT_SAVE_ADDR), 1, &(saveFlag));
dfp("rtcWrite_time\n");	
		return RE_ERR;
	}
	else {
		rtcRead_time(RTC_SECONDS_REG_ADDR,rtc_time);

		sysInform.cSysTime.second =	rtc_time[0];
		sysInform.cSysTime.minute = rtc_time[1];
		sysInform.cSysTime.hour = rtc_time[2];
				rtc_time[3];
		sysInform.cSysDate.date = rtc_time[4];
		sysInform.cSysDate.month = rtc_time[5];
		sysInform.cSysDate.year = rtc_time[6];
		
		int i;
		printf("rtc_time = "); for(i=0 ; i<7; i++) {	printf("%x : ",rtc_time[i]);	} printf("\n");	

#if 0
		eepromRead_Data((MAC_ADDRESS_INIT_SAVE_ADDR), 1, &(saveFlag));
		if(saveFlag != DATA_SAVE_FLAG) {	// 맥어드레스가 초기화 안되어 .
			make_MACADDR(MAKE_MAC);
dfp("make_MACADDR(MAKE_MAC)\n");
			return RE_OK;
		}
		else {
			make_MACADDR(LOAD_MAC);
dfp("make_MACADDR(LOAD_MAC)\n");
			return RE_OK;
		}
#endif		
	}

}

// 처음으로 제어기가 연결되면 mac address를 생성한다.
void make_MACADDR(int NewMAC)
{
	unsigned char macAddr[7];
	unsigned int time;
//	unsigned char time;
	char tmpMac[5];
	int i;

	eepromRead_Data(MAC_ADDRESS_ADDR_SAVE,MAC_ADDR_READ_LEN,&macAddr);

	if(NewMAC == MAKE_MAC) { 
		
// 난수 생성에 필요한 time
/*
		time = (unsigned int)(sysInform.cSysDate.year << 26) + (unsigned int)(sysInform.cSysDate.month << 22) + (unsigned int)(sysInform.cSysDate.date << 17) 
				+ (unsigned int)(sysInform.cSysTime.hour << 12) + (unsigned int)(sysInform.cSysTime.minute << 6) + (unsigned int)(sysInform.cSysTime.second);
dp("time=%d\n",time);

		time = (unsigned int)(sysInform.cSysTime.minute << 6) + (unsigned int)(sysInform.cSysTime.second);
dp("time=%d\n",time);		
		time = (unsigned int)(sysInform.cSysTime.minute << 6);
dp("time=%d\n",time);
		time = (unsigned int)(sysInform.cSysTime.minute << 6) + (unsigned int)(sysInform.cSysTime.second) + 35;
dp("time=%d\n",time);
*/
		time = (unsigned int)(sysInform.cSysDate.year << 10) + (unsigned int)(sysInform.cSysDate.month << 8) + (unsigned int)(sysInform.cSysDate.date << 6) 
				+ (unsigned int)(sysInform.cSysTime.hour << 4) + (unsigned int)(sysInform.cSysTime.minute << 2) + (unsigned int)(sysInform.cSysTime.second);

dp("time=%d\n",time);
/*
//		if(devState.connected == YES) {	// 처음 연결할 때 초기화 한다. - 기기마다 다른 맥어드레스를 생성하기 위하여 실행한다.
dp("y%x\n",sysInform.cSysDate.year);
dp("m%x\n",sysInform.cSysDate.month);
dp("d%x\n",sysInform.cSysDate.date);
dp("h%x\n",sysInform.cSysTime.hour);
dp("m%x\n",sysInform.cSysTime.minute);
dp("s%d\n",sysInform.cSysTime.second);
*/
			srand(time);
int r[6];			
			for(i=0; i<ETH_ADDR_LEN; i++) {
				r[i] = rand();
				my_mac[i] = r[i] % 256;
dfp("rand = %x %x time=%d\n",r[i], my_mac[i], time);
			}

			my_mac[0] = ETHERNET_CONF_ETHADDR0;
			my_mac[1] = ETHERNET_CONF_ETHADDR1;

dfp("sysInform.cSerialNo = %s \n",sysInform.cSerialNo);
/*
			strncpy(tmpMac, &sysInform.cSerialNo[4],2);	
			tmpMac[2] = '\0';
			my_mac[4] = atoi(tmpMac);
*/
			strncpy(tmpMac, &sysInform.cSerialNo[10],2);	
			tmpMac[2] = '\0';
			my_mac[5] = atoi(tmpMac);
			
iprintf("[New MAC] my_mac[0]=[0x%x] [0x%x] [0x%x] [0x%x] [0x%x] [0x%x] \n\r",
	my_mac[0],my_mac[1],my_mac[2],my_mac[3],my_mac[4],my_mac[5]);

			macAddr[0] = DATA_SAVE_FLAG;	
			macAddr[1] = my_mac[0];
			macAddr[2] = my_mac[1];
			macAddr[3] = my_mac[2];
			macAddr[4] = my_mac[3];
			macAddr[5] = my_mac[4];		
			macAddr[6] = my_mac[5];			
			eepromWrite_Data(MAC_ADDRESS_ADDR_SAVE,MAC_ADDR_READ_LEN,&macAddr);	
	}
	else {	// LOAD_MAC
		if(macAddr[0]	== DATA_SAVE_FLAG) {
	//		eepromRead_Data(MAC_ADDRESS_ADDR_0,ETH_ADDR_LEN,&my_mac);
			my_mac[0] = macAddr[1];
			my_mac[1] = macAddr[2];
			my_mac[2] = macAddr[3];
			my_mac[3] = macAddr[4];
			my_mac[4] = macAddr[5];
			my_mac[5] = macAddr[6];		
iprintf("[loadMac] my_mac[0]=[0x%x] [0x%x] [0x%x] [0x%x] [0x%x] [0x%x] \n\r",
	my_mac[0],my_mac[1],my_mac[2],my_mac[3],my_mac[4],my_mac[5]);

		}
		else if(NewMAC == APPLY_MAC) { 	// 받든값으로 초기화 
		
		}
		else {	// 초기화 
// 디폴트 값이 적용된다.
			my_mac[0] = ETHERNET_CONF_ETHADDR0;
			my_mac[1] = ETHERNET_CONF_ETHADDR1;
			my_mac[2] = ETHERNET_CONF_ETHADDR2;
			my_mac[3] = ETHERNET_CONF_ETHADDR3;
			my_mac[4] = ETHERNET_CONF_ETHADDR4;
			my_mac[5] = ETHERNET_CONF_ETHADDR5 + devSet.IP_switch;
			
dfp("[default MAC] my_mac[0]=[0x%x] [0x%x] [0x%x] [0x%x] [0x%x] [0x%x] \n\r",
	my_mac[0],my_mac[1],my_mac[2],my_mac[3],my_mac[4],my_mac[5]);
		}
	}
}

void saveEthernetConfig(int who)
{
	unsigned char saveFlag = DATA_SAVE_FLAG;
	unsigned char data[21];

	data[0] = DATA_SAVE_FLAG;

	memcpy(&data[1], &my_ip, 4);
	memcpy(&data[5], &my_gateway, 4);
	memcpy(&data[9], &my_netMask, 4);

//	memcpy(&data[1], &(ethernetConfig.cIPAddress), ETHERNET_CONFIG_DATA_READSIZE);
	eepromWrite_Data((ETHERNET_CONFIG_DATA_SAVE), ETHERNET_CONFIG_DATA_READSIZE + 1, &(data[0]));
}

void init_IpAddr(unsigned char sw)
{
	my_ip[0] =	ETHERNET_CONF_IPADDR0;
	my_ip[1] =	ETHERNET_CONF_IPADDR1;
	my_ip[2] =	ETHERNET_CONF_IPADDR2;
	my_ip[3] =	ETHERNET_CONF_IPADDR3 + sw;

// macaddress는 제외 
#if 0
	my_mac[0] = ETHERNET_CONF_ETHADDR0;
	my_mac[1] = ETHERNET_CONF_ETHADDR1;
	my_mac[2] = ETHERNET_CONF_ETHADDR2;		
	my_mac[3] = ETHERNET_CONF_ETHADDR3;
	my_mac[4] = ETHERNET_CONF_ETHADDR4;
	my_mac[5] = ETHERNET_CONF_ETHADDR5;		
#endif

	my_netMask[0] = ETHERNET_CONF_NET_MASK0;
	my_netMask[1] = ETHERNET_CONF_NET_MASK1;
	my_netMask[2] = ETHERNET_CONF_NET_MASK2;
	my_netMask[3] = ETHERNET_CONF_NET_MASK3;

	my_gateway[0] = ETHERNET_CONF_GATEWAY_ADDR0;
	my_gateway[1] = ETHERNET_CONF_GATEWAY_ADDR1;
	my_gateway[2] = ETHERNET_CONF_GATEWAY_ADDR2;
	my_gateway[3] = ETHERNET_CONF_GATEWAY_ADDR3;
}

void loadEthernetConfig(int who)
{
	unsigned char saveFlag;// = DATA_SAVE_FLAG;
	unsigned char data[21];

	make_MACADDR(LOAD_MAC);

	//eepromRead_Data(ETHERNET_CONFIG_DATA_SAVE,ETHERNET_CONFIG_DATA_READSIZE,&data);
	eepromRead_Data(ETHERNET_CONFIG_DATA_SAVE,ETHERNET_CONFIG_DATA_READSIZE,data);

	saveFlag = data[0];

	if(devSet.IP_switch == 7) { // default ip사용  // DEFAULT_IPADDRESS

		init_IpAddr(0);

		saveEthernetConfig(0);
	}
	else {
//	if(who == LOAD_IPADDRESS) { // load ip
//	}
		if(saveFlag	== DATA_SAVE_FLAG) {
//			eepromRead_Data(AS_IP_ADDRESS_ADDR0,IP_ADDR_LEN,&my_ip);
			my_ip[0] = data[1];
			my_ip[1] = data[2];
			my_ip[2] = data[3];
//			my_ip[3] = data[4];
			my_ip[3] = data[4] + devSet.IP_switch;

			my_gateway[0] = data[5];
			my_gateway[1] = data[6];
			my_gateway[2] = data[7];
			my_gateway[3] = data[8];

			my_netMask[0] = data[9];
			my_netMask[1] = data[10];
			my_netMask[2] = data[11];
			my_netMask[3] = data[12];		
	dfp("load === my_ip[0]=[0x%x] [0x%x] [0x%x] [0x%x] \n\r",
					my_ip[0],my_ip[1],my_ip[2],my_ip[3]);
			
		}
		else {	// 초기화 default - rotory sw
			init_IpAddr(devSet.IP_switch);
		}
		
	}

	sprintf(ethernetConfig.cIPAddress,"%d.%d.%d.%d",my_ip[0], my_ip[1], my_ip[2], my_ip[3]);
	dprintf("%s %3d.%3d.%3d.%3d\n",ethernetConfig.cIPAddress, my_ip[0], my_ip[1], my_ip[2], my_ip[3]);

	sprintf(ethernetConfig.cGateway,"%d.%d.%d.%d",my_gateway[0], my_gateway[1], my_gateway[2], my_gateway[3]);
	dprintf("%s %3d.%3d.%3d.%3d\n",ethernetConfig.cGateway, my_gateway[0], my_gateway[1], my_gateway[2], my_gateway[3]);

	sprintf(ethernetConfig.cNetmask,"%d.%d.%d.%d",my_netMask[0], my_netMask[1], my_netMask[2], my_netMask[3]);
	dprintf("%s %3d.%3d.%3d.%3d\n",ethernetConfig.cNetmask, my_netMask[0], my_netMask[1], my_netMask[2], my_netMask[3]);







	strcpy(ethernetConfig.cFirmwareVersion, FIRMWARE_VERSION);
	strcpy(ethernetConfig.cMachineVersion, MACHINE_VER);
	
}

#if 0
void initSystemInform()
{
	unsigned char saveFlag;
	
//	strcpy(sysInform.cModel, "YL9152 LC AutoSampler");
//	strcpy(sysInform.cVersion, "1.0.0");
	strcpy(sysInform.cModel, SYSTEM_MODEL);
	strcpy(sysInform.cVersion, SYSTEM_VERSION);

	eepromRead_Data(SYSTEM_INFORM_DATA_SAVE, 1 /*sizeof(saveFlag)*/, &saveFlag);
	if(saveFlag == DATA_SAVE_FLAG) {
//		eepromRead_Data((SYSTEM_INFORM_DATA_ADDR), 37 /*sizeof(sysInform)*/, &(sysInform.cVersion/*cSerialNo*/));
// serialNo , InstallDate 만 읽는다.
		eepromRead_Data((SYSTEM_INFORM_DATA_ADDR), SYSTEM_INFORM_DATA_READSIZE, &(sysInform.cSerialNo));
dfp("sysInform.cInstallDate.year = %d - %d - %d---- \n\r",sysInform.cInstallDate.year,sysInform.cInstallDate.month,sysInform.cInstallDate.date);

//strcpy(sysInform.cIPAddress, "10.10.10.30");
//strcpy(sysInform.cPortNo, "4242");
strcpy(sysInform.cIPAddress, CETHERNET_CONF_IPADDR);
strcpy(sysInform.cPortNo, CIP_PORT_NUMBER);

	}
	else {
//		strcpy(sysInform.cSerialNo, "AS2017171112");
		strcpy(sysInform.cSerialNo, "CO2020111111");		

		sysInform.cInstallDate.year = 20;
		sysInform.cInstallDate.month = 11;
		sysInform.cInstallDate.date = 11;
		
//	strcpy(sysInform.cIPAddress, "10.10.10.30");
//		strcpy(sysInform.cPortNo, "4242");
		strcpy(sysInform.cIPAddress, CETHERNET_CONF_IPADDR);
		strcpy(sysInform.cPortNo, CIP_PORT_NUMBER);
dfp("NO DATA---- \n\r");
	}
}

void saveSystemInform(int who)
{
	unsigned char saveFlag = DATA_SAVE_FLAG;
	unsigned char data[21];

	data[0] = DATA_SAVE_FLAG;

	switch(who) {
		case 0:
// serialNo , InstallDate 만 저장
			memcpy(&data[1], &(sysInform.cSerialNo) , SYSTEM_INFORM_DATA_READSIZE);
			eepromWrite_Data((SYSTEM_INFORM_DATA_SAVE), SYSTEM_INFORM_DATA_READSIZE + 1, &(data[0]));
			break;
		case 1:
// serialNo , InstallDate 만 저장
			memcpy(&data[1], &(sysInform.cSerialNo) , SYSTEM_INFORM_DATA_READ_SERIAL);
			eepromWrite_Data((SYSTEM_INFORM_DATA_SAVE), SYSTEM_INFORM_DATA_READ_SERIAL + 1, &(data[0]));
 			break;
		case 2:
			memcpy(&data[1], &(sysInform.cSerialNo) , SYSTEM_INFORM_DATA_READ_SERIAL);
			eepromWrite_Data((SYSTEM_INFORM_DATA_SAVE), SYSTEM_INFORM_DATA_READ_SERIAL + 1, &(data[0]));
 			break;
	}
	
}
#endif

