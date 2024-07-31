#include <sys/stat.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <cstdio>
#include <sstream>
#include "fs-creator.h"
#include "helpers/formatter.h"

static const int32_t kTmpLen = 1024;

static char _tmpb[kTmpLen];

static const char* kLibDir = "/lib";
static const char* kUsrDir = "/usr";
static const char* kIncDir = "/inc";
static const char* kConfDir = "/conf";
static const char* kUtilDir = "/butl";

FsCreator::FsCreator()
{
}


void FsCreator::Configure(const std::string& drvname,
  const std::string& outpath,
  const std::string& commoninfo,
  const std::string& driverinfo,
  uint32_t highVer,
  uint32_t lowVer)
{
  FS.file.libdir = outpath + kLibDir;
  FS.file.usrdir = outpath + kUsrDir;
  FS.file.incdir = outpath + kIncDir;
  FS.file.confdir = outpath + kConfDir;
  FS.file.utildir = outpath + kUtilDir;
  // directory valid and exists, set all the values
  FS.gen.DrvName_orig = drvname;
  FS.gen.DRVNAME = str_toupper(drvname);
  FS.gen.drvname = str_tolower(drvname);

  FS.file.core_h.dir = outpath;
  FS.file.core_h.fname = FS.gen.drvname + ".h";
  FS.file.core_h.fpath = FS.file.libdir + "/" + FS.file.core_h.fname;

  FS.file.core_c.dir = outpath;
  FS.file.core_c.fname = FS.gen.drvname + ".c";
  FS.file.core_c.fpath = FS.file.libdir + "/" + FS.file.core_c.fname;

  FS.file.util_h.dir = outpath;
  FS.file.util_h.fname = FS.gen.drvname + "-binutil" + ".h";
  FS.file.util_h.fpath = FS.file.utildir + "/" + FS.file.util_h.fname;

  FS.file.util_c.dir = outpath;
  FS.file.util_c.fname = FS.gen.drvname + "-binutil" + ".c";
  FS.file.util_c.fpath = FS.file.utildir + "/" + FS.file.util_c.fname;

  FS.file.fmon_h.dir = outpath;
  FS.file.fmon_h.fname = FS.gen.drvname + "-fmon.h";
  FS.file.fmon_h.fpath = FS.file.libdir + "/" + FS.file.fmon_h.fname;

  FS.file.fmon_c.dir = outpath;
  FS.file.fmon_c.fname = FS.gen.drvname + "-fmon.c";
  FS.file.fmon_c.fpath = FS.file.usrdir + "/" + FS.file.fmon_c.fname;

  snprintf(_tmpb, kTmpLen, "%s_USE_BITS_SIGNAL", FS.gen.DRVNAME.c_str());
  FS.gen.usebits_def = _tmpb;

  snprintf(_tmpb, kTmpLen, "%s_USE_DIAG_MONITORS", FS.gen.DRVNAME.c_str());
  FS.gen.usemon_def = _tmpb;

  snprintf(_tmpb, kTmpLen, "%s_USE_MONO_FMON", FS.gen.DRVNAME.c_str());
  FS.gen.usemonofmon_def = _tmpb;

  snprintf(_tmpb, kTmpLen, "%s_USE_SIGFLOAT", FS.gen.DRVNAME.c_str());
  FS.gen.usesigfloat_def = _tmpb;

  snprintf(_tmpb, kTmpLen, "%s_USE_CANSTRUCT", FS.gen.DRVNAME.c_str());
  FS.gen.usesruct_def = _tmpb;

  snprintf(_tmpb, kTmpLen, "%s_AUTO_ROLL", FS.gen.DRVNAME.c_str());
  FS.gen.useroll_def = _tmpb;

  snprintf(_tmpb, kTmpLen, "%s_AUTO_CSM", FS.gen.DRVNAME.c_str());
  FS.gen.usecsm_def = _tmpb;

  snprintf(_tmpb, kTmpLen, "VER_%s_MAJ", FS.gen.DRVNAME.c_str());
  FS.gen.verhigh_def = _tmpb;

  snprintf(_tmpb, kTmpLen, "VER_%s_MIN", FS.gen.DRVNAME.c_str());
  FS.gen.verlow_def = _tmpb;

  // load start info to fdescriptor
  FS.gen.start_driver_info = driverinfo;
  FS.gen.start_common_info = commoninfo;
  FS.gen.hiver = highVer;
  FS.gen.lowver = lowVer;
}

bool FsCreator::GetOutputPath(std::string& path, bool rw, std::string& finalpath)
{
  if (!path.empty() && path.back() == '/')
  {
    path.pop_back();
  }

  // check if the path is a proper directory
  if (!std::filesystem::is_directory(path))
  {
    return false;
  }

  if (rw)
  {
    // if rw is true, copy path to finalpath and return true
    finalpath = path;
    return true;
  }
  else
  {
    // if rw is false, find a unique directory path
    for (int dirnum = 0; dirnum < 1000; ++dirnum)
    {
      std::ostringstream oss;
      oss << path << "/" << std::setw(3) << std::setfill('0') << dirnum;
      std::string candidate = oss.str();

      if (!std::filesystem::exists(candidate))
      {
        finalpath = candidate;
        return true;
      }
    }

    // if no unique path found, return false
    return false;
  }
}

bool FsCreator::PrepareDirectory(bool rw)
{
  bool ret = false;

  // find free directory
  struct stat info;

  std::string work_dir_path;
  const auto& basepath = FS.file.core_h.dir;

  work_dir_path = basepath;
  ret = true;

  // for this case check only if directory exists
  if (stat(work_dir_path.c_str(), &info) != 0)
  {
    if (!std::filesystem::create_directory(work_dir_path))
    {
      ret = false;
    }
  }

  std::filesystem::create_directory(FS.file.libdir);
  std::filesystem::create_directory(FS.file.usrdir);
  std::filesystem::create_directory(FS.file.incdir);
  std::filesystem::create_directory(FS.file.confdir);
  std::filesystem::create_directory(FS.file.utildir);

  return ret;
}

std::string FsCreator::CreateSubDir(std::string basepath, std::string sub, bool rw)
{
  std::string ret = basepath;
  struct stat info;

  if (basepath.size() == 0 || sub.size() == 0)
  {
    return "";
  }

  if (basepath.back() != '/')
  {
    basepath.append("/");
  }

  basepath.append(sub);

  bool ok = true;

  if (stat(basepath.c_str(), &info) != 0 && rw)
  {
    // directory already exists and rewrite option is requested
    ok = std::filesystem::remove(basepath);
  }

  if (!ok)
  {
    // error on removing directory
    return "";
  }

  if (std::filesystem::create_directory(basepath) != 0)
  {
    ret = "";
  }

  return basepath;
}
