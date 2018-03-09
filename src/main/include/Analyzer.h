#pragma once

#include <stack>
#include "CodeAttribute.h"

/*
 * aload_1
 * invi a(Object)Object
 * invi b(Object)Object
 * aconst_null
 * stack: slot1 -> a(Object)Object -> b(Object)Object, null
 *
 * Show only immediate source?
 * null,astore_x,aload_x,invoke ? (loaded from variable slot x, returned from aconst_null)?
 */

size_t traceInvokeInstance(const CodeAttribute &code, size_t invokeBci) {
  size_t pos;
  std::stack methodStack;

  uint8_t opCode = code.getOpcode(0);
  switch (opCode) {

  }
}