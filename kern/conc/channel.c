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

	// protecting the process queue
	acquire_spinlock(&ProcessQueues.qlock);

	//releasing the lock so the next process can acquire it
	release_spinlock(lk);

	struct Env* currentProcess = get_cpu_proc();
	// block the running process on the given channel
	currentProcess->env_status = ENV_BLOCKED;
	enqueue(&chan->queue, currentProcess);

	// scheduling the next to be ready process;
	sched();
	// acquire lock when wake up
	acquire_spinlock(lk);

	release_spinlock(&ProcessQueues.qlock);

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

	// remove the first process in the blocked queue
	if(chan->queue.size ==0){
		return;
	}

	//guarding the process queue
	acquire_spinlock(&ProcessQueues.qlock);

	struct Env* currentProcess = dequeue(&chan->queue);
	// change its state to ready
	currentProcess->env_status = ENV_READY;
	// add it to the ready queue
	sched_insert_ready0(currentProcess);

	release_spinlock(&ProcessQueues.qlock);
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

	acquire_spinlock(&ProcessQueues.qlock);

	uint32 limit = chan->queue.size;
	for (uint32 i = 0; i < limit; ++i) {

		if(chan->queue.size ==0){
			return;
		}
		struct Env* currentProcess = dequeue(&chan->queue);
		// change its state to ready
		currentProcess->env_status = ENV_READY;
		// add it to the ready queue
		sched_insert_ready0(currentProcess);
	}

	release_spinlock(&ProcessQueues.qlock);

	//TODO: [PROJECT'24.MS1 - #12] [4] LOCKS - wakeup_all

}

