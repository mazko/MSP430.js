#include "contiki.h"
#include "contiki-net.h"
#include "net/ip/dhcpc.h"
#include <stdio.h>

process_event_t dhcp_client_event_updated;
PROCESS_NAME(display_process);

PROCESS(dhcp_client_process, "DHCP client process");
// see apps/dhcp.c, ip64-ipv4-dhcp.c

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dhcp_client_process, ev, data)
{
  PROCESS_BEGIN();

  dhcp_client_event_updated = process_alloc_event();

  dhcpc_init(uip_lladdr.addr, sizeof(uip_lladdr.addr));
  dhcpc_request();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event || ev == PROCESS_EVENT_TIMER);
    dhcpc_appcall(ev, data);
  }

  PROCESS_END();
}

void dhcpc_configured(const struct dhcpc_state *s)
{
  uip_sethostaddr(&s->ipaddr);
  uip_setnetmask(&s->netmask);
  uip_setdraddr(&s->default_router);
  uip_nameserver_update(&s->dnsaddr, UIP_NAMESERVER_INFINITE_LIFETIME);

  printf("DHCP Configured\n");
  resolv_set_hostname(resolv_get_hostname()); // refresh mdns new ip

  process_post(&display_process, dhcp_client_event_updated, NULL);
}

void dhcpc_unconfigured(const struct dhcpc_state *s)
{
  static const uip_ipaddr_t nulladdr;

  uip_sethostaddr(&nulladdr);
  uip_setnetmask(&nulladdr);
  uip_setdraddr(&nulladdr);

  printf("DHCP Unconfigured\n");

  process_post(&display_process, dhcp_client_event_updated, NULL);
}
