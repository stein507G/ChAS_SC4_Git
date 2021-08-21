#include "m2sxxx.h"
#include "conf_eth.h"

extern unsigned char my_mac[];

const uint8_t * sys_cfg_get_mac_address(void)
{
    static uint8_t mac_address[6];

    mac_address[0] = my_mac[0];
    mac_address[1] = my_mac[1];
    mac_address[2] = my_mac[2];
    mac_address[3] = my_mac[3];
    mac_address[4] = my_mac[4];
    mac_address[5] = my_mac[5];

    return (const uint8_t *)mac_address;
}
