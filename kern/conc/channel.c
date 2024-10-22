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

	acquire_spinlock(&ProcessQueues.qlock);

	release_spinlock(lk);

	struct Env* currentProcess = get_cpu_proc();
	// block the running process on the given channel
	currentProcess->env_status = ENV_BLOCKED;
	cprintf("in sleep, return value for holding_lock(qlock)between : %d  \n", holding_spinlock(&ProcessQueues.qlock));
//	&ProcessQueues.qlock.cpu = mycpu();
	enqueue(&chan->queue, currentProcess);

	// scheduling the next to be ready process;
	cprintf("in sleep before sched\n");
	sched();

	cprintf("in sleep after sched\n accuired spinlock given in parameter %d \n ", holding_spinlock(lk ));
	cprintf("alock value %d \n", holding_spinlock(&ProcessQueues.qlock) );
	// acquire lock when wake up
	acquire_spinlock(lk);

	release_spinlock(&ProcessQueues.qlock);
	cprintf("process queue lock release in sleep \n");

	//cprintf("in sleep, return value for holding_lock(qlock) after release: %d \n", holding_spinlock(&ProcessQueues.qlock));
	//TODO: [PROJECT'24.MS1 - #10] [4] LOCKS - sleep
	cprintf("END OF SLEEP\n");
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


	cprintf("in wakeup one 1st thing in func \n blocked processes:\nqlock value: %d", holding_spinlock(&ProcessQueues.qlock));
	struct Env* element;
	LIST_FOREACH(element, &chan->queue){
		cprintf("process with id : %d \n", element->env_id);
	}


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

	cprintf("in wake up one before sched_insert_ready \n");
	sched_insert_ready0(currentProcess);
	cprintf("in wake up one aftersched_insert_ready \n");
	cprintf("done insertion in ready\n");

	cprintf("process queue lock release in wakeup one\n");
	//b3d el print barg3 le sleep b3d el sched call
	release_spinlock(&ProcessQueues.qlock);
	cprintf("END OF WAKEUP_ONE\n qlock value : %d \n", holding_spinlock(&ProcessQueues.qlock));
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
	cprintf("in wakeup all\n size of vhan queue %d \n" , chan->queue.size);
	struct Env *element;
	acquire_spinlock(&ProcessQueues.qlock);
	LIST_FOREACH(element, &(chan->queue))
	{
		cprintf("in foreach with process id = %d \n", element->env_id);
		wakeup_one(chan);
	}
	release_spinlock(&ProcessQueues.qlock);
	//TODO: [PROJECT'24.MS1 - #12] [4] LOCKS - wakeup_all
	cprintf("END OF WAKEUP_ALL\n");
}

