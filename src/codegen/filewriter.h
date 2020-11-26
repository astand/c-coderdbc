#pragma once

#include <stdint.h>

#include <string>
#include <sstream>

class FileWriter
{
public:

	FileWriter();
	~FileWriter();

	void Flush();
	void AppendText(const char* text, size_t len);
	void AppendLine(const char* text, size_t len);

	void AppendText(const std::string& str);
	void AppendLine(const std::string& str);

private:

	std::stringstream strm;

};

FileWriter::FileWriter()
{
}

FileWriter::~FileWriter()
{
}