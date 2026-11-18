/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: newlib_lock.c
* Description	: Makes the newlib heap (malloc/free, used by frozen json_scanf
*				  and stdio) safe to use from several FreeRTOS tasks.
*				  newlib calls __malloc_lock/__malloc_unlock around every heap
*				  operation; the default versions do nothing.
*
* Notes			: Calls nest correctly: vTaskSuspendAll() is reference counted.
*				  Must not be called from an ISR (malloc is never used in ISRs).
*****************************************************************************/
#include <reent.h>
#include <sys/types.h>
#include "FreeRTOS.h"
#include "task.h"
#include "newlib_lock.h"

void __malloc_lock(struct _reent *r);
void __malloc_unlock(struct _reent *r);

void __malloc_lock(struct _reent *r)
{
	(void)r;
	vTaskSuspendAll();
}

void __malloc_unlock(struct _reent *r)
{
	(void)r;
	(void)xTaskResumeAll();
}

#if MM_MEM_STATS
extern caddr_t _sbrk(int incr);

/* Current top of the newlib heap. newlib never returns memory to sbrk in this
 * build, so this is also the high-water mark. */
void *mm_sbrk_top(void)
{
	return (void *)_sbrk(0);
}
#endif
