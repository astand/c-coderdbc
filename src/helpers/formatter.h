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

/**
 * @brief Makes input string valid C-identifier
 *
 * @param s source string
 * @return std::string
 */
std::string make_c_name(const std::string& s);
