#include "formatter.h"
#include <algorithm>

static const size_t kMaxWorkArrLength = 4096;

static char work_buff[kMaxWorkArrLength] = { 0 };

static const std::string __typeprint[8] =
{
  "int8_t",
  "int16_t",
  "int32_t",
  "int64_t",
  "uint8_t",
  "uint16_t",
  "uint32_t",
  "uint64_t"
};

const char* StrPrint(const char* format, ...)
{
  va_list args;
  va_start(args, format);

  vsnprintf(work_buff, kMaxWorkArrLength, format, args);

  va_end(args);
  return work_buff;
}

std::string PrintType(uint8_t id)
{
  if (id < 8)
  {
    return __typeprint[id];
  }

  return "";
}

std::string str_toupper(std::string s)
{
  std::transform(s.begin(), s.end(), s.begin(),
    [](unsigned char c)
  {
    return std::toupper(c);
  });
  return s;
}

std::string str_tolower(std::string s)
{
  std::transform(s.begin(), s.end(), s.begin(),
    [](unsigned char c)
  {
    return std::tolower(c);
  });
  return s;
}
