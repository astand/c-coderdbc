#include "mon-generator.h"
#include "helpers/formatter.h"

uint32_t MonGenerator::FillHeader(FileWriter& wr, std::vector<CiExpr_t*>& sigs,
  const GenDescriptor_t& gsett)
{
  wr.Append("#ifdef %s_USE_MONO_FMON", gsett.DRVNAME.c_str());
  wr.Append();

  wr.Append("void _FMon_MONO_%s(FrameMonitor_t* _mon, uint32_t msgid);", gsett.drvname.c_str());
  wr.Append();

  for (auto it = sigs.begin(); it != sigs.end(); ++it)
  {

    auto msg = &((*it)->msg);
    wr.Append("#define FMon_%s_%s(x, y) _FMon_MONO_%s((x), (y))", msg->Name.c_str(),
      gsett.drvname.c_str(),
      gsett.drvname.c_str());
  }

  wr.Append();
  wr.Append("#else");
  wr.Append();

  for (auto it = sigs.begin(); it != sigs.end(); ++it)
  {
    auto msg = &((*it)->msg);
    wr.Append("void _FMon_%s_%s(FrameMonitor_t* _mon, uint32_t msgid);",
      msg->Name.c_str(), gsett.drvname.c_str());
  }

  wr.Append();

  for (auto it = sigs.begin(); it != sigs.end(); ++it)
  {
    auto msg = &((*it)->msg);
    wr.Append("#define FMon_%s_%s(x, y) _FMon_%s_%s((x), (y))",
      msg->Name.c_str(), gsett.drvname.c_str(),
      msg->Name.c_str(), gsett.drvname.c_str());
  }

  wr.Append();
  wr.Append("#endif");
  wr.Append();

  return 0;
}

uint32_t MonGenerator::FillSource(FileWriter& wr, std::vector<CiExpr_t*>& sigs,
  const GenDescriptor_t& gsett)
{
  wr.Append("#ifdef %s_USE_MONO_FMON", gsett.DRVNAME.c_str());
  wr.Append();
  wr.Append("void _FMon_MONO_%s(FrameMonitor_t* _mon, uint32_t msgid)", gsett.drvname.c_str());
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
      msg->Name.c_str(), gsett.drvname.c_str());
  }

  wr.Append("#endif // %s_USE_MONO_FMON", gsett.DRVNAME.c_str());
  wr.Append();

  return 0;
}
