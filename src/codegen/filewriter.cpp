#include <iostream>
#include <fstream>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include "filewriter.h"

template<size_t N = 10000>
std::string __print_loc__(const char* format, va_list args)
{
  char work_buff[N] = {0};
  auto ret = vsnprintf(work_buff, N, format, args);

  if (ret >= N)
  {
    std::cout << "Attention (!) the line has been shortened : " << std::endl;
    std::cout << work_buff << std::endl;
  }

  // make string from local array
  return work_buff;
}

void FileWriter::Flush()
{
  strm.clear();
}

void FileWriter::Flush(const std::string& fpath)
{
  std::ofstream wfile;
  wfile.open(fpath, std::ios::out);
  wfile << strm.rdbuf();
  wfile.close();
  Flush();
}

void FileWriter::Append(const char* frmt, ...)
{
  va_list args;
  va_start(args, frmt);
  auto ret = __print_loc__(frmt, args);
  va_end(args);
  Append(ret);
}

void FileWriter::AppendText(const char* frmt, ...)
{
  va_list args;
  va_start(args, frmt);
  auto ret = __print_loc__(frmt, args);
  va_end(args);
  AppendText(ret);
}

void FileWriter::AppendText(const std::string& str)
{
  strm << str;
}

void FileWriter::Append(const std::string& str)
{
  AppendText(str);

  if (str.empty() == false)
  {
    NewLine(str.back());
  }
}

void FileWriter::Append(size_t empty_lines)
{
  for (auto i = empty_lines; i != 0; --i)
  {
    NewLine();
  }
}

void FileWriter::NewLine(const char c)
{
  if (c != '\n')
  {
    strm << '\n';
  }
}

