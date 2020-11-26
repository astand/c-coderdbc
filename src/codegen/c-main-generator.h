#pragma once

#include <stdint.h>
#include "c-sigprinter.h"
#include "../types/message.h"

class CiMainGenerator
{
public:
	CiMainGenerator();

	void Generate(std::vector<MessageDescriptor_t*>& msgs, std::string drvname, std::string dirpath);

private:
	
	bool SetFinalPath(std::string dirpath);

private:

	std::string work_dir_path;
	
	std::string workstr;

	CSigPrinter* sigprt;

};
