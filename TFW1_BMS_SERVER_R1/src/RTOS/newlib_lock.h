/*****************************************************************************
* Module Name	: newlib_lock.h
* Description	: Task-safe newlib heap and memory statistics switch.
*****************************************************************************/
#ifndef NEWLIB_LOCK_H_
#define NEWLIB_LOCK_H_

/* 1 = print heap/stack/TLS-pool statistics over UART after every server poll. */
#ifndef MM_MEM_STATS
#define MM_MEM_STATS	0
#endif

#if MM_MEM_STATS
void *mm_sbrk_top(void);
#endif

#endif /* NEWLIB_LOCK_H_ */
