#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0
#define LWIP_IPV6                   0
#define LWIP_ARP                    1
#define LWIP_DNS                    1
#define LWIP_TCP                    1
#define LWIP_UDP                    1
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    4096
#define MEMP_NUM_PBUF               16
#define MEMP_NUM_UDP_PCB            4
#define MEMP_NUM_TCP_PCB            4
#define MEMP_NUM_TCP_SEG            16
#define MEMP_NUM_SYS_TIMEOUT        6
#define PBUF_POOL_SIZE              16
#define TCP_QUEUE_OOSEQ             0
#define TCP_MSS                     1460
#define TCP_SND_BUF                 (2*TCP_MSS)
#define TCP_WND                     (2*TCP_MSS)
#define LWIP_HAVE_LOOPIF            0

#endif /* __LWIPOPTS_H__ */
