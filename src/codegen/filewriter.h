#pragma once

#include <stdint.h>

#include <string>
#include <sstream>

class FileWriter {
 public:

  void Flush();
  void Flush(const std::string& filename);

  void Append(const char* frmt, ...);
  void Append(const std::string& str);

  void AppendText(const char* frmt, ...);
  void AppendText(const std::string& str);

  void Append(size_t empty_lines = 1);

 private:

  void NewLine(const char c = ' ');

  std::stringstream strm;

};
