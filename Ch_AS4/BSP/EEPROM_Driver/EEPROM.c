/****************************************************************************
* Company: Actel Corporation
*
* File: EEPROM.c
* File history:
*     Revision: Initial version   Date: May 04, 2010
*
* Description:
*             EEPROM(M24512-R from ST) driver.
*             It handles the write and read operation
*
* Author: Pavan Marisetti
*         pavan.marisetti@actel.com
*         Corporate Applications Engineering
*
****************************************************************************/

#include "config.h"

#include "EEPROM.h"
//#include "debug_inform.h"

void PCF8574_init()
{
// MASTER Mode
	MSS_I2C_init( I2C_INSTANCE, PCF8574_SLAVE_ADDRESS, MSS_I2C_PCLK_DIV_256 );
}

//static uint8_t g_master_rx_buf[BUFFER_SIZE];
//instance = do_read_transaction(SLAVE_SER_ADDR, g_master_rx_buf, sizeof(g_master_rx_buf));
//mss_i2c_status_t do_read_transaction
mss_i2c_status_t PCF8574_read
(
//    uint8_t serial_addr,
    uint8_t * rx_buffer,
    uint8_t read_length
)
{
    mss_i2c_status_t status;

//    char buf[5];
//    buf[1] = LED_StateData & 0xff;
//    buf[0] = (LED_StateData >> 8) & 0xff;
    
//    MSS_I2C_read(I2C_MASTER, serial_addr, rx_buffer, read_length, MSS_I2C_RELEASE_BUS);
    MSS_I2C_read(I2C_INSTANCE, PCF8574_SLAVE_ADDRESS, rx_buffer, read_length, MSS_I2C_RELEASE_BUS);    

    status = MSS_I2C_wait_complete(I2C_INSTANCE, DEMO_I2C_TIMEOUT);

    return status;
}


mss_i2c_status_t I2C_PCF8574_read
(
    uint8_t serial_addr,
    uint8_t * rx_buffer,
    uint8_t read_length
)
{
    mss_i2c_status_t status;

//    MSS_I2C_read(I2C_MASTER, serial_addr, rx_buffer, read_length, MSS_I2C_RELEASE_BUS);
//    MSS_I2C_read(I2C_INSTANCE, PCF8574_SLAVE_ADDRESS, rx_buffer, read_length, MSS_I2C_RELEASE_BUS);
    MSS_I2C_read(I2C_INSTANCE, serial_addr, rx_buffer, read_length, MSS_I2C_RELEASE_BUS);

    status = MSS_I2C_wait_complete(I2C_INSTANCE, DEMO_I2C_TIMEOUT);

    return status;
}

mss_i2c_status_t I2C_PCF8574_write
(
    uint8_t serial_addr,
    uint8_t * write_buffer,
    uint8_t write_size
)
{
    mss_i2c_status_t status;

    MSS_I2C_write(I2C_INSTANCE, serial_addr, write_buffer, write_size, MSS_I2C_RELEASE_BUS);

    status = MSS_I2C_wait_complete(I2C_INSTANCE, DEMO_I2C_TIMEOUT);

    return status;
}


void PCF8574_TRAY_SEN_init( void )
{
// MASTER Mode
	MSS_I2C_init( I2C_INSTANCE, PCF8574_TRAY_SEN_SLAVE_ADDRESS, MSS_I2C_PCLK_DIV_256 );
}

void PCF8574_ROTARY_init( void )
{
// MASTER Mode
	MSS_I2C_init( I2C_INSTANCE, PCF8574_ROTARY_SLAVE_ADDRESS, MSS_I2C_PCLK_DIV_256 );
}

void PCF8574_LED_CON_init( void )
{
// MASTER Mode
	MSS_I2C_init( I2C_INSTANCE, PCF8574_LED_CON_SLAVE_ADDRESS, MSS_I2C_PCLK_DIV_256 );
}

void RTC_BQ32002DR_init( void )
{
// MASTER Mode
	MSS_I2C_init( I2C_INSTANCE, RTC_BQ32002DR_SLAVE_ADDRESS, MSS_I2C_PCLK_DIV_256 );
}



//=================================================================================
//=================================================================================
// 1byte 쓰기 
mss_i2c_status_t
rtcWrite_byte
(
    uint8_t reg_address,
//    uint16_t size_in_bytes,
//    uint8_t write_buffer
    uint8_t data
)
{
    /* Local buffer to hold the maximum of 1 bytes with 1 bytes of address*/
    uint8_t write_buffer_local[2];	// register - 1byte , data - 1byte
    /* variable to hold the number of bytes */
    uint8_t no_of_bytes;

     /* Conversion of 16 bit address in to MSB address bits A15/A7 and
        LSB address bits A6/A0 */
//     write_buffer_local[ADDR_FIRST_LOC] = start_address >> 8;
//     write_buffer_local[reg_address] = start_address ;
     write_buffer_local[0] = reg_address ;
     write_buffer_local[1] = data;
     no_of_bytes = 2;

     MSS_I2C_write( I2C_INSTANCE, RTC_BQ32002DR_SLAVE_ADDRESS, write_buffer_local,
                    no_of_bytes, MSS_I2C_RELEASE_BUS );

     I2C_STATUS = MSS_I2C_wait_complete( I2C_INSTANCE ,DEMO_I2C_TIMEOUT);

     /* Delay between Stop and Start conditions, This delay can be 2 to 3 cycles
        of I2C clock but here the delay is high. This is because the EEPROM and
        EtherCAT are connected to same I2C bus on the Development Kit. The
        EtherCAT SCL and SDA lines are interchanged with MSS signals. Because
        of this there are noisy trasactions on the I2C bus for every Operation
        of I2C, to skip those transactions the delay is provided */
//     delay_btw_transfers (100000);

return I2C_STATUS;

		 
}

mss_i2c_status_t
rtcWrite_time
(
//    uint8_t reg_address,
    uint8_t *data
)
{
    /* Local buffer to hold the maximum of 1 bytes with 1 bytes of address*/
//    uint8_t write_buffer_local[8];	// register - 1byte , data - 7byte
    /* variable to hold the number of bytes */
    uint8_t no_of_bytes;

     /* Conversion of 16 bit address in to MSB address bits A15/A7 and
        LSB address bits A6/A0 */
//     write_buffer_local[0] = reg_address ;
//     write_buffer_local[1] = data;
		 
     no_of_bytes = 8;

     MSS_I2C_write( I2C_INSTANCE, RTC_BQ32002DR_SLAVE_ADDRESS, data,
                    no_of_bytes, MSS_I2C_RELEASE_BUS );

     I2C_STATUS = MSS_I2C_wait_complete( I2C_INSTANCE ,DEMO_I2C_TIMEOUT);

     /* Delay between Stop and Start conditions, This delay can be 2 to 3 cycles
        of I2C clock but here the delay is high. This is because the EEPROM and
        EtherCAT are connected to same I2C bus on the Development Kit. The
        EtherCAT SCL and SDA lines are interchanged with MSS signals. Because
        of this there are noisy trasactions on the I2C bus for every Operation
        of I2C, to skip those transactions the delay is provided */
//     delay_btw_transfers (100000);

return I2C_STATUS;

}

// 1Byte 읽기
mss_i2c_status_t
rtcRead_byte
(
    uint8_t reg_address,
    uint8_t *read_data
)
{
    /* Local buffer to hold 2 bytes of address */
    uint8_t read_addr[2];
   /* Conversion of 16 bit address in to MSB address bits A15/A7 and
      LSB address bits A6/A0 */
//    read_addr[ADDR_FIRST_LOC] = start_address >> 8;
//    read_addr[ADDR_SECOND_LOC] = start_address ;

    read_addr[0] = reg_address ;
    {
        MSS_I2C_write( I2C_INSTANCE, RTC_BQ32002DR_SLAVE_ADDRESS, read_addr,
                       (RTC_REG_ADDR_LEN_IN_BYTES), MSS_I2C_HOLD_BUS );
        I2C_STATUS = MSS_I2C_wait_complete( I2C_INSTANCE ,DEMO_I2C_TIMEOUT);

        MSS_I2C_read( I2C_INSTANCE, RTC_BQ32002DR_SLAVE_ADDRESS, read_data, 1, MSS_I2C_RELEASE_BUS );
        I2C_STATUS = MSS_I2C_wait_complete( I2C_INSTANCE ,DEMO_I2C_TIMEOUT);
        /* Delay between Stop and Start conditions, This delay can be 2 to 3 cycles
           of I2C clock but here the delay is high. This is because the EEPROM and
           EtherCAT are connected to same I2C bus on the Development Kit. The
           EtherCAT SCL and SDA lines are interchanged with MSS signals. Because
           of this there are noisy trasactions on the I2C bus for every Operation
           of I2C, to skip those transactions the delay is provided */
//        delay_btw_transfers (10000000);

			return I2C_STATUS;
    }
}

mss_i2c_status_t
rtcRead_time
(
    uint8_t reg_address,
    uint8_t *read_data
)
{
    /* Local buffer to hold 1 bytes of address */ // regster address = 1 byte
    uint8_t read_addr[2];	
   /* Conversion of 16 bit address in to MSB address bits A15/A7 and
      LSB address bits A6/A0 */
//    read_addr[ADDR_FIRST_LOC] = start_address >> 8;
//    read_addr[ADDR_SECOND_LOC] = start_address ;
    read_addr[0] = reg_address ;
	 
    {
        MSS_I2C_write( I2C_INSTANCE, RTC_BQ32002DR_SLAVE_ADDRESS, read_addr,
                       (RTC_REG_ADDR_LEN_IN_BYTES), MSS_I2C_HOLD_BUS );
        I2C_STATUS = MSS_I2C_wait_complete( I2C_INSTANCE ,DEMO_I2C_TIMEOUT);

        MSS_I2C_read( I2C_INSTANCE, RTC_BQ32002DR_SLAVE_ADDRESS, read_data, 9, MSS_I2C_RELEASE_BUS );
        I2C_STATUS = MSS_I2C_wait_complete( I2C_INSTANCE ,DEMO_I2C_TIMEOUT);
        /* Delay between Stop and Start conditions, This delay can be 2 to 3 cycles
           of I2C clock but here the delay is high. This is because the EEPROM and
           EtherCAT are connected to same I2C bus on the Development Kit. The
           EtherCAT SCL and SDA lines are interchanged with MSS signals. Because
           of this there are noisy trasactions on the I2C bus for every Operation
           of I2C, to skip those transactions the delay is provided */
//        delay_btw_transfers (10000000);

			return I2C_STATUS;
    }
}

























/***************************************************************************//**
  EEPROM_init()
  See "EEPROM.h" for details of how to use this function.
 */
void EEPROM_init()
{
//	MSS_I2C_init(I2C_INSTANCE_0, EEPROM_SLAVE_ADDRESS, MSS_I2C_PCLK_DIV_256);
//	MSS_I2C_init( I2C_INSTANCE, EEPROM_SLAVE_ADDRESS, EEPROM_I2C_CLK_FREQ );
//	MSS_I2C_init( &g_mss_i2c1, EEPROM_SLAVE_ADDRESS, MSS_I2C_PCLK_DIV_256 );
// MASTER Mode
	MSS_I2C_init( I2C_INSTANCE, EEPROM_SLAVE_ADDRESS, MSS_I2C_PCLK_DIV_256 );
}

// 1byte 쓰기 
EEPROM_status_t
eepromWrite_byte
(
    uint16_t start_address,
//    uint16_t size_in_bytes,
    uint8_t *write_buffer
)
{
    /* Local buffer to hold the maximum of 1 bytes with 2 bytes of address*/
    uint8_t write_buffer_local[3];
    /* variable to hold the number of bytes */
    uint8_t no_of_bytes;

     /* Conversion of 16 bit address in to MSB address bits A15/A7 and
        LSB address bits A6/A0 */
     write_buffer_local[ADDR_FIRST_LOC] = start_address >> 8;
     write_buffer_local[ADDR_SECOND_LOC] = start_address ;
     write_buffer_local[2] = *write_buffer;
     no_of_bytes = 3;

     MSS_I2C_write( I2C_INSTANCE, EEPROM_SLAVE_ADDRESS, write_buffer_local,
                    no_of_bytes, MSS_I2C_RELEASE_BUS );

     I2C_STATUS = MSS_I2C_wait_complete( I2C_INSTANCE ,DEMO_I2C_TIMEOUT);

     /* Delay between Stop and Start conditions, This delay can be 2 to 3 cycles
        of I2C clock but here the delay is high. This is because the EEPROM and
        EtherCAT are connected to same I2C bus on the Development Kit. The
        EtherCAT SCL and SDA lines are interchanged with MSS signals. Because
        of this there are noisy trasactions on the I2C bus for every Operation
        of I2C, to skip those transactions the delay is provided */
     delay_btw_transfers (100000);

     /* Returns the EEPROM write status depending on the I2C status*/
     if(I2C_STATUS == MSS_I2C_SUCCESS)
     {
#if	EEPROM_ADJUST_DATA_SAVE_FIX			 	
// 딜레이가 많이 필요하다.
//iprintf("EEPROM_WRITE_SUCCESS = %d\n",size_in_bytes);		
			delay_btw_transfers (100000);
#endif		 	
         return EEPROM_WRITE_SUCCESS;
     }
     else
     {
#if	EEPROM_ADJUST_DATA_SAVE_FIX			 	
// 딜레이가 많이 필요하다.
//iprintf("EEPROM_WRITE_SUCCESS = %d\n",size_in_bytes);		
			delay_btw_transfers (100000);
#endif			 	
         return EEPROM_WRITE_UNSUCCESS;
     }
}


// PAGE Size보다 적은 데이터 저장
EEPROM_status_t
eepromWrite_Data
(
    uint16_t start_address,
    uint16_t size_in_bytes,
    uint8_t * write_buffer
)
{
		/* Local buffer to hold the maximum of 128(Page size) bytes with 2 bytes of address*/
		uint8_t write_buffer_local[130];
		/* variable to hold the number of bytes */
		uint8_t no_of_bytes;

		/* Conversion of 16 bit address in to MSB address bits A15/A7 and
		  LSB address bits A6/A0 */
		write_buffer_local[ADDR_FIRST_LOC] = start_address >> 8;
		write_buffer_local[ADDR_SECOND_LOC] = start_address ;
//dp("write_buffer_local[0]=0x%02x %02x\n",write_buffer_local[0],write_buffer_local[1]);        		
		for(no_of_bytes=0; no_of_bytes <= size_in_bytes; no_of_bytes++)
		{
		   write_buffer_local[no_of_bytes + ADDR_LEN_IN_BYTES ] = *write_buffer++;
		}

		MSS_I2C_write( I2C_INSTANCE, EEPROM_SLAVE_ADDRESS, write_buffer_local,
		              (size_in_bytes + ADDR_LEN_IN_BYTES), MSS_I2C_RELEASE_BUS );

		I2C_STATUS = MSS_I2C_wait_complete( I2C_INSTANCE ,DEMO_I2C_TIMEOUT);

		/* Delay between Stop and Start conditions, This delay can be 2 to 3 cycles
		  of I2C clock but here the delay is high. This is because the EEPROM and
		  EtherCAT are connected to same I2C bus on the Development Kit. The
		  EtherCAT SCL and SDA lines are interchanged with MSS signals. Because
		  of this there are noisy trasactions on the I2C bus for every Operation
		  of I2C, to skip those transactions the delay is provided */
		  
		delay_btw_transfers (100000);

		/* Returns the EEPROM write status depending on the I2C status*/
		if(I2C_STATUS == MSS_I2C_SUCCESS)
		{
#if	EEPROM_ADJUST_DATA_SAVE_FIX	
// 딜레이가 많이 필요하다.
dp("EEPROM_WRITE_SUCCESS = %d\n",size_in_bytes);		
//		delay_btw_transfers (100000);
#endif
		   return EEPROM_WRITE_SUCCESS;
		}
		else
		{
#if	EEPROM_ADJUST_DATA_SAVE_FIX			
dp("EEPROM_WRITE_UNSUCCESS = %d\n",size_in_bytes);				
//		delay_btw_transfers (100000);
#endif
		   return EEPROM_WRITE_UNSUCCESS;
		}
}

int eepromWrite_Data_Verify(uint16_t start_address, uint16_t size_in_bytes, uint8_t * write_buffer)
{
	int i,j;

	unsigned char *da;
	unsigned char *da2;
	unsigned char data[50];
	unsigned char err=0;
	unsigned char retry=0;
	unsigned char wr = 1;

	while(wr) {
		eepromWrite_Data(start_address, size_in_bytes, write_buffer);	

	// da = (unsigned char*)(&devSet.adjust.saveFlag);
//		eepromRead_Data(HOME_ADJUST_DATA_STARTADDR,size_in_bytes,&data);	
		eepromRead_Data(HOME_ADJUST_DATA_STARTADDR,size_in_bytes,data);	
//diprintf("eepromWrite_Data_Verify\n");
		for(i=0;i<size_in_bytes;i++) {
			if(write_buffer[i] != data[i]) {
				++err;
			}
		}
			
	diprintf("err = %d \n",err);
		if(err == 0) {
			return err;	
		}
		else {
			retry++;
	iprintf("===========================================================retry = %d \n",retry);		
			if(retry >= 3) {
			iprintf("WRITE FAIL ===================================================err = %d \n",err);					
				return err;	
			}
			err = 0;
		}
	}
}

int eepromWrite_Data_Verify_2nd(uint16_t start_address, uint16_t size_in_bytes, uint8_t * write_buffer)
{
	int i,j;

	unsigned char *da;
	unsigned char *da2;
	unsigned char data[45],data2nd[45];
	unsigned char err=0;
	unsigned char retry=0;
	unsigned char wr = 1;

	while(wr) {
		eepromWrite_Data(start_address, size_in_bytes, write_buffer);	

	// da = (unsigned char*)(&devSet.adjust.saveFlag);
//		eepromRead_Data(HOME_ADJUST_2ND_STARTADDR,size_in_bytes,&data);	
		eepromRead_Data(HOME_ADJUST_2ND_STARTADDR,size_in_bytes,data);	
	//		eepromRead_Data(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust_2nd),devSet.adjust_2nd.saveFlag);	

		for(i=0;i<size_in_bytes;i++) {
			if(write_buffer[i] != data[i])  {
				++err;
			}
		}

diprintf("2nd err = %d \n",err);		
		if(err == 0) {
			return err;	
		}
		else {
			retry++;
iprintf("2nd ===========================================================retry = %d \n",retry);		
			if(retry >= 3) {
			iprintf("2nd WRITE FAIL ===================================================err = %d \n",err);					
				return err;	
			}
			err = 0;
		}
	}
}



/***************************************************************************/
/*
  EEPROM_write()
  See "EEPROM.h" for details of how to use this function.
 */

EEPROM_status_t
EEPROM_write
(
    uint16_t start_address,
    uint16_t size_in_bytes,
    uint8_t * write_buffer
)
{
    /* Local buffer to hold the maximum of 128(Page size) bytes with 2 bytes of address*/
    uint8_t write_buffer_local[130];
    /* variable to hold the number of bytes */
    uint8_t no_of_bytes;

    /* Address range checking*/
    if((start_address +  size_in_bytes )< 65537)
    {
       /* Splitting the total data in to pages and writing to the EEPROM*/
       while((size_in_bytes / PAGE_SIZE_IN_BYTES) != 0)
       {
           /* Conversion of 16 bit address in to MSB address bits A15/A7 and
              LSB address bits A6/A0 */
           write_buffer_local[ADDR_FIRST_LOC] = start_address >> 8;
           write_buffer_local[ADDR_SECOND_LOC] = start_address ;
           for(no_of_bytes=0;no_of_bytes<PAGE_SIZE_IN_BYTES;no_of_bytes++)
           {
               write_buffer_local[no_of_bytes + ADDR_LEN_IN_BYTES ] = *write_buffer++;
           }
           MSS_I2C_write( I2C_INSTANCE, EEPROM_SLAVE_ADDRESS, write_buffer_local,
                          (PAGE_SIZE_IN_BYTES + ADDR_LEN_IN_BYTES), MSS_I2C_RELEASE_BUS );
           I2C_STATUS = MSS_I2C_wait_complete( I2C_INSTANCE ,DEMO_I2C_TIMEOUT);
           size_in_bytes = size_in_bytes - PAGE_SIZE_IN_BYTES ;
           start_address = start_address + PAGE_SIZE_IN_BYTES ;
           /* Delay between Stop and Start conditions, This delay can be 2 to 3 cycles
           of I2C clock but here the delay is high. This is because the EEPROM and
           EtherCAT are connected to same I2C bus on the Development Kit. The
           EtherCAT SCL and SDA lines are interchanged with MSS signals. Because
           of this there are noisy trasactions on the I2C bus for every Operation
           of I2C, to skip those transactions the delay is provided */

           delay_btw_transfers (100000);
       }
       if((size_in_bytes > 0) & (size_in_bytes < PAGE_SIZE_IN_BYTES))
       {
           /* Conversion of 16 bit address in to MSB address bits A15/A7 and
              LSB address bits A6/A0 */
           write_buffer_local[ADDR_FIRST_LOC] = start_address >> 8;
           write_buffer_local[ADDR_SECOND_LOC] = start_address ;
           for(no_of_bytes=0; no_of_bytes <= size_in_bytes; no_of_bytes++)
           {
               write_buffer_local[no_of_bytes + ADDR_LEN_IN_BYTES ] = *write_buffer++;
           }

           MSS_I2C_write( I2C_INSTANCE, EEPROM_SLAVE_ADDRESS, write_buffer_local,
                          (size_in_bytes + ADDR_LEN_IN_BYTES), MSS_I2C_RELEASE_BUS );
//  	dp("size_in_bytes2 = %d\n",size_in_bytes);
           I2C_STATUS = MSS_I2C_wait_complete( I2C_INSTANCE ,DEMO_I2C_TIMEOUT);

           /* Delay between Stop and Start conditions, This delay can be 2 to 3 cycles
              of I2C clock but here the delay is high. This is because the EEPROM and
              EtherCAT are connected to same I2C bus on the Development Kit. The
              EtherCAT SCL and SDA lines are interchanged with MSS signals. Because
              of this there are noisy trasactions on the I2C bus for every Operation
              of I2C, to skip those transactions the delay is provided */
           delay_btw_transfers (100000);
       }
       /* Returns the EEPROM write status depending on the I2C status*/
       if(I2C_STATUS == MSS_I2C_SUCCESS)
       {
           return EEPROM_WRITE_SUCCESS;
       }
       else
       {
           return EEPROM_WRITE_UNSUCCESS;
       }
    }
    /* Returns the EEPROM address fault status if the start address + size of the bytes
    crosses the total lenght of the EEPROM*/
    else
    {
        return EEPROM_ADDRESS_FAULT;
    }
}

// 1Byte 읽기
EEPROM_status_t
eepromRead_byte
(
    uint16_t start_address,
    uint8_t  * read_buffer
)
{
    /* Local buffer to hold 2 bytes of address */
    uint8_t read_addr[2];
   /* Conversion of 16 bit address in to MSB address bits A15/A7 and
      LSB address bits A6/A0 */
    read_addr[ADDR_FIRST_LOC] = start_address >> 8;
    read_addr[ADDR_SECOND_LOC] = start_address ;

    {
        MSS_I2C_write( I2C_INSTANCE, EEPROM_SLAVE_ADDRESS, read_addr,
                       (ADDR_LEN_IN_BYTES), MSS_I2C_HOLD_BUS );
        I2C_STATUS = MSS_I2C_wait_complete( I2C_INSTANCE ,DEMO_I2C_TIMEOUT);

        MSS_I2C_read( I2C_INSTANCE, EEPROM_SLAVE_ADDRESS, read_buffer, 1, MSS_I2C_RELEASE_BUS );
        I2C_STATUS = MSS_I2C_wait_complete( I2C_INSTANCE ,DEMO_I2C_TIMEOUT);
        /* Delay between Stop and Start conditions, This delay can be 2 to 3 cycles
           of I2C clock but here the delay is high. This is because the EEPROM and
           EtherCAT are connected to same I2C bus on the Development Kit. The
           EtherCAT SCL and SDA lines are interchanged with MSS signals. Because
           of this there are noisy trasactions on the I2C bus for every Operation
           of I2C, to skip those transactions the delay is provided */
//        delay_btw_transfers (10000000);
delay_btw_transfers (100000);
        if(I2C_STATUS == MSS_I2C_SUCCESS)
        {
            return EEPROM_READ_SUCCESS;
        }
        else
        {
            return EEPROM_READ_UNSUCCESS;
        }
    }
}

EEPROM_status_t eepromRead_Data
(
    uint16_t start_address,
    uint16_t size_in_bytes,
    uint8_t  * read_buffer
)
{
    /* Local buffer to hold 2 bytes of address */
    uint8_t read_addr[2];
   /* Conversion of 16 bit address in to MSB address bits A15/A7 and
      LSB address bits A6/A0 */
    read_addr[ADDR_FIRST_LOC] = start_address >> 8;
    read_addr[ADDR_SECOND_LOC] = start_address ;

    MSS_I2C_write( I2C_INSTANCE, EEPROM_SLAVE_ADDRESS, read_addr,
                   (ADDR_LEN_IN_BYTES), MSS_I2C_HOLD_BUS );

    I2C_STATUS = MSS_I2C_wait_complete( I2C_INSTANCE ,DEMO_I2C_TIMEOUT);
/*		
if(I2C_STATUS == MSS_I2C_SUCCESS)    {
dp("EEPROM_READ_SUCCESS -I2C_STATUS\n");        
//    return EEPROM_READ_SUCCESS;
}
else    {
dp("EEPROM_READ_UNSUCCESS -I2C_STATUS\n");                
//    return EEPROM_READ_UNSUCCESS;
}
*/

    MSS_I2C_read( I2C_INSTANCE, EEPROM_SLAVE_ADDRESS, read_buffer,size_in_bytes, MSS_I2C_RELEASE_BUS );
    I2C_STATUS = MSS_I2C_wait_complete( I2C_INSTANCE ,DEMO_I2C_TIMEOUT);
    /* Delay between Stop and Start conditions, This delay can be 2 to 3 cycles
       of I2C clock but here the delay is high. This is because the EEPROM and
       EtherCAT are connected to same I2C bus on the Development Kit. The
       EtherCAT SCL and SDA lines are interchanged with MSS signals. Because
       of this there are noisy trasactions on the I2C bus for every Operation
       of I2C, to skip those transactions the delay is provided */
//    delay_btw_transfers (10000000);
//    delay_btw_transfers (5000000);    

    delay_btw_transfers (100000);    

    if(I2C_STATUS == MSS_I2C_SUCCESS)    {
dp("EEPROM_READ_SUCCESS \n");
      return EEPROM_READ_SUCCESS;
    }
    else    {
dp("EEPROM_READ_UNSUCCESS \n");
      return EEPROM_READ_UNSUCCESS;
    }
}

/***************************************************************************//**
  EEPROM_read()
  See "EEPROM.h" for details of how to use this function.
 */

EEPROM_status_t
EEPROM_read
(
    uint16_t start_address,
    uint16_t size_in_bytes,
    uint8_t  * read_buffer
)
{
    /* Local buffer to hold 2 bytes of address */
    uint8_t read_addr[2];
   /* Conversion of 16 bit address in to MSB address bits A15/A7 and
      LSB address bits A6/A0 */
    read_addr[ADDR_FIRST_LOC] = start_address >> 8;
    read_addr[ADDR_SECOND_LOC] = start_address ;
    /* Address range checking*/
    if((start_address +  size_in_bytes )< 65537)
    {
        MSS_I2C_write( I2C_INSTANCE, EEPROM_SLAVE_ADDRESS, read_addr,
                       (ADDR_LEN_IN_BYTES), MSS_I2C_HOLD_BUS );
        I2C_STATUS = MSS_I2C_wait_complete( I2C_INSTANCE ,DEMO_I2C_TIMEOUT);

        MSS_I2C_read( I2C_INSTANCE, EEPROM_SLAVE_ADDRESS, read_buffer,size_in_bytes, MSS_I2C_RELEASE_BUS );
        I2C_STATUS = MSS_I2C_wait_complete( I2C_INSTANCE ,DEMO_I2C_TIMEOUT);
        /* Delay between Stop and Start conditions, This delay can be 2 to 3 cycles
           of I2C clock but here the delay is high. This is because the EEPROM and
           EtherCAT are connected to same I2C bus on the Development Kit. The
           EtherCAT SCL and SDA lines are interchanged with MSS signals. Because
           of this there are noisy trasactions on the I2C bus for every Operation
           of I2C, to skip those transactions the delay is provided */
//        delay_btw_transfers (10000000);
delay_btw_transfers (100000);
        if(I2C_STATUS == MSS_I2C_SUCCESS)
        {
// 	dp("EEPROM_READ_SUCCESS \n");
            return EEPROM_READ_SUCCESS;
        }
        else
        {
// 	dp("EEPROM_READ_UNSUCCESS \n");
            return EEPROM_READ_UNSUCCESS;
        }
    }
    /* Returns the EEPROM address fault status if the start address + size of the bytes
    crosses the total lenght of the EEPROM*/
    else
    {
// 	dp("EEPROM_ADDRESS_FAULT \n");
       return EEPROM_ADDRESS_FAULT;
    }
}

/***************************************************************************//**
  delay_btw_transfers ( volatile uint32_t n)
  See "EEPROM.h" for details of how to use this function.
 */
void delay_btw_transfers ( volatile uint32_t n)
{
    while(n!=0)
    {
        n--;
    }
}

// =================================================================================================



