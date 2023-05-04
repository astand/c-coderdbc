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

std::string IndentedString(size_t n, const std::string& source, const char c)
{
  if (source.length() >= n)
  {
    return source;
  }
  else
  {
    std::string indent(n - source.length(), c);
    return source + indent;
  }
}

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

std::string str_trim(std::string s)
{
  size_t passed = 0;

  if (s.empty())
  {
    return s + '\n';
  }

  passed = 0;

  while (passed < s.size())
  {
    if (s[s.size() - passed - 1] > ' ')
    {
      break;
    }

    ++passed;
  }

  if (passed != 0)
  {
    // remove tail with non-printable values
    s.erase(s.size() - passed, passed);
  }

  return s;
}

template<char L, char U>
static inline bool in_range(const char c)
{
  return ((c >= L) && (c <= U));
}

static bool is_first_valid(const char c)
{
  return in_range<'a', 'z'>(c) || in_range<'A', 'Z'>(c) || c == '_';
}

static bool is_nonfirst_valid(const char c)
{
  return is_first_valid(c) || in_range<'0', '9'>(c);
}

std::string make_c_name(const std::string& s)
{
  std::string ret{};

  if (s.length() == 0)
  {
    return ret;
  }

  for (auto i = 0; i < s.length(); i++)
  {
    if ((ret.length() == 0 && is_first_valid(s[i])) ||
      (ret.length() > 0 && is_nonfirst_valid(s[i])))
    {
      // basic C-identifier rule
      ret += s[i];
    }
    else if (s[i] == ' ')
    {
      // special case for whitespaces
      ret += '_';
    }
  }

  return ret;
}
