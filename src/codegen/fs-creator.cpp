#include <sys/stat.h>
#include <filesystem>
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

bool FsCreator::PrepareDirectory(std::string drvname, std::string basepath, bool rw, std::string& strinfo)
{
  bool ret = false;

  // find free directory
  struct stat info;

  std::string work_dir_path;

  if (rw)
  {
    work_dir_path = basepath;
    ret = true;

    // for this case check only if directory exists
    if (stat(work_dir_path.c_str(), &info) != 0)
    {
      if (std::filesystem::create_directory(work_dir_path) != 0)
      {
        ret = false;
      }
    }
  }
  else
  {
    std::string separator = basepath.at(basepath.size() - 1) == '/' ? "" : "/";

    for (int32_t dirnum = 0; dirnum < 1000; dirnum++)
    {
      snprintf(_tmpb, kTmpLen, "%03d", dirnum);
      work_dir_path = basepath + separator + _tmpb;

      if (stat(work_dir_path.c_str(), &info) != 0)
      {
        if (std::filesystem::create_directory(work_dir_path))
        {
          ret = true;
          break;
        }
      }
      else if (info.st_mode & S_IFDIR)
      {
        // directory exists, try next num
        continue;
      }
      else
      {
        if (std::filesystem::create_directory(work_dir_path) != 0)
        {
          ret = false;
          break;
        }
      }
    }
  }

  FS.libdir = work_dir_path + kLibDir;

  if (std::filesystem::create_directory(FS.libdir))
  {
    // ret = false;
  }

  FS.usrdir = work_dir_path + kUsrDir;

  if (std::filesystem::create_directory(FS.usrdir))
  {
    // ret = false;
  }

  FS.incdir = work_dir_path + kIncDir;

  if (std::filesystem::create_directory(FS.incdir))
  {
    // ret = false;
  }

  FS.confdir = work_dir_path + kConfDir;

  if (std::filesystem::create_directory(FS.confdir))
  {
    // ret = false;
  }

  FS.utildir = work_dir_path + kUtilDir;

  if (std::filesystem::create_directory(FS.utildir))
  {
    // ret = false;
  }

  if (true)
  {
    // directory valid and exists, set all the values
    FS.DrvName_orig = drvname;
    FS.DRVNAME = str_toupper(drvname);
    FS.drvname = str_tolower(drvname);

    FS.core_h.dir = work_dir_path;
    FS.core_h.fname = FS.drvname + ".h";
    FS.core_h.fpath = FS.libdir + "/" + FS.core_h.fname;

    FS.core_c.dir = work_dir_path;
    FS.core_c.fname = FS.drvname + ".c";
    FS.core_c.fpath = FS.libdir + "/" + FS.core_c.fname;

    FS.util_h.dir = work_dir_path;
    FS.util_h.fname = FS.drvname + "-binutil" + ".h";
    FS.util_h.fpath = FS.utildir + "/" + FS.util_h.fname;

    FS.util_c.dir = work_dir_path;
    FS.util_c.fname = FS.drvname + "-binutil" + ".c";
    FS.util_c.fpath = FS.utildir + "/" + FS.util_c.fname;

    FS.fmon_h.dir = work_dir_path;
    FS.fmon_h.fname = FS.drvname + "-fmon.h";
    FS.fmon_h.fpath = FS.libdir + "/" + FS.fmon_h.fname;

    FS.fmon_c.dir = work_dir_path;
    FS.fmon_c.fname = FS.drvname + "-fmon.c";
    FS.fmon_c.fpath = FS.usrdir + "/" + FS.fmon_c.fname;

    snprintf(_tmpb, kTmpLen, "%s_USE_BITS_SIGNAL", FS.DRVNAME.c_str());
    FS.usebits_def = _tmpb;

    snprintf(_tmpb, kTmpLen, "%s_USE_DIAG_MONITORS", FS.DRVNAME.c_str());
    FS.usemon_def = _tmpb;

    snprintf(_tmpb, kTmpLen, "%s_USE_SIGFLOAT", FS.DRVNAME.c_str());
    FS.usesigfloat_def = _tmpb;

    snprintf(_tmpb, kTmpLen, "%s_USE_CANSTRUCT", FS.DRVNAME.c_str());
    FS.usesruct_def = _tmpb;

    snprintf(_tmpb, kTmpLen, "%s_AUTO_ROLL", FS.DRVNAME.c_str());
    FS.useroll_def = _tmpb;

    snprintf(_tmpb, kTmpLen, "%s_AUTO_CSM", FS.DRVNAME.c_str());
    FS.usecsm_def = _tmpb;

    snprintf(_tmpb, kTmpLen, "VER_%s_MAJ", FS.DRVNAME.c_str());
    FS.verhigh_def = _tmpb;

    snprintf(_tmpb, kTmpLen, "VER_%s_MIN", FS.DRVNAME.c_str());
    FS.verlow_def = _tmpb;

    // load start info to fdescriptor
    FS.start_info.clear();

    if (strinfo.size() > 0)
    {
      FS.start_info = strinfo;
    }
  }

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

  if (basepath.at(basepath.size() - 1) != '/')
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
