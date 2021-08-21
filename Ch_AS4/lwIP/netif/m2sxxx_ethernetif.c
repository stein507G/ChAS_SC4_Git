/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/* Add the required include Files
 */
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "mss_ethernet_mac.h"
#include "m2sxxx.h"
#include "sys_cfg.h"

#ifndef LWIP_DHCP
#define LWIP_DHCP	0
#endif

/* Header Files required by LWIP */
#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#include "netif/etharp.h"

#include "inc/util.h"

const uint8_t * sys_cfg_get_mac_address(void);

/*
 * PHY addresses of most common SmartFusion2 boards.
 */
#define M88E1340_PHY_ADDR                   0x01
#define M88E1111_PHY_ADDR                   0x07
#define MAX24288_PHY_ADDR                   0x00
#define KSZ8051MNL_PHY_ADDR                 0x00

/* Define those to better describe your network interface. */
#define IFNAME0 'E0'

#define BUFFER_USED     1u
#define BUFFER_EMPTY    0u
#define RELEASE_BUFFER  BUFFER_EMPTY

extern void mem_init();
extern void memp_init();

extern unsigned char my_ip[];		
extern unsigned char my_netMask[];	
extern unsigned char my_gateway[];
extern unsigned char my_mac[];

extern uint8_t FrameReceived;

uint32_t get_user_eth_speed_choice(void);

/* Forward declarations. */
/* MAC configuration record */
mss_mac_cfg_t g_mac_config;

/* Buffers for Tx and Rx */
static uint8_t g_mac_tx_buffer[MSS_MAC_MAX_TX_BUF_SIZE];
static volatile uint32_t g_mac_tx_buffer_used = RELEASE_BUFFER;
static uint8_t g_mac_rx_buffer[MSS_MAC_MAX_RX_BUF_SIZE];
static volatile uint32_t g_mac_rx_buffer_data_valid;

//struct netif * g_p_mac_netif = 0;
struct netif netif;

extern volatile uint32_t LocalTime;		// 210621 CJKIM

void mac_rx_callback
(
    uint8_t * p_rx_packet,
    uint32_t pckt_length,
    void * caller_info
);

static void ethernetif_input
(
    struct netif *netif,
    uint8_t * p_rx_packet,
    uint32_t pckt_length
);

err_t
ethernetif_init(struct netif *netif);

#if 0
void LwIP_Init(void)
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
#if LWIP_DHCP == 1
//#ifndef USE_DHCP
  uint8_t iptab[4];
  uint8_t iptxt[20];
#endif

  /* Initializes the dynamic memory heap defined by MEM_SIZE.*/
  mem_init();

  /* Initializes the memory pools defined by MEMP_NUM_x.*/
  memp_init();

//#ifdef USE_DHCP
#if LWIP_DHCP == 1
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else

//	printf("IP address, Net mask, GW address are defined in <conf_eth.h>\r\n");

#if 0
	IP4_ADDR(&ipaddr, ETHERNET_CONF_IPADDR0, 	ETHERNET_CONF_IPADDR1, 		 ETHERNET_CONF_IPADDR2, 		ETHERNET_CONF_IPADDR3);
	IP4_ADDR(&netmask, ETHERNET_CONF_NET_MASK0, ETHERNET_CONF_NET_MASK1 , 	 ETHERNET_CONF_NET_MASK2, 		ETHERNET_CONF_NET_MASK3);
	IP4_ADDR(&gw, ETHERNET_CONF_GATEWAY_ADDR0,  ETHERNET_CONF_GATEWAY_ADDR1, ETHERNET_CONF_GATEWAY_ADDR2, 	ETHERNET_CONF_GATEWAY_ADDR3);

	printf("Static IP address  %d.%d.%d.%d\r\n", ETHERNET_CONF_IPADDR3, ETHERNET_CONF_IPADDR2, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR0);
#else
	IP4_ADDR(&ipaddr, my_ip[0], my_ip[1], my_ip[2], my_ip[3]);
	IP4_ADDR(&netmask, my_netMask[0], my_netMask[1], my_netMask[2], my_netMask[3]);
	IP4_ADDR(&gw, my_gateway[0], my_gateway[1], my_gateway[2], my_gateway[3]);

	printf("Static IP address  %d.%d.%d.%d\r\n", my_ip[3], my_ip[2], my_ip[1], my_ip[0]);
#endif
#endif

	/* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
			struct ip_addr *netmask, struct ip_addr *gw,
			void *state, err_t (* init)(struct netif *netif),
			err_t (* input)(struct pbuf *p, struct netif *netif))

	Adds your network interface to the netif_list. Allocate a struct
	netif and pass a pointer to this structure as the first argument.
	Give pointers to cleared ip_addr structures when using DHCP,
	or fill them with sane numbers otherwise. The state pointer may be NULL.

	The init function pointer must point to a initialization function for
	your ethernet netif interface. The following code illustrates it's use.*/
	netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

	/*  Registers the default network interface.*/
	netif_set_default(&netif);

	/*  When the netif is fully configured this function must be called.*/
	netif_set_up(&netif);
}
#endif

void netif_config(void)
{
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;

//#ifdef USE_DHCP
#if LWIP_DHCP == 1
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else

	iprintf("IP address, Net mask, GW address are defined in <conf_eth.h>\r\n");

#if 1
	IP4_ADDR( &ipaddr,my_ip[0],my_ip[1],my_ip[2],my_ip[3] );
	IP4_ADDR( &netmask,my_netMask[0],my_netMask[1],my_netMask[2],my_netMask[3] );
	IP4_ADDR( &gw,my_gateway[0],my_gateway[1],my_gateway[2],my_gateway[3] );
	printf("Static IP address  %d.%d.%d.%d\r\n", my_ip[0], my_ip[1], my_ip[2], my_ip[3]);
#else	
	IP4_ADDR(&ipaddr, ETHERNET_CONF_IPADDR0, 	ETHERNET_CONF_IPADDR1, 		 ETHERNET_CONF_IPADDR2, 		ETHERNET_CONF_IPADDR3);
	IP4_ADDR(&netmask, ETHERNET_CONF_NET_MASK0, ETHERNET_CONF_NET_MASK1 , 	 ETHERNET_CONF_NET_MASK2, 		ETHERNET_CONF_NET_MASK3);
	IP4_ADDR(&gw, ETHERNET_CONF_GATEWAY_ADDR0,  ETHERNET_CONF_GATEWAY_ADDR1, ETHERNET_CONF_GATEWAY_ADDR2, 	ETHERNET_CONF_GATEWAY_ADDR3);
	printf("Static IP address  %d.%d.%d.%d\r\n", ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2, ETHERNET_CONF_IPADDR3);
#endif	
#endif

	 /* add the network interface */
	//netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);
#if NET_CONNECT_TEST
	netif_add(&netif, &ipaddr, &netmask, &gw, NULL, ethernetif_init, ip_input);
#else
	netif_add(&netif, &ipaddr, &netmask, &gw, NULL, ethernetif_init, ethernet_input);
#endif

	/*  Registers the default network interface.*/
	netif_set_default(&netif);
	//if (netif_is_link_up(&netif))
	//{
		// When the netif is fully configured this function must be called
		netif_set_up(&netif);
	//}
	//else
	//{
		// When the netif link is down this function must be called
	//	netif_set_down(&netif);
	//}

	/* Set the link callback function, this function is called on change of link status*/
	//netif_set_link_callback(&netif, ethernetif_update_config);
}






static void packet_tx_complete_handler(void * caller_info);

static void low_level_init(struct netif *netif);
static err_t low_level_output(struct netif *netif, struct pbuf *p);

static struct pbuf * low_level_input
(
    struct netif *netif,
    uint8_t * p_rx_packet,
    uint32_t pckt_length
);

/**=============================================================================
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init(struct netif *netif)
{
#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */
    /*
     * Initialize the snmp variables and counters inside the struct netif.
     * The last argument should be replaced with your link speed, in units
     * of bits per second.
     */
    NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

    netif->state = 0;
    netif->name[0] = 'E';
    netif->name[1] = '0';

    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
    netif->output = etharp_output;
    netif->linkoutput = low_level_output;

    /* initialize the hardware */
    low_level_init(netif);

    return ERR_OK;
}

/**=============================================================================
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(struct netif *netif)
{
    const uint8_t * own_hw_adr;
    /*turn off the watchdog*/
    SYSREG->WDOG_CR = 0;
    
    /* We only have one network Interface */
    /* Initialize the Network interface */
    netif->num = 1;
    own_hw_adr = sys_cfg_get_mac_address();
    netif->hwaddr[0] = own_hw_adr[0];
    netif->hwaddr[1] = own_hw_adr[1];
    netif->hwaddr[2] = own_hw_adr[2];
    netif->hwaddr[3] = own_hw_adr[3];
    netif->hwaddr[4] = own_hw_adr[4];
    netif->hwaddr[5] = own_hw_adr[5];
    
    //printf("My hwaddr -> %02X:%02X:%02X:%02X:%02X:%02X\r\n",
    //		netif->hwaddr[0],
    //		netif->hwaddr[1],
    //		netif->hwaddr[2],
    //		netif->hwaddr[3],
    //		netif->hwaddr[4],
    //		netif->hwaddr[5]);

    netif->hwaddr_len = 6; /* Defined in LWIP ETHARP_HWADDR_LEN; */

    /* maximum transfer unit */
//    netif->mtu = 1500;
    netif->mtu = TCP_MSS;		// RYU_TCP		
		
    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

    /* Keep track of netif pointer for use by interrupt service routine. */
    //g_p_mac_netif = netif;
    
    /*--------------------- Initialize packet containers ---------------------*/
    g_mac_tx_buffer_used = RELEASE_BUFFER;
    g_mac_rx_buffer_data_valid = RELEASE_BUFFER;
    
    /*-------------------------- Initialize the MAC --------------------------*/
    /*
     * The interrupt can cause a context switch, so ensure its priority is
     * between configKERNEL_INTERRUPT_PRIORITY and
     * configMAX_SYSCALL_INTERRUPT_PRIORITY.
     */
    //NVIC_SetPriority(EthernetMAC_IRQn,
    //                 configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    
    /*
     * Get the default configuration for the Ethernet MAC and change settings
     * to match the system/application. The default values typically changed
     * are:
     *  - interface:
     *      Specifies the interface used to connect the Ethernet MAC to the PHY.
     *      Example choice are MII, GMII, TBI.
     *  - phy_addr:
     *      Specifies the MII management interface address of the external PHY.
     *  - mac_addr:
     *      Specifies the MAC address of the device. This number should be
     *      unique on the network the device is connected to.
     *  - speed_duplex_select:
     *      Specifies the allowed speed and duplex mode for setting up a link.
     *      This can be used to specify the set of allowed speeds and duplex
     *      modes used during auto-negotiation or force the link speed to a
     *      specific speed and duplex mode.
     */
    MSS_MAC_cfg_struct_def_init(&g_mac_config);
#if 1	// SOFTCONSOLE_V4
  	// softconsole 4
#else
    //g_mac_config.interface = TBI;
    g_mac_config.interface = MII;
#endif
    g_mac_config.phy_addr = KSZ8051MNL_PHY_ADDR;
    g_mac_config.speed_duplex_select= MSS_MAC_ANEG_ALL_SPEEDS ;

    g_mac_config.mac_addr[0] = own_hw_adr[0];
    g_mac_config.mac_addr[1] = own_hw_adr[1];
    g_mac_config.mac_addr[2] = own_hw_adr[2];
    g_mac_config.mac_addr[3] = own_hw_adr[3];
    g_mac_config.mac_addr[4] = own_hw_adr[4];
    g_mac_config.mac_addr[5] = own_hw_adr[5];

    iprintf("g_mac_config.mac_addr [ %x:%x:%x:%x:%x:%x ]\n",
		g_mac_config.mac_addr[0],
		g_mac_config.mac_addr[1],
		g_mac_config.mac_addr[2],
		g_mac_config.mac_addr[3],
		g_mac_config.mac_addr[4],
		g_mac_config.mac_addr[5]);

    /*
     * Initialize MAC with specified configuration. The Ethernet MAC is
     * functional after this function returns but still requires transmit and
     * receive buffers to be allocated for communications to take place.
     */

    MSS_MAC_init(&g_mac_config);
    
    /*
     * Register MAC interrupt handler listener functions. These functions will
     * be called  by the MAC driver when a packet ahs been sent or received.
     * These callback functions are intended to help managing transmit and
     * receive buffers by indicating when a transmit buffer can be released or
     * a receive buffer has been filled with an rx packet.
     */
    MSS_MAC_set_tx_callback(packet_tx_complete_handler);
    MSS_MAC_set_rx_callback(mac_rx_callback);

    /* 
     * Allocate a receive buffer. This function will need to be called each time
     * a packet is received to hand back the receive buffer to the MAC driver
     */
    MSS_MAC_receive_pkt(g_mac_rx_buffer, 0);
}

/**=============================================================================
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */



static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
    struct pbuf *q;
    uint16_t pckt_length = 0u;
    uint32_t pbuf_chain_end = 0u;
    
    uint8_t tx_status;
    uint32_t Timeout=0;

	uint32_t elapse = 0;
	__IO uint32_t localtime = 0;


//(+)210629 CJKIM, lwIP 硫�紐⑤━ 遺�議� Test�⑹�쇰� ���� �щ� ���� �� ���� 媛���
#if 1 // 210621 CJKIM Added , g_mac_tx_buffer_used Flag媛� Timeout 源�吏� ��湲� ���� ���� 諛������� 異�媛���.

#define	MAC_SEND_INTERVAL	10		//(+)210715 CJKIM, 3 -> 10mSec

	uint8_t intervalFlag = 0;
	static uint32_t end_time = 0;
	
	// Send Interval minium 3mSec
	do{
		localtime = LocalTime;//get_mili_local_time(); 
		if(localtime >= end_time) {
			elapse = localtime - end_time;
		}
		else {
			//elapse = (MAX_UINT32_VALUE - end_time) - localtime;
			elapse = (UINT_MAX - end_time) -  localtime;
		}

		if(elapse <= MAC_SEND_INTERVAL) {
			intervalFlag = 1;
		}

	} while(elapse <= MAC_SEND_INTERVAL); 	//210621 CJKIM 3 mSec

	if(intervalFlag) {
		LWIP_DEBUG_E(LWIP_DBG_ERROR_ON, 
		  "low_level_output: TX Packet within Interval [%d]\n", MAC_SEND_INTERVAL);
	}
		
	end_time =  localtime;

#endif

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    /*--------------------------------------------------------------------------
     * Wait for packet buffer to become free.
     */
    while(BUFFER_USED == g_mac_tx_buffer_used)
    {
    	Timeout++;
    	if(Timeout > 0xfffff) {
			//210621 CJKIM , Timeout 諛��� �щ� ����.
			LWIP_DEBUG_E(LWIP_DBG_ERROR_ON, 
		      "low_level_output: TX Buffer Used Timeout. Buffer[%ld]\n", g_mac_tx_buffer_used);
    		g_mac_tx_buffer_used = RELEASE_BUFFER;
    	}
    }
    g_mac_tx_buffer_used = BUFFER_USED;
    
    /*--------------------------------------------------------------------------
     * Copy pbuf chain into single buffer.
     */
    q = p;
    do {
        memcpy(&g_mac_tx_buffer[pckt_length], q->payload, q->len);
        pckt_length += q->len;
        if(q->len == q->tot_len)
        {
            pbuf_chain_end = 1u;
        }
        else
        {
            q = q->next;
        }
    } while(0u == pbuf_chain_end);
    
    /*--------------------------------------------------------------------------
     * Initiate packet transmit. Keep retrying until there is room in the MAC Tx
     * ring.
     */

	//(+)210610 CJKIM, For Tx Packet ���� ��媛� ����
	//LWIP_DEBUG_I(LWIP_DBG_ON, "T+\n");	//(+)210610 CJKIM, Tx Packet ���� ��媛� ����
	Timeout = 0;	//210621 CJKIM , Infinite loop 諛��� �щ� ����.
    do {
    	Timeout++;
    	if(Timeout > 0xfffff) {
			//210621 CJKIM
			LWIP_DEBUG_E(LWIP_DBG_ERROR_ON, 
		      "low_level_output: Sending.... tx_status[%d]\n", tx_status);
			break;		//(+)210722 CJKIM, Bug Fix
    	}		
		
        tx_status = MSS_MAC_send_pkt(g_mac_tx_buffer,
                                     pckt_length,
                                     (void *)&g_mac_tx_buffer_used);
    } while(MSS_MAC_SUCCESS != tx_status);
	//LWIP_DEBUG_I(LWIP_DBG_ON, "T-\n");	//(+)210610 CJKIM, Tx Packet ���� ��媛� ����

    
#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

	//(+)210629 CJKIM, lwIP 硫�紐⑤━ 遺�議� Test�⑹�쇰� ���� �щ� ���� �� ���� 媛���
	// 210621 CJKIM Added , packet buffer to become free. 
	// ������ Callback ���� 泥�由� ���댁�� ���� Timeout 源�吏� ��湲� ���� ���� 諛������� 異�媛���.

	//(+)210716 CJKIM, Ethernet Interrupt ���� 泥�由� ����濡� ����. (���� ����濡� ����由�)
	//g_mac_tx_buffer_used = RELEASE_BUFFER;
	
#if 0 //(+)210715 CJKIM, Tx Done 2mS delay , Fot Test

#define	MAC_SEND_DONE_INTERVAL	2		//(+)210715 CJKIM, 
	
		uint32_t tx_done_start_time = 0;
		uint32_t tx_done_end_time = 0;
		
		tx_done_start_time = LocalTime;//get_mili_local_time();
		do{
			
			tx_done_end_time = LocalTime;//get_mili_local_time(); 
			if(tx_done_end_time >= tx_done_start_time) {
				elapse = tx_done_end_time - tx_done_start_time;
			}
			else {
				elapse = (UINT_MAX - tx_done_start_time) -  tx_done_end_time;
			}
			
		} while(elapse < MAC_SEND_DONE_INTERVAL);	
#endif


    return ERR_OK;
}

/**=============================================================================
 *
 */
static void packet_tx_complete_handler(void * caller_info)
{
    /*
     * caller_info points to g_mac_tx_buffer_used. Signal that content of
     * g_mac_tx_buffer has been sent by the MAC by resetting
     * g_mac_tx_buffer_used.
     */
    *((volatile uint32_t *)caller_info) = RELEASE_BUFFER;		// 210621 CJKIM , volatile added
	//LWIP_DEBUG_I(LWIP_DBG_ON, "TC\n");	//(+)210621 CJKIM,  Callback ����
}

/**=============================================================================
    Bottom-half of receive packet handler
*/

void mac_rx_callback
(
    uint8_t * p_rx_packet,
    uint32_t pckt_length,
    void * caller_info
)
{
    if(BUFFER_EMPTY == g_mac_rx_buffer_data_valid)
    {
        g_mac_rx_buffer_data_valid = BUFFER_USED;
        
//        FrameReceived = 1;

        //if(g_p_mac_netif != 0)
        //if(&netif != 0)
        //{
            ethernetif_input(&netif, p_rx_packet, pckt_length);
//            iprintf("Rx(%ld)Bytes - %02X %02X\r\n",pckt_length, g_mac_rx_buffer[0],g_mac_rx_buffer[1]);
            //tcp_write(g_p_mac_netif.);
            //tcp_test_send();
        //}
    }
}




/**=============================================================================
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
static void
ethernetif_input
(
    struct netif *netif,
    uint8_t * p_rx_packet,
    uint32_t pckt_length
)
{
    struct eth_hdr *ethhdr;
    struct pbuf *p;

    /* move received packet into a new pbuf */
    p = low_level_input(netif, p_rx_packet, pckt_length);
    /* no packet could be read, silently ignore this */
    if (p == NULL) return;
    /* points to packet payload, which starts with an Ethernet header */
    ethhdr = p->payload;

    //if((unsigned)ethhdr->src.addr[0] == 0xA4)
    //	printf("Len : %4ld, %02X %02X\r\n",pckt_length,*(uint8_t *)p->payload,*(((uint8_t *)p->payload)+1));

    switch (htons(ethhdr->type)) {
    /* IP or ARP packet? */
    case ETHTYPE_IP:
    case ETHTYPE_ARP:
        /* full packet send to tcpip_thread to process */
        if (netif->input(p, netif)!=ERR_OK)
        { /* LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n")); */
           pbuf_free(p);
           p = NULL;
        }
    break;

    default:
        pbuf_free(p);
        p = NULL;
        break;
    }
}


/**=============================================================================
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
#if 1
static struct pbuf *
low_level_input
(
    struct netif *netif,
    uint8_t * p_rx_packet,
    uint32_t pckt_length
)
{
    struct pbuf *p, *q;
    u16_t len;

    p = NULL;
    
    /* Confirm / Wait till packet is received */
/*    
    while( !MSS_MAC_is_pkt_received( &rx_pkt ) )
    {
        ;
    }
*/    
    /* Obtain the size of the packet and put it into the "len"
       variable. */
    len = pckt_length;

    if(len > 0)
    {

#if ETH_PAD_SIZE
        len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

        /* We allocate a pbuf chain of pbufs from the pool. */
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

        if (p != NULL)
        {
            uint16_t length = 0;

#if ETH_PAD_SIZE
            pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
            /* Transfer the received packet to local buffer */
//            MSS_MAC_transfer_rx_pkt(g_mac_rx_buffer,&rx_pkt);
            /* We iterate over the pbuf chain until we have read the entire
             * packet into the pbuf. */
            for(q = p; q != NULL; q = q->next)
            {
                /* Read enough bytes to fill this pbuf in the chain. The
                 * available data in the pbuf is given by the q->len
                 * variable. */
                /* read data into(q->payload, q->len); */
                memcpy(q->payload, &g_mac_rx_buffer, q->len);
                length += q->len;
            }
            /* Assign next packet for reception */
//            rx_pkt.lock = EMPTY;
//            MSS_MAC_receive_pkt(&rx_pkt , 1);
            MSS_MAC_receive_pkt(g_mac_rx_buffer, 0);

            g_mac_rx_buffer_data_valid = RELEASE_BUFFER;
#if ETH_PAD_SIZE
            pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
        }
        else
        {
            /* Assign next packet for reception */
//            MSS_MAC_receive_pkt(&rx_pkt,1);
//            MSS_MAC_discard_pkt(&rx_pkt);
            MSS_MAC_receive_pkt(g_mac_rx_buffer, 0);
            g_mac_rx_buffer_data_valid = RELEASE_BUFFER;
        }
    }
    
    return p;
}

#else
static struct pbuf *
low_level_input(struct netif *netif)
{
    struct pbuf *p, *q;
    u16_t len;

    p = NULL;
    
    /* Confirm / Wait till packet is received */
    while( !MSS_MAC_is_pkt_received( &rx_pkt ) )
    {
        ;
    }
    /* Obtain the size of the packet and put it into the "len"
       variable. */
    len = rx_pkt.size;

    if(len > 0)
    {

#if ETH_PAD_SIZE
        len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

        /* We allocate a pbuf chain of pbufs from the pool. */
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

        if (p != NULL)
        {
            uint16_t length = 0;

#if ETH_PAD_SIZE
            pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
            /* Transfer the received packet to local buffer */
            MSS_MAC_transfer_rx_pkt(g_mac_rx_buffer,&rx_pkt);
            /* We iterate over the pbuf chain until we have read the entire
             * packet into the pbuf. */
            for(q = p; q != NULL; q = q->next)
            {
                /* Read enough bytes to fill this pbuf in the chain. The
                 * available data in the pbuf is given by the q->len
                 * variable. */
                /* read data into(q->payload, q->len); */
                memcpy(q->payload, &g_mac_rx_buffer, q->len);
                length += q->len;
            }
            /* Assign next packet for reception */
            MSS_MAC_receive_pkt(&rx_pkt , 1);
#if ETH_PAD_SIZE
            pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
        }
        else
        {
            /* Assign next packet for reception */
//            MSS_MAC_receive_pkt(&rx_pkt,1);
            MSS_MAC_discard_pkt(&rx_pkt);
        }
    }
    
    return p;
}
#endif

