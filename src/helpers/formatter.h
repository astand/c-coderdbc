#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>

std::string IndentedString(size_t n, const std::string& source, const char c = ' ');

const char* StrPrint(const char* format, ...);

std::string PrintType(uint8_t tid);

std::string str_toupper(std::string s);

std::string str_tolower(std::string s);

std::string str_trim(std::string s);

/// @brief Function prints double value with dropping tailing zeros
/// @param value value to format
/// @param presicsion maximal precision length
/// @param usedot true for forcibly print presicion 1 (one digit after dot)
/// @return pointer to internal char array with value representation
const char* prt_double(double value, size_t presicsion, bool usedot = true);

/**
 * @brief Makes input string valid C-identifier
 *
 * @param s source string
 * @return std::string
 */
std::string make_c_name(const std::string& s);

template<size_t N = 4096>
std::string StrPrintLoc(const char* format, ...)
{
  va_list args;
  va_start(args, format);

  // TODO: make N sanitizing here to prevent memory errors
  char work_buff[N] = {0};

  vsnprintf(work_buff, N, format, args);

  va_end(args);

  // make string from local array
  return work_buff;
}
