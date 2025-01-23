#include <stdio.h>
#include "port_common.h"

#include "wizchip_conf.h"
#include "w5x00_spi.h"
#include "socket.h"

/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2) // Send and receive cache size
#define _LOOPBACK_DEBUG_

wiz_NetInfo net_info = {
    .mac = {0x00, 0x08, 0xdc, 0x16, 0xed, 0x11}, // Define MAC variables
    .ip = {192, 168, 11, 33},                    // Define IP variables
    .sn = {255, 255, 255, 0},                    // Define subnet variables
    .gw = {192, 168, 11, 1},                     // Define gateway variables
    .dns = {168, 126, 63, 1},                    // Define DNS  variables
    .dhcp = NETINFO_STATIC};                     // Define the DNCP mode

static uint8_t ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};
static uint16_t local_port = 8000; // Local port

static void set_clock_khz(void);
int32_t loopback_tcps_multi_socket(uint8_t *buf, uint16_t port);

int main()
{
    set_clock_khz();

    /*mcu init*/
    stdio_init_all(); // Initialize the main control peripheral.
    wizchip_spi_initialize();
    wizchip_cris_initialize();
    wizchip_reset();
    wizchip_initialize(); // spi initialization
    wizchip_check();

    network_initialize(net_info);

    print_network_information(net_info); // Read back the configuration information and print it

    while (true)
    {
        loopback_tcps_multi_socket(ethernet_buf, local_port);
    }
}

static void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}

int32_t loopback_tcps_multi_socket(uint8_t *buf, uint16_t port)
{
    static uint8_t sn = 0;
    int32_t ret;
    uint16_t size = 0, sentsize = 0;
    uint8_t destip[4];
    uint16_t destport;

    switch (getSn_SR(sn))
    {
    case SOCK_ESTABLISHED:
        if (getSn_IR(sn) & Sn_IR_CON)
        {
            getSn_DIPR(sn, destip);
            destport = getSn_DPORT(sn);

            printf("%d:Connected - %d.%d.%d.%d : %d\r\n", sn, destip[0], destip[1], destip[2], destip[3], destport);
            setSn_IR(sn, Sn_IR_CON);
        }
        if ((size = getSn_RX_RSR(sn)) > 0) // Don't need to check SOCKERR_BUSY because it doesn't not occur.
        {
            if (size > ETHERNET_BUF_MAX_SIZE)
                size = ETHERNET_BUF_MAX_SIZE;
            ret = recv(sn, buf, size);
            if (ret <= 0)
                return ret; // check SOCKERR_BUSY & SOCKERR_XXX. For showing the occurrence of SOCKERR_BUSY.
            size = (uint16_t)ret;
                        buf[size] = 0x00;

            sentsize = 0;
            while (size != sentsize)
            {
                ret = send(sn, buf + sentsize, size - sentsize);
                if (ret < 0)
                {
                    close(sn);
                    return ret;
                }
                sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
            }
            getSn_DIPR(sn, destip);
            destport = getSn_DPORT(sn);
            printf("socket%d from:%d.%d.%d.%d port: %d  message:%s\r\n", sn, destip[0], destip[1], destip[2], destip[3], destport, buf);
        }
        break;
    case SOCK_CLOSE_WAIT:
#ifdef _LOOPBACK_DEBUG_
        // printf("%d:CloseWait\r\n",sn);
#endif
        if ((ret = disconnect(sn)) != SOCK_OK)
            return ret;
#ifdef _LOOPBACK_DEBUG_
        printf("%d:Socket Closed\r\n", sn);
#endif
        break;
    case SOCK_INIT:
#ifdef _LOOPBACK_DEBUG_
        printf("%d:Listen, TCP server loopback, port [%d]\r\n", sn, port);
#endif
        if ((ret = listen(sn)) != SOCK_OK)
            return ret;
        break;
    case SOCK_CLOSED:
#ifdef _LOOPBACK_DEBUG_
        // printf("%d:TCP server loopback start\r\n",sn);
#endif
        if ((ret = socket(sn, Sn_MR_TCP, port, Sn_MR_ND)) != sn)
            return ret;
#ifdef _LOOPBACK_DEBUG_
            // printf("%d:Socket opened\r\n",sn);
#endif
        break;
    default:
        break;
    }
#if (_WIZCHIP_ == W5100S)
    if (sn < 4)
    {
        sn++;
    }
    else
    {
        sn = 0;
    }
#elif (_WIZCHIP_ == W5500)
    if (sn < 8)
    {
        sn++;
    }
    else
    {
        sn = 0;
    }
#endif

    return 1;
}