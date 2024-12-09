// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value) {
	//TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_semaphore is not implemented yet");
	//Your Code is Here...
	struct __semdata* sem = (struct __semdata*) smalloc(semaphoreName,
			sizeof(struct __semdata), 1);
	strcpy(sem->name, semaphoreName);
	sem->count = value;
	sem->lock = 0;

	sys_init_queue(&sem->queue);

	struct semaphore semaphor;
	semaphor.semdata = sem;

	return semaphor;
}
struct semaphore get_semaphore(int32 ownerEnvID, char* semaphoreName)
{
	//TODO: [PROJECT'24.MS3 - #03] [2] USER-LEVEL SEMAPHORE - get_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_semaphore is not implemented yet");
	//Your Code is Here...
	void* sem=sget(ownerEnvID,semaphoreName);
		if(sem==NULL){
			panic("Semaphore not found or no mem");
		}
		struct semaphore semaphor;
		semaphor.semdata=(struct __semdata*)sem;

		return semaphor;
}

void wait_semaphore(struct semaphore sem) {
	//TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wait_semaphore is not implemented yet");
	//Your Code is Here...
	while (xchg(&sem.semdata->lock, 1) != 0);
	cprintf("after aquiring semaphore lock\n");
	sem.semdata->count--;
	if (sem.semdata->count < 0) {
		cprintf("will block this env\n");
		sys_block_env_sem(&sem.semdata->queue, &sem.semdata->lock);
	}
	sem.semdata->lock=0;
	cprintf("released semaphore lock\n");
}

void signal_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("signal_semaphore is not implemented yet");
	//Your Code is Here...
	cprintf("will aquire semaphore lock in signal current lock value: %d\n", sem.semdata->lock);
	while (xchg(&sem.semdata->lock, 1) != 0);
	cprintf("aquired semaphore lock in signal\n");
	sem.semdata->count++;
	if(sem.semdata->count>=0){
		cprintf("will put env in ready queue\n");
		sys_release_env_sem(&sem.semdata->queue);
	}
	sem.semdata->lock=0;
	cprintf("released semaphore lock\n");
}

int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}
