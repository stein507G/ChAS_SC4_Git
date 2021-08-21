
#ifndef FPGA_CHAS_H_
#define FPGA_CHAS_H_

//#define FPGA_BASE_ADDR   		(0x50000000U)

//#define FPGA_WRITE_WORD(X,Y)		 *(volatile uint32_t *) (FPGA_BASE_ADDR + ((X)<<2)) = (Y)
#define FPGA_WRITE_WORD(X,Y)		 *(volatile unsigned int *) (FPGA_BASE_ADDR + ((X)<<2)) = (Y)
#define FPGA_READ_WORD(X)				*(volatile uint32_t *) (FPGA_BASE_ADDR + ((X)<<2))

#define CPU_PWR_CON_LOW			FPGA_WRITE_WORD(W0_POWER_BTN, ((0 << POW_CON_5V_IN) | (1 << POW_BTN_EN)))
#define CPU_PWR_CON_HIGH		FPGA_WRITE_WORD(W0_POWER_BTN, ((1 << POW_CON_5V_IN) | (1 << POW_BTN_EN)))

#if NOT_USE_BUZZER
#define BUZZER_CON_OFF			FPGA_WRITE_WORD(W3_BUZZER_CON, 0x0)
#define BUZZER_CON_ON				FPGA_WRITE_WORD(W3_BUZZER_CON, 0x0)
#else
#define BUZZER_CON_OFF			FPGA_WRITE_WORD(W3_BUZZER_CON, 0x0)
#define BUZZER_CON_ON				FPGA_WRITE_WORD(W3_BUZZER_CON, 0x1)
#endif

#define EXT_LED_CON_OFF			FPGA_WRITE_WORD(W2_LED_EXT_CON, 0x0)
#define EXT_LED_CON_ON			FPGA_WRITE_WORD(W2_LED_EXT_CON, 0x1)

// ChrozenLC Oven
#define INTERNAL_FAN_OFF		EXT_LED_CON_OFF
#define INTERNAL_FAN_ON			EXT_LED_CON_ON

#define PELTIER_CON_ON			FPGA_WRITE_WORD(W6_ON_OFF, 0x1)
#define PELTIER_CON_OFF			FPGA_WRITE_WORD(W6_ON_OFF, 0x0)

#define PELTIER_SET_COOLING 	FPGA_WRITE_WORD(W5_HEAT_COOL, 0x1)
#define PELTIER_SET_HEATING		FPGA_WRITE_WORD(W5_HEAT_COOL, 0x0)

#define PELTIER_1_DATA(X)		FPGA_WRITE_WORD(W0_PWM1_OFFSET, (X))
#define PELTIER_2_DATA(X)		FPGA_WRITE_WORD(W1_PWM2_OFFSET, (X))

#define PELTIER_DATA(X)			PELTIER_1_DATA(X); PELTIER_2_DATA(X)

#define HeaterPowerOn(X)		{ PELTIER_DATA(X);	PELTIER_SET_HEATING;	PELTIER_CON_ON; }
#define HeaterPowerOff()		{ PELTIER_DATA(0);	PELTIER_SET_HEATING;	PELTIER_CON_OFF;}

#define CoolerPowerOn(X)		{ PELTIER_DATA(X);	PELTIER_SET_COOLING;	PELTIER_CON_ON;	}
#define CoolerPowerOff()		{ PELTIER_DATA(0);	PELTIER_SET_COOLING;	PELTIER_CON_OFF;}


#define FAN_1_CON_ON			FPGA_WRITE_WORD(W4_FAN_ON_OFF, 0x1)
#define FAN_1_CON_OFF			FPGA_WRITE_WORD(W4_FAN_ON_OFF, 0x0)

#define FAN_1_DATA(X)		FPGA_WRITE_WORD(W2_PWM3_OFFSET, (X))

// 20210407 - PWM Control FAN
// Oven Power(15V)  저항 삽입 - <<AFC1212D-F00>>
// FanConOn(100);	// 25:동작안함.   30:동작(소음) 40:동작(소음) 60:동작(소음)
// 동작 딜레이 잇음 .
#define FanConOn(X)				{ FAN_1_DATA(X); FAN_1_CON_ON; }
#define FanConOff()				{ FAN_1_DATA(0); FAN_1_CON_OFF; }


#define PUMP_CON_ON			FPGA_WRITE_WORD(W7_PUMP_ON_OFF, 0x1)
#define PUMP_CON_OFF		FPGA_WRITE_WORD(W7_PUMP_ON_OFF, 0x0)

#define PUMP_DATA(X)		FPGA_WRITE_WORD(W3_PWM4_OFFSET, (X))

#define PumpConOn(X)				{ PUMP_DATA(X); PUMP_CON_ON; }
//#define PumpConOff()				{ PUMP_DATA(255); PUMP_CON_OFF; }
#define PumpConOff()				{ PUMP_DATA(0); PUMP_CON_OFF; }

#if MAIN_POWRER_15V
//#define INTERNAL_FAN_SPEED_NORMAL		120	// 7V
//#define INTERNAL_FAN_SPEED		170 // 10V
// 저항 삽입
#define INTERNAL_FAN_SPEED_NORMAL		220
#define INTERNAL_FAN_SPEED		255

// 20210407 - PWM Control FAN
// Oven용 Fan(15V) 저항 삽입 - <<AUB0812VH-SP00>>
// 0 에서도 동작함.
#define INTERNAL_FAN_SPEED_NORMAL		20 // 최저속도 0  // 50
#define INTERNAL_FAN_SPEED		150

#else
#define INTERNAL_FAN_SPEED_NORMAL		140	// 255	// 200
#define INTERNAL_FAN_SPEED		220 // 180	// 255	// 200
#endif

//#define IntFanConOn(X)				{ PUMP_DATA(X); PUMP_CON_ON; }
#define IntFanConOn(X)				{ INTERNAL_FAN_ON; PUMP_DATA(X); PUMP_CON_ON; }
//#define PumpConOff()				{ PUMP_DATA(255); PUMP_CON_OFF; }
//#define IntFanConOff()				{ PUMP_DATA(0); PUMP_CON_OFF; }										// Fan이 최저속도로 동작함
#define IntFanConOff()				{ INTERNAL_FAN_OFF; PUMP_DATA(0); PUMP_CON_OFF; }		// Fan을 완전히 off
// ======================================================================================================================
// 수정하시오 
// Go Val
// ryu_Oven
#if 0
#define OVEN_INJPOS_DATA1		// ovenStt.btInjData = (ovenStt.btInjData & 0xdf) | ( (gOVEN_STATE.btValve1<<5) & 0x20)
#define OVEN_INJPOS_DATA2		// ovenStt.btInjData = (ovenStt.btInjData & 0xef) | ( (gOVEN_STATE.btValve2<<4) & 0x10)

#define OVEN_INJPOS_CONT1		OVEN_INJPOS_DATA1;	//OVEN_POW_RDY_INJ_DATA_OUT
#define OVEN_INJPOS_CONT2		OVEN_INJPOS_DATA2;	//OVEN_POW_RDY_INJ_DATA_OUT

#define OVEN_INJPOS_CONTROL		OVEN_INJPOS_DATA1; OVEN_INJPOS_DATA2;	//OVEN_POW_RDY_INJ_DATA_OUT
#else
#define OVEN_INJPOS_DATA1		pfGoSamVal1 = goSam1
#define OVEN_INJPOS_DATA2		pfGoSamVal2 = goSam2

#define OVEN_INJPOS_CONT1		pfGoSamVal1 = goSam1
#define OVEN_INJPOS_CONT2		pfGoSamVal2 = goSam2

//#define OVEN_INJPOS_CONTROL		OVEN_INJPOS_CONT1; OVEN_INJPOS_CONT2;
#endif

#define READY_LED_ON		//LED2_ON				// temp ready
#define READY_LED_OFF		//LED2_OFF			// temp not ready
#define READY_LED_BLINK		//LED2_BLINK			// Gradient mode (run)
#define GRADIENT_LED_BLINK	READY_LED_BLINK		// Gradient mode (run)


#define MARK_OUT_CONTACT_OPEN			// Mark Out 포트없음.
#define MARK_OUT_CONTACT_CLOSE		// Mark Out 포트없음.
#define MARK_OUT_HIGH			MARK_OUT_CONTACT_OPEN
#define MARK_OUT_LOW			MARK_OUT_CONTACT_CLOSE
#define MARK_OUT_ON				MARK_OUT_CONTACT_OPEN
#define MARK_OUT_OFF			MARK_OUT_CONTACT_CLOSE

#if 1 // ryu_Oven  수저하시오 
#define START_OUT_CONTACT_OPEN		//	rGPEDAT = (rGPEDAT | 0x1<<5)	// GPE5
#define START_OUT_CONTACT_CLOSE		// 	rGPEDAT = (rGPEDAT & ~(0x1<<5))	// 
#define START_OUT_HIGH			START_OUT_CONTACT_OPEN
#define START_OUT_LOW			START_OUT_CONTACT_CLOSE
#define START_OUT_ON			START_OUT_CONTACT_OPEN
#define START_OUT_OFF			START_OUT_CONTACT_CLOSE
#endif

#define START_IN_FALLING_EDGE_DETECTION		FPGA_WRITE_WORD(W1_START_CONFIG, 0x0)				// low detect
#define START_IN_RISING_EDGE_DETECTION		FPGA_WRITE_WORD(W1_START_CONFIG, 0x1)				// high detect

#define REMOTE_READY_OUT_CONTACT_CLOSE		FPGA_WRITE_WORD(W5_REMOTE_OUT, 0x0)
#define REMOTE_READY_OUT_CONTACT_OPEN			FPGA_WRITE_WORD(W5_REMOTE_OUT, 0x1)
#define REMOTE_START_OUT_CONTACT_CLOSE		FPGA_WRITE_WORD(W6_START_OUT, 0x0)
#define REMOTE_START_OUT_CONTACT_OPEN			FPGA_WRITE_WORD(W6_START_OUT, 0x1)

// 수정할 것 - 회로추가 - ready signal을 사용한다.
#define AUX_SIG_OUT_CONTACT_CLOSE		FPGA_WRITE_WORD(W5_REMOTE_OUT, 0x0)
#define AUX_SIG_OUT_CONTACT_OPEN		FPGA_WRITE_WORD(W5_REMOTE_OUT, 0x1)


#endif // FPGA_CHAS_H_
