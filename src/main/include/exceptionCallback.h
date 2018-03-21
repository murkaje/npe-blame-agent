#pragma once

#include <vector>
#include <cstdint>

#include "jni.h"
#include "jvmti.h"

void JNICALL callback_Exception(jvmtiEnv *jvmti,
                                JNIEnv *jni,
                                jthread thread,
                                jmethodID method,
                                jlocation location,
                                jobject exception,
                                jmethodID catch_method,
                                jlocation catch_location);