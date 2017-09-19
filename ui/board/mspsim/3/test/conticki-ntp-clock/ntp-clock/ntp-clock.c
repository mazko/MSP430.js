#include "contiki.h"
#include "contiki-net.h"

PROCESS_NAME(ntp_client_process);
PROCESS_NAME(display_process);
PROCESS_NAME(dhcp_client_process);

AUTOSTART_PROCESSES(
  &resolv_process,
#if NETSTACK_CONF_WITH_IPV4
  &dhcp_client_process,
#endif
  &ntp_client_process,
  &display_process);