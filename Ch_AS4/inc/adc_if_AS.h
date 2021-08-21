/*
 * adc_if.h
 *
 *  Created on: 2017. 10. 18.
 *      Author: Han
 */

#ifndef ADC_IF_AS_H_
#define ADC_IF_AS_H_


#define ADC_STATE_READY    		0xC0

/* ADC Functional state */
typedef enum {
    ADCST_POR_READY		= 0x0,
    ADCST_UNLOCK		= 0x1,
    ADCST_CONFIG_REGS	= 0x2,
    ADCST_ENABLE		= 0x3,
    ADCST_WAKEUP		= 0x4,
    ADCST_LOCKREG		= 0x5,
    ADCST_RUN			= 0x10,
} ADC_STATE_E;



typedef struct
{
	ADC_STATE_E	State;
	uint8_t		tick1ms;
	uint8_t		procen; // periodic process enable
	uint8_t		i;
	uint8_t		cnt;	// general counter
	uint8_t		chkcnt; // check counter
	uint8_t		intr;
	uint32_t	Dat[30]; // 4ch x 5frame
	uint8_t 	Dbgen;
} ADC_ST; //

void adc_init(void);
void (*pfAdcProcess)(void);
void adc_process(void);
void adc_process_blank(void);
void ADC_reading(void);

void adc_Reset(void);
void adc_Set(void);
void adc_InterruptEnable(void);
uint32_t adc_CmdNull(void);
uint32_t adc_CmdUnlock(void);
uint32_t adc_CmdEnable(void);
uint32_t adc_CmdWakeup(void);
uint32_t adc_CmdLock(void);

uint32_t adc_CmdReadReg(uint32_t addr);
uint32_t adc_CmdWriteReg(uint32_t addr, uint8_t data);
void adc_GetData(void);

void OvenTempReadyCheck();

void initTempCalData();
void tempConstCalc(unsigned char act);


#endif /* ADC_IF_AS_H_ */

