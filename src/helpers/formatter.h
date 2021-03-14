#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>

const char* StrPrint(const char* format, ...);

std::string PrintType(uint8_t tid);

std::string str_toupper(std::string s);

std::string str_tolower(std::string s);

std::string str_trim(std::string s);
