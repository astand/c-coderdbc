#pragma once

#include <stdint.h>
#include <string>
#include "../types/outfile.h"

typedef struct
{
  std::string libdir;
  std::string usrdir;
  std::string incdir;
  std::string confdir;
  std::string utildir;

  OutFileDescriptor_t core_h;
  OutFileDescriptor_t core_c;

  OutFileDescriptor_t util_h;
  OutFileDescriptor_t util_c;

  OutFileDescriptor_t fmon_h;
  OutFileDescriptor_t fmon_c;
} FsDescriptor_t;

typedef struct
{
  // original driver name view
  std::string DrvName_orig;
  // low case driver name
  std::string drvname;
  // up case driver name
  std::string DRVNAME;

  std::string usebits_def;
  std::string usesruct_def;
  std::string usemon_def;
  std::string usemonofmon_def;
  std::string usesigfloat_def;
  std::string useroll_def;
  std::string usecsm_def;

  // for dbc version info
  std::string verhigh_def;
  std::string verlow_def;

  // text comment to be placed at the beggining of the driver's related source files
  std::string start_driver_info;
  // text comment to be placed at the beggining of each source files
  std::string start_common_info;

  uint32_t hiver{0};
  uint32_t lowver{0};

  bool no_fmon{false};
  bool no_inc{false};
  bool no_config{false};
} GenDescriptor_t;

typedef struct
{
  FsDescriptor_t file;
  GenDescriptor_t gen;
} AppSettings_t;

// This class is used to build all neccessary string -ed
// value that will be required during code generation
// (paths, file names, drvnames, common defines etc)
// if preparation ends with true, the collection of
// values can be used.
class FsCreator {
 public:
  FsCreator();

  void Configure(const std::string& drvname,
    const std::string& outpath,
    const std::string& commoninfo,
    const std::string& driverinfo,
    uint32_t highVer,
    uint32_t lowVer);

  bool GetOutputPath(std::string& path, bool rw, std::string& finalpath);

  bool PrepareDirectory(bool rw);

  std::string CreateSubDir(std::string basepath, std::string subdir, bool rm = true);

  AppSettings_t FS;

};

