#include <stdio.h>
#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <sys/mman.h>
#include <bits/sysconf.h>
#include "vm/Common.h"
#include <dlfcn.h>
#include "inlineHook.h"
#include "DexFile.h"
#include "DexClass.h"
#import "com_shark_androidinlinehook_MainActivity.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "SharkChilli", __VA_ARGS__))

//定义DexReadAndVerifyClassData方法类型
typedef DexClassData *(*DexReadAndVerifyClassData)(const u1 **, const u1 *);
//定义DexClassDef方法类型
typedef const DexClassDef *(*OldDexFindClass)(const DexFile *pFile, const char *descriptor);
//定义OldDexFindClass用来保存被hook的方法
OldDexFindClass oldDexFindClass;

//打开的so文件
void *dvmLib;

//打印指令函数
void printMethodInsns(const DexFile *pFile, DexMethod *pDexMethod) {
    const DexCode *dexCode = dexGetCode(pFile, pDexMethod);
    LOGI("method insns size:%d", dexCode->insnsSize);
    const u2 *insns = dexCode->insns;
    for (int i = 0; i < dexCode->insnsSize; i++) {
        LOGI("insns:%d", (*(insns++)));
    }
}

//修改内存读写属性
int changeMemWrite(int start_add) {
    /* 获取操作系统一个页的大小, 一般是 4KB == 4096 */
    long page_size = sysconf(_SC_PAGESIZE);
    //必须是整页修改，不然无效
    start_add = start_add - (start_add % page_size);
    //LOGI("code add:0x%x, pagesize:%d", start_add, (int)page_size);
    int result = mprotect((void *) start_add, page_size, PROT_READ | PROT_WRITE);
    return result;
}



const DexClassDef *newDexFindClass(const DexFile *pFile, const char *descriptor) {
    //只关注需要修改的类Lcom/shark/calculate/CoreUtils;
    int cmp = strcmp("Lcom/shark/calculate/CoreUtils;", descriptor);
    if (cmp == 0) {
        //执行原来的逻辑得到类结构信息
        const DexClassDef *pClassDef = oldDexFindClass(pFile, descriptor);
        if (pClassDef == NULL) {
            return pClassDef;
        }
        //打印信息
        LOGI("class def:%d", (int)pClassDef);
        LOGI("class dex find class name:%s", descriptor);
        //我们需要调用DexReadAndVerifyClassData得到DexClassData代码结构，所以需要得到其地址
        //依然需要用IDA打开libdvm.so文件查看DexReadAndVerifyClassData函数的导出名称：
        DexReadAndVerifyClassData getClassData = (DexReadAndVerifyClassData) dlsym(
                dvmLib, "_Z25dexReadAndVerifyClassDataPPKhS0_");
        const u1 *pEncodedData = dexGetClassData(pFile, pClassDef);
        DexClassData *pClassData = getClassData(&pEncodedData, NULL);

        DexClassDataHeader header = pClassData->header;
        //打印对象方法数量
        LOGI("method size:%d", header.directMethodsSize);
        //得到首个对象方法的指针
        DexMethod *pDexDirectMethod = pClassData->directMethods;
        u1 *ptr = (u1 *) pDexDirectMethod;
        //循环遍历每个方法
        for (int i = 0; i < header.directMethodsSize; i++) {
            //这里每个方法都是相邻的，每个大小都是DexMethod结构体的大小
            pDexDirectMethod = (DexMethod *) (ptr + sizeof(DexMethod) * i);
            //得到方法名称
            const DexMethodId *methodId = dexGetMethodId(pFile, pDexDirectMethod->methodIdx);
            const char *methodName = dexStringById(pFile, methodId->nameIdx);
            //如果是getPwd方法就进行替换逻辑
            if (strcmp("getPwd", methodName) == 0) {
                LOGI("pDexDirectMethod methodName:%s", methodName);
                //打印指令
                printMethodInsns(pFile, pDexDirectMethod);
                //修改内存页属性
                int start_add = (int) (pFile->baseAddr + pDexDirectMethod->codeOff);

                int result = changeMemWrite(start_add);
                LOGI("mp result:%d", result);

                //获取方法对应DexCode结构
                DexCode *dexCode = (DexCode *) dexGetCode(pFile, pDexDirectMethod);
                //下面就是覆盖指令了
                u2 new_ins[3] = {26, 33, 17};
                memcpy(dexCode->insns, &new_ins, 3 * sizeof(u2));
                printMethodInsns(pFile,pDexDirectMethod);
            }
        }
        return pClassDef;
    } else{
        //执行原来的逻辑
        return oldDexFindClass(pFile,descriptor);
    }

}

void *getDexFindClass(void *funclib) {
    void *func = dlsym(funclib, "_Z12dexFindClassPK7DexFilePKc");
    LOGI("func:%p", func);
    if (registerInlineHook((uint32_t) func,
                           (uint32_t) newDexFindClass, (uint32_t **) &oldDexFindClass) !=
        ELE7EN_OK) {
        LOGI("registerInlineHook ERROR");
        return NULL;
    }
    return func;
}

void hookDvm() {
    dvmLib = dlopen("/system/lib/libdvm.so", RTLD_LAZY);
    LOGI("dvmLib:%p", dvmLib);
    void *func = getDexFindClass(dvmLib);

    if (inlineHook((uint32_t) func) != ELE7EN_OK) {
        LOGI("inlineHook ERROR");
        return;
    }
}

/*
 * Class:     com_shark_androidinlinehook_MainActivity
 * Method:    startInlineHook
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_shark_androidinlinehook_MainActivity_startInlineHook
        (JNIEnv *, jclass) {
    LOGI("hook start");
    hookDvm();
    LOGI("hook end");
}

