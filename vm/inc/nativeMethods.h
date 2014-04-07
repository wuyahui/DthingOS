/* DO NOT EDIT THIS FILE - it is machine generated */
#include <kni.h>

/* Header for native method signature information. */

#ifndef __NATIVE_METHODS_H__
#define __NATIVE_METHODS_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* name;
    const char* signature;
    KniFunc fnPtr;
} KniNativeMethodInfo;

typedef struct {
    const char* classpath;
    KniNativeMethodInfo* methods;
    int methodCount;
} KniClassNativeMethodInfo;

#define NATIVE_CLASSES_COUNT 17

/* com.yarlungsoft.util.SystemInputStream native APIs */
KniNativeMethodInfo gJava_com_yarlungsoft_util_SystemInputStream[1] = {
    {"readN", "([BII)I", (KniFunc)Java_com_yarlungsoft_util_SystemInputStream_readN},
};

/* com.yarlungsoft.util.SystemPrintStream native APIs */
KniNativeMethodInfo gJava_com_yarlungsoft_util_SystemPrintStream[2] = {
    {"flush", "()V",  (KniFunc)Java_com_yarlungsoft_util_SystemPrintStream_flush},
    {"write", "(I)V", (KniFunc)Java_com_yarlungsoft_util_SystemPrintStream_write},
};

/* java.io.File native APIs */
KniNativeMethodInfo gJava_java_io_File[8] = {
    {"createFile0",   "(Ljava/lang/String;)Z",                   (KniFunc)Java_java_io_File_createFile0},
    {"delete0",       "(Ljava/lang/String;)Z",                   (KniFunc)Java_java_io_File_delete0},
    {"exists0",       "(Ljava/lang/String;)Z",                   (KniFunc)Java_java_io_File_exists0},
    {"isDirectory0",  "(Ljava/lang/String;)Z",                   (KniFunc)Java_java_io_File_isDirectory0},
    {"isFile0",       "(Ljava/lang/String;)Z",                   (KniFunc)Java_java_io_File_isFile0},
    {"lastModified0", "(Ljava/lang/String;)J",                   (KniFunc)Java_java_io_File_lastModified0},
    {"length0",       "(Ljava/lang/String;)J",                   (KniFunc)Java_java_io_File_length0},
    {"list0",         "(Ljava/lang/String;)[Ljava/lang/String;", (KniFunc)Java_java_io_File_list0},
};

/* java.lang.AsyncIO native APIs */
KniNativeMethodInfo gJava_java_lang_AsyncIO[3] = {
    {"getCurNotifierState", "()I",  (KniFunc)Java_java_lang_AsyncIO_getCurNotifierState},
    {"setCurNotifierState", "(I)I", (KniFunc)Java_java_lang_AsyncIO_setCurNotifierState},
    {"waitSignalOrTimeOut", "()V",  (KniFunc)Java_java_lang_AsyncIO_waitSignalOrTimeOut},
};

/* java.lang.Class native APIs */
KniNativeMethodInfo gJava_java_lang_Class[7] = {
    {"forName",          "(Ljava/lang/String;)Ljava/lang/Class;", (KniFunc)Java_java_lang_Class_forName},
    {"isInstance",       "(Ljava/lang/Object;)Z",                 (KniFunc)Java_java_lang_Class_isInstance},
    {"isAssignableFrom", "(Ljava/lang/Class;)Z",                  (KniFunc)Java_java_lang_Class_isAssignableFrom},
    {"isInterface",      "()Z",                                   (KniFunc)Java_java_lang_Class_isInterface},
    {"isArray",          "()Z",                                   (KniFunc)Java_java_lang_Class_isArray},
    {"newInstance0",     "()Ljava/lang/Object;",                  (KniFunc)Java_java_lang_Class_newInstance0},
    {"getName",          "()Ljava/lang/String;",                  (KniFunc)Java_java_lang_Class_getName},
};

/* java.lang.DThread native APIs */
KniNativeMethodInfo gJava_java_lang_DThread[5] = {
    {"start0",         "()V",                   (KniFunc)Java_java_lang_Thread_start},
    {"sleep0",         "(J)V",                  (KniFunc)Java_java_lang_Thread_sleep},
    {"activeCount0",   "()I",                   (KniFunc)Java_java_lang_Thread_activeCount},
    {"currentThread0", "()Ljava/lang/DThread;", (KniFunc)Java_java_lang_Thread_currentThread},
    {"isAlive0",       "()Z",                   (KniFunc)Java_java_lang_Thread_isAlive},
};

/* java.lang.Double native APIs */
KniNativeMethodInfo gJava_java_lang_Double[3] = {
    {"doubleToLongBits",    "(D)J", (KniFunc)Java_java_lang_Double_doubleToLongBits},
    {"doubleToRawLongBits", "(D)J", (KniFunc)Java_java_lang_Double_doubleToRawLongBits},
    {"longBitsToDouble",    "(J)D", (KniFunc)Java_java_lang_Double_longBitsToDouble},
};

/* java.lang.Float native APIs */
KniNativeMethodInfo gJava_java_lang_Float[3] = {
    {"floatToIntBits",    "(F)I", (KniFunc)Java_java_lang_Float_floatToIntBits},
    {"floatToRawIntBits", "(F)I", (KniFunc)Java_java_lang_Float_floatToRawIntBits},
    {"intBitsToFloat",    "(I)F", (KniFunc)Java_java_lang_Float_intBitsToFloat},
};

/* java.lang.Math native APIs */
KniNativeMethodInfo gJava_java_lang_Math[4] = {
    {"cos",  "(D)D", (KniFunc)Java_java_lang_Math_cos},
    {"sin",  "(D)D", (KniFunc)Java_java_lang_Math_sin},
    {"sqrt", "(D)D", (KniFunc)Java_java_lang_Math_sqrt},
    {"tan",  "(D)D", (KniFunc)Java_java_lang_Math_tan},
};

/* java.lang.Object native APIs */
KniNativeMethodInfo gJava_java_lang_Object[6] = {
    {"internalClone", "(Ljava/lang/Cloneable;)Ljava/lang/Object;", (KniFunc)Java_java_lang_Object_internalClone},
    {"getClass",      "()Ljava/lang/Class;",                       (KniFunc)Java_java_lang_Object_getClass},
    {"hashCode",      "()I",                                       (KniFunc)Java_java_lang_Object_hashCode},
    {"notify",        "()V",                                       (KniFunc)Java_java_lang_Object_notify},
    {"notifyAll",     "()V",                                       (KniFunc)Java_java_lang_Object_notifyAll},
    {"wait",          "(JI)V",                                     (KniFunc)Java_java_lang_Object_wait},
};

/* java.lang.Runtime native APIs */
KniNativeMethodInfo gJava_java_lang_Runtime[3] = {
    {"freeMemory",  "()J", (KniFunc)Java_java_lang_Runtime_freeMemory},
    {"totalMemory", "()J", (KniFunc)Java_java_lang_Runtime_totalMemory},
    {"gc",          "()V", (KniFunc)Java_java_lang_Runtime_gc},
};

/* java.lang.String native APIs */
KniNativeMethodInfo gJava_java_lang_String[1] = {
    {"intern", "()Ljava/lang/String;", (KniFunc)Java_java_lang_String_intern},
};

/* java.lang.System native APIs */
KniNativeMethodInfo gJava_java_lang_System[3] = {
    {"arraycopy",         "(Ljava/lang/Object;ILjava/lang/Object;II)V", (KniFunc)Java_java_lang_System_arraycopy},
    {"currentTimeMillis", "()J",                                        (KniFunc)Java_java_lang_System_currentTimeMillis},
    {"identityHashCode",  "(Ljava/lang/Object;)I",                      (KniFunc)Java_java_lang_System_identityHashCode},
};

/* java.lang.Thread native APIs */
KniNativeMethodInfo gJava_java_lang_Thread[11] = {
    {"activeCount",   "()I",                   (KniFunc)Java_java_lang_Thread_activeCount},
    {"currentThread", "()Ljava/lang/Thread;",  (KniFunc)Java_java_lang_Thread_currentThread},
    {"interrupt",     "()V",                   (KniFunc)Java_java_lang_Thread_interrupt},
    {"interrupted",   "()Z",                   (KniFunc)Java_java_lang_Thread_interrupted},
    {"isAlive",       "()Z",                   (KniFunc)Java_java_lang_Thread_isAlive},
    {"isInterrupted", "()Z",                   (KniFunc)Java_java_lang_Thread_isInterrupted},
    {"setPriority",   "(I)V",                  (KniFunc)Java_java_lang_Thread_setPriority},
    {"sleep",         "(JI)V",                 (KniFunc)Java_java_lang_Thread_sleep},
    {"start",         "()V",                   (KniFunc)Java_java_lang_Thread_start},
    {"yield",         "()V",                   (KniFunc)Java_java_lang_Thread_yield},
    {"holdsLock",     "(Ljava/lang/Object;)Z", (KniFunc)Java_java_lang_Thread_holdsLock},
};

/* java.lang.Throwable native APIs */
KniNativeMethodInfo gJava_java_lang_Throwable[1] = {
    {"printStackTrace0", "(Ljava/lang/Object;)V", (KniFunc)Java_java_lang_Throwable_printStackTrace0},
};

/* java.net.NetNativeBridge native APIs */
KniNativeMethodInfo gJava_java_net_NetNativeBridge[9] = {
    {"isNetworkInited", "()Z",          (KniFunc)Java_java_net_NetNativeBridge_isNetworkInited},
    {"startUpNetwork",  "([B)I",        (KniFunc)Java_java_net_NetNativeBridge_startUpNetwork},
    {"socket0",         "(Z)I",         (KniFunc)Java_java_net_NetNativeBridge_socket0},
    {"connect0",        "(IIII)I",      (KniFunc)Java_java_net_NetNativeBridge_connect0},
    {"recv0",           "(I[BII)I",     (KniFunc)Java_java_net_NetNativeBridge_recv0},
    {"send0",           "(I[BII)I",     (KniFunc)Java_java_net_NetNativeBridge_send0},
    {"recvfrom0",       "(I[BIIIIII)I", (KniFunc)Java_java_net_NetNativeBridge_recvfrom0},
    {"sendto0",         "(I[BIIIII)I",  (KniFunc)Java_java_net_NetNativeBridge_sendto0},
    {"closeSocket0",    "(I)I",         (KniFunc)Java_java_net_NetNativeBridge_closeSocket0},
};

/* java.util.TimeZone native APIs */
KniNativeMethodInfo gJava_java_util_TimeZone[1] = {
    {"getDefaultId", "()Ljava/lang/String;", (KniFunc)Java_java_util_TimeZone_getDefaultId},
};

KniClassNativeMethodInfo gNativeMthTab[NATIVE_CLASSES_COUNT] = {
    {"Lcom/yarlungsoft/util/SystemInputStream;", gJava_com_yarlungsoft_util_SystemInputStream, 1},
    {"Lcom/yarlungsoft/util/SystemPrintStream;", gJava_com_yarlungsoft_util_SystemPrintStream, 2},
    {"Ljava/io/File;",                           gJava_java_io_File,                           8},
    {"Ljava/lang/AsyncIO;",                      gJava_java_lang_AsyncIO,                      3},
    {"Ljava/lang/Class;",                        gJava_java_lang_Class,                        7},
    {"Ljava/lang/DThread;",                      gJava_java_lang_DThread,                      5},
    {"Ljava/lang/Double;",                       gJava_java_lang_Double,                       3},
    {"Ljava/lang/Float;",                        gJava_java_lang_Float,                        3},
    {"Ljava/lang/Math;",                         gJava_java_lang_Math,                         4},
    {"Ljava/lang/Object;",                       gJava_java_lang_Object,                       6},
    {"Ljava/lang/Runtime;",                      gJava_java_lang_Runtime,                      3},
    {"Ljava/lang/String;",                       gJava_java_lang_String,                       1},
    {"Ljava/lang/System;",                       gJava_java_lang_System,                       3},
    {"Ljava/lang/Thread;",                       gJava_java_lang_Thread,                       11},
    {"Ljava/lang/Throwable;",                    gJava_java_lang_Throwable,                    1},
    {"Ljava/net/NetNativeBridge;",               gJava_java_net_NetNativeBridge,               9},
    {"Ljava/util/TimeZone;",                     gJava_java_util_TimeZone,                     1},
};

#ifdef __cplusplus
}
#endif
#endif // __NATIVE_METHODS_H__
