/*
 * config.h
 *
 *  Created on: 2015. 4. 7.
 *      Author: chanii
 */

#ifndef CONFIG_H_
#define CONFIG_H_

/**************************************************************************/
/* Standard	Includes */
/**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <limits.h>


#include "option_AS.h"		// #include "option_Oven.h"

#include "Chrozen_AS_hw_platform.h"

/***************************** CMSIS includes ************************/
#include "system_m2sxxx.h"
#include "sysconfig.h"
#include "interrupt.h"

/***************************** Driver includes **********************/
//#include "mss_gpio.h"
#include "mss_uart.h"
//#include "mss_rtc.h"
#include "mss_watchdog.h"
#include "mss_timer.h"
#include "mss_nvm.h"
//#include "mss_spi.h"
#include "mss_ethernet_mac.h"


#include "lwip/tcp.h"


#include "queue.h"	//(+)210715 CJKIM, added


//====================================================================
// ryu
#include "def_com.h"
//#include "debug_inform.h"

#include "define_AS.h"		// #include "def_Oven.h"
//#include "Def_Motor.h"

#include "Def_stepCon.h"
#include "protocolType_AS.h"
#include "comType_AS.h"
#include "protocol_AS.h"

#include "fpga_ChAS.h"

#include "PID_Control_AS.h"

#include "rs422_if.h"


//#include "hplc_type.h"


#include "util.h"


#include "debug_inform.h"

#endif /* CONFIG_H_ */
