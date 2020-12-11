#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdarg.h>
#include <filesystem>
#include <algorithm>
#include <regex>

#include "c-main-generator.h"

static const size_t kMaxDirNum = 1000;

static const size_t kWBUFF_len = 2048;

static char wbuff[kWBUFF_len] = { 0 };

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

char* PrintF(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  vsnprintf(wbuff, kWBUFF_len, format, args);
  va_end(args);
  return wbuff;
}

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
  // 1 step is to define final directory for source code bunch
  mhead.dir = work_dir_path;
  mhead.fname = drvname + ".h";
  mhead.fpath = mhead.dir + "/" + mhead.fname;

  // 2 step is to print main head file
  fwriter->AppendLine("#pragma once", 2);
  fwriter->AppendLine("#ifdef __cplusplus\nextern \"C\" {\n#endif", 2);
  fwriter->AppendLine("#include <stdint.h>", 2);

  snprintf(wbuff, kWBUFF_len, "#ifdef %s", usediag_str.c_str());
  fwriter->AppendLine(wbuff);

  fwriter->AppendText(
    "// This file must define:\n"
    "// base monitor struct\n"
    "// function signature for CRC calculation\n"
    "// function signature for getting system tick value (100 us step)\n"
    "#include \"canmonitorutil.h\"\n"
    "\n"

  );

  fwriter->AppendLine(PrintF("#endif // %s", usediag_str.c_str()), 3);

  for (size_t num = 0; num < sigprt->sigs_expr.size(); num++)
  {
    // write message typedef s and additional expressions
    MessageDescriptor_t& m = sigprt->sigs_expr[num]->msg;

    fwriter->AppendLine(PrintF("// def @%s CAN Message (%-4d %#x)", m.Name.c_str(), m.MsgID, m.MsgID));
    fwriter->AppendLine(PrintF("#define %s_IDE (%uU)", m.Name.c_str(), m.IsExt));
    fwriter->AppendLine(PrintF("#define %s_DLC (%uU)", m.Name.c_str(), m.DLC));
    fwriter->AppendLine(PrintF("#define %s_CANID (%#x)", m.Name.c_str(), m.MsgID));

    if (m.Cycle > 0)
    {
      fwriter->AppendLine(PrintF("#define %s_CYC (%dU)", m.Name.c_str(), m.Cycle));
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

    // empty line before struct definition
    fwriter->AppendLine("\n");

    fwriter->AppendLine(PrintF("typedef struct"));

    fwriter->AppendLine("{\n");

    // Write section for bitfielded part
    fwriter->AppendLine(PrintF("#ifdef %s", usebits_str.c_str()), 2);

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

    fwriter->AppendLine(PrintF("#endif // %s", usebits_str.c_str()), 2);

    // start mon1 section
    fwriter->AppendLine(PrintF("#ifdef %s", usebits_str.c_str()), 2);
    fwriter->AppendLine("  FrameMonitor_t mon1;", 2);
    fwriter->AppendLine(PrintF("#endif // %s", usebits_str.c_str()), 2);
    fwriter->AppendLine(PrintF("} %s_t;", m.Name.c_str()), 2);
  }

  fwriter->AppendLine("// Function signatures", 2);

  for (size_t num = 0; num < sigprt->sigs_expr.size(); num++)
  {
    // write message typedef s and additional expressions
    MessageDescriptor_t& m = sigprt->sigs_expr[num]->msg;
    
    fwriter->AppendLine(
      PrintF("uint32_t Unpack_%s_%s(%s_t* _m, const uint8_t* _d, uint8_t dlc_);",
             m.Name.c_str(), drvname.c_str(), m.Name.c_str()));

    fwriter->AppendLine(PrintF("#ifdef %s", usestruct_str.c_str()));
    
    fwriter->AppendLine(
      PrintF("uint32_t Pack_%s_%s(const %s_t* _m, __CoderDbcCanFrame_t__* cframe);",
             m.Name.c_str(), drvname.c_str(), m.Name.c_str()));
    
    fwriter->AppendLine("#else");
    
    fwriter->AppendLine(
      PrintF("uint32_t Pack_%s_%s(const %s_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide);",
             m.Name.c_str(), drvname.c_str(), m.Name.c_str()));

    fwriter->AppendLine(PrintF("#endif // %s", usestruct_str.c_str()), 2);
  }

  fwriter->AppendLine("#ifdef __cplusplus\n}\n#endif");
  fwriter->Flush(mhead.fpath);

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
  usestruct_str = wbuff;
}
