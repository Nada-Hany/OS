// Sleeping locks

#include "inc/types.h"
#include "inc/x86.h"
#include "inc/memlayout.h"
#include "inc/mmu.h"
#include "inc/environment_definitions.h"
#include "inc/assert.h"
#include "inc/string.h"
#include "sleeplock.h"
#include "channel.h"
#include "../cpu/cpu.h"
#include "../proc/user_environment.h"

void init_sleeplock(struct sleeplock *lk, char *name)
{
	init_channel(&(lk->chan), "sleep lock channel");
	init_spinlock(&(lk->lk), "lock of sleep lock");
	strcpy(lk->name, name);
	lk->locked = 0;
	lk->pid = 0;
}
int holding_sleeplock(struct sleeplock *lk)
{
	int r;
	acquire_spinlock(&(lk->lk));
	r = lk->locked && (lk->pid == get_cpu_proc()->env_id);
	release_spinlock(&(lk->lk));
	return r;
}
//==========================================================================

void acquire_sleeplock(struct sleeplock *lk)
{

	struct spinlock *lock = &lk->lk;
	struct Channel *channel = &lk->chan;

	acquire_spinlock(&lk->lk);

	while(lk->locked){
		sleep(channel, lock);
	}
	//lk->pid = get_cpu_proc()->env_id;
	lk->locked = 1;
	release_spinlock(lock);

	//TODO: [PROJECT'24.MS1 - #13] [4] LOCKS - acquire_sleeplock

}

void release_sleeplock(struct sleeplock *lk)
{
	struct Channel *channel = &lk->chan;

	acquire_spinlock(&lk->lk);

	// wakeup all processes so the next one can acquire the lock
	wakeup_all(channel);
	// releasing sleeplock for next process
	lk->locked = 0;

	release_spinlock(&lk->lk);

	//TODO: [PROJECT'24.MS1 - #14] [4] LOCKS - release_sleeplock
}





