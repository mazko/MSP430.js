#ifndef PLATFORM_CONF_H_
#define PLATFORM_CONF_H_

/* CPU target speed in Hz */
#define F_CPU 2457600uL /*2457600uL*/

/* Our clock resolution, this is the same as Unix HZ. */
#define CLOCK_CONF_SECOND 32UL

#define BAUD2UBR(baud) ((F_CPU/baud))

#define CCIF
#define CLIF

#define HAVE_STDINT_H
#include "msp430def.h"

/* Types for clocks and uip_stats */
typedef unsigned short uip_stats_t;
typedef unsigned long clock_time_t;
typedef unsigned long off_t;

#endif /* PLATFORM_CONF_H_ */
