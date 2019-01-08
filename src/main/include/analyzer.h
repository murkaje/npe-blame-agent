#pragma once

#include "bytecode/CodeAttribute.h"
#include "bytecode/Method.h"

std::string traceDetailedCause(const ConstPool &cp, const CodeAttribute &code, const LocalVariableTable &vars, size_t location, size_t stackExcess);

std::string describeNPEInstruction(const Method &currentFrameMethod, const ConstPool &cp, const CodeAttribute &code, const LocalVariableTable &vars, size_t location);