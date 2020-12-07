#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <algorithm>
#include <regex>

#include "c-main-generator.h"

static const size_t kMaxDirNum = 1000;

static const size_t kWBUFF_len = 2048;
static char wbuff[kWBUFF_len] = {0};

static std::string __typeprint[] =
{
  "int8_t",
  "int16_t",
  "int32_t",
  "int64_t",
  "uint8_t",
  "uint16_t",
  "uint32_t",
  "uint64_t"
};

std::string str_toupper(std::string s)
{
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c)
  {
    return std::toupper(c);
  });
  return s;
}

CiMainGenerator::CiMainGenerator()
{
  sigprt = new CSigPrinter;
  fwriter = new FileWriter;
}

void CiMainGenerator::Generate(std::vector<MessageDescriptor_t*>& msgs,
                               std::string drvname,
                               std::string dirpath)
{
  // Load income messages to sig printer
  sigprt->LoadMessages(msgs);

  std::sort(sigprt->sigs_expr.begin(), sigprt->sigs_expr.end(),
            [](const CiExpr_t* a, const CiExpr_t* b) -> bool
  {
    return a->msg.MsgID < b->msg.MsgID;
  });

  auto dirok = SetFinalPath(dirpath);

  if (!dirok)
  {
    // TODO: handle error if directory cannot be used
  }

  SetCommonValues(drvname);

  // work_dir_path has the base dir path to gen files
  mhead.dir = work_dir_path;
  mhead.fname = drvname + ".h";
  mhead.fpath = mhead.dir + "/" + mhead.fname;

  // 1 step is to define final directory for source code bunch
  fwriter->AppendLine("#pragma once", 3);
  fwriter->AppendLine("#ifdef __cplusplus\nextern \"C\" {\n#endif", 2);
  fwriter->AppendLine("#include <stdint.h>");

  for (size_t num = 0; num < sigprt->sigs_expr.size(); num++)
  {
    // write message typedef s and additional expressions
    MessageDescriptor_t& m = sigprt->sigs_expr[num]->msg;

    snprintf(wbuff, kWBUFF_len, "// def @%s CAN Message (%-4d %#x)", m.Name.c_str(), m.MsgID, m.MsgID);
    fwriter->AppendLine(wbuff);
    snprintf(wbuff, kWBUFF_len, "#define %s_IDE (%uU)", m.Name.c_str(), m.IsExt);
    fwriter->AppendLine(wbuff);
    snprintf(wbuff, kWBUFF_len, "#define %s_DLC (%uU)", m.Name.c_str(), m.DLC);
    fwriter->AppendLine(wbuff);
    snprintf(wbuff, kWBUFF_len, "#define %s_CANID (%#x)", m.Name.c_str(), m.MsgID);
    fwriter->AppendLine(wbuff);

    if (m.Cycle > 0)
    {
      snprintf(wbuff, kWBUFF_len, "#define %s_CYC (%dU)", m.Name.c_str(), m.Cycle);
      fwriter->AppendLine(wbuff);
    }

    if (m.CommentText.size() > 0)
    {
      fwriter->AppendLine("// -- " + m.CommentText);
    }

    size_t max_sig_name_len = 27;

    for (size_t signum = 0; signum < m.Signals.size(); signum++)
    {
      SignalDescriptor_t& s = m.Signals[signum];

      // TODO: print signal to_S and from_S definitions if necessary
      //string ret = cprint.PrintSignalPackExpression(sig, msg.MessageName);

      //if (ret != null)
      //{
      //  fwriter->AppendLine(ret);
      //}
      if (s.Name.size() > max_sig_name_len)
        max_sig_name_len = s.Name.size();
    }

    snprintf(wbuff, kWBUFF_len, "typedef struct");
    fwriter->AppendLine(wbuff);

    fwriter->AppendLine("{\n");

    // Write section for bitfielded part
    snprintf(wbuff, kWBUFF_len, "#ifdef %s", usebits_str.c_str());
    fwriter->AppendLine(wbuff, 2);

    for (size_t signum = 0; signum < m.Signals.size(); signum++)
    {
      SignalDescriptor_t& sig = m.Signals[signum];
      // Write bit-fielded part
      WriteSigStructField(sig, true, max_sig_name_len);
    }

    // Write clean part
    fwriter->AppendLine("#else", 2);

    for (size_t signum = 0; signum < m.Signals.size(); signum++)
    {
      SignalDescriptor_t& sig = m.Signals[signum];
      // Write clean signals
      WriteSigStructField(sig, false, max_sig_name_len);
    }

    snprintf(wbuff, kWBUFF_len, "#endif // %s", usebits_str.c_str());
    fwriter->AppendLine(wbuff, 2);

    //if (CodeSett.Code.UseMonitors == 1)
    //  fwriter->AppendLine("  FrameMonitor_t mon1;");

    snprintf(wbuff, kWBUFF_len, "} %s_t;", m.Name.c_str());
    fwriter->AppendLine(wbuff, 2);
    //fwriter->AppendLine("} " + msg.MessageName + "_t;");
    //fwriter->AppendLine();
  }

  fwriter->AppendLine("#ifdef __cplusplus\n}\n#endif");
  fwriter->Flush(mhead.fpath);
  // 2 step is to print main head file

  // 3 step is to print main source file

  // 4 step is to pring fmon head file

  // 5 step is to print fmon source file
}

void CiMainGenerator::WriteSigStructField(const SignalDescriptor_t& sig, bool bits, size_t padwidth)
{
  if (sig.CommentText.size() > 0)
  {
    fwriter->AppendLine("  // " + std::regex_replace(sig.CommentText, std::regex("\n"), "\n  // "));
  }

  if (sig.ValueText.size() > 0)
  {
    fwriter->AppendLine("  // " + std::regex_replace(sig.ValueText, std::regex("\n"), "\n  // "));
  }

  std::string dtype = "";

  dtype += "  " + __typeprint[(int)sig.Type] + " " + sig.Name;

  if (bits && (sig.LengthBit < 8))
  {
    snprintf(wbuff, kWBUFF_len, " : %d", sig.LengthBit);
    dtype += wbuff;
  }

  dtype += ";";

  std::string pad = " ";

  dtype += pad.insert(0, padwidth + 16 - dtype.size(), ' ');

  fwriter->AppendText(dtype);

  pad = " // ";
  pad += (sig.Signed) ? " [-]" : "    ";

  fwriter->AppendText(pad);

  snprintf(wbuff, kWBUFF_len, " Bits=%2d", sig.LengthBit);
  fwriter->AppendText(wbuff);

  if (sig.Unit.size() > 0)
  {
    snprintf(wbuff, kWBUFF_len, " Unit:'%-13s'", sig.Unit.c_str());
    fwriter->AppendText(wbuff);
  }

  if (sig.Offset != 0)
  {
    snprintf(wbuff, kWBUFF_len, " Offset= %-18f", sig.Offset);
    fwriter->AppendText(wbuff);
  }

  if (sig.Factor != 1)
  {
    snprintf(wbuff, kWBUFF_len, " Factor= %-15d", sig.LengthBit);
    fwriter->AppendText(wbuff);
  }

  fwriter->AppendLine("", 2);
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

void CiMainGenerator::SetCommonValues(const std::string& drvname)
{
  DRVNAME = str_toupper(drvname);

  snprintf(wbuff, kWBUFF_len, "%s_USE_BITS_SIGNAL", DRVNAME.c_str());
  usebits_str = wbuff;

  snprintf(wbuff, kWBUFF_len, "%s_USE_DIAG_MONITORS", DRVNAME.c_str());
  usediag_str = wbuff;

  snprintf(wbuff, kWBUFF_len, "%s_USE_CANSTRUCT", DRVNAME.c_str());
  canframe_str = wbuff;
}
