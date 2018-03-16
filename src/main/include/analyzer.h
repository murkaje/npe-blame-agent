#pragma once

#include "CodeAttribute.h"

std::string traceDetailedCause(const ConstPool &constPool, const CodeAttribute &code, const LocalVariableTable &vars, size_t location, size_t stackExcess);

std::string describeNPEInstruction(const ConstPool &cp, const CodeAttribute &code, const LocalVariableTable &vars, size_t location);