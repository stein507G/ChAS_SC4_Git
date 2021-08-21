/**
 * @file
 * Statistics module
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#include "lwip/debug.h" 		//(+)210603 CJKIM, added
#include "lwip/opt.h"

#if LWIP_STATS /* don't build if not configured for use in lwipopts.h */

#include "lwip/def.h"
#include "lwip/stats.h"
#include "lwip/mem.h"

#include <string.h>

struct stats_ lwip_stats;

#if LWIP_STATS_DISPLAY
void
stats_display_proto(struct stats_proto *proto, char *name)
{
  LWIP_DEBUG_I(MEMP_DEBUG, "\n%s\n\t", name);
  LWIP_DEBUG_I(MEMP_DEBUG, "xmit: %"STAT_COUNTER_F"\n\t", proto->xmit); 
  LWIP_DEBUG_I(MEMP_DEBUG, "recv: %"STAT_COUNTER_F"\n\t", proto->recv); 
  LWIP_DEBUG_I(MEMP_DEBUG, "fw: %"STAT_COUNTER_F"\n\t", proto->fw); 
  LWIP_DEBUG_I(MEMP_DEBUG, "drop: %"STAT_COUNTER_F"\n\t", proto->drop); 
  LWIP_DEBUG_I(MEMP_DEBUG, "chkerr: %"STAT_COUNTER_F"\n\t", proto->chkerr); 
  LWIP_DEBUG_I(MEMP_DEBUG, "lenerr: %"STAT_COUNTER_F"\n\t", proto->lenerr); 
  LWIP_DEBUG_I(MEMP_DEBUG, "memerr: %"STAT_COUNTER_F"\n\t", proto->memerr); 
  LWIP_DEBUG_I(MEMP_DEBUG, "rterr: %"STAT_COUNTER_F"\n\t", proto->rterr); 
  LWIP_DEBUG_I(MEMP_DEBUG, "proterr: %"STAT_COUNTER_F"\n\t", proto->proterr); 
  LWIP_DEBUG_I(MEMP_DEBUG, "opterr: %"STAT_COUNTER_F"\n\t", proto->opterr); 
  LWIP_DEBUG_I(MEMP_DEBUG, "err: %"STAT_COUNTER_F"\n\t", proto->err); 
  LWIP_DEBUG_I(MEMP_DEBUG, "cachehit: %"STAT_COUNTER_F"\n", proto->cachehit); 
}

#if IGMP_STATS
void
stats_display_igmp(struct stats_igmp *igmp)
{
  LWIP_DEBUG_I(MEMP_DEBUG, "\nIGMP\n\t");
  LWIP_DEBUG_I(MEMP_DEBUG, "lenerr: %"STAT_COUNTER_F"\n\t", igmp->lenerr); 
  LWIP_DEBUG_I(MEMP_DEBUG, "chkerr: %"STAT_COUNTER_F"\n\t", igmp->chkerr); 
  LWIP_DEBUG_I(MEMP_DEBUG, "v1_rxed: %"STAT_COUNTER_F"\n\t", igmp->v1_rxed); 
  LWIP_DEBUG_I(MEMP_DEBUG, "join_sent: %"STAT_COUNTER_F"\n\t", igmp->join_sent); 
  LWIP_DEBUG_I(MEMP_DEBUG, "leave_sent: %"STAT_COUNTER_F"\n\t", igmp->leave_sent); 
  LWIP_DEBUG_I(MEMP_DEBUG, "unicast_query: %"STAT_COUNTER_F"\n\t", igmp->unicast_query); 
  LWIP_DEBUG_I(MEMP_DEBUG, "report_sent: %"STAT_COUNTER_F"\n\t", igmp->report_sent); 
  LWIP_DEBUG_I(MEMP_DEBUG, "report_rxed: %"STAT_COUNTER_F"\n\t", igmp->report_rxed); 
  LWIP_DEBUG_I(MEMP_DEBUG, "group_query_rxed: %"STAT_COUNTER_F"\n", igmp->group_query_rxed);
}
#endif /* IGMP_STATS */

#if MEM_STATS || MEMP_STATS
void
stats_display_mem(struct stats_mem *mem, char *name)
{
  LWIP_DEBUG_I(MEMP_DEBUG, "\nMEM %s\n\t", name);
  LWIP_DEBUG_I(MEMP_DEBUG, "avail: %"U32_F"\n\t", (u32_t)mem->avail); 
  LWIP_DEBUG_I(MEMP_DEBUG, "used: %"U32_F"\n\t", (u32_t)mem->used); 
  LWIP_DEBUG_I(MEMP_DEBUG, "max: %"U32_F"\n\t", (u32_t)mem->max); 
  LWIP_DEBUG_I(MEMP_DEBUG, "err: %"U32_F"\n", (u32_t)mem->err);
}

#if MEMP_STATS
void
stats_display_memp(struct stats_mem *mem, int index)
{
  char * memp_names[] = {
#define LWIP_MEMPOOL(name,num,size,desc) desc,
#include "lwip/memp_std.h"
  };
  if(index < MEMP_MAX) {
    stats_display_mem(mem, memp_names[index]);
  }
}
#endif /* MEMP_STATS */
#endif /* MEM_STATS || MEMP_STATS */

#if SYS_STATS
void
stats_display_sys(struct stats_sys *sys)
{
  LWIP_DEBUG_I(MEMP_DEBUG, "\nSYS\n\t");
  LWIP_DEBUG_I(MEMP_DEBUG, "sem.used: %"U32_F"\n\t", (u32_t)sys->sem.used); 
  LWIP_DEBUG_I(MEMP_DEBUG, "sem.max:  %"U32_F"\n\t", (u32_t)sys->sem.max); 
  LWIP_DEBUG_I(MEMP_DEBUG, "sem.err:  %"U32_F"\n\t", (u32_t)sys->sem.err); 
  LWIP_DEBUG_I(MEMP_DEBUG, "mbox.used: %"U32_F"\n\t", (u32_t)sys->mbox.used); 
  LWIP_DEBUG_I(MEMP_DEBUG, "mbox.max:  %"U32_F"\n\t", (u32_t)sys->mbox.max); 
  LWIP_DEBUG_I(MEMP_DEBUG, "mbox.err:  %"U32_F"\n\t", (u32_t)sys->mbox.err); 
}
#endif /* SYS_STATS */

void
stats_display(void)
{
  s16_t i;

  LINK_STATS_DISPLAY();
  ETHARP_STATS_DISPLAY();
  IPFRAG_STATS_DISPLAY();
  IP_STATS_DISPLAY();
  IGMP_STATS_DISPLAY();
  ICMP_STATS_DISPLAY();
  UDP_STATS_DISPLAY();
  TCP_STATS_DISPLAY();
  MEM_STATS_DISPLAY();
  for (i = 0; i < MEMP_MAX; i++) {
    MEMP_STATS_DISPLAY(i);
  }
  SYS_STATS_DISPLAY();
}
#endif /* LWIP_STATS_DISPLAY */

#endif /* LWIP_STATS */

