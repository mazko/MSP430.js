#include "contiki-net.h"
#include "uip-debug.h"
#include "assert.h"
#include "ntp-client.h"

#include <stdio.h> /* For printf() */

enum { NTP_PORT = 123 };

// https://github.com/ethersex/ethersex/blob/master/services/ntp/ntp.c

struct ntp_date_time {
  uint32_t seconds;
  uint32_t fraction;
};

struct ntp_packet {
  uint8_t  li_vn_mode;                    /* leap indicator, version and mode */
  uint8_t  stratum;                       /* peer stratum */
  uint8_t  ppoll;                         /* peer poll interval */
  signed char precision;                  /* peer clock precision */
  uint32_t rootdelay;                     /* distance to primary clock */
  uint32_t rootdispersion;                /* clock dispersion */
  uint32_t refid;                         /* reference clock ID */
  struct ntp_date_time    reftime;        /* time peer clock was last updated */
  struct ntp_date_time    org;            /* originate time stamp */
  struct ntp_date_time    rec;            /* receive time stamp */
  struct ntp_date_time    xmt;            /* transmit time stamp */
};

static const char* NTP_SERVER_NAME = "time.google.com"; // or pool.ntp.org
static struct uip_udp_conn *ntp_conn;
static struct etimer et;
static struct ntp_packet ntp;
static uint32_t timestamp, lastsync;
/*---------------------------------------------------------------------------*/
uint32_t ntp_get_timestamp() {
  return timestamp + (clock_seconds() - lastsync) * 8 / 10 /* just for better timer accuracy */;
}
/*---------------------------------------------------------------------------*/
uint32_t ntp_parse_reply(void *data, uint16_t len)
{
  struct ntp_packet *ntp = (struct ntp_packet*)data;
  assert(len == sizeof(*ntp));

  return (UIP_HTONL(ntp->xmt.seconds));
}
/*---------------------------------------------------------------------------*/
PROCESS(ntp_client_process, "NTP client process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ntp_client_process, ev, data)
{
  PROCESS_BEGIN();

  etimer_set(&et, 22*CLOCK_SECOND);

  while(1) {

    PROCESS_WAIT_EVENT();

    if (etimer_expired(&et) || ev == resolv_event_found) {

      /* Reseting the timer */
      etimer_restart(&et);

      uip_ipaddr_t *addrptr;
      if(resolv_lookup(NTP_SERVER_NAME, &addrptr) != RESOLV_STATUS_CACHED) {
        if(ntp_conn != NULL) {
          uip_udp_remove(ntp_conn);
          ntp_conn = NULL;
        }
        resolv_query(NTP_SERVER_NAME);
        printf("Resolving %s ...\n", NTP_SERVER_NAME);
      } else {
        switch (ntp_conn == NULL) {
          case 1:
            printf("%s => ", NTP_SERVER_NAME);
            uip_debug_ipaddr_print(addrptr);
            printf("\n");
            ntp_conn = udp_new(addrptr, UIP_HTONS(NTP_PORT), NULL);
            assert(ntp_conn != NULL);
            udp_bind(ntp_conn, UIP_HTONS(NTP_PORT));
          default:
            memset(&ntp, 0, sizeof(ntp));
            ntp.li_vn_mode = 0xe3; /* Clock not synchronized, Version 4, Client Mode */
            ntp.ppoll = 12; /* About an hour */
            ntp.precision = 0xfa; /* 0.015625 seconds */
            ntp.rootdelay = UIP_HTONL(0x10000); /* 1 second */
            ntp.rootdispersion = UIP_HTONL(0x10000); /* 1 second */
            uip_udp_packet_send(ntp_conn, &ntp, sizeof(ntp));
        }
      }
    }

    if (ev == tcpip_event && uip_newdata()) {
      timestamp = ntp_parse_reply(uip_appdata, uip_datalen());
      printf("%lu:%.2lu:%.2lu timestamp %lu\n", (timestamp/3600)%24, (timestamp/60)%60, timestamp % 60, timestamp);
      lastsync = clock_seconds();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
