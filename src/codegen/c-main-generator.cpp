#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <algorithm>

#include "c-main-generator.h"

static const size_t kMaxDirNum = 1000;

static const size_t kWBUFF_len = 2048;
static char wbuff[kWBUFF_len] = { 0 };

CiMainGenerator::CiMainGenerator()
{
  sigprt = new CSigPrinter;
}

void CiMainGenerator::Generate(std::vector<MessageDescriptor_t*>& msgs, std::string drvname, std::string dirpath)
{
  // Load income messages to sig printer  
  sigprt->LoadMessages(msgs);

  std::sort(sigprt->sigs_expr.begin(), sigprt->sigs_expr.end(),
    [](const CiExpr_t* a, const CiExpr_t* b) -> bool
    {
      return a->msg.MsgID < b->msg.MsgID;
    });

  SetFinalPath(dirpath);
  // 1 step is to define final directory for source code bunch

  // 2 step is to print main head file
  
  // 3 step is to print main source file

  // 4 step is to pring fmon head file

  // 5 step is to print fmon source file

}

bool CiMainGenerator::SetFinalPath(std::string dirpath)
{
  // find free directory
  struct stat info;

  for (int32_t dirnum = 0; dirnum < 1000; dirnum++)
  {
    snprintf(wbuff, kWBUFF_len, "%03d", dirnum);
    work_dir_path = dirpath + "/" + wbuff;

    if (stat(work_dir_path.c_str(), &info) != 0)
    {
      if (std::filesystem::create_directory(work_dir_path))
        return true;
      else
        return false;
    }
    else if (info.st_mode & S_IFDIR)
    {
      // directory exists, try next num
      continue;
    }
    else
    {
      if (std::filesystem::create_directory(work_dir_path) != 0)
        return false;
    }
  }

  return true;
}
