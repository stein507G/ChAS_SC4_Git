/*******************************************************************************
 * (c) Copyright 2011-2015 Microsemi SoC Products Group.  All rights reserved.
 *
 * SmartFusion2 Microcontroller Subsystem 10/100/1000 Mbps Ethernet MAC bare metal
 * software driver public API.
 *
 * SVN $Revision: 7657 $
 * SVN $Date: 2015-08-13 17:49:00 +0530 (Thu, 13 Aug 2015) $
 */
/*=========================================================================*//**
  @mainpage SmartFusion2 MSS Ethernet MAC Bare Metal Driver.

  @section intro_sec Introduction
    The SmartFusion2 microcontroller subsystem (MSS) includes one 10/100/1000
    Mbps Ethernet MAC hardware peripheral. The MSS Ethernet MAC supports MII /
    RMII / GMII / TBI interfaces to the physical layer devices (PHY). This
    software driver provides a set of functions for controlling the MSS Ethernet
    MAC as part of a bare metal system where no operating system is available.
    The drivers can be adapted for use as part of an operating system, but the
    implementation of the adaptation layer between the driver and the operating
    system's driver model is outside the scope of the driver.

  @section hw_dependencies Hardware Flow Dependencies
    The configuration of all features of the MSS Ethernet MAC is covered by this
    driver with the exception of the selection of the Ethernet PHY connected to
    the MAC, the size of the transmit and receive rings, PHY interface and the
    MDIO address of the CoreRGMII/SGMII.

    The number of transmit and receive buffers is also selected using the 
    Firmware Catalog configuration dialog window. The Firmware Catalog generates
    a C header file containing these choices which is used as part of the MSS
    Ethernet MAC implementation. The selected values will be used to define
    the size of the transmit and receive descriptor rings within the driver.
    They define the maximum number of transmit and receive packets that can be
    queued.

    The driver for the Ethernet PHY connected to the SmartFusion2 MSS Ethernet
    MAC is selected using the Firmware Catalog configuration dialog window. It
    gives the option of using one of a set of supported PHYs. This choice will
    affect which source file will be generated by the Firmware Catalog.
    
    The PHY interface is selecting using the Firmware Catalog configuration
    dialog window. This driver supports MII, RMII, GMII, RGMII, TBI, SGMII and
    1000baseX interface. Refer Table-1 for explanation of PHY interface.

    The MDIO address of the �CoreRGMII/SGMII� is also selecting using the 
    Firmware Catalog configuration dialog window. Note that this selection is
    applicable only when PHY interface is RGMII/SGMII/1000BaseX mode. This 
    selection is ignored when PHY interface is MII/RMII/GMII/TBI mode.
    
    The base address, register addresses and interrupt number assignment for the
    MSS Ethernet MAC are defined as constants in the SmartFusion2 CMSIS HAL.
    You must ensure that the latest SmartFusion2 CMSIS HAL is included in the
    project settings of the software tool chain used to build your project and
    that it is generated into your project.

    The Firmware Catalog will generate definitions based on the settings entered
    on this dialog into the driver�s mss_ethernet_mac_user_config.h file.

            Table 1 � PHY Interface
   ----------------------------------------------------------------------------------
   | PHY         |�MSS_MAC_PHY_INTERFACE�|  Explanation                             |
   | InterFace   |  Value                |                                          |
   |-------------|----------------|-------------------------------------------------|
   | MII         |    0           |    MSS MAC operates in MII mode and Interfaces  |
   |             |                |    with MII PHY directly.                       |
   |             |                |                                                 |   
   | GMII        |    1           |    MSS MAC operates in GMII mode and Interfaces |
   |             |                |    with GMII PHY directly.                      |
   |             |                |                                                 |
   | TBI         |    2           |    MSS MAC operates in TBI mode(Internal        |
   |             |                |    MSGMII module is enabled in SGMII mode).     |
   |             |                |    Interfaces with SGMII PHY using SERDES       |
   |             |                |    and SGMII type link ANEG.                    |      
   |             |                |                                                 |
   | SGMII       |    3           |    MSS MAC operates in GMII mode. External      |
   |             |                |    CoreSGMII module is enabled in SGMII.        |
   |             |                |    Interfaces with SGMII PHY using SERDES       |
   |             |                |    and SGMII type link ANEG.                    |
   |             |                |                                                 |
   | 1000BaseX   |    4           |    MSS MAC operates in GMII mode. External      |
   |             |                |    CoreSGMII module is enabled in 1000BaseX.    |
   |             |                |    Interfaces with remote partner over Fiber    |
   |             |                |    link using SERDES and 1000BaseX type         |
   |             |                |    link ANEG.                                   |
   |             |                |                                                 |
   | RGMII       |    5           |    MSS MAC operates in GMII mode. External      |
   |             |                |    CoreRGMII is used to interface with          |
   |             |                |    RGMII PHY.                                   |
   |             |                |                                                 |
   | RMII        |    6           |    MSS MAC operates in MII mode. External       |
   |             |                |    CoreRMII is used to interface with RMII PHY. | 
   ----------------------------------------------------------------------------------
  @section theory_op Theory of Operation
    The MSS Ethernet MAC driver functions are grouped into the following
    categories:
        - Initialization and configuration
        - Transmit operations
        - Receive operations
        - Reading link status and statistics
        
    Initialization and Configuration
    The SmartFusion2 Ethernet MAC driver is initialized and configured by
    calling the MSS_MAC_init()function. The MSS_MAC_init() function takes a
    pointer to a configuration data structure as parameter. This data structure
    contains all the configuration information required to initialize and
    configure the Ethernet MAC. 
    The Ethernet MAC driver provides the MSS_MAC_cfg_struct_def_init() function
    to initialize the configuration data structure to default value. It is
    recommended to use this function to retrieve the default configuration than
    overwrite the defaults with the application specific settings such as 
    PHY address, allowed link speeds, link duplex mode and MAC address.
    
    The following functions are used as part of the initialization and
    configuration process:
        - MSS_MAC_cfg_struct_def_init()
        - MSS_MAC_init()
        
    Transmit Operations
    The SmartFusion2 Ethernet MAC driver transmit operations are interrupt
    driven. The application must register a transmit call-back function with the
    driver using the MSS_MAC_set_tx_callback() function. This call-back function
    will be called by the Ethernet MAC driver every time a packet has been sent.
    The application must call the MSS_MAC_send_pkt() function every time it
    wants to transmit a packet. The application must pass a pointer to the
    buffer containing the packet to send. It is the application�s responsibility
    to manage the memory allocated to store the transmit packets. The Ethernet
    MAC driver only requires a pointer to the buffer containing the packet and
    the packet size. The Ethernet MAC driver will call the transmit call-back
    function registered using the MSS_MAC_set_tx_callback() function once a
    packet is sent. The transmit call-back function is supplied by the
    application and can be used to release the memory used to store the packet
    that was sent.
    The following functions are used as part of transmit and receive operations:
        - MSS_MAC_send_pkt()
        - MSS_MAC_set_tx_callback()
        
    Receive Operations
    The SmartFusion2 Ethernet MAC driver receive operations are interrupt
    driven. The application must first register a receive call-back function
    using the MSS_MAC_set_rx_callback() function. The application can then
    allocate receive buffers to the Ethernet MAC driver by calling the
    MSS_MAC_receive_pkt() function. This function can be called multiple times
    to allocate more than one receive buffer. The Ethernet MAC driver will then
    call the receive call-back whenever a packet is received into one of the
    receive buffer. It will hand back the receive buffer to the application for
    packet processing. This buffer will not be reused by the Ethernet MAC driver
    unless it is re-allocated to the driver by a call to MSS_MAC_receive_pkt().
    The following functions are used as part of transmit and receive operations:
        - MSS_MAC_receive_pkt()
        - MSS_MAC_set_rx_callback()
        
    Reading Status and Statistics
    The Ethernet MAC driver provides the following functions to retrieve the
    current link status and statistics.
        - MSS_MAC_get_link_status()
        - MSS_MAC_read_stat()

 *//*=========================================================================*/
#ifndef MSS_ETHERNET_MAC_H_
#define MSS_ETHERNET_MAC_H_

#include "mss_ethernet_mac_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * State definitions
 */
#define MSS_MAC_DISABLE                     ( (uint8_t)0 )
#define MSS_MAC_ENABLE                      ( (uint8_t)1 )

/*******************************************************************************
 * API Function return value definitions
 */
#define MSS_MAC_SUCCESS                     MSS_MAC_ENABLE
#define MSS_MAC_FAILED                      MSS_MAC_DISABLE
  
/********************************************************************************
  The following definitions are used with function MSS_MAC_get_link_status() to
  report the link�s status.
 */
#define MSS_MAC_LINK_DOWN                   0u
#define MSS_MAC_LINK_UP                     1u

#define MSS_MAC_HALF_DUPLEX                 0u
#define MSS_MAC_FULL_DUPLEX                 1u  

/*******************************************************************************
 * Broadcast MAC address 
 */
#define MSS_MAC_BROADCAST_MAC_ADDRESS       0xFF,0xFF,0xFF,0xFF,0xFF,0xFF

/*******************************************************************************
 * Maximum MAC frame size (packet size)
 */
#define MSS_MAC_MAX_PACKET_SIZE             1518u

/***************************************************************************//**
  The definition below is provided to specify that the MSS_MAC_init() function
  should attempt to discover the address of the PHY connected to the MAC�s
  management interface.It can be used with mss_mac_cfg_t configuration parameter 
  phy_addr and mss_mac_cfg_t as a parameter to a call to the MSS_MAC_init() function.
  
  Note: To auto detect the PHY address, this drivers scans the valid MDIO addresses 
  starting from �0� for valid data.If CoreRGMII/CoreSGMII and PHY are connected to
  the MAC�s management interface, then you must make sure that the PHY device
  mdio address is less than the CoreRGMII/CoreSGMII mdio address.

 */
#define MSS_MAC_AUTO_DETECT_PHY_ADDRESS     (uint8_t)255U

/*******************************************************************************
 * Per Packet Overrides masks in descriptor packet size parameters
 */
#define MSS_MAC_FIFO_TX_CTRL_FRAME          ( (uint32_t)0x00100000 )
#define MSS_MAC_FIFO_TX_NO_CTRL_FRAME       ( ~MSS_MAC_FIFO_TX_CTRL_FRAME )
#define MSS_MAC_FIFO_TX_PERPKT_PAD_FCS      ( (uint32_t)0x00040000 )
#define MSS_MAC_FIFO_TX_PERPKT_NO_PAD_FCS   ( ~MSS_MAC_FIFO_TX_PERPKT_PAD_FCS )
#define MSS_MAC_FIFO_TX_PERPKT_FCS          ( (uint32_t)0x00020000 )
#define MSS_MAC_FIFO_TX_PERPKT_NO_FCS       ( ~MSS_MAC_FIFO_TX_PERPKT_FCS )
#define MSS_MAC_FIFO_TX_PERPKT_ENABLE       ( (uint32_t)0x00010000 )
#define MSS_MAC_FIFO_TX_PERPKT_DISABLE      ( ~MSS_MAC_FIFO_TX_PERPKT_ENABLE )

/*******************************************************************************
 * Transmit and receive packet buffer sizes
 */
#define MSS_MAC_MAX_TX_BUF_SIZE                     MSS_MAC_MAX_PACKET_SIZE
#define MSS_MAC_MAX_RX_BUF_SIZE                     MSS_MAC_MAX_PACKET_SIZE

/*******************************************************************************
 * Defines for configuration parameters
 */
/* Auto-negotiation enable / disable */
#define MSS_MAC_AUTO_NEGOTIATION_ENABLE             MSS_MAC_ENABLE
#define MSS_MAC_AUTO_NEGOTIATION_DISABLE            MSS_MAC_DISABLE

/* FIFO error detection & correction enable / disable */
#define MSS_MAC_ERR_DET_CORR_ENABLE                 MSS_MAC_ENABLE
#define MSS_MAC_ERR_DET_CORR_DISABLE                MSS_MAC_DISABLE

/* Huge frame support enable / disable */
#define MSS_MAC_HUGE_FRAME_ENABLE                   MSS_MAC_ENABLE
#define MSS_MAC_HUGE_FRAME_DISABLE                  MSS_MAC_DISABLE

/* Length field checking enable / disable */
#define MSS_MAC_LENGTH_FILED_CHECK_ENABLE           MSS_MAC_ENABLE
#define MSS_MAC_LENGTH_FILED_CHECK_DISABLE          MSS_MAC_DISABLE

/* Padding and CRC enable / disable */
#define MSS_MAC_PAD_N_CRC_ENABLE                    MSS_MAC_ENABLE
#define MSS_MAC_PAD_N_CRC_DISABLE                   MSS_MAC_DISABLE

/* Append CRC enable / disable */
#define MSS_MAC_CRC_ENABLE                          MSS_MAC_ENABLE
#define MSS_MAC_CRC_DISABLE                         MSS_MAC_DISABLE

/* Fullduplex mode enable / disable */
#define MSS_MAC_FULLDUPLEX_ENABLE                   MSS_MAC_ENABLE
#define MSS_MAC_FULLDUPLEX_DISABLE                  MSS_MAC_DISABLE

/* Loopback mode enable / disable */
#define MSS_MAC_LOOPBACK_ENABLE                     MSS_MAC_ENABLE
#define MSS_MAC_LOOPBACK_DISABLE                    MSS_MAC_DISABLE

/* Receiver flow control enable / disable */
#define MSS_MAC_RX_FLOW_CTRL_ENABLE                 MSS_MAC_ENABLE
#define MSS_MAC_RX_FLOW_CTRL_DISABLE                MSS_MAC_DISABLE

/* Transmission flow control enable / disable */
#define MSS_MAC_TX_FLOW_CTRL_ENABLE                 MSS_MAC_ENABLE
#define MSS_MAC_TX_FLOW_CTRL_DISABLE                MSS_MAC_DISABLE

/* Excessive defer enable / disable */
#define MSS_MAC_EXSS_DEFER_ENABLE                   MSS_MAC_ENABLE
#define MSS_MAC_EXSS_DEFER_DISABLE                  MSS_MAC_DISABLE

/* No-backoff enable / disable */
#define MSS_MAC_NO_BACKOFF_ENABLE                   MSS_MAC_ENABLE
#define MSS_MAC_NO_BACKOFF_DISABLE                  MSS_MAC_DISABLE

/* Backpressure no-backoff enable / disable */
#define MSS_MAC_BACKPRESS_NO_BACKOFF_ENABLE         MSS_MAC_ENABLE
#define MSS_MAC_BACKPRESS_NO_BACKOFF_DISABLE        MSS_MAC_DISABLE

/* Alternative binary exponential backoff enable / disable */
#define MSS_MAC_ABEB_ENABLE                         MSS_MAC_ENABLE
#define MSS_MAC_ABEB_DISABLE                        MSS_MAC_DISABLE

/* Supress preamble enable / disable */
#define MSS_MAC_SUPPRESS_PREAMBLE_ENABLE            MSS_MAC_ENABLE
#define MSS_MAC_SUPPRESS_PREAMBLE_DISABLE           MSS_MAC_DISABLE

/* Auto-scan PHYs enable / disable */
#define MSS_MAC_PHY_AUTOSCAN_ENABLE                 MSS_MAC_ENABLE
#define MSS_MAC_PHY_AUTOSCAN_DISABLE                MSS_MAC_DISABLE

/* Preamble length default value and maximum value */
#define MSS_MAC_PREAMLEN_DEFVAL                     ( (uint8_t)(0x07) )
#define MSS_MAC_PREAMLEN_MAXVAL                     ( (uint8_t)(0x0F) )

/* Byte / Nibble mode  */
#define MSS_MAC_NIBBLE_MODE                         ( (uint8_t)(0x00) )
#define MSS_MAC_BYTE_MODE                           ( (uint8_t)(0x01) )

/* IPG/IFG values */
#define MSS_MAC_MINIFG_MAXVAL                       ( (uint8_t)(0xFF) )
#define MSS_MAC_MINIFG_DEFVAL                       ( (uint8_t)(0x50) )
#define MSS_MAC_BTBIFG_MAXVAL                       ( (uint8_t)(0x7F) )
#define MSS_MAC_BTBIFG_DEFVAL                       ( (uint8_t)(0x60) )
#define MSS_MAC_NONBTBIFG_DEFVAL                    ( (uint16_t)((0x4000u) | 0x0060u) )
#define MSS_MAC_NONBTBIFG_MAXVAL                    ( (uint16_t)(0x3FFF) )

/* Number of maximum retransmission tries */
#define MSS_MAC_MAXRETX_MAXVAL                      ( (uint8_t)(0x1F) )
#define MSS_MAC_MAXRETX_DEFVAL                      ( (uint8_t)(0x0F) )
#define MSS_MAC_ABEBTRUNC_MAXVAL                    ( (uint8_t)(0x0F) )
#define MSS_MAC_ABEBTRUNC_DEFVAL                    ( (uint8_t)(0x0A) )

/* PHY clock divider values */
#define MSS_MAC_DEF_PHY_CLK                         ( (uint8_t)(0x07) )
#define MSS_MAC_BY4_PHY_CLK                         ( (uint8_t)(0x01) )
#define MSS_MAC_BY6_PHY_CLK                         ( (uint8_t)(0x02) )
#define MSS_MAC_BY8_PHY_CLK                         ( (uint8_t)(0x03) )
#define MSS_MAC_BY10_PHY_CLK                        ( (uint8_t)(0x04) )
#define MSS_MAC_BY14_PHY_CLK                        ( (uint8_t)(0x05) )
#define MSS_MAC_BY20_PHY_CLK                        ( (uint8_t)(0x06) )
#define MSS_MAC_BY28_PHY_CLK                        ( (uint8_t)(0x07) )

/* Default & Maximum PHY addresses */
#define MSS_MAC_DEFAULT_PHY                         ( DP83848 )
#define MSS_MAC_PHYADDR_MAXVAL                      ( (uint8_t)(0x1F) )
#define MSS_MAC_PHYREGADDR_MAXVAL                   ( (uint8_t)(0x1F) )
#define MSS_MAC_PHY_ADDR_DEFVAL                     ( (uint8_t)(0x00) )
#define MSS_MAC_PHYREGADDR_DEFVAL                   ( (uint8_t)(0x00) )

/* Maximum frame length default & maximum values */
#define MSS_MAC_MAXFRAMELEN_DEFVAL                  ( (uint16_t)(0x0600) )
#define MSS_MAC_MAXFRAMELEN_MAXVAL                  ( (uint16_t)(0x0600) )

/* Slottime (Collision window) default & maximum values */
#define MSS_MAC_SLOTTIME_DEFVAL                     ( (uint16_t)(0x0037) )
#define MSS_MAC_SLOTTIME_MAXVAL                     ( (uint16_t)(0x03FF) )

/* Frame drop mask default value */
#define MSS_MAC_FRAME_DROP_MASK_DEFVAL              ( (uint32_t)(0x00000000) )

/*
 * Options for speed configuration.
 */
#define MSS_MAC_ANEG_10M_FD             0x00000001u
#define MSS_MAC_ANEG_10M_HD             0x00000002u
#define MSS_MAC_ANEG_100M_FD            0x00000004u
#define MSS_MAC_ANEG_100M_HD            0x00000008u
#define MSS_MAC_ANEG_1000M_FD           0x00000010u
#define MSS_MAC_ANEG_1000M_HD           0x00000020u
#define MSS_MAC_ANEG_ALL_SPEEDS         (MSS_MAC_ANEG_10M_FD | MSS_MAC_ANEG_10M_HD | \
                                         MSS_MAC_ANEG_100M_FD | MSS_MAC_ANEG_100M_HD | \
                                         MSS_MAC_ANEG_1000M_FD | MSS_MAC_ANEG_1000M_HD)

/**************************************************************************/
/* Public Function declarations                                           */
/**************************************************************************/

/***************************************************************************//**
  The MSS_MAC_cfg_struct_def_init() function initializes a mss_mac_cfg_t
  configuration data structure to default values. The default configuration uses
  the MII interface connected to a PHY at address 0x00 which is set to
  auto-negotiate at all available speeds up to 1000Mbps. This default
  configuration can then be used as parameter to MSS_MAC_init(). Typically the
  default configuration would be modified to suit the application before being
  passed to MSS_MAC_init(). 
 
  @param cfg
  This parameter is a pointer to a mss_mac_cfg_t data structure that will be used as
  parameter to function MSS_MAC_init().
 
  @return
  This function does not return a value.

  Example:
  The example below demonstrates the use of the MSS_MAC_cfg_struct_def_init()
  function. It retrieves the default MAC configuration and modifies it to
  connect through an MII Ethernet PHY at MII management interface address 0x01.
  This example also demonstrates how to assign the device's MAC address and
  force a 100Mbps full duplex link.
  @code
    mss_mac_cfg_t mac_config;
    
    MSS_MAC_cfg_struct_def_init(&mac_config);

    mac_config.phy_addr = 0x01;
    mac_config.speed_duplex_select = MSS_MAC_ANEG_100M_FD;
    mac_config.mac_addr[0] = 0xC0u;
    mac_config.mac_addr[1] = 0xB1u;
    mac_config.mac_addr[2] = 0x3Cu;
    mac_config.mac_addr[3] = 0x88u;
    mac_config.mac_addr[4] = 0x88u;
    mac_config.mac_addr[5] = 0x88u;
    
    MSS_MAC_init(&mac_config);
  @endcode
 */
void 
MSS_MAC_cfg_struct_def_init
(
    mss_mac_cfg_t * cfg
);

/***************************************************************************//**
  The MSS_MAC_init() function initializes the Ethernet MAC hardware and driver
  internal data structures. The MSS_MAC_init() function takes a pointer to a
  configuration data structure of type mss_mac_cfg_t as parameter. This
  configuration data structure contains all the information required to
  configure the Ethernet MAC. The MSS_MAC_init() function initializes the
  descriptor rings and their pointers to initial values. The MSS_MAC_init()
  function enables DMA Rx packet received and Tx packet sent interrupts.
  The configuration passed to the MSS_MAC_init() function specifies the type of
  interface used to connect the Ethernet MAC and Ethernet PHY as well as the PHY
  MII management interface address. It also specifies the allowed link speed and
  duplex mode. It is at this point that the application chooses if the link
  speed and duplex mode will be auto-negotiate with the link partner or forced
  to a specific speed and duplex mode.
 
  @param cfg
   This parameter is a pointer to a data structure of type mss_mac_cfg_t containing
   the Ethernet MAC�s requested configuration. You must initialize this data
   structure by first calling the MSS_MAC_cfg_struct_def_init() function to fill
   the configuration data structure with default values. You can then overwrite
   some of the default settings with the ones specific to your application
   before passing this data structure as parameter to the call to the
   MSS_MAC_init() function. You must at a minimum overwrite the mac_addr[6]
   array of the configuration data structure to contain a unique value used as
   the device�s MAC address. 

  @return
    This function does not return a value.

  Example:
  @code
    mss_mac_cfg_t cfg;

    MSS_MAC_cfg_struct_def_init(&cfg);
    
    cfg.mac_addr[0] = 0xC0u;
    cfg.mac_addr[1] = 0xB1u;
    cfg.mac_addr[2] = 0x3Cu;
    cfg.mac_addr[3] = 0x88u;
    cfg.mac_addr[4] = 0x88u;
    cfg.mac_addr[5] = 0x88u;

    MSS_MAC_init(&cfg);
  @endcode
 */
void 
MSS_MAC_init
( 
    mss_mac_cfg_t * cfg 
);

/***************************************************************************//**
  The MSS_MAC_set_tx_callback() function registers the function that will be
  called by the Ethernet MAC driver when a packet has been sent.
 
  @param tx_complete_handler
    This parameter is a pointer to the function that will be called when a
    packet is sent by the Ethernet MAC.
  
  @return
    This function does not return a value.

 */
void MSS_MAC_set_tx_callback
(
    mss_mac_transmit_callback_t tx_complete_handler
);

/***************************************************************************//**
  The MSS_MAC_set_rx_callback() function registers the function that will be
  called by the Ethernet MAC driver when a packet is received.
  
  @param rx_callback
    This parameter is a pointer to the function that will be called when the a
    packet is received by Ethernet MAC.
  
  Example:
  The example below demonstrates the use of the MSS_MAC_set_rx_callback()
  function. The init() function calls the MSS_MAC_set_rx_callback() function to
  register the rx_callback() receive callback function with the Ethernet MAC
  driver. The MSS_MAC_receive_pkt() function is then called to assign the
  rx_buffer to an Ethernet MAC descriptor for packet reception. The rx_callback
  function will be called by the Ethernet MAC driver once a packet has been
  received into rx_buffer. The rx_callback() function calls the
  process_rx_packet() application function to process the received packet then
  calls MSS_MAC_receive_pkt() to reallocate rx_buffer to receive another packet.
  The rx_callback() function will be called again every time a packet is
  received to process the received packet and reallocate rx_buffer for packet
  reception.
  
  @code
    uint8_t rx_buffer[MSS_MAC_MAX_RX_BUF_SIZE];
    
    void rx_callback
    (
        uint8_t * p_rx_packet,
        uint32_t pckt_length,
        void * p_user_data
    )
    {
        process_rx_packet(p_rx_packet, pckt_length);
        MSS_MAC_receive_pkt(rx_buffer, (void *)0);
    }
    
    void init(void)
    {
        MSS_MAC_set_rx_callback(rx_callback);
        MSS_MAC_receive_pkt(rx_buffer, (void *)0);
    }
  @endcode
 */
void MSS_MAC_set_rx_callback
(
    mss_mac_receive_callback_t rx_callback
);

/***************************************************************************//**
  The MSS_MAC_send_pkt()function initiates the transmission of a packet. It
  places the buffer containing the packet to send into one of the Ethernet MAC�s
  transmit descriptors. This function is non-blocking. It will return
  immediately without waiting for the packet to be sent. The Ethernet MAC driver
  indicates that the packet is sent by calling the transmit completion handler
  registered by a call to MSS_MAC_set_tx_callback().
  
  @param tx_buffer
    This parameter is a pointer to the buffer containing the packet to send.
    
  @param tx_length
    This parameter specifies the length in bytes of the packet to send.
    
  @param p_user_data
    This parameter is a pointer to an optional application defined data
    structure. Its usage is left to the application. It is intended to help the
    application manage memory allocated to store packets. The Ethernet MAC
    driver does not make use of this pointer. The Ethernet MAC driver will pass
    back this pointer to the application as part of the call to the transmit
    completion handler registered by the application.
    
  @return
    This function returns 1 on successfully assigning the packet to a 
    transmit descriptor. It returns 0 otherwise.

  Example:
  This example demonstrates the use of the MSS_MAC_send_pkt() function. The
  application registers the tx_complete_callback() transmit completion callback
  function with the Ethernet MAC driver by a call to
  MSS_MAC_set_tx_callback(). The application dynamically allocates memory for an
  application defined packet_t data structure, builds a packet and calls
  send_packet(). The send_packet() function extracts the pointer to the buffer
  containing the data to transmit and its length from the tx_packet data
  structure and passes these to MSS_MAC_send_pkt(). It also passes the pointer
  to tx_packet as the p_user_data parameter. The Ethernet MAC driver call
  tx_complete_callback() once the packet is sent. The tx_complete_callback()
  function uses the p_user_data, which points to tx_packet, to release memory
  allocated by the application to stored the transmit packet.
  @code
    
    void tx_complete_handler(void * p_user_data);
    
    void init(void)
    {
        MSS_MAC_set_tx_callback(tx_complete_handler);
    }
    
    void tx_complete_handler(void * p_user_data)
    {
        release_packet_memory(p_user_data);
    }
    
    void send_packet(app_packet_t * packet)
    {
        MSS_MAC_send_pkt(packet->buffer, packet->length, packet);
    }
    
  @endcode
 */
uint8_t 
MSS_MAC_send_pkt
(
    uint8_t const *   tx_buffer,
    uint32_t    tx_length,
    void *      p_user_data
);

/***************************************************************************//**
  The MSS_MAC_receive_pkt() function assigns a buffer to one of  the Ethernet
  MAC�s receive descriptors. The receive buffer specified as parameter will be
  used to receive one single packet. The receive buffer will be handed back to
  the application via a call to the receive callback function assigned through a
  call to MSS_MAC_set_rx_callback(). The MSS_MAC_receive_pkt() function will
  need to be called again pointing to the same buffer if more packets are to be
  received into this same buffer after the packet has been processed by the
  application.
  The MSS_MAC_receive_pkt() function is non-blocking. It will return immediately
  and does not wait for a packet to be received. The application needs to
  implement a receive callback function to be notified that a packet has been
  received.
  The p_user_data parameter can be optionally used to point to a memory
  management data structure managed by the application.

 
  @param rx_pkt_buffer
    This parameter is a pointer to a memory buffer. It points to the memory that
    will be assigned to one of the Ethernet MAC�s receive descriptors. It must
    point to a buffer large enough to contain the largest possible packet.
  
  @param p_user_data
    This parameter is intended to help the application manage memory. Its usage
    is left to the application. The Ethernet MAC driver does not make use of
    this pointer. The Ethernet MAC driver will pass this pointer back to the
    application as part of the call to the application�s receive callback
    function to help the application associate the received packet with the
    memory it allocated prior to the call to MSS_MAC_receive_pkt().
 
  @return
    This function returns 1 on successfully assigning the buffer to a receive
    descriptor. It returns 0 otherwise.
  
  Example:
  The example below demonstrates the use of the MSS_MAC_receive_pkt() function
  to handle packet reception using two receive buffers. The init() function
  calls the MSS_MAC_set_rx_callback() function to register the rx_callback()
  receive callback function with the Ethernet MAC driver. The
  MSS_MAC_receive_pkt() function is then called twice to assign rx_buffer_1 and
  rx_buffer_2 to Ethernet MAC descriptors for packet reception. The rx_callback
  function will be called by the Ethernet MAC driver once a packet has been
  received into one of the receive buffers. The rx_callback() function calls the
  process_rx_packet() application function to process the received packet then
  calls MSS_MAC_receive_pkt() to reallocate the receive buffer to receive
  another packet. The rx_callback() function will be called again every time a
  packet is received to process the received packet and reallocate rx_buffer for
  packet reception.
  Please note the use of the p_user_data parameter to handle the buffer
  reassignment to the Ethernet MAC as part of the rx_callback() function. This
  is a simplistic use of p_user_data. It is more likely that p_user_data would
  be useful to keep track of a pointer to a TCP/IP stack packet container data
  structure dynamically allocated. In this more complex use case, the first
  parameter of MSS_MAC_receive_pkt() would point to the actual receive buffer
  and the second parameter would point to a data structure used to free the
  receive buffer memory once the packet has been consumed by the TCP/IP stack.

  @code
    uint8_t rx_buffer_1[MSS_MAC_MAX_RX_BUF_SIZE];
    uint8_t rx_buffer_2[MSS_MAC_MAX_RX_BUF_SIZE];
    
    void rx_callback
    (
        uint8_t * p_rx_packet,
        uint32_t pckt_length,
        void * p_user_data
    )
    {
        process_rx_packet(p_rx_packet, pckt_length);
        MSS_MAC_receive_pkt((uint8_t *)p_user_data, p_user_data);
    }
    
    void init(void)
    {
        MSS_MAC_set_rx_callback(rx_callback);
        MSS_MAC_receive_pkt(rx_buffer_1, (void *)rx_buffer_1);
        MSS_MAC_receive_pkt(rx_buffer_2, (void *)rx_buffer_2);
    }
  @endcode
 */
uint8_t
MSS_MAC_receive_pkt
(
    uint8_t * rx_pkt_buffer,
    void * p_user_data
);

/***************************************************************************//**
  The MSS_MAC_get_link_status () function retrieves the status of the link from
  the Ethernet PHY. It returns the current state of the Ethernet link. The speed
  and duplex mode of the link is also returned via the two pointers passed as
  parameter if the link is up.
  This function also adjusts the Ethernet MAC�s internal configuration if some
  of the link characteristics have changed since the previous call to this
  function.
  
  @param speed
    This parameter is a pointer to variable of type mss_mac_speed_t where the
    current link speed will be stored if the link is up. This variable is not
    updated if the link is down. This parameter can be set to zero if the caller
    does not need to find out the link speed.
  
  @param fullduplex
    This parameter is a pointer to an unsigned character where the current link
    duplex mode will be stored if the link is up. This variable is not updated
    if the link is down.
  
  @return
    This function returns 1 if the link is up. It returns 0 if the link is down.
  
  Example:
  @code
    uint8_t link_up;
    mss_mac_speed_t speed;
    uint8_t full_duplex
    link_up = MSS_MAC_get_link_status(&speed, &full_duplex);
  @endcode
 */
uint8_t MSS_MAC_get_link_status
(
    mss_mac_speed_t * speed,
    uint8_t *     fullduplex
);


/***************************************************************************//**
  The MSS_MAC_read_stat()  function reads the transmit and receive statistics of
  the Ethernet MAC. This function can be used to read one of 17 receiver
  statistics, 20 transmitter statistics and 7 frame type statistics as defined
  in the mss_mac_stat_t enumeration. 
 
  @param stat
    This parameter of type mss_mac_stat_t identifies the statistic that will be
    read.

  @return
    This function returns the value of the requested statistic.

  Example:
  @code
    uint32_t tx_pkts_cnt = MSS_MAC_read_stat(MSS_MAC_TX_PKT_CNT);
  @endcode
 */
uint32_t 
MSS_MAC_read_stat
(
    mss_mac_stat_t stat
);

/***************************************************************************//**
  @brief MSS_MAC_clear_statistics()
    The MSS_MAC_clear_statistics() function clears all the statistics counter
    registers.
    
  @param
    This function does not take any parameters
    
  @return
    This function does not return a value.
 */
void MSS_MAC_clear_statistics
(
    void
);

/***************************************************************************//**
  The MSS_MAC_read_phy_reg() function reads the Ethernet PHY register specified
  as parameter. It uses the MII management interface to communicate with the
  Ethernet PHY. This function is used part of the Ethernet PHY drivers provided
  alongside the Ethernet MAC driver. You only need to use this function if
  writing your own Ethernet PHY driver.
 
  @param phyaddr
    This parameter is the 5-bit address of the Ethernet PHY on the MII
    management interface. This address is typically defined through Ethernet PHY
    hardware configuration signals. Please refer to the Ethernet PHY�s datasheet
    for details of how this address is assigned.
 
  @param regaddr
    This parameter is the address of the Ethernet register that will be read.
    This address is the offset of the register within the Ethernet PHY�s
    register map.

  @return 
    This function returns the 16-bit value of the requested register.

  Example:
  @code
    #include �phy.h�
    uint16_t read_phy_status(uint8_t phy_addr)
    {
        uint16_t phy_status = MSS_MAC_read_phy_reg(phy_addr , MII_BMSR);
        return phy_status;
    }
  @endcode
 */
uint16_t
MSS_MAC_read_phy_reg
(
    uint8_t phyaddr,
    uint8_t regaddr
);

/***************************************************************************//**
  The MSS_MAC_write_phy_reg() function writes a 16-bit value to the specified
  Ethernet PHY register. . It uses the MII management interface to communicate
  with the Ethernet PHY. This function is used part of the Ethernet PHY drivers
  provided alongside the Ethernet MAC driver. You only need to use this function
  if writing your own Ethernet PHY driver.
 
  @param phyaddr
    This parameter is the 5-bit address of the Ethernet PHY on the MII
    management interface. This address is typically defined through Ethernet PHY
    hardware configuration signals. Please refer to the Ethernet PHY�s datasheet
    for details of how this address is assigned.
 
  @param regaddr
    This parameter is the address of the Ethernet register that will be written.
    This address is the offset of the register within the Ethernet PHY�s
    register map.

  @param regval
    The parameter is the 16-bit value that will be written into the specified
    PHY register.

  @return
    This function does not return a value.

  Example:
  @code
    #include "mss_ethernet_sgmii_phy.h"
    #include "phy.h"

    void rest_sgmii_phy(void)
    {
        MSS_MAC_write_phy_reg(SGMII_PHY_ADDR, MII_BMCR, 0x8000);
    }
  @endcode
 */
void 
MSS_MAC_write_phy_reg
(
    uint8_t phyaddr,
    uint8_t regaddr,
    uint16_t regval
);

//210621 CJKIM Added
void
MSS_MAC_enable_intr
(
);

void
MSS_MAC_disable_intr
(
);


#ifdef __cplusplus
}
#endif

#endif /* MSS_ETHERNET_MAC_H_ */
