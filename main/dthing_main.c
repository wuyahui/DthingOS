#include <std_global.h>
#include "vm_common.h"

#if defined(ARCH_ARM_SPD)
#include <priority_app.h>
#endif

extern int32_t DVM_main(int32_t argc, int8_t * argv[]);

#if defined(ARCH_X86)

int main(int argc, char *argv[])
{
	DVM_main(argc, argv);
	return 1;
}

#elif defined(ARCH_ARM_SPD)

#define DvmLog SCI_TRACE_LOW

#define DTHING_THREAD_STACK_SIZE         (8*1024)
#define DTHING_THREAD_QUEUE_SIZE         (20*sizeof(uint32)*SCI_QUEUE_ITEM_SIZE)

uint32_t dvm_threadId = SCI_INVALID_BLOCK_ID;
uint8*   g_dvm_mem_space_ptr = NULL;

static void Dthing_IThreadProc(int argc, void * argv)
{
    int status =0;
    int    arg = 2;
    char * arga[2] = {"-cp","D:\\helloword.dex"};

    DVM_main(arg,arga);
    
    if(SCI_INVALID_BLOCK_ID != dvm_threadId)
    {
        status = SCI_TerminateThread(dvm_threadId);
        status = SCI_DeleteThread(dvm_threadId);
        dvm_threadId = SCI_INVALID_BLOCK_ID;
    }

    if(NULL != g_dvm_mem_space_ptr)
    {
        SCI_FREE(g_dvm_mem_space_ptr);
        g_dvm_mem_space_ptr = PNULL;
    }
}

bool_t Dthing_startVM(void)
{
    DvmLog("[Dthing log]:g_dvm_mem_space_ptr try to alloc");
    

    DvmLog("[Dthing log]:g_dvm_mem_space_ptr try to alloc");
    //try to alloc java heap
    g_dvm_mem_space_ptr = (uint8*)SCI_ALLOCA(/*1024*2048*/ DTHING_THREAD_STACK_SIZE +DTHING_THREAD_QUEUE_SIZE);

    DvmLog("[Dthing log]:g_dvm_mem_space_ptr=0x%x",g_dvm_mem_space_ptr);
    
    if(PNULL == g_dvm_mem_space_ptr)
    {
        DvmLog("[Dthing log]:g_dvm_mem_space_ptr MMI_BL_MALLOC failed");
        return FALSE;
    }

    dvm_threadId = SCI_CreateAppThreadEx
                (
                    "DthingTask",
                    Dthing_IThreadProc,
                    0,
                    NULL,
                    g_dvm_mem_space_ptr,
                    DTHING_THREAD_STACK_SIZE,
                    PRI_JBED_TASK,
                    SCI_PREEMPT,
                    "DthingQueue",
                    (g_dvm_mem_space_ptr+DTHING_THREAD_STACK_SIZE),
                    DTHING_THREAD_QUEUE_SIZE,
                    SCI_AUTO_START
                );
                
    DvmLog("[Dthing log]:dvm_threadId = %d ",dvm_threadId);                

    return TRUE;
}
#endif