#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/uart0.h"
#include "contiki-net.h"

#include "draw.h"

#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

static const struct uip_eth_addr ETH_MAC_ADDR = {{0x00,0x00,0xe2,0x58,0xb6,0x6b}};

PROCESS_NAME(enc28j60_process); // makes compiler happy

/*---------------------------------------------------------------------------*/
#if 0
static void init_static_ip() {
  uip_ipaddr_t addr;

  uip_ipaddr(&addr, 10,0,0,2);
  PRINTF("IP Address:  %u.%u.%u.%u\n", uip_ipaddr_to_quad(&addr));
  uip_sethostaddr(&addr);

  uip_ipaddr(&addr, 255,255,255,0);
  uip_setnetmask(&addr);
  PRINTF("Subnet Mask: %u.%u.%u.%u\n", uip_ipaddr_to_quad(&addr));

  uip_ipaddr(&addr, 10,0,0,1);
  uip_setdraddr(&addr);
  PRINTF("Def. Router: %u.%u.%u.%u\n", uip_ipaddr_to_quad(&addr));
}
#endif
/*---------------------------------------------------------------------------*/
int
main()
{

  WDTCTL = WDTPW | WDTHOLD;     // Stop watchdog timer

  // DCO = 7, RSEL = 7, f = 4.5 MHz
  DCOCTL = DCO2 + DCO1 + DCO0; 
  BCSCTL1 = XT2OFF + RSEL0 + RSEL1 + RSEL2;

  // leds
  P2OUT = 0; P3OUT = 0; P4OUT = 0; P5OUT = 0; P6OUT = 0;
  P2DIR = P3DIR = P4DIR = P5DIR = P6DIR = 0xFF;

  uart0_init(BAUD2UBR(115200)); /* Must come before first printf */
  draw_init();
  draw_clr();

  // see minimal-net platform
  clock_init();

  process_init();
  /* procinit_init initializes RPL which sets a ctimer for the first DIS */
  /* We must start etimers and ctimers,before calling it */
  process_start(&etimer_process, NULL);
  ctimer_init();

  PRINTF(CONTIKI_VERSION_STRING " started. \n");

  uip_setethaddr(ETH_MAC_ADDR); // first
  //init_static_ip();
  process_start(&enc28j60_process, NULL);
  process_start(&tcpip_process, NULL); // ICMP

  autostart_start(autostart_processes);

  while(1) {
#if DEBUG_SLEEP
    int n;
    clock_time_t next_event;
    
    n = process_run();
    next_event = etimer_next_expiration_time() - clock_time();

    if(n > 0)
      printf("sleep: %d events pending\n",n);
    else
      printf("sleep: next event @ T-%.03f\n",(double)next_event / (double)CLOCK_SECOND);
#else
    process_run();
#endif

    process_poll(&enc28j60_process);

    /* wpcap doesn't appear to support select, so
     * we can't idle the process on windows. 
     * next_event = 0; */
    // so sleep(next_event) is not mandatory i suppose
    etimer_request_poll();
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
void uip_log(char *msg) { puts(msg); }
/*---------------------------------------------------------------------------*/
void
_xassert(const char *file, int line)
{
   puts("\n!!!!!!!!!!!! failed assertion !!!!!!!!!!!!");
   printf("%s:%u", file, line);
   for(;;) {
     continue;
   }
}