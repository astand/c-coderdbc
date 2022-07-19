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

  FS.file.libdir = work_dir_path + kLibDir;

  if (std::filesystem::create_directory(FS.file.libdir))
  {
    // ret = false;
  }

  FS.file.usrdir = work_dir_path + kUsrDir;

  if (std::filesystem::create_directory(FS.file.usrdir))
  {
    // ret = false;
  }

  FS.file.incdir = work_dir_path + kIncDir;

  if (std::filesystem::create_directory(FS.file.incdir))
  {
    // ret = false;
  }

  FS.file.confdir = work_dir_path + kConfDir;

  if (std::filesystem::create_directory(FS.file.confdir))
  {
    // ret = false;
  }

  FS.file.utildir = work_dir_path + kUtilDir;

  if (std::filesystem::create_directory(FS.file.utildir))
  {
    // ret = false;
  }

  if (true)
  {
    // directory valid and exists, set all the values
    FS.gen.DrvName_orig = drvname;
    FS.gen.DRVNAME = str_toupper(drvname);
    FS.gen.drvname = str_tolower(drvname);

    FS.file.core_h.dir = work_dir_path;
    FS.file.core_h.fname = FS.gen.drvname + ".h";
    FS.file.core_h.fpath = FS.file.libdir + "/" + FS.file.core_h.fname;

    FS.file.core_c.dir = work_dir_path;
    FS.file.core_c.fname = FS.gen.drvname + ".c";
    FS.file.core_c.fpath = FS.file.libdir + "/" + FS.file.core_c.fname;

    FS.file.util_h.dir = work_dir_path;
    FS.file.util_h.fname = FS.gen.drvname + "-binutil" + ".h";
    FS.file.util_h.fpath = FS.file.utildir + "/" + FS.file.util_h.fname;

    FS.file.util_c.dir = work_dir_path;
    FS.file.util_c.fname = FS.gen.drvname + "-binutil" + ".c";
    FS.file.util_c.fpath = FS.file.utildir + "/" + FS.file.util_c.fname;

    FS.file.fmon_h.dir = work_dir_path;
    FS.file.fmon_h.fname = FS.gen.drvname + "-fmon.h";
    FS.file.fmon_h.fpath = FS.file.libdir + "/" + FS.file.fmon_h.fname;

    FS.file.fmon_c.dir = work_dir_path;
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
    FS.gen.start_info.clear();

    if (strinfo.size() > 0)
    {
      FS.gen.start_info = strinfo;
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
