// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value)
{
	//TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("create_semaphore is not implemented yet");
	//Your Code is Here...
//	struct semaphore sem = smalloc(semaphoreName, sizeof(struct semaphore), 1);
//	sem.semdata->name=semaphoreName;
//	sem.semdata->count=value;
//	sem.semdata->lock=0;
//	LIST_INIT(sem.semdata->queue);
//	return sem;
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

void wait_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("wait_semaphore is not implemented yet");
	//Your Code is Here...
}

void signal_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("signal_semaphore is not implemented yet");
	//Your Code is Here...
//	dequeue(sem.semdata->queue);
}

int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}
