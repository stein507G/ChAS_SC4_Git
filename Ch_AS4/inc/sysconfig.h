/*
 * sysconfig.h
 *
 *  Created on: 2015. 4. 14.
 *      Author: chanii
 */

#ifndef SYSCONFIG_H_
#define SYSCONFIG_H_

void cpu_init(void);

void InterruptEnable_Main(void);

void init_Control_port(); // init_CoreGpio();

void init_system();

void Init_rtcTime();

int rtcTime_Macaddr_Init();

void make_MACADDR(int NewMAC);
void saveEthernetConfig(int who);
void init_IpAddr(unsigned char sw);
void loadEthernetConfig(int who);

#if 0
void initSystemInform();
void saveSystemInform(int who);
#endif

#endif /* SYSCONFIG_H_ */

