#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include <stdio.h>

/* Include user defined options first */
#include "conf_eth.h"

#define LWIP_PROVIDE_ERRNO 1

/* These two control is reclaimer functions should be compiled
   in. Should always be turned on (1). */
#define MEM_RECLAIM             1
#define MEMP_RECLAIM            1

/*
 * enable SYS_LIGHTWEIGHT_PROT in lwipopts.h if you want inter-task protection
 * for certain critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT            1

/**
 * NO_SYS==1: Provides VERY minimal functionality. Otherwise,
 * use lwIP facilities.
 */
#define NO_SYS                  1

//#define ETH_PAD_SIZE 2

/* ---------- Memory options ---------- */
/**
 * MEM_LIBC_MALLOC==1: Use malloc/free/realloc provided by your C-library
 * instead of the lwip internal allocator. Can save code size if you
 * already use it.
 */
#define MEM_LIBC_MALLOC                 0
//#define MEM_LIBC_MALLOC                 1		// ryu_OVEN   -- HardFault_Handler

/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT           4

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#if KCJ_TCP_LWIPOPTS_H
#define MEM_SIZE                10 * 1024
#else
#define MEM_SIZE                4 * 1024		// ryu_OVEN
//#define MEM_SIZE                10 * 1024		// ryu_OVEN	- 램영역확인후 사용(heap memory) - production-smartfusion2-execute-in-place.ld
//#define MEM_SIZE                14 * 1024		// RYU_TCP
#endif

//#define MEMP_SANITY_CHECK       1
#define MEMP_SANITY_CHECK       0			// ryuOven // 20210604

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF           6
//#define MEMP_NUM_PBUF           3			// RYU_TCP

/* Number of raw connection PCBs */
#define MEMP_NUM_RAW_PCB                1

/* ---------- UDP options ---------- */
#define LWIP_UDP                0
//  #define LWIP_UDP                1// RYU_TCP
#define UDP_TTL                 255
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One per active UDP "connection". */
#define MEMP_NUM_UDP_PCB        1

/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP connections. */
#if KCJ_TCP_LWIPOPTS_H      
#define MEMP_NUM_TCP_PCB        14
#else
//#define MEMP_NUM_TCP_PCB        14
//#define MEMP_NUM_TCP_PCB        6		// 20210604   
//#define MEMP_NUM_TCP_PCB        4		// ryu_OVEN   
#define MEMP_NUM_TCP_PCB        2		// 20210610   
#endif

/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 4

/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#if KCJ_TCP_LWIPOPTS_H   
#define MEMP_NUM_TCP_SEG        16			// ryuOven20210202
#else
//efine MEMP_NUM_TCP_SEG        16			// ryuOven20210202	20210615
#define MEMP_NUM_TCP_SEG        6
#endif

/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#define MEMP_NUM_SYS_TIMEOUT    6

/* The following four are used only with the sequential API and can be
   set to 0 if the application only will use the raw API. */
/* MEMP_NUM_NETBUF: the number of struct netbufs. */
#define MEMP_NUM_NETBUF         3
/* MEMP_NUM_NETCONN: the number of struct netconns. */
#define MEMP_NUM_NETCONN        6


/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE          6
//#define PBUF_POOL_SIZE          3				// RYU_TCP

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
//#define PBUF_POOL_BUFSIZE       1024

/* PBUF_LINK_HLEN: the number of bytes that should be allocated for a
   link level header. */
#define PBUF_LINK_HLEN          16

/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define TCP_TTL                 255

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */

#define TCP_QUEUE_OOSEQ         1
//#define TCP_QUEUE_OOSEQ         0			//  Ryu_SF2 - 20210610

/* TCP Maximum segment size. */
#if KCJ_TCP_LWIPOPTS_H   
#define TCP_MSS                 1460
#else
//#define TCP_MSS                 1500
//#define TCP_MSS                 536			// ryu_Oven		- 패킷 여러개가 연속적으로 들어오면(536보다 커지면) 문제 발생
// 20210517
#define TCP_MSS                 1024			// ryu_SF2
#endif

/* TCP receive window. */
#if KCJ_TCP_LWIPOPTS_H   
#define TCP_WND                 (4 * TCP_MSS)
#else
//#define TCP_WND                 1500
#define TCP_WND                 1600
#endif


/* TCP sender buffer space (bytes). */
// RYU_TCP
//연결을 위한 TCP 송신 버터 공간
//* 송신자 버퍼 공간(송신을 위한 데이터 큐)을 제한한다. 최적의 성능을 위해서는 원격 호스트의 TCP 윈도우 크기와 같아야 함.
//모든 동적 연결은 데이터 양을 위한 버퍼를 유념하여, MEM_SIZE로 정의된 충분한 RAM공간의 확인하거나 동시 활성 연결 수의 제한을 염두해야 함.
//[출처] LwIP 메모리 설정 옵션|작성자 카드한장
#if KCJ_TCP_LWIPOPTS_H
#define TCP_SND_BUF             (2 * TCP_MSS)
#else
//#define TCP_SND_BUF             2150
#define TCP_SND_BUF             TCP_MSS * 2// 4300	// 4096		// RYU_TCP  / / ryu_Oven
#endif

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#if KCJ_TCP_LWIPOPTS_H
#define TCP_SND_QUEUELEN				((6 * (TCP_SND_BUF) + (TCP_MSS - 1))/(TCP_MSS))
//#define TCP_SND_QUEUELEN				((4 * (TCP_SND_BUF) + (TCP_MSS - 1))/(TCP_MSS))
#else
//#define TCP_SND_QUEUELEN        6 * TCP_SND_BUF/TCP_MSS
// 20210604
#define TCP_SND_QUEUELEN        4 * TCP_SND_BUF/TCP_MSS			// ryuOven20210202
//#define TCP_SND_QUEUELEN        2 * TCP_SND_BUF/TCP_MSS			// ryuAS 20210518
#endif

#if KCJ_TCP_LWIPOPTS_H
#define LWIP_MAX(x , y)  (((x) > (y)) ? (x) : (y))
#define LWIP_MIN(x , y)  (((x) < (y)) ? (x) : (y))

 /**
 * TCP_SNDLOWAT: TCP writable space (bytes). This must be less than
 * TCP_SND_BUF. It is the amount of space which must be available in the
 * TCP snd_buf for select to return writable (combined with TCP_SNDQUEUELOWAT).
 */
#define TCP_SNDLOWAT                    LWIP_MIN(LWIP_MAX(((TCP_SND_BUF)/2), (2 * TCP_MSS) + 1), (TCP_SND_BUF) - 1)

 /**
 * TCP_WND_UPDATE_THRESHOLD: difference in window to trigger an
 * explicit window update
 */
#define TCP_WND_UPDATE_THRESHOLD        LWIP_MIN((TCP_WND / 4), (TCP_MSS * 4))

 /**
 * PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. The default is
 * designed to accommodate single full size TCP frame in one pbuf, including
 * TCP_MSS, IP header, and link header.
 */
#define PBUF_POOL_BUFSIZE               LWIP_MEM_ALIGN_SIZE(TCP_MSS+PBUF_IP_HLEN+PBUF_TRANSPORT_HLEN+PBUF_LINK_ENCAPSULATION_HLEN+PBUF_LINK_HLEN)

#else

#endif


/* Maximum number of retransmissions of data segments. */
#define TCP_MAXRTX              12

/* Maximum number of retransmissions of SYN segments. */
#define TCP_SYNMAXRTX           4

/* ---------- ARP options ---------- */
#define ARP_TABLE_SIZE 10
#define ARP_QUEUEING 0

/* ---------- IP options ---------- */
/* Define IP_FORWARD to 1 if you wish to have the ability to forward
   IP packets across network interfaces. If you are going to run lwIP
   on a device with only one network interface, define this to 0. */
#define IP_FORWARD              0		// Ryu_SF2 - 20210601
//#define IP_FORWARD              1				// ryu_OVEN		

/* If defined to 1, IP options are allowed (but not parsed). If
   defined to 0, all packets with IP options are dropped. */
#define IP_OPTIONS              1
//#define IP_OPTIONS              0		// Ryu_SF2 - 20210601

/* ---------- ICMP options ---------- */
#define ICMP_TTL                255


/* ---------- DHCP options ---------- */
/* Define LWIP_DHCP to 1 if you want DHCP configuration of
   interfaces. DHCP is not implemented in lwIP 0.5.1, however, so
   turning this on does currently not work. */
#define LWIP_DHCP               0

#define LWIP_ARP 1

/* 1 if you want to do an ARP check on the offered address
   (recommended). */
#define DHCP_DOES_ARP_CHECK     0

//#define TCPIP_THREAD_PRIO               lwipINTERFACE_TASK_PRIORITY


#define TCPIP_THREAD_STACKSIZE 4096







/* ---------- Statistics options ---------- */
//#define LWIP_DEBUG

#define LWIP_STATS 0
#define LWIP_STATS_DISPLAY 0

#if LWIP_STATS
#define LINK_STATS 1
#define IP_STATS   1
#define ICMP_STATS 1
#define UDP_STATS  1
#define TCP_STATS  1
#define MEM_STATS  1
#define MEMP_STATS 1
#define PBUF_STATS 1
#define SYS_STATS  1
#endif /* STATS */


/*----- Default Value for LWIP_DBG_MIN_LEVEL: LWIP_DBG_LEVEL_ALL ---*/
//#define LWIP_DBG_MIN_LEVEL 				LWIP_DBG_MASK_LEVEL
#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_WARNING


#define MEMP_DEBUG                  LWIP_DBG_OFF //LWIP_DBG_ON
#define TCP_DEBUG                   LWIP_DBG_OFF //LWIP_DBG_ON
#define TCP_INPUT_DEBUG             LWIP_DBG_OFF //LWIP_DBG_ON
//#define TCP_FR_DEBUG                LWIP_DBG_ON
//#define TCP_RTO_DEBUG								LWIP_DBG_ON
//#define TCP_CWND_DEBUG              LWIP_DBG_ON
//#define TCP_WND_DEBUG								LWIP_DBG_ON
#define TCP_OUTPUT_DEBUG            LWIP_DBG_OFF //LWIP_DBG_ON
#define TCP_QLEN_DEBUG              LWIP_DBG_OFF //LWIP_DBG_ON

#define ETHARP_DEBUG								LWIP_DBG_OFF

#define LWIP_DBG_TYPES_ON         	LWIP_DBG_OFF //LWIP_DBG_ON



// ryuOven
#define LWIP_NETIF_LINK_CALLBACK		1












// 20210617
// 짧게하면 -- 연속적으로 패킷이 들어왔을 때 처리하는 능력이 올라간다.
//#define TCP_TMR_INTERVAL       75  /* The TCP timer interval in milliseconds. */ // ryu 20210617




#endif /* __LWIPOPTS_H__ */
