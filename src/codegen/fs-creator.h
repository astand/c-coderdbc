#pragma once

#include <stdint.h>
#include <string>
#include "../types/outfile.h"

typedef struct
{
  // original driver name view
  std::string DrvName_orig;
  // low case driver name
  std::string drvname;
  // up case driver name
  std::string DRVNAME;

  OutFileDescriptor_t core_h;
  OutFileDescriptor_t core_c;

  OutFileDescriptor_t util_h;
  OutFileDescriptor_t util_c;

  OutFileDescriptor_t fmon_h;
  OutFileDescriptor_t fmon_c;

  std::string usebits_def;
  std::string usesruct_def;
  std::string usemon_def;
  std::string usesigfloat_def;
  std::string useroll_def;
  std::string usecsm_def;

  // for dbc version info
  std::string verhigh_def;
  std::string verlow_def;

  // inforamtion to be placed at the start of each source file
  std::string start_info;

} FsDescriptor_t;

// This class is used to build all neccessary string -ed
// value that will be required during code generation
// (paths, file names, drvnames, common defines etc)
// if preparation ends with true, the collection of
// values can be used.
class FsCreator {
 public:
  FsCreator();

  bool PrepareDirectory(std::string drvname, std::string basepath, bool rw, std::string& info);

  FsDescriptor_t FS;

};

