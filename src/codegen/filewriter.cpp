#include "filewriter.h"

void FileWriter::Flush()
{
	strm.clear();
}

void FileWriter::AppendText(const char* text, size_t len)
{
	strm.write(text, len);
}

void FileWriter::AppendLine(const char* text, size_t len)
{
	AppendText(text, len);
	AppendText("\n", 1);
}


void FileWriter::AppendText(const std::string& str)
{
	strm.str(str);
}

void FileWriter::AppendLine(const std::string& str)
{
	AppendText(str);
	AppendText("\n", 1);
}

