#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>

const char* StrPrint(const char* format, ...);

std::string PrintType(uint8_t tid);
