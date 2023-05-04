#include <iostream>
#include <fstream>
#include "filewriter.h"


FileWriter::FileWriter()
{
}

FileWriter::~FileWriter()
{
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

void FileWriter::AppendText(const char* text)
{
  std::string str = text;
  AppendText(str);
}

void FileWriter::AppendLine(const char* text, int32_t post_empty_lines)
{
  AppendText(text);

  for (int32_t i = 0; i < post_empty_lines; i++)
  {
    AppendText("\n");
  }
}


void FileWriter::AppendText(const std::string& str)
{
  strm << str;
}

void FileWriter::AppendLine(const std::string& str)
{
  AppendText(str);
  AppendText("\n");
}

