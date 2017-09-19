#include "contiki.h"
#include "contiki-net.h"
#include "dhcp-client.h"
#include "ntp-client.h"
#include "draw.h"
#include "uip-debug.h"
#include "ip64-addr.h"
#include "assert.h"

/*---------------------------------------------------------------------------*/
PROCESS(display_process, "Clock display process");
/*---------------------------------------------------------------------------*/
// https://github.com/shuckc/contiki-dspic/blob/master/platform/dp-webplatform/apps/ntpclient.c
struct ntp_tm {
  // date
  uint16_t year;   // year
  uint8_t month;   // month within year (1..12)
  uint8_t day;     // day within month (1..31)
  uint8_t weekday; // day within week (0..6)
  // time
  uint8_t hour;    // hour within day (0..23)
  uint8_t minute;  // minute within hour (0..59)
  uint8_t second;  // second within minute (0..59)
};
#define FIRSTYEAR     1900    // start year
#define FIRSTDAY      1       // 1.1.1900 was a Monday (0 = Sunday)
void
ntp_tmtime(uint32_t sec, struct ntp_tm *tm) {

  static const uint8_t DayOfMonth[] = { 31, 29, 31, 30, 31, 30, 
             31, 31, 30, 31, 30, 31 };

  uint16_t day;
  uint8_t year;
  uint16_t dayofyear;
  uint8_t leap400;
  uint8_t month;

  // adjust timezone
  // sec += 3600 * NTP_TZ;

  tm->second = sec % 60;
  sec /= 60;
  tm->minute = sec % 60;
  sec /= 60;
  tm->hour = sec % 24;
  day = sec / 24;

  tm->weekday = (day + FIRSTDAY) % 7;   // weekday

  year = FIRSTYEAR % 100;     // 0..99
  leap400 = 4 - ((FIRSTYEAR - 1) / 100 & 3);  // 4, 3, 2, 1

  for(;;){
    dayofyear = 365;
    if( (year & 3) == 0 ){
      dayofyear = 366;          // leap year
      if( year == 0 || year == 100 || year == 200 ) // 100 year exception
        if( --leap400 )         // 400 year exception
          dayofyear = 365;
    }
    if( day < dayofyear )
      break;
    day -= dayofyear;
    year++;         // 00..136 / 99..235
  }
  tm->year = year + FIRSTYEAR / 100 * 100;  // + century

  if( dayofyear & 1 && day > 58 )   // no leap year and after 28.2.
    day++;          // skip 29.2.

  for( month = 1; day >= DayOfMonth[month-1]; month++ )
    day -= DayOfMonth[month-1];

  tm->month = month;        // 1..12
  tm->day = day + 1;        // 1..31
}
/*---------------------------------------------------------------------------*/
static void draw_fill(const uint8_t p, const char c) {
  uint8_t i;
  for (i = 0; i < (128 / 6); i++) {
    draw_char(1 + (i * 6) , p, c);
  }
}
/*---------------------------------------------------------------------------*/
#if NETSTACK_CONF_WITH_IPV4
/*---------------------------------------------------------------------------*/
static void draw_ip(const uint8_t p, const char* s, const uip_ipaddr_t *ip_addr) {
  draw_printf(1, p, "%s %u.%u.%u.%-21u" /* 128 / 6 = 21 chars max */, s,
  ip_addr->u8[0],
  ip_addr->u8[1],
  ip_addr->u8[2],
  ip_addr->u8[3]);
}
/*---------------------------------------------------------------------------*/
#endif /* NETSTACK_CONF_WITH_IPV4 */
/*---------------------------------------------------------------------------*/
#if NETSTACK_CONF_WITH_IPV6
/*---------------------------------------------------------------------------*/
static void draw_ip6() {
  int i = 0, j = 0;
  int interface_count = 0;
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    if(uip_ds6_if.addr_list[i].isused) {
      printf("IPV6 Address: ");
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      printf("\n");
      interface_count++;

      // draw display
      if (++j == 1) {
        draw_printf(1, 3, "%x:%x:%x:%-21x" /* 128 / 6 = 21 chars max */,
          uip_htons(uip_ds6_if.addr_list[i].ipaddr.u16[0]),
          uip_htons(uip_ds6_if.addr_list[i].ipaddr.u16[1]),
          uip_htons(uip_ds6_if.addr_list[i].ipaddr.u16[2]),
          uip_htons(uip_ds6_if.addr_list[i].ipaddr.u16[3]));

        draw_printf(1, 4, "%x:%x:%x:%-21x" /* 128 / 6 = 21 chars max */,
          uip_htons(uip_ds6_if.addr_list[i].ipaddr.u16[4]),
          uip_htons(uip_ds6_if.addr_list[i].ipaddr.u16[5]),
          uip_htons(uip_ds6_if.addr_list[i].ipaddr.u16[6]),
          uip_htons(uip_ds6_if.addr_list[i].ipaddr.u16[7]));
      } else if (j == 2) {
        draw_printf(1, 6, "%x:%x:%x:%-21x" /* 128 / 6 = 21 chars max */,
          uip_htons(uip_ds6_if.addr_list[i].ipaddr.u16[0]),
          uip_htons(uip_ds6_if.addr_list[i].ipaddr.u16[1]),
          uip_htons(uip_ds6_if.addr_list[i].ipaddr.u16[2]),
          uip_htons(uip_ds6_if.addr_list[i].ipaddr.u16[3]));

        draw_printf(1, 7, "%x:%x:%x:%-21x" /* 128 / 6 = 21 chars max */,
          uip_htons(uip_ds6_if.addr_list[i].ipaddr.u16[4]),
          uip_htons(uip_ds6_if.addr_list[i].ipaddr.u16[5]),
          uip_htons(uip_ds6_if.addr_list[i].ipaddr.u16[6]),
          uip_htons(uip_ds6_if.addr_list[i].ipaddr.u16[7]));
      }
    }
  }
  assert(interface_count > 0);
}
/*---------------------------------------------------------------------------*/
#endif /* NETSTACK_CONF_WITH_IPV6 */
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(display_process, ev, data)
{
  PROCESS_BEGIN();

  etimer_set(&et, CLOCK_SECOND / 3);
  draw_fill(1, '_');

  while(1) {

      PROCESS_WAIT_EVENT();

      if (etimer_expired(&et)) {
        etimer_restart(&et);
        static uint32_t lastTime;
        if (ntp_get_timestamp() != lastTime) {
          lastTime = ntp_get_timestamp();
          struct ntp_tm tm;
          ntp_tmtime(lastTime, &tm);
          /* 128 / 6 = 21 chars max */
          draw_printf(1, 0, "%u:%.2u:%.2u %u/%.2u/%-21.2u",
            tm.hour, tm.minute, tm.second,
            tm.day, tm.month, tm.year);
        }
      }

      #if NETSTACK_CONF_WITH_IPV4
      if (ev == dhcp_client_event_updated) {
        uip_ipaddr_t addr;
        uip_gethostaddr(&addr);
        draw_ip(3, "IP:", &addr);
        uip_getnetmask(&addr);
        draw_ip(5, "MK:", &addr);
        uip_getdraddr(&addr);
        draw_ip(7, "GW:", &addr);
      }
      #else
      if (ev == resolv_event_found /* TODO: neighbor / route event */) {
        draw_ip6();
      }
      #endif
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
