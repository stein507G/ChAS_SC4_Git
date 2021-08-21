
#ifndef __SYS_RTXC_H__
#define __SYS_RTXC_H__

#include "lwipopts.h"
#if (NO_SYS == 0)

#define SYS_MBOX_NULL 0
#define SYS_SEM_NULL  0

typedef u8_t sys_sem_t;
typedef u8_t sys_mbox_t;
typedef u32_t sys_thread_t;
#else
#define SYS_MBOX_NULL 0
#define SYS_SEM_NULL  0

#endif
#endif /* __SYS_RTXC_H__ */

