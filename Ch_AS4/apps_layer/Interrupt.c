/*
 * Interrupt.c
 *
 *  Created on: 2015. 1. 7.
 *      Author: chanii
 */

#include "config.h"

//#include "Ch_oven_type.h"

#include "rs422_if.h"
#include "adc_if_AS.h"


uint32_t LocalTime;
uint32_t System_ticks = 0;
extern void Time_Update(void);

//uint8_t TimeEvent = NO;
volatile int TimeEvent = NO;
//uint8_t Time1S = NO;
//uint16_t Time1S_cnt = 0;
//uint8_t Time100ms = NO;
//uint16_t Time100ms_cnt = 0;

//extern GCDBG_ST gcdbg;
extern RS422_ST rs422if;
extern ADC_ST adcif;


//extern Remote_t remoteif;

//extern Gpio_RemoteIn_t	rmtin;			// Remote in : 현재상태 

//extern uint32_t	gbFabricGpioInt;
//extern uint32_t	remoteStartInCnt;
extern uint32_t	gbFabricRemoteInt;

//extern SYSTEM_CONFIG_t sysConfig;









//.word  Reset_Handler
//.word  NMI_Handler

__attribute__((__interrupt__)) void NMI_Handler( void )
{
	iprintf("NMI_Handler\r\n");
	//Soft_Reset_CPU();
}

__attribute__((__interrupt__)) void HardFault_Handler( void )
{
	uint32_t i;
	iprintf("HardFault_Handler\r\n");
	while(1){
		//Soft_Reset_CPU();
		i=0xffffffff;
		while(i--)
		{
		};
	};
}
__attribute__((__interrupt__)) void MemManage_Handler( void )
{
	iprintf("MemManage_Handler\r\n");
}
__attribute__((__interrupt__)) void BusFault_Handler( void )
{
	iprintf("BusFault_Handler\r\n");
}
__attribute__((__interrupt__)) void UsageFault_Handler( void )
{
	iprintf("UsageFault_Handler\r\n");
}

__attribute__((__interrupt__)) void WdogWakeup_IRQHandler( void )
{
	iprintf("WdogWakeup_IRQHandler\r\n");
	//Soft_Reset_CPU();
}



/*==============================================================================
 * TIM1 interrupt.
 * Do not use printf clause (Antony)
 */
__attribute__((__interrupt__)) void Timer1_IRQHandler( void )
{
	TimeEvent = 1; // 1ms time event flag :
//	adcif.tick1ms = 1;

	LocalTime++ ;// LocalTime += SYSTEMTICK_PERIOD_MS;	//	Time_Update();  Netconf.c에서 이동 
	System_ticks++;

	MSS_TIM1_clear_irq(); /* Clear TIM1 interrupt */
}

/*==============================================================================
 * TIM1 interrupt.
 */
__attribute__((__interrupt__)) void Timer2_IRQHandler( void )
{
    MSS_TIM2_clear_irq(); /* Clear TIM1 interrupt */
}

// APC Read - rxv_intr
__attribute__((__interrupt__)) void FabricIrq0_IRQHandler( void )
{
	NVIC_ClearPendingIRQ( FabricIrq0_IRQn );
#if RS422_ENABLE
	rs422if.rx_intr=1;
#endif

}

// ADC 
 __attribute__((__interrupt__)) void FabricIrq1_IRQHandler( void )
{
	NVIC_ClearPendingIRQ( FabricIrq1_IRQn );
	adcif.intr=1;
}

// Remote Start
__attribute__((__interrupt__)) void FabricIrq2_IRQHandler( void )
{

	gbFabricRemoteInt = 1;

	NVIC_ClearPendingIRQ( FabricIrq2_IRQn );

}

// APC Auto Transmission completion interrupt
__attribute__((__interrupt__)) void FabricIrq3_IRQHandler( void )
{
	NVIC_ClearPendingIRQ( FabricIrq3_IRQn );
#if RS422_ENABLE
#if RS422_MASTER
	rs422if.AutoEnd=1;
#endif
#endif
}

__attribute__((__interrupt__)) void FabricIrq4_IRQHandler( void )
{
	NVIC_ClearPendingIRQ( FabricIrq4_IRQn );

}

__attribute__((__interrupt__)) void FabricIrq5_IRQHandler( void )
{
	NVIC_ClearPendingIRQ( FabricIrq5_IRQn );

}

__attribute__((__interrupt__)) void FabricIrq6_IRQHandler( void )
{
//	mainDevStt.readKey = FPGA_KEY_READ;
//	FPGA_START_ENABLE(0);	// 키입력 불가능

	NVIC_ClearPendingIRQ( FabricIrq6_IRQn );
}

__attribute__((__interrupt__)) void FabricIrq7_IRQHandler( void )
{
	NVIC_ClearPendingIRQ( FabricIrq7_IRQn );

}


__attribute__((__interrupt__)) void FabricIrq8_IRQHandler( void )
{
	NVIC_ClearPendingIRQ( FabricIrq8_IRQn );

}




//__attribute__((__interrupt__)) 
void mss_uart1_rx_handler(mss_uart_instance_t * this_uart)
{
/*	
//      uint8_t rx_buff[MAX_INT_RECEIVE_SIZE];
      uint8_t rx_buff[64];
      uint32_t rx_size  = 0;
      rx_size = MSS_UART_get_rx(this_uart, rx_buff, sizeof(rx_buff));
//      process_rx_data(rx_buff, rx_size);
//dp("mss_uart1_rx_handler = %d\n",rx_size);
dp0("mss_uart1_rx_handler = %d\n");
	//NVIC_ClearPendingIRQ( UART1_IRQn );
	//rd_size = MSS_UART_get_rx( this_uart,  R_Buff, RBUF_SIZE);
*/
#if 0
{
	uint16_t RxCount,rxCnt1,rxCnt2;
	uint8_t rxbuf[MAX_INT_RECEIVE_SIZE];
	uint16_t rxhead;
#if 1
	RxCount = MSS_UART_get_rx(this_uart, rxbuf, /*sizeof(rx_buff)*/ MAX_INT_RECEIVE_SIZE);
//	RxCount = ::read(fd,rxbuf,MAX_INT_RECEIVE_SIZE);

//dprintfn("read=%d",RxCount);

 	rxhead = lcdif.rxhead_int + RxCount;	// 261 = 254 + 7

	if(rxhead > RX_COM_BUFF_SIZE) {
		rxCnt2 = rxhead - RX_COM_BUFF_SIZE;	// 5 = 261 - 256
		rxCnt1 = RxCount - rxCnt2;			// 2 = 7 - 5
		memcpy(&lcdif.rxbuf[lcdif.rxhead_int],&rxbuf,rxCnt1);

		memcpy(&lcdif.rxbuf[0],&rxbuf[rxCnt1],rxCnt2);
	}
	else {
		memcpy(&lcdif.rxbuf[lcdif.rxhead_int],&rxbuf,RxCount);
	}

	lcdif.rxhead_int = rxhead;
	lcdif.rxhead_int &= RX_COM_BUFF_CHECK; //	RX_HEAD_CHECK1;

	lcdif.intr = YES;
//	LCD_PacketParse();
//	emit sigRs232AnalysisData2();
#endif
}
#endif
}

extern volatile size_t g_tx_size;
extern volatile const uint8_t * g_tx_buffer;

__attribute__((__interrupt__)) void uart_tx_handler(mss_uart_instance_t * this_uart)
{
    size_t size_in_fifo;

    if(g_tx_size > 0)
    {
        size_in_fifo = MSS_UART_fill_tx_fifo(this_uart,
                                             (const uint8_t *)g_tx_buffer,
                                             g_tx_size);

        if(size_in_fifo  ==  g_tx_size)
        {
            g_tx_size = 0;
            MSS_UART_disable_irq(this_uart, MSS_UART_TBE_IRQ);
        }
        else
        {
            g_tx_buffer = &g_tx_buffer[size_in_fifo];
            g_tx_size = g_tx_size - size_in_fifo;
        }
    }
    else
    {
        g_tx_size = 0;
        MSS_UART_disable_irq(this_uart, MSS_UART_TBE_IRQ);
    }
}










