/*
 * channel.c
 *
 *  Created on: Sep 22, 2024
 *      Author: HP
 */
#include "channel.h"
#include <kern/proc/user_environment.h>
#include <kern/cpu/sched.h>
#include <inc/string.h>
#include <inc/disk.h>

//===============================
// 1) INITIALIZE THE CHANNEL:
//===============================
// initialize its lock & queue
void init_channel(struct Channel *chan, char *name)
{
	strcpy(chan->name, name);
	init_queue(&(chan->queue));
}

//===============================
// 2) SLEEP ON A GIVEN CHANNEL:
//===============================
// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
// Ref: xv6-x86 OS code
void sleep(struct Channel *chan, struct spinlock* lk)
{
//	acquire_spinlock(lk);
	release_spinlock(lk);

	struct spinlock queueGuard;
	init_spinlock(&queueGuard, "queueGuard");
	struct spinlock *tmp = &queueGuard;

	struct Env* currentProcess = get_cpu_proc();

	acquire_spinlock(tmp);


	// block the running process on the given channel
	currentProcess->env_status = ENV_BLOCKED;
//	struct Env_Queue* q = &chan->queue;
	enqueue(&chan->queue, currentProcess);

	// scheduling the next to be ready process;
	sched();

	release_spinlock(tmp);
	//TODO: [PROJECT'24.MS1 - #10] [4] LOCKS - sleep

}

//==================================================
// 3) WAKEUP ONE BLOCKED PROCESS ON A GIVEN CHANNEL:
//==================================================
// Wake up ONE process sleeping on chan.
// The qlock must be held.
// Ref: xv6-x86 OS code
// chan MUST be of type "struct Env_Queue" to hold the blocked processes

void wakeup_one(struct Channel *chan)
{
	struct spinlock queueGuard;
	init_spinlock(&queueGuard, "queueGuard");
	struct spinlock *tmp = &queueGuard;

//	struct Env_Queue* q = &chan->queue;
	// making sure there's blocked processes


	acquire_spinlock(tmp);
	// remove the first process in the blocked queue
	struct Env* currentProcess = dequeue(&chan->queue);
	// change its state to ready
	currentProcess->env_status = ENV_READY;
	// add it to the ready queue
	sched_insert_ready0(currentProcess);

	release_spinlock(tmp);


//	if(chan->queue.size !=0){


	//}

	//TODO: [PROJECT'24.MS1 - #11] [4] LOCKS - wakeup_one.
}

//====================================================
// 4) WAKEUP ALL BLOCKED PROCESSES ON A GIVEN CHANNEL:
//====================================================
// Wake up all processes sleeping on chan.
// The queues lock must be held.
// Ref: xv6-x86 OS code
// chan MUST be of type "struct Env_Queue" to hold the blocked processes

void wakeup_all(struct Channel *chan)
{
	struct Env *element;
	LIST_FOREACH(element, &(chan->queue))
	{
		wakeup_one(chan);
	}
	//TODO: [PROJECT'24.MS1 - #12] [4] LOCKS - wakeup_all

}

