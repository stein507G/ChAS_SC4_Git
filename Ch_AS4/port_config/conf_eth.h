


#ifndef _CONF_ETH_H_
#define _CONF_ETH_H_

#if 0		// ryuOven 20210105

#define HTTP_USED 1

/*! define stack size for WEB server task */
#define lwipBASIC_WEB_SERVER_STACK_SIZE   256

/*! define stack size for TFTP server task */
#define lwipBASIC_TFTP_SERVER_STACK_SIZE  1024

/*! define stack size for SMTP Client task */
#define lwipBASIC_SMTP_CLIENT_STACK_SIZE  256

/*! define stack size for lwIP task */
#define lwipINTERFACE_STACK_SIZE          512

/*! define stack size for netif task */
#define netifINTERFACE_TASK_STACK_SIZE    512

/*! define WEB server priority */
#define ethWEBSERVER_PRIORITY             ( tskIDLE_PRIORITY + 2 )

/*! define TFTP server priority */
#define ethTFTPSERVER_PRIORITY            ( tskIDLE_PRIORITY + 3 )

/*! define SMTP Client priority */
#define ethSMTPCLIENT_PRIORITY            ( tskIDLE_PRIORITY + 5 )

/*! define lwIP task priority */
#define lwipINTERFACE_TASK_PRIORITY       ( configMAX_PRIORITIES - 1 )

/*! define netif task priority */
#define netifINTERFACE_TASK_PRIORITY      ( configMAX_PRIORITIES - 1 )

/*! Number of threads that can be started with sys_thread_new() */
#define SYS_THREAD_MAX                      16

/*! Number of receive buffers */
#define ETHERNET_CONF_NB_RX_BUFFERS        20

/*! USE_RMII_INTERFACE must be defined as 1 to use an RMII interface, or 0
to use an MII interface. */
#define ETHERNET_CONF_USE_RMII_INTERFACE   0

/*! Number of Transmit buffers */
#define ETHERNET_CONF_NB_TX_BUFFERS        10

/*! Size of each Transmit buffer. */
#define ETHERNET_CONF_TX_BUFFER_SIZE       1024
//#define ETHERNET_CONF_TX_BUFFER_SIZE       128

/*! Clock definition */
//#define ETHERNET_CONF_SYSTEM_CLOCK         48000000

/*! Use Auto Negociation to get speed and duplex */
#define ETHERNET_CONF_AN_ENABLE                      1

/*! Do not use auto cross capability */
#define ETHERNET_CONF_AUTO_CROSS_ENABLE              0
/*! use direct cable */
#define ETHERNET_CONF_CROSSED_LINK                   0

#endif

#if 1
/* ethernet default parameters */
/*! MAC address definition.  The MAC address must be unique on the network. */
#define ETHERNET_CONF_ETHADDR0                        0x00u
#define ETHERNET_CONF_ETHADDR1                        0x50u		// AS
#define ETHERNET_CONF_ETHADDR2                        0x51u
#define ETHERNET_CONF_ETHADDR3                        0x52u
#define ETHERNET_CONF_ETHADDR4                        0x53u
//#define ETHERNET_CONF_ETHADDR1                        0x30u			// Oven
//#define ETHERNET_CONF_ETHADDR2                        0x25u
//#define ETHERNET_CONF_ETHADDR3                        0x33u
//#define ETHERNET_CONF_ETHADDR4                        0x33u
//#define ETHERNET_CONF_ETHADDR5                        0x40u
#define ETHERNET_CONF_ETHADDR5                        ETHERNET_CONF_IPADDR3			// 0x40u

//#define IP_PORT_NUMBER  3000
#define IP_PORT_NUMBER  							4242
#define CIP_PORT_NUMBER  							"4242"


/*! The IP address being used. */
#define ETHERNET_CONF_IPADDR0                         10
#define ETHERNET_CONF_IPADDR1                         10
#define ETHERNET_CONF_IPADDR2                         10
#define ETHERNET_CONF_IPADDR3                         52 // 40

#define CETHERNET_CONF_IPADDR 							"10.10.10.52"


/*! The gateway address being used. */
#define ETHERNET_CONF_GATEWAY_ADDR0                   10
#define ETHERNET_CONF_GATEWAY_ADDR1                   10
#define ETHERNET_CONF_GATEWAY_ADDR2                   10
#define ETHERNET_CONF_GATEWAY_ADDR3                   1

/*! The network mask being used. */
#define ETHERNET_CONF_NET_MASK0                       255
#define ETHERNET_CONF_NET_MASK1                       255
#define ETHERNET_CONF_NET_MASK2                       255
#define ETHERNET_CONF_NET_MASK3                       0
#endif


// ryu
#define AS_MY_PORT			4242
#define HTTP_Port				80
#define TELNET_PORT			23

#define AS_MY_IP_ADDR0			10
#define AS_MY_IP_ADDR1			10
#define AS_MY_IP_ADDR2			10
#define AS_MY_IP_ADDR3			91

#define CLIENT_IP_ADDR0			10
#define CLIENT_IP_ADDR1			10
#define CLIENT_IP_ADDR2			10
#define CLIENT_IP_ADDR3			100

#endif

