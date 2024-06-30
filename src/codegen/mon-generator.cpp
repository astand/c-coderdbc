#include <cstdlib>
#include <regex>
#include "mon-generator.h"
#include "helpers/formatter.h"

uint32_t MonGenerator::FillHeader(FileWriter& wr, std::vector<CiExpr_t*>& sigs,
  const AppSettings_t& aset)
{
  if (aset.gen.start_info.size() > 0)
  {
    // replace all '\n' on "\n //" for c code comment text
    wr.Append("// " + std::regex_replace(aset.gen.start_info, std::regex("\n"), "\n// "));
  }

  wr.Append("#pragma once");
  wr.Append();

  wr.Append("#ifdef __cplusplus\nextern \"C\" {\n#endif");
  wr.Append();

  wr.Append("// DBC file version");
  wr.Append("#define %s_FMON (%uU)", aset.gen.verhigh_def.c_str(), aset.gen.hiver);
  wr.Append("#define %s_FMON (%uU)", aset.gen.verlow_def.c_str(), aset.gen.lowver);
  wr.Append();

  wr.Append("#include \"%s-config.h\"", aset.gen.drvname.c_str());
  wr.Append();

  // put diagmonitor ifdef selection for including @drv-fmon header
  // with FMon_* signatures to call from unpack function
  wr.Append("#ifdef %s", aset.gen.usemon_def.c_str());
  wr.Append();
  wr.Append("#include \"canmonitorutil.h\"");
  wr.Append("/*\n\
This file contains the prototypes of all the functions that will be called\n\
from each Unpack_*name* function to detect DBC related errors\n\
It is the user responsibility to defined these functions in the\n\
separated .c file. If it won't be done the linkage error will happen\n*/");
  wr.Append();

  wr.Append("#ifdef %s_USE_MONO_FMON", aset.gen.DRVNAME.c_str());
  wr.Append();

  wr.Append("void _FMon_MONO_%s(FrameMonitor_t* _mon, uint32_t msgid);", aset.gen.drvname.c_str());
  wr.Append();

  for (auto it = sigs.begin(); it != sigs.end(); ++it)
  {

    auto msg = &((*it)->msg);
    wr.Append("#define FMon_%s_%s(x, y) _FMon_MONO_%s((x), (y))", msg->Name.c_str(),
      aset.gen.drvname.c_str(),
      aset.gen.drvname.c_str());
  }

  wr.Append();
  wr.Append("#else");
  wr.Append();

  for (auto it = sigs.begin(); it != sigs.end(); ++it)
  {
    auto msg = &((*it)->msg);
    wr.Append("void _FMon_%s_%s(FrameMonitor_t* _mon, uint32_t msgid);",
      msg->Name.c_str(), aset.gen.drvname.c_str());
  }

  wr.Append();

  for (auto it = sigs.begin(); it != sigs.end(); ++it)
  {
    auto msg = &((*it)->msg);
    wr.Append("#define FMon_%s_%s(x, y) _FMon_%s_%s((x), (y))",
      msg->Name.c_str(), aset.gen.drvname.c_str(),
      msg->Name.c_str(), aset.gen.drvname.c_str());
  }

  wr.Append();
  wr.Append("#endif");
  wr.Append();

  wr.Append("#endif // %s", aset.gen.usemon_def.c_str());
  wr.Append();

  wr.Append("#ifdef __cplusplus\n}\n#endif");

  return 0;
}

uint32_t MonGenerator::FillSource(FileWriter& wr, std::vector<CiExpr_t*>& sigs,
  const AppSettings_t& aset)
{
  if (aset.gen.start_info.size() > 0)
  {
    wr.Append("// " + std::regex_replace(aset.gen.start_info, std::regex("\n"), "\n// "));
  }

  wr.Append("#include \"%s\"", aset.file.fmon_h.fname.c_str());
  wr.Append();
  // put diagmonitor ifdef selection for including @drv-fmon header
  // with FMon_* signatures to call from unpack function
  wr.Append("#ifdef %s", aset.gen.usemon_def.c_str());
  wr.Append();

  wr.Append("/*\n\
Put the monitor function content here, keep in mind -\n\
next generation will completely clear all manually added code (!)\n\
*/\n\n");

  wr.Append("#ifdef %s_USE_MONO_FMON", aset.gen.DRVNAME.c_str());
  wr.Append();
  wr.Append("void _FMon_MONO_%s(FrameMonitor_t* _mon, uint32_t msgid)", aset.gen.drvname.c_str());
  wr.Append("{");
  wr.Append("  (void)_mon;");
  wr.Append("  (void)msgid;");
  wr.Append("}");
  wr.Append();
  wr.Append("#else");
  wr.Append();

  for (auto it = sigs.begin(); it != sigs.end(); ++it)
  {
    auto msg = &((*it)->msg);
    wr.Append("void _FMon_%s_%s(FrameMonitor_t* _mon, uint32_t msgid)\n{\n  (void)_mon;\n  (void)msgid;\n}\n\n",
      msg->Name.c_str(), aset.gen.drvname.c_str());
  }

  wr.Append("#endif // %s_USE_MONO_FMON", aset.gen.DRVNAME.c_str());
  wr.Append();

  wr.Append("#endif // %s", aset.gen.usemon_def.c_str());

  return 0;
}
