#include "vm_common.h"
#include "schd.h"
#include "dthread.h"
#include "interpApi.h"
#include "vmTime.h"
#include "AsyncIO.h"
#include "vm_app.h"

#ifdef DVM_LOG
#undef DVM_LOG
#endif

#define DVM_LOG     DVMTraceInf


volatile u1 schedulerFlag  = 0;
volatile u8 g_last_tick_record = 0;
volatile bool_t isDvmRunning = FALSE;


void Schd_InitThreadLists(void)
{
    Thread * guard = NULL;

    /*first node of readyList/otherList is guard,not used as normal thread*/
    guard = dthread_alloc(kMinStackSize);
    DVM_ASSERT(guard != NULL);
    guard->threadId = READY_GUARD_ID;
    readyThreadListHead = guard;

    guard = dthread_alloc(kMinStackSize);
    DVM_ASSERT(guard != NULL);
    guard->threadId = OTHER_GUARD_ID;
    otherThreadListHead = guard;

    guard = NULL;
    currentThread   = NULL;
    schedulerFlag = 0;

    dthread_create_ghost();
}

void Schd_FinalThreadLists(void)
{
    Thread * guard = readyThreadListHead->next;

    /*final ready list*/
    while(guard != NULL)
    {
        guard->pre->next = guard->next;
        if(guard->next != NULL)
        {
            guard->next->pre = guard->pre;
        }

        dthread_delete(guard);
        guard = readyThreadListHead->next;
    }
#if 0
    DVM_FREE((void*)(readyThreadListHead->interpStackStart - readyThreadListHead->interpStackSize));
    DVM_FREE(readyThreadListHead);
#else
    dthread_delete(readyThreadListHead);
#endif
    readyThreadListHead = NULL;

    /*final other list*/
    guard = otherThreadListHead->next;
    while(guard != NULL)
    {
        guard->pre->next = guard->next;
        if(guard->next != NULL)
        {
            guard->next->pre = guard->pre;
        }
        dthread_delete(guard);
        guard = otherThreadListHead->next;
    }
    dthread_delete(otherThreadListHead);
    otherThreadListHead = NULL;

    /**/
    if(currentThread != NULL)
    {
        dthread_delete(currentThread);
        currentThread = NULL;
    }

    dthread_delete_ghost();
}

void Schd_ChangeThreadState(Thread * thread,THREAD_STATE_E newState)
{
    DVM_ASSERT(thread != NULL);
    DVM_ASSERT(thread != readyThreadListHead);
    DVM_ASSERT(thread != otherThreadListHead);

    if(newState >THREAD_INITED && newState <=THREAD_DEAD)
    {
        thread->threadState = newState;
    }

    if(newState ==THREAD_DEAD)
    {
        u8 curTick = vmtime_getTickCount();
        DVM_LOG("create time:%ld \n",thread->creatTime);
        DVM_LOG("kill time:%ld \n",curTick);
        DVM_LOG("life time:%ld \n",curTick- thread->creatTime);
    }
    return;
}

/*must put READY STATE thread to the list head*/
void Schd_PushToReadyListHead(Thread * thread)
{
    Thread * tmp = readyThreadListHead;

    DVM_ASSERT(thread != readyThreadListHead);

    if(thread == NULL)
    {
        return;
    }

    /*step 1:delete thread node from list*/
    do
    {
        if(tmp == thread)
        {
            if(tmp->pre != NULL)
                tmp->pre->next = tmp->next;
            if(tmp->next!= NULL)
                tmp->next->pre = tmp->pre;
            break;
        }
        tmp = tmp->next;
    }while(tmp!=NULL);

    /*step 2:add thread node at list head*/
    thread->next = readyThreadListHead->next;
    if(readyThreadListHead->next != NULL)
    {
        readyThreadListHead->next->pre = thread;
    }
    readyThreadListHead->next = thread;
    thread->pre = readyThreadListHead;
}


/*must put READY STATE thread to the list tail*/
void Schd_PushToReadyListTail(Thread * thread)
{
    Thread * tmp = NULL;
    Thread * last = NULL;

    DVM_ASSERT(thread != readyThreadListHead);

    if(thread == NULL)
    {
        return;
    }

    tmp = readyThreadListHead;

    do
    {
        if(tmp == thread)
        {
            return;   /*find thread in the list,return,not need to move it to tail*/
        }
        last = tmp;
        tmp=tmp->next;
    }while(tmp!=NULL);

    last->next = thread;
    thread->next = NULL;
    thread->pre = last;
}

/*get next READY thread from ready list head*/
Thread * Schd_PopFromReadyList(void)
{
    Thread * tmp = readyThreadListHead->next;

    if(readyThreadListHead->next == NULL)
    {
        return NULL;
    }

    if(readyThreadListHead->next->next != NULL)
        readyThreadListHead->next->next->pre = readyThreadListHead;
    readyThreadListHead->next = readyThreadListHead->next->next;

    return tmp;
}

/*push non-READY thread to other list*/
void Schd_PushToOtherList(Thread * thread)
{
    Thread * tmp = otherThreadListHead;

    DVM_ASSERT(thread != otherThreadListHead);

    if(thread == NULL)
    {
        return;
    }

    /*step 1:delete thread node from list*/
    do
    {
        if(tmp == thread)
        {
            if(tmp->pre != NULL)
                tmp->pre->next = tmp->next;
            if(tmp->next!= NULL)
                tmp->next->pre = tmp->pre;
            break;
        }
        tmp = tmp->next;
    }while(tmp!=NULL);

    /*step 2:add thread node at list head*/
    thread->next = otherThreadListHead->next;
    if(otherThreadListHead->next != NULL)
    {
        otherThreadListHead->next->pre = thread;
    }
    otherThreadListHead->next = thread;
    thread->pre = otherThreadListHead;
}

/*get READY thread,one per pop,get untill return null*/
Thread * Schd_PopReadyFromOtherList(void)
{
    Thread * tmp = otherThreadListHead->next;
    Thread * ret = NULL;

    if(tmp == NULL)
    {
        return NULL;
    }

    do
    {
        if(tmp->threadState == THREAD_READY)
        {
            ret = tmp;
            tmp->pre->next = tmp->next;
            if(tmp->next !=NULL)
                tmp->next->pre = tmp->pre;

            return ret;
        }
        tmp = tmp->next;
    }while(tmp != NULL);

    return NULL;
}

Thread * Schd_PopFromOtherList(Thread * thd)
{
    Thread * tmp = otherThreadListHead->next;
    Thread * ret = NULL;

    if(tmp == NULL)
    {
        return NULL;
    }

    do
    {
        if(tmp == thd)
        {
            ret = tmp;
            tmp->pre->next = tmp->next;
            if(tmp->next !=NULL)
                tmp->next->pre = tmp->pre;

            return ret;
        }
        tmp = tmp->next;
    }while(tmp != NULL);

    return NULL;
}

void Schd_ResumeToReady(Thread *thd)
{
    if(thd == NULL || thd->threadState <= THREAD_ACTIVE)
    {
        return;
    }

    DVM_ASSERT(thd == Schd_PopFromOtherList(thd));
    thd->threadState = THREAD_READY;
    Schd_PushToReadyListHead(thd);
}

void Schd_DecSleepTime(u4 bkupTime)
{
    u8 this_tick = vmtime_getTickCount();
    Thread * tmp = otherThreadListHead->next;
    u8 deltaTime = this_tick - g_last_tick_record;

    //to avoid 'deltaTime >> 0'
    deltaTime = deltaTime > (2*bkupTime) ? (g_last_tick_record = this_tick,2*bkupTime) : (g_last_tick_record = this_tick,deltaTime);
    while(tmp != NULL)
    {
        switch(tmp->threadState)
        {
            case THREAD_TIME_SUSPENDED:
            {
                DVM_LOG("Thread %d is in sleep,deltaTime=%d\n",tmp->threadId,deltaTime);
                tmp->sleepTime = (tmp->sleepTime > deltaTime) ? (tmp->sleepTime - deltaTime) : (tmp->threadState = THREAD_READY,0) ;
            }
            break;

            case THREAD_TIMEWAIT_MONITOR_SUSPENDED:
            {
                //complish here
                tmp->sleepTime = (tmp->sleepTime > deltaTime) ? (tmp->sleepTime - deltaTime) : (tmp->threadState = THREAD_TRYGET_MONITOR_SUSPENDED,0) ;
                if(tmp->threadState == THREAD_TRYGET_MONITOR_SUSPENDED)
                {
                    //DVM_ASSERT(0);
                    Sync_bindTryLockToMonitor(tmp,false);
                }
            }
            break;

            case THREAD_ASYNCIO_SUSPENDED:
            {
                if(ASYNC_Signalled(tmp))
                {
                    tmp->threadState = THREAD_READY;
                    ASYNC_SetAIOState(tmp,ASYNC_IO_DONE);
                }
                else if(ASYNC_AllowTimeOut(tmp))
                {
                    tmp->sleepTime = (tmp->sleepTime > deltaTime) ? (tmp->sleepTime - deltaTime) : (tmp->threadState = THREAD_READY,0) ;
                    if(tmp->threadState == THREAD_READY)
                    {
                        ASYNC_SetAIOState(tmp,ASYNC_IO_TIMEOUT);
                    }
                }


            }
            break;

            default:break;
        }
        tmp = tmp->next;
    }

    tmp = NULL;
    do
    {
        tmp = Schd_PopReadyFromOtherList();
        if(tmp != NULL)
        {
            DVM_LOG("Thread %d sleep over!\n",tmp->threadId);
            Schd_PushToReadyListHead(tmp);
        }
    }while(tmp!=NULL);

}

/*delete and free DEAD state thread*/
void Schd_DelDeadThread(void)
{
    Thread * tmp  = otherThreadListHead->next;
    Thread * find = NULL;

    while(tmp != NULL)
    {
        if(tmp->threadState == THREAD_DEAD)
        {
            DVM_LOG("Destroy thread:%d \n",tmp->threadId);
            find = tmp->pre;
            tmp->pre->next = tmp->next;
            if(tmp->next != NULL)
            {
                tmp->next->pre = tmp->pre;
            }
#if 0
            DVM_FREE((void*)(tmp->interpStackStart - tmp->interpStackSize));
            DVM_FREE(tmp);
#else
            dthread_delete(tmp);
#endif
            tmp = find;
        }
        tmp = tmp->next;
    }
}

/*calc ready thread account*/
int Schd_ReadyThreadAccount(void)
{
    int accu = 0;
    Thread * tmp  = readyThreadListHead->next;

    while(tmp!=NULL)
    {
        accu++;
        tmp = tmp->next;
    }

    return accu;
}

/*calc other thread account,if don't want to put *
 *dead thread into account,call Schd_DelDeadThread() in advance */
int Schd_OtherThreadAccount(void)
{
    int accu = 0;
    Thread * tmp  = otherThreadListHead->next;

    while(tmp!=NULL)
    {
        accu++;
        tmp = tmp->next;
    }

    return accu;
}

/* tatal thread account,but dead ones exclude*/
int Schd_ThreadAccountInTotal(void)
{
    int accu = (currentThread != NULL) ? 1 : 0;

    Schd_DelDeadThread();
    accu += Schd_ReadyThreadAccount() + Schd_OtherThreadAccount();
    return accu;
}

Thread * Schd_FindThreadByJavaObj(Object * javaObj)
{
    Thread * find = NULL;
    Thread * tmp  = NULL;

    /*is current?*/
    if(currentThread != NULL && (currentThread->threadObj == javaObj))
    {
        find = currentThread;
        goto FIND_OVER;
    }

    /*find from ready list*/
    tmp  = readyThreadListHead->next;
    while(tmp!=NULL)
    {
        if(tmp->threadObj == javaObj)
        {
            find = tmp;
            goto FIND_OVER;
        }
        tmp = tmp->next;
    }

    /*find from other list*/
    tmp  = otherThreadListHead->next;
    while(tmp!=NULL)
    {
        if(tmp->threadObj == javaObj)
        {
            find = tmp;
            goto FIND_OVER;
        }
        tmp = tmp->next;
    }

FIND_OVER:
    return find;
}

void Schd_handleSpecial()
{
	//to check ota task
	vm_create_otaTask();

    //to check tck task
    vm_create_tckTask();

	//todo
}

#ifdef ARCH_ARM_SPD		
////WORKAROUND keywords: [dthing-workaround-nix-1] in workaround-readme.md
extern void workaround_alive_task_check(void);
extern void schd_check_timer_create(void);
extern void schd_check_timer_destroy(void);
#endif

void run_check_point(void){
#ifdef ARCH_ARM_SPD	
	////WORKAROUND keywords: [dthing-workaround-nix-1] in workaround-readme.md
	workaround_alive_task_check();
#endif
}

u8 Schd_GetLastRunTick(void){
	return g_last_tick_record;
}

void Schd_SCHEDULER(void)
{
    JValue pResult;
    unsigned long lastMs = vmtime_getTickCount();
    g_last_tick_record = lastMs;

#ifdef ARCH_ARM_SPD	
	schd_check_timer_create();
#endif


    while(1)
    {
    SCHD_RESTART:
        isDvmRunning = TRUE;
        
        Schd_handleSpecial();
        
        if(currentThread != NULL)
        {
            if(currentThread->threadState == THREAD_ACTIVE)
            {
                currentThread->threadState = THREAD_READY;
                Schd_PushToReadyListTail(currentThread);
                currentThread = NULL;
            }
            else
            {
                Schd_PushToOtherList(currentThread);
                currentThread = NULL;
            }
        }

        currentThread = Schd_PopFromReadyList();

        DVM_LOG("\nThis loop Time run %d Ms\n",vmtime_getTickCount() - lastMs);
        lastMs = vmtime_getTickCount();
        if(currentThread != NULL)
        {
            currentThread->threadState = THREAD_ACTIVE;
            pResult.j = 0;
            vmtime_startTimer();
            //call interp!
#ifdef ARCH_X86
            (*currentThread->cb)(currentThread);
#endif
            dvmInterpretEntry(currentThread,&pResult);

            /*Thread exe over*/
            if(pResult.j == 0xacacacac )
            {
                DVM_LOG("The thread(id:%d) exe over!\n",currentThread->threadId);
                Schd_ChangeThreadState(currentThread,THREAD_DEAD);
            }
        }
        else    /*no threads to run*/
        {
            Schd_DelDeadThread();

            if(Schd_ThreadAccountInTotal() ==0)
            {
                isDvmRunning = FALSE;
                goto SCHD_END;
            }
            vmtime_startTimer();
            do{
                DVM_LOG("No ready thread,waiting...\n");
                vmtime_sleep(10);
            }while(!CAN_SCHEDULE());
        }

        Schd_DecSleepTime(SCHEDULER_TIMER);

		run_check_point();

        goto SCHD_RESTART;

    SCHD_END:
        isDvmRunning = FALSE;
        DVM_LOG("Schd over!\n");
        break;
    }

#ifdef ARCH_ARM_SPD	
	schd_check_timer_destroy();
#endif
}

int IsDvmRunning()
{
    return (int)isDvmRunning;
}

/*API to interpret*/
void Schd_Interpret(Thread * self)
{
    //
}


