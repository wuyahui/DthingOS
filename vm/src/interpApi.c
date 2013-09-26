
#include <vm_common.h>
#include <interpApi.h>
#include <interpState.h>
#include <interpOpcode.h>
#include <dthread.h>
#include <kni.h>
#include <Object.h>


void dvmInterpretEntry(Thread * self,JValue *pResult)
{
    InterpSaveState interpSaveState;
    ExecutionSubModes savedSubModes;
	Method * method;
    /*
     * Save interpreter state from previous activation, linking
     * new to last.
     */
    interpSaveState = self->interpSave;
    self->interpSave.prev = &interpSaveState;
    /*
     * Strip out and save any flags that should not be inherited by
     * nested interpreter activation.
     */
    savedSubModes = (ExecutionSubModes)(
              self->interpBreak.ctl.subMode & LOCAL_SUBMODE);
    if (savedSubModes != kSubModeNormal) {
#if  __NIX__
        dvmDisableSubMode(self, savedSubModes);
#endif
    }


#if defined(WITH_TRACKREF_CHECKS)
    self->interpSave.debugTrackedRefStart =
        dvmReferenceTableEntries(&self->internalLocalRefTable);
#endif
    self->debugIsMethodEntry = true;


    /*
     * method must be set in advance
     */
	if(self->bInterpFirst)
	{
		method = (Method *)self->entryMthd;
		self->interpSave.method = method;
		self->interpSave.pc = method->insns;
	}
	else
	{
		method = (struct Method *)self->interpSave.method;
		//self->interpSave.pc = method->insns;
	}	
    self->interpSave.curFrame = (u4*) self->interpSave.curFrame;
    

    assert(!dvmIsNativeMethod(method));

#if __NIX__
    /*
     * Make sure the class is ready to go.  Shouldn't be possible to get
     * here otherwise.
     */
    if (method->clazz->status < CLASS_INITIALIZING ||
        method->clazz->status == CLASS_ERROR)
    {
        dvmDumpThread(self, false);
        dvmAbort();
    }
#endif

    // Call the interpreter
    dvmInterpretPortable(self);

    *pResult = self->interpSave.retval;

    /* Restore interpreter state from previous activation */
    //self->interpSave = interpSaveState;

#if __NIX__
    if (savedSubModes != kSubModeNormal) 
	{
        dvmEnableSubMode(self, savedSubModes);
    }	
#endif
}

#define SUPPORT_MAX_PARAMS  10

void dvmInterpretMakeNativeCall(const u4* args, JValue* pResult, const Method* method, Thread* self)
{
	KniFunc nativefunc = NULL;
	Object * thisClz = NULL;   //if static: class ptr;if non static :instance this ptr
	u4 argvs[SUPPORT_MAX_PARAMS] = {0};
	int idx = 0;
	int offset = 0;

	if(dvmIsStaticMethod(method))
	{
		/*nix: maybe error!*/
		thisClz = (Object*) method->clazz;
	}
	else
	{
		thisClz = ((Object*) args)->clazz ;  //the first param of Fp is "this ptr"
		offset++;
	}
	argvs[idx++] = (u4) thisClz;

	/*prase other params to argvs*/
	{
        const char* shorty = &method->shorty[1];        /* skip return type */
        while (*shorty != '\0') 
		{
#if 1
			argvs[idx] = (u4) args[offset];
			idx++;
			offset++;
			shorty++;
#else			
            switch (*shorty++) 
			{
            case 'L':
                printf("  local %d: 0x%08x", idx, args[offset]);
				argvs[idx] = (u4) args[offset];
				idx++;
                break;
            case 'D':
            case 'J':
                //idx++;
                break;
            default:
                /* Z B C S I -- do nothing */
                break;
            }
            //idx++;
			offset++;
#endif
        }
    }

	nativefunc = Kni_findFuncPtr(method);
	DVM_ASSERT(nativefunc != NULL);
	nativefunc(argvs,pResult);

}

/**/
void dvmCallClinitMethod(const Method* method, Object* obj)
{
	JValue pResult;
	dthread_fill_ghost(method,obj);
	dvmInterpretEntry(ghostThread,&pResult);
}

/*
 * Copy data for a fill-array-data instruction.  On a little-endian machine
 * we can just do a memcpy(), on a big-endian system we have work to do.
 *
 * The trick here is that dexopt has byte-swapped each code unit, which is
 * exactly what we want for short/char data.  For byte data we need to undo
 * the swap, and for 4- or 8-byte values we need to swap pieces within
 * each word.
 */
static void copySwappedArrayData(void* dest, const u2* src, u4 size, u2 width)
{
#ifdef DVM_BIG_ENDIAN
    int i;

    switch (width) {
    case 1:
        /* un-swap pairs of bytes as we go */
        for (i = (size-1) & ~1; i >= 0; i -= 2) {
            ((u1*)dest)[i] = ((u1*)src)[i+1];
            ((u1*)dest)[i+1] = ((u1*)src)[i];
        }
        /*
         * "src" is padded to end on a two-byte boundary, but we don't want to
         * assume "dest" is, so we handle odd length specially.
         */
        if ((size & 1) != 0) {
            ((u1*)dest)[size-1] = ((u1*)src)[size];
        }
        break;
    case 2:
        /* already swapped correctly */
        CRTL_memcpy(dest, src, size*width);
        break;
    case 4:
        /* swap word halves */
        for (i = 0; i < (int) size; i++) {
            ((u4*)dest)[i] = (src[(i << 1) + 1] << 16) | src[i << 1];
        }
        break;
    case 8:
        /* swap word halves and words */
        for (i = 0; i < (int) (size << 1); i += 2) {
            ((int*)dest)[i] = (src[(i << 1) + 3] << 16) | src[(i << 1) + 2];
            ((int*)dest)[i+1] = (src[(i << 1) + 1] << 16) | src[i << 1];
        }
        break;
    default:
        DVMTraceErr("Unexpected width %d in copySwappedArrayData", width);
        dvmAbort();
        break;
    }
#else
    CRTL_memcpy(dest, src, size*width);
#endif
}


/*
 * Fill the array with predefined constant values.
 *
 * Returns true if job is completed, otherwise false to indicate that
 * an exception has been thrown.
 */
bool_t dvmInterpHandleFillArrayData(ArrayObject* arrayObj, const u2* arrayData)
{
    u2 width;
    u4 size;

    if (arrayObj == NULL) {
        dvmThrowNullPointerException(NULL);
        return FALSE;
    }
    //assert (!IS_CLASS_FLAG_SET(((Object *)arrayObj)->clazz,
    //                           CLASS_ISOBJECTARRAY));

    /*
     * Array data table format:
     *  ushort ident = 0x0300   magic value
     *  ushort width            width of each element in the table
     *  uint   size             number of elements in the table
     *  ubyte  data[size*width] table of data values (may contain a single-byte
     *                          padding at the end)
     *
     * Total size is 4+(width * size + 1)/2 16-bit code units.
     */
    if (arrayData[0] != kArrayDataSignature) {
        dvmThrowInternalError("bad array data magic");
        return FALSE;
    }

    width = arrayData[1];
    size = arrayData[2] | (((u4)arrayData[3]) << 16);

    if (size > arrayObj->length) {
        dvmThrowArrayIndexOutOfBoundsException(arrayObj->length, size);
        return FALSE;
    }
    copySwappedArrayData(arrayObj->contents, &arrayData[4], size, width);
    return TRUE;
}

