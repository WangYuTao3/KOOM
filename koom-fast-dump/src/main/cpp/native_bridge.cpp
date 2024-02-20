/*
 * Copyright (c) 2021. Kwai, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Created by Qiushi Xue <xueqiushi@kuaishou.com> on 2021.
 *
 */

#include <android/log.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <hprof_dump.h>
#include <jni.h>
#include <kwai_linker/kwai_dlfcn.h>
#include <log/log.h>
#include <pthread.h>
#include <unistd.h>
#include <wait.h>

#include <string>

#undef LOG_TAG
#define LOG_TAG "JNIBridge"

using namespace kwai::leak_monitor;
char* strToChar(JNIEnv* env, jstring jstr) {
  char* rtn = NULL;
  jclass clsstring = env->FindClass("java/lang/String");
  jstring strencode = env->NewStringUTF("GB2312");
  jmethodID mid = env->GetMethodID(clsstring, "getBytes",
                                   "(Ljava/lang/String;)[B");
  jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
  jsize alen = env->GetArrayLength(barr);
  jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
  if (alen > 0) {
    rtn = (char*) malloc(alen + 1);
    memcpy(rtn, ba, alen);
    rtn[alen] = 0;
  }
  env->ReleaseByteArrayElements(barr, ba, 0);
  return rtn;
}

#ifdef __cplusplus
extern "C" {
#endif

/**
 * JNI bridge for hprof dump
 */
JNIEXPORT void JNICALL Java_com_kwai_koom_fastdump_ForkJvmHeapDumper_nativeInit(
    JNIEnv *env ATTRIBUTE_UNUSED, jobject jobject ATTRIBUTE_UNUSED) {
  HprofDump::GetInstance().Initialize();
}

JNIEXPORT jint JNICALL
Java_com_kwai_koom_fastdump_ForkJvmHeapDumper_suspendAndFork(
    JNIEnv *env ATTRIBUTE_UNUSED, jobject jobject ATTRIBUTE_UNUSED, jstring path) {
  char *c = strToChar(env,path);
  return HprofDump::GetInstance().SuspendAndFork(c);
}

JNIEXPORT void JNICALL
Java_com_kwai_koom_fastdump_ForkJvmHeapDumper_exitProcess(
    JNIEnv *env ATTRIBUTE_UNUSED, jobject jobject ATTRIBUTE_UNUSED) {
  ALOGI("process %d will exit!", getpid());
  _exit(0);
}

JNIEXPORT jboolean JNICALL
Java_com_kwai_koom_fastdump_ForkJvmHeapDumper_resumeAndWait(
    JNIEnv *env ATTRIBUTE_UNUSED, jobject jobject ATTRIBUTE_UNUSED, jint pid) {
  return HprofDump::GetInstance().ResumeAndWait(pid);
}

#ifdef __cplusplus
}
#endif
