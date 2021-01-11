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

CiMainGenerator::CiMainGenerator()
{
  sigprt = new CSigPrinter;
  fwriter = new FileWriter;
}

void CiMainGenerator::Generate(std::vector<MessageDescriptor_t*>& msgs, const FsDescriptor_t& fsd)
{
  // Load income messages to sig printer
  sigprt->LoadMessages(msgs);

  // save pointer to output file descriptor struct to
  // enable using this information inside class member functions
  fdesc = &fsd;

  std::sort(sigprt->sigs_expr.begin(), sigprt->sigs_expr.end(),
            [](const CiExpr_t* a, const CiExpr_t* b) -> bool
  {
    return a->msg.MsgID < b->msg.MsgID;
  });

  // 2 step is to print main head file
  fwriter->AppendLine("#pragma once", 2);
  fwriter->AppendLine("#ifdef __cplusplus\nextern \"C\" {\n#endif", 2);
  fwriter->AppendLine("#include <stdint.h>", 2);

  fwriter->AppendLine(PrintF("#ifdef %s", fsd.usemon_def.c_str()));

  fwriter->AppendText(
    "// This file must define:\n"
    "// base monitor struct\n"
    "// function signature for CRC calculation\n"
    "// function signature for getting system tick value (100 us step)\n"
    "#include \"canmonitorutil.h\"\n"
    "\n"
  );

  fwriter->AppendLine(PrintF("#endif // %s", fsd.usemon_def.c_str()), 3);

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
    fwriter->AppendLine(PrintF("#ifdef %s", fsd.usebits_def.c_str()), 2);

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

    fwriter->AppendLine(PrintF("#endif // %s", fsd.usebits_def.c_str()), 2);

    // start mon1 section
    fwriter->AppendLine(PrintF("#ifdef %s", fsd.usebits_def.c_str()), 2);
    fwriter->AppendLine("  FrameMonitor_t mon1;", 2);
    fwriter->AppendLine(PrintF("#endif // %s", fsd.usebits_def.c_str()), 2);
    fwriter->AppendLine(PrintF("} %s_t;", m.Name.c_str()), 2);
  }

  fwriter->AppendLine("// Function signatures", 2);

  for (size_t num = 0; num < sigprt->sigs_expr.size(); num++)
  {
    // write message typedef s and additional expressions
    MessageDescriptor_t& m = sigprt->sigs_expr[num]->msg;

    fwriter->AppendLine(
      PrintF("uint32_t Unpack_%s_%s(%s_t* _m, const uint8_t* _d, uint8_t dlc_);",
             m.Name.c_str(), fsd.DrvName_orig.c_str(), m.Name.c_str()));

    fwriter->AppendLine(PrintF("#ifdef %s", fsd.usesruct_def.c_str()));

    fwriter->AppendLine(
      PrintF("uint32_t Pack_%s_%s(const %s_t* _m, __CoderDbcCanFrame_t__* cframe);",
             m.Name.c_str(), fsd.DrvName_orig.c_str(), m.Name.c_str()));

    fwriter->AppendLine("#else");

    fwriter->AppendLine(
      PrintF("uint32_t Pack_%s_%s(const %s_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide);",
             m.Name.c_str(), fsd.DrvName_orig.c_str(), m.Name.c_str()));

    fwriter->AppendLine(PrintF("#endif // %s", fsd.usesruct_def.c_str()), 2);
  }

  fwriter->AppendLine("#ifdef __cplusplus\n}\n#endif");

  // save fwrite cached text to file
  fwriter->Flush(fsd.core_h.fpath);

  // 3 step is to print main source file
  // include main header file
  fwriter->AppendLine(PrintF("#include ""%s""", fsd.core_h.fname.c_str()), 3);

  // put diagmonitor ifdef selection for including @drv-fmon header
  // with FMon_* signatures to call from unpack function
  fwriter->AppendLine(PrintF("#ifdef %s", fsd.usemon_def.c_str()));

  fwriter->AppendText(
    "// This file must define:\n"
    "// base monitor struct\n"
    "// function signature for CRC calculation\n"
    "// function signature for getting system tick value (100 us step)\n");

  fwriter->AppendLine(PrintF("#include ""%s-fmon.h""", fsd.drvname.c_str()), 2);

  fwriter->AppendLine(PrintF("#endif // %s", fsd.usemon_def.c_str()), 3);

  // for each message 3 functions must be defined - 1 unpack function,
  // 2: pack with raw signature
  // 3: pack with canstruct
  for (size_t num = 0; num < sigprt->sigs_expr.size(); num++)
  {
    // write message typedef s and additional expressions
    MessageDescriptor_t& m = sigprt->sigs_expr[num]->msg;

    // first function
    fwriter->AppendLine(
      PrintF("uint32_t Unpack_%s_%s(%s_t* _m, const uint8_t* _d, uint8_t dlc_)\n{",
             m.Name.c_str(), fsd.DrvName_orig.c_str(), m.Name.c_str()));

    WriteUnpackBody(sigprt->sigs_expr[num]);

    fwriter->AppendLine("}", 2);


    fwriter->AppendLine(PrintF("#ifdef %s", fsd.usesruct_def.c_str()));

    // second function
    fwriter->AppendLine(
      PrintF("uint32_t Pack_%s_%s(const %s_t* _m, __CoderDbcCanFrame_t__* cframe);",
             m.Name.c_str(), fsd.DrvName_orig.c_str(), m.Name.c_str()));

    fwriter->AppendLine("#else");

    // third function
    fwriter->AppendLine(
      PrintF("uint32_t Pack_%s_%s(const %s_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide);",
             m.Name.c_str(), fsd.DrvName_orig.c_str(), m.Name.c_str()));

    fwriter->AppendLine(PrintF("#endif // %s", fsd.usesruct_def.c_str()), 2);
  }

  fwriter->Flush(fsd.core_c.fpath);
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
    dtype += PrintF(" : %d", sig.LengthBit);
  }

  dtype += ";";

  std::string pad = " ";

  dtype += pad.insert(0, padwidth + 16 - dtype.size(), ' ');

  fwriter->AppendText(dtype);

  pad = " // ";
  pad += (sig.Signed) ? " [-]" : "    ";

  fwriter->AppendText(pad);

  fwriter->AppendText(PrintF(" Bits=%2d", sig.LengthBit));

  if (sig.Unit.size() > 0)
  {
    fwriter->AppendText(PrintF(" Unit:'%-13s'", sig.Unit.c_str()));
  }

  if (sig.Offset != 0)
  {
    fwriter->AppendText(PrintF(" Offset= %-18f", sig.Offset));
  }

  if (sig.Factor != 1)
  {
    fwriter->AppendText(PrintF(" Factor= %-15d", sig.LengthBit));
  }

  fwriter->AppendLine("", 2);
}

void CiMainGenerator::WriteUnpackBody(const CiExpr_t* sgs)
{
  for (size_t num = 0; num < sgs->to_signals.size(); num++)
  {
    auto expr = sgs->to_signals[num];

    fwriter->AppendLine(PrintF("  _m->%s = %s;", sgs->msg.Signals[num].Name.c_str(), expr.c_str()));
  }

  fwriter->AppendLine("");

  fwriter->AppendLine(PrintF("#ifdef %s", fdesc->usemon_def.c_str()));

  fwriter->AppendLine("  // check DLC correctness");
  fwriter->AppendLine(PrintF("  _m->mon1.dlc_error = (dlc_ < %s_DLC);", sgs->msg.Name.c_str()));


  // TODO: put CRC and ROLLING COUNTER tests here
  // 1
  // 2


  fwriter->AppendLine("  _m->mon1.last_cycle = GetSysTick();");
  fwriter->AppendLine("  _m->mon1.frame_cnt++;", 2);

  auto Fmon_func = "FMon_" + sgs->msg.Name + "_" + fdesc->drvname;

  fwriter->AppendLine(PrintF("  %s(&_m->mon1);", Fmon_func.c_str()));

  fwriter->AppendLine(PrintF("#endif // %s", fdesc->usemon_def.c_str()), 2);

  fwriter->AppendLine(PrintF(" return %s_CANID;", sgs->msg.Name.c_str()));
}
