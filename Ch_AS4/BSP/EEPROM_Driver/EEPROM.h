/****************************************************************************
* Company: Microsemi Corporation
*
* File: EEPROM.h
* File history:
*     Revision: Initial version   Date: May 04, 2010
*
* Description:
*             EEPROM(M24512-R from ST) driver.
*             It defines the Initialisation, Write, read and delay functions
*
* Author: Pavan Marisetti
*         pavan.marisetti@microsemi.com
*         Corporate Applications Engineering
*
****************************************************************************/

#ifndef EEPROM_H_
#define EEPROM_H_

#include "bsp_config.h"
#include "mss_i2c.h"

/* I2C master instance, This can be changed if the EEPROM is connected to
I2C0 on the user custom board*/

#define I2C_INSTANCE            &g_mss_i2c1
#define I2C_INSTANCE_0            &g_mss_i2c0
//#define I2C_INSTANCE            &g_mss_i2c0
//#define I2C_INSTANCE_0            &g_mss_i2c0


//===============================================================================================
//===============================================================================================
/*
  <<MSS I2C �ʱ�ȭ ��ƾ>>
-------------------------------------------------- ----------------------------
	MSS_I2C_init () �Լ��� �ϵ���� �� �����͸� �ʱ�ȭ�ϰ� �����մϴ�
	SmartFusion MSS I2C �� �ϳ��� ����.
-------------------------------------------------- ----------------------------
	@param this_i2c :
		this_i2c �Ű� ������ �ʱ�ȭ �� MSS I2C �ϵ���� ����� �ĺ��ϴ� mss_i2c_instance_t ������ ���� �������Դϴ�. 
		MSS I2C 0 �� MSS I2C 1�� ���� ���õ� g_mss_i2c0 �� g_mss_i2c1�� ���� �� ���� ������ ������ �ֽ��ϴ�. 
		�� �Ű� ������ I2C ����̹��� ���� �� g_mss_i2c0 �Ǵ� g_mss_i2c1 ���� ������ ������ ������ ���մϴ�.

	@param ser_address :
		�� �Ű� ������ �ʱ�ȭ�Ǵ� MSS I2C �ֺ� ��ġ�� I2C ���� �ּҸ� �����մϴ�. 
		MSS I2C �ν��Ͻ��� �����ϴ� I2C ���� �ּ��Դϴ�.
		MSS I2C �ֺ� ��ġ�� ������ �Ǵ� �����̺� ��忡�� �۵� �� �� �ֽ��ϴ�.
		���� �ּҴ� I2C �����̺� ����� ��쿡�� �߿��մϴ�.
		������ ��忡�� MSS I2C���� ���� �ּҰ� �ʿ����� �������� �Ű� ������ ���� �߿����� �ʽ��ϴ�. 
		�����̺� ��忡�� I2C ��ġ�� ������� �������� ���� �����̺� �ּ� ������ �Ű� ������ ���� �� �� �ֽ��ϴ�. 
		�׷��� MSS I2C�� ������ ��忡�� �����̺� ���� ��ȯ �� ������ ����Ǵ� �ý��ۿ����� ��ȿ�� ���� �����̺� �ּҷ� MSS I2C ��ġ�� �ʱ�ȭ�ϴ� ���� �����ϴ�.

***�߿�
***�߿�
***�߿�
		I2C ��ġ�� �����̺� �ּҸ� �����ϴ� ������ �Լ��� �����Ƿ� �����̺� �ּҸ� �����ؾ� �� ������ MSS_I2C_init () �Լ��� ȣ���ؾ��մϴ�.
*/		
//===============================================================================================
//===============================================================================================
// ******* PCF8574 Slave address

// 20200514
// new Board
#define PCF8574_SLAVE_ADDRESS		0x20
// 20201020
#define PCF8574_TRAY_SEN_SLAVE_ADDRESS		0x20
#define PCF8574_ROTARY_SLAVE_ADDRESS		0x22
#define PCF8574_LED_CON_SLAVE_ADDRESS		0x21

void PCF8574_init( void );

mss_i2c_status_t I2C_PCF8574_read
(
    uint8_t serial_addr,
    uint8_t * rx_buffer,
    uint8_t read_length
);

mss_i2c_status_t I2C_PCF8574_write
(
    uint8_t serial_addr,
    uint8_t * write_buffer,
    uint8_t write_size
);

void PCF8574_TRAY_SEN_init( void );
void PCF8574_ROTARY_init( void );
void PCF8574_LED_CON_init( void );


#define RTC_BQ32002DR_SLAVE_ADDRESS		0x68

#define RTC_REG_ADDR_LEN_IN_BYTES			1

	#define RTC_SECONDS_REG_ADDR					0x00
	#define RTC_MINUTES_REG_ADDR					0x01
	#define RTC_CENT_HOURS_REG_ADDR				0x02
	#define RTC_DAY_REG_ADDR					0x03
		#define	RTC_DAY_SUNDAY						1
		#define	RTC_DAY_MONDAY						2		
		#define	RTC_DAY_TUESDAY						3
		#define	RTC_DAY_WEDNESDAY					4
		#define	RTC_DAY_THURSDAY					5
		#define	RTC_DAY_FRIDAY						6		
		#define	RTC_DAY_SATURDAY					7
	#define RTC_DATE_REG_ADDR					0x04
	#define RTC_MONTH_REG_ADDR				0x05
	#define RTC_YEARS_REG_ADDR					0x06
	#define RTC_CAL_CFG1_REG_ADDR					0x07
	
	#define RTC_CFG2_REG_ADDR					0x09

	#define RTC_SFKET1_REG_ADDR					0x20
	#define RTC_SFKET2_REG_ADDR					0x21
	#define RTC_SFR_REG_ADDR						0x22

void RTC_BQ32002DR_init( void );


mss_i2c_status_t rtcWrite_byte
(
    uint8_t reg_address,
    uint8_t data
);

mss_i2c_status_t rtcWrite_time
(
    uint8_t *data
);

mss_i2c_status_t rtcRead_byte
(
    uint8_t reg_address,
    uint8_t *read_data
);

mss_i2c_status_t rtcRead_time
(
    uint8_t reg_address,
    uint8_t *read_data
);


//===============================================================================================
//===============================================================================================
//******* EEPROM Slave address
//******* �����߿�......
//******* mss_i2c����̹� 3.0���� ���������� �Ѻ�Ʈ ����Ʈ�ϱ��� ��巹�� ���..
//******* 3���� ��޴�..... ����̹��� �̵����� ������....
//*******#define EEPROM_SLAVE_ADDRESS    0xA0
#define EEPROM_SLAVE_ADDRESS    0x50
//===============================================================================================
//===============================================================================================
/* I2C Clock frequency*/
#define EEPROM_I2C_CLK_FREQ    MSS_I2C_PCLK_DIV_256
/* EEPROM Page size in bytes*/
#if MY_EEPROM_DEVICE == AT24C256
	#define PAGE_SIZE_IN_BYTES      64		// At24C256		256Kbit == 32KByte
#elif MY_EEPROM_DEVICE == M24512
	#define PAGE_SIZE_IN_BYTES      128		// M24512			512Kbit == 64KByte
#endif	

/* EEPROM Address lenght in bytes*/
#define ADDR_LEN_IN_BYTES       2
/* EEPROM Address first location*/
#define ADDR_FIRST_LOC          0
/* EEPROM Address second location*/
#define ADDR_SECOND_LOC         1

mss_i2c_status_t I2C_STATUS;

#define DEMO_I2C_TIMEOUT 			3000u	// 3000u

//===============================================================================================
// AutoSampler Adjustment Data
// Address 0x00~0x7fff			==> 256Kbit == 32KByte
//===============================================================================================
#define DATA_SAVE_FLAG						0xA5

#define HOME_ADJUST_DATA_STARTADDR			0x0400		// 36byte
// 20181010
#define	EEPROM_ADJUST_DATA_SAVE_FIX				1
#if	EEPROM_ADJUST_DATA_SAVE_FIX
	#define HOME_ADJUST_2ND_STARTADDR			0x0450		// 
	#define HOME_ADJUST_CHECKSUM_STARTADDR			0x04C0		// 	

	#define HOME_ADJUST_VERSION_CHECK_ADDR			0x04E0		// 8byte	" 1.1.0 c"  "xx.x.x c" �ǵڿ� Checksum
	#define HOME_ADJUST_VERSION_CHECK_SIZE			4 

	#define FIRMWARE_VERSION_ADDR			0x04F0		// 8byte	" 1.1.0 c"  "xx.x.x c" �ǵڿ� Checksum
	#define FIRMWARE_VERSION_SIZE			8 

	
// 0x12 0x34 0xaa 0x55 �̸� ���ο� �߿��� �� ����� ������ �Ѵ�.
	#define DATA_SAVE_CONFIRM_ADDR			0x04F0		// 2byte	
#endif

#define MAC_ADDR_READ_LEN				7
#define MAC_ADDRESS_ADDR_SAVE		0x0500
#define MAC_ADDRESS_ADDR_0			0x0501
#define MAC_ADDRESS_ADDR_1			0x0502
#define MAC_ADDRESS_ADDR_2			0x0503
#define MAC_ADDRESS_ADDR_3			0x0504
#define MAC_ADDRESS_ADDR_4			0x0505
#define MAC_ADDRESS_ADDR_5			0x0506

#define IP_ADDR_READ_LEN				5
#define AS_IP_ADDRESS_ADDR_SAVE	0x0510
#define AS_IP_ADDRESS_ADDR0			0x0511
#define AS_IP_ADDRESS_ADDR1			0x0512
#define AS_IP_ADDRESS_ADDR2			0x0513
#define AS_IP_ADDRESS_ADDR3			0x0514

#define SYSTEM_CONFIG_DATA_SAVE		0x540
#define SYSTEM_CONFIG_DATA				0x541		// 44byte

#define SPECIAL_DATA_SAVE				0x05F0
#define SPECIAL_DATA_ADDR				0x05F1


//#define SYSTEM_INFORM_DATA_SAVE			0x0600	
//#define SYSTEM_INFORM_DATA_ADDR			0x0601	
#define SYSTEM_INFORM_DATA_SAVE			0x0610	
#define SYSTEM_INFORM_DATA_ADDR			(SYSTEM_INFORM_DATA_SAVE + 1)
#define SYSTEM_INFORM_DATA_READSIZE		20
#define SYSTEM_INFORM_DATA_READ_SERIAL		17
#define SYSTEM_INFORM_DATA_READ_INSTALLDATE		3

#define USED_TIME_DATA_SAVE			0x0700
#define USED_TIME_DATA_ADDR			0x0701

#define TEMP_CAL_DATA_SAVE			0x0800
#define TEMP_CAL_DATA_ADDR			0x0801

#define ETHERNET_CONFIG_DATA_SAVE			0x0840	
#define ETHERNET_CONFIG_DATA_ADDR			(ETHERNET_CONFIG_DATA_SAVE + 1)
#define ETHERNET_CONFIG_DATA_READSIZE		20
//#define ETHERNET_CONFIG_DATA_READ_SERIAL		17
//#define ETHERNET_CONFIG_DATA_READ_INSTALLDATE		3



//===================================================================================
// TEMP_CALIBRATION DATA
#define TEMP_CAL_DATA_SAVE_ADDR			0x0A00				// 1 byte	- saveFlag 0xA5
#define TEMP_CAL_DATA_SET_1ST_ADDR			0x0A04		// 4 byte	- float Set[0]	- #define TEMP_CAL_SET_1ST					35.0
#define TEMP_CAL_DATA_SET_2ST_ADDR			0x0A08		// 4 byte	- float Set[1]	- #define TEMP_CAL_SET_2ND					60.0
#define TEMP_CAL_DATA_MEA_1ST_ADDR			0x0A0C		// 4 byte	- float Mea[0]
#define TEMP_CAL_DATA_MEA_2ST_ADDR			0x0A10		// 4 byte	- float Mea[1]


// RTC_INIT_SAVE_ADDR - �ʱ�ȭ�� �����̸� MAC_ADDRESS �ֱ�ȭ ����
#define RTC_INIT_SAVE_ADDR							0x0B00		// ó������ �����ΰ�
#define MAC_ADDRESS_INIT_SAVE_ADDR			0x0B10		// 
#define IP_ADDRESS_INIT_SAVE_ADDR				0x0B20		// USER Input

/***************************************************************************//**
 This enumeration is used to specific EEPROM Write status
 */
typedef enum
{
    EEPROM_WRITE_SUCCESS = 0,
    EEPROM_READ_SUCCESS,
    EEPROM_WRITE_UNSUCCESS,
    EEPROM_READ_UNSUCCESS,
    EEPROM_ADDRESS_FAULT
} EEPROM_status_t;

/****************************************************************************
  The EEPROM_initfunction initializes the MSS_I2C to communicate with the
  EEPROM.
 */
void EEPROM_init( void );

/***************************************************************************/
/*
  The EEPROM_write() function is used by MSS I2C masters to transmit data
  organized as a specified number of bytes, which are mentioned in the EEPROM
  spec(M24512-R).
  @param start_address
    The start_address parameter specifies the start address of the EEPROM, where
    data has to be written

  @param size_in_bytes
    The size_in_bytes parameter specifies the number of bytes to be written to
    EEPROM from write_buffer.

  @param write_buffer
    The write_buffer parameter is a pointer to the buffer from where the data
    has to be transmitted

    Note 1:   The summation of start_address and size_in_bytes should be in the
    range of 1 to 65536

    Note 2:   The start_address should be in the range of 0 to 65536

    Note 3:   The size_in_bytes should be in the range of 1 to 65536


  EEPROM write example:
  @code
int main()
{
    uint16_t i, start_address=0;
    uint8_t write_data[300];
    MSS_WD_disable();
    EEPROM_init();

    for(i=0;i<128;i++)
    {
        write_data[i] = 0x44;
    }
    for(i=128;i<256;i++)
    {
        write_data[i] = 0x55;
    }
    for(i=256;i<300;i++)
    {
        write_data[i] = 0x66;
    }

    EEPROM_write(start_address,300,write_data);


    return(0);
}

@endcode */

EEPROM_status_t
eepromWrite_byte
(
    uint16_t start_address,
    uint8_t * write_buffer
);

EEPROM_status_t
eepromWrite_Data
(
    uint16_t start_address,
    uint16_t size_in_bytes,    
    uint8_t * write_buffer
);


EEPROM_status_t
EEPROM_write
(
    uint16_t start_address,
    uint16_t size_in_bytes,
    uint8_t  * write_buffer
);

/***************************************************************************/
/*
  The EEPROM_read() function is used by MSS I2C masters to receive the data
  as specified by the number of bytes.

  @param start_address
    The start_address parameter specifies the start address of the EEPROM, from
    where the data has to be received

  @param size_in_bytes
    The size_in_bytes parameter specifies the number of bytes to be read from
    EEPROM to read_buffer.

  @param read_buffer
    The read_buffer parameter is a pointer to the buffer to where the data
    has to be received

    Note 1:   The summation of start_address and size_in_bytes should be in the
    range of 1 to 65536

    Note 2:   The start_address should be in the range of 0 to 65536

    Note 3:   The size_in_bytes should be in the range of 1 to 65536


  EEPROM read example:
  @code
int main()
{
    uint16_t i, start_address=0;
    uint8_t read_data[300];
    MSS_WD_disable();
    EEPROM_init();

    EEPROM_read(start_address,300,read_data);

    return(0);
}

@endcode */

EEPROM_status_t
eepromRead_byte
(
    uint16_t start_address,
    uint8_t  * read_buffer
);

EEPROM_status_t
eepromRead_Data
(
    uint16_t start_address,
    uint16_t size_in_bytes,
    uint8_t  * read_buffer
);

EEPROM_status_t
EEPROM_read
(
    uint16_t start_address,
    uint16_t size_in_bytes,
    uint8_t * read_buffer
);

/***************************************************************************/
/*
  The delay_btw_transfers ( volatile uint32_t n) function is used provide
  delay between two transfers (delay between stop and start conditions)
*/
void delay_btw_transfers ( volatile uint32_t n);

int eepromWrite_Data_Verify(uint16_t start_address, uint16_t size_in_bytes, uint8_t * write_buffer);
int eepromWrite_Data_Verify_2nd(uint16_t start_address, uint16_t size_in_bytes, uint8_t * write_buffer);
#endif
