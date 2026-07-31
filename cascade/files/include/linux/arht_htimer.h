/*********************************************************************************
 * decleration and function prototype for Speed Test module
 *
 * Copyright (C) 2016 Econet Technologies, Corp.
 * All Rights Reserved.
 *
 *********************************************************************************/
#ifndef AIROHA_TIMER_H_
#define	AIROHA_TIMER_H_

#include <linux/hrtimer.h>

enum airoha_timer_repeat {
	AIROHA_TIMER_NO_REPEAT,
	AIROHA_TIMER_REPEAT,
};

typedef struct airoha_timer_para_s {
	unsigned long exp_time;
} airoha_timer_para_t;

typedef struct airoha_timer_s {
	void* timer_handle;
	unsigned long time;
	int	(*func)(airoha_timer_para_t *);
} airoha_timer_t;

/**
 * airoha_timer_create - create a timer
 * @timer:	airoha_timer, caller provide the handle func by timer->func
 *
 * Returns: 0 successful, unsuccessful otherwise
 *
 * recommend call it when module is loaded, and should be deleted
 * by calling airoha_timer_delete before module is unloaded.
 */
extern int airoha_timer_create(airoha_timer_t *timer);

/**
 * airoha_timer_start - start the timer
 * @timer:	airoha_timer
 *
 */
extern void airoha_timer_start(airoha_timer_t *timer);

/**
 * airoha_timer_stop - stop the timer
 * @timer:	airoha_timer
 *
 */
extern void airoha_timer_stop(airoha_timer_t *timer);

/**
 * airoha_timer_delete - delete the timer
 * @timer:	airoha_timer
 *
 * Should be called before module is unloaded.
 */
extern void airoha_timer_delete(airoha_timer_t *timer);

#endif

