#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdarg.h>
#include <filesystem>
#include <algorithm>
#include <regex>
#include "helpers/formatter.h"

#include "c-main-generator.h"

static const size_t kMaxDirNum = 1000;

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
  Gen_MainHeader();

  // 3 step is to print main source file
  Gen_MainSource();

  // 4 step is to pring fmon head file
  Gen_FMonHeader();

  // 5 step is to print fmon source file
  Gen_FMonSource();
}

void CiMainGenerator::Gen_MainHeader()
{
  fwriter->AppendLine("#pragma once", 2);
  fwriter->AppendLine("#ifdef __cplusplus\nextern \"C\" {\n#endif", 2);
  fwriter->AppendLine("#include <stdint.h>", 2);

  fwriter->AppendLine("// include current dbc-driver compilation config");
  fwriter->AppendLine(StrPrint("#include \"%s-config.h\"", fdesc->drvname.c_str()), 2);

  fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usemon_def.c_str()));

  fwriter->AppendText(
    "// This file must define:\n"
    "// base monitor struct\n"
    "// function signature for CRC calculation\n"
    "// function signature for getting system tick value (100 us step)\n"
    "#include \"canmonitorutil.h\"\n"
    "\n"
  );

  fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usemon_def.c_str()), 3);

  for (size_t num = 0; num < sigprt->sigs_expr.size(); num++)
  {
    // write message typedef s and additional expressions
    MessageDescriptor_t& m = sigprt->sigs_expr[num]->msg;

    fwriter->AppendLine(StrPrint("// def @%s CAN Message (%-4d %#x)", m.Name.c_str(), m.MsgID, m.MsgID));
    fwriter->AppendLine(StrPrint("#define %s_IDE (%uU)", m.Name.c_str(), m.IsExt));
    fwriter->AppendLine(StrPrint("#define %s_DLC (%uU)", m.Name.c_str(), m.DLC));
    fwriter->AppendLine(StrPrint("#define %s_CANID (%#x)", m.Name.c_str(), m.MsgID));

    if (m.Cycle > 0)
    {
      fwriter->AppendLine(StrPrint("#define %s_CYC (%dU)", m.Name.c_str(), m.Cycle));
    }

    if (m.CommentText.size() > 0)
    {
      fwriter->AppendLine("// -- " + m.CommentText);
    }

    size_t max_sig_name_len = 27;

    for (size_t signum = 0; signum < m.Signals.size(); signum++)
    {
      SignalDescriptor_t& s = m.Signals[signum];

      if (!s.IsSimpleSig)
      {
        fwriter->AppendText(sigprt->PrintPhysicalToRaw(&s, fdesc->DRVNAME));
      }

      if (s.Name.size() > max_sig_name_len)
        max_sig_name_len = s.Name.size();
    }

    fwriter->AppendText("\n");

    fwriter->AppendLine(StrPrint("typedef struct"));

    fwriter->AppendLine("{");

    // Write section for bitfielded part
    fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usebits_def.c_str()), 2);

    SignalDescriptor_t rollsig;

    if (m.RollSig != nullptr)
    {
      // rolling counter is detected
      rollsig = (*m.RollSig);
      rollsig.CommentText = "";
      rollsig.Name += "_expt";
    }

    for (size_t signum = 0; signum < m.Signals.size(); signum++)
    {
      SignalDescriptor_t& sig = m.Signals[signum];
      // Write bit-fielded part
      WriteSigStructField(sig, true, max_sig_name_len);
    }

    if (m.RollSig != nullptr)
    {
      fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->useroll_def.c_str()), 2);
      WriteSigStructField(rollsig, true, max_sig_name_len);
      fwriter->AppendLine(StrPrint("#endif // %s", fdesc->useroll_def.c_str()), 2);
    }

    // Write clean part
    fwriter->AppendLine("#else", 2);

    for (size_t signum = 0; signum < m.Signals.size(); signum++)
    {
      SignalDescriptor_t& sig = m.Signals[signum];
      // Write clean signals
      WriteSigStructField(sig, false, max_sig_name_len);
    }

    if (m.RollSig != nullptr)
    {
      fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->useroll_def.c_str()), 2);
      WriteSigStructField(rollsig, false, max_sig_name_len);
      fwriter->AppendLine(StrPrint("#endif // %s", fdesc->useroll_def.c_str()), 2);
    }

    fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usebits_def.c_str()), 2);

    // start mon1 section
    fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usemon_def.c_str()), 2);
    fwriter->AppendLine("  FrameMonitor_t mon1;", 2);
    fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usemon_def.c_str()), 2);
    fwriter->AppendLine(StrPrint("} %s_t;", m.Name.c_str()), 2);
  }

  fwriter->AppendLine("// Function signatures", 2);

  for (size_t num = 0; num < sigprt->sigs_expr.size(); num++)
  {
    // write message typedef s and additional expressions
    MessageDescriptor_t& m = sigprt->sigs_expr[num]->msg;

    fwriter->AppendLine(StrPrint("uint32_t Unpack_%s_%s(%s_t* _m, const uint8_t* _d, uint8_t dlc_);",
                                 m.Name.c_str(), fdesc->DrvName_orig.c_str(), m.Name.c_str()));

    fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usesruct_def.c_str()));

    fwriter->AppendLine(StrPrint("uint32_t Pack_%s_%s(%s_t* _m, __CoderDbcCanFrame_t__* cframe);",
                                 m.Name.c_str(), fdesc->DrvName_orig.c_str(), m.Name.c_str()));

    fwriter->AppendLine("#else");

    fwriter->AppendLine(StrPrint("uint32_t Pack_%s_%s(%s_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide);",
                                 m.Name.c_str(), fdesc->DrvName_orig.c_str(), m.Name.c_str()));

    fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usesruct_def.c_str()), 2);
  }

  fwriter->AppendLine("#ifdef __cplusplus\n}\n#endif");

  // save fwrite cached text to file
  fwriter->Flush(fdesc->core_h.fpath);
}

void CiMainGenerator::Gen_MainSource()
{
  // include main header file
  fwriter->AppendLine(StrPrint("#include \"%s\"", fdesc->core_h.fname.c_str()), 3);

  // put diagmonitor ifdef selection for including @drv-fmon header
  // with FMon_* signatures to call from unpack function
  fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usemon_def.c_str()));

  fwriter->AppendText(
    "// This file must define:\n"
    "// base monitor struct\n"
    "// function signature for CRC calculation\n"
    "// function signature for getting system tick value (100 us step)\n");

  fwriter->AppendLine(StrPrint("#include \"%s-fmon.h\"", fdesc->drvname.c_str()), 2);

  fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usemon_def.c_str()), 3);

  // for each message 3 functions must be defined - 1 unpack function,
  // 2: pack with raw signature
  // 3: pack with canstruct
  for (size_t num = 0; num < sigprt->sigs_expr.size(); num++)
  {
    // write message typedef s and additional expressions
    MessageDescriptor_t& m = sigprt->sigs_expr[num]->msg;

    // first function
    fwriter->AppendLine(StrPrint("uint32_t Unpack_%s_%s(%s_t* _m, const uint8_t* _d, uint8_t dlc_)\n{",
                                 m.Name.c_str(), fdesc->DrvName_orig.c_str(), m.Name.c_str()));

    // put dirt trick to avoid warning about unusing parameter
    // (dlc) when monitora are disabled. trick is better than
    // selection different signatures because of external API consistency
    fwriter->AppendLine("  (void)dlc_;");

    WriteUnpackBody(sigprt->sigs_expr[num]);

    fwriter->AppendLine("}", 2);

    // next one is the pack function for using with CANFrame struct
    fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usesruct_def.c_str()), 2);

    // second function
    fwriter->AppendLine(StrPrint("uint32_t Pack_%s_%s(%s_t* _m, __CoderDbcCanFrame_t__* cframe)",
                                 m.Name.c_str(), fdesc->DrvName_orig.c_str(), m.Name.c_str()));

    WritePackStructBody(sigprt->sigs_expr[num]);

    fwriter->AppendLine("#else", 2);

    // third function
    fwriter->AppendLine(StrPrint("uint32_t Pack_%s_%s(%s_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide)",
                                 m.Name.c_str(), fdesc->DrvName_orig.c_str(), m.Name.c_str()));

    WritePackArrayBody(sigprt->sigs_expr[num]);

    fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usesruct_def.c_str()), 2);
  }

  fwriter->Flush(fdesc->core_c.fpath);
}

void CiMainGenerator::Gen_FMonHeader()
{
  fwriter->AppendLine("#pragma once", 2);

  fwriter->AppendLine("#ifdef __cplusplus\nextern \"C\" {\n#endif", 2);

  fwriter->AppendLine(StrPrint("#include \"%s-config.h\"", fdesc->drvname.c_str()), 2);

  // put diagmonitor ifdef selection for including @drv-fmon header
  // with FMon_* signatures to call from unpack function
  fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usemon_def.c_str()), 2);
  fwriter->AppendLine("#include \"canmonitorutil.h\"");
  fwriter->AppendLine("/*\n\
This file contains the prototypes of all the functions that will be called\n\
from each Unpack_*name* function to detect DBC related errors\n\
It is the user responsibility to defined these functions in the\n\
separated .c file. If it won't be done the linkage error will happen\n*/", 2);

  for (size_t num = 0; num < sigprt->sigs_expr.size(); num++)
  {
    auto msg = &(sigprt->sigs_expr[num]->msg);
    fwriter->AppendLine(StrPrint("void FMon_%s_%s(FrameMonitor_t* _mon);",
                                 msg->Name.c_str(), fdesc->drvname.c_str()));
  }

  fwriter->AppendLine(StrPrint("\n#endif // %s", fdesc->usemon_def.c_str()), 2);

  fwriter->AppendLine("#ifdef __cplusplus\n}\n#endif");

  fwriter->Flush(fdesc->fmon_h.fpath);
}

void CiMainGenerator::Gen_FMonSource()
{
  fwriter->AppendLine(StrPrint("#include \"%s\"", fdesc->fmon_h.fname.c_str()), 2);
  // put diagmonitor ifdef selection for including @drv-fmon header
// with FMon_* signatures to call from unpack function
  fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usemon_def.c_str()), 2);

  fwriter->AppendLine("/*\n\
Put the monitor function content here, keep in mind -\n\
next generation will completely clear all manually added code (!)\n\
*/\n");

  for (size_t num = 0; num < sigprt->sigs_expr.size(); num++)
  {
    auto msg = &(sigprt->sigs_expr[num]->msg);
    fwriter->AppendLine(StrPrint("void FMon_%s_%s(FrameMonitor_t* _mon)\n{\n\}\n",
                                 msg->Name.c_str(), fdesc->drvname.c_str()));
  }

  fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usemon_def.c_str()));

  fwriter->Flush(fdesc->fmon_c.fpath);
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

  dtype += "  " + PrintType((int)sig.Type) + " " + sig.Name;

  if (bits && (sig.LengthBit < 8))
  {
    dtype += StrPrint(" : %d", sig.LengthBit);
  }

  dtype += ";";

  std::string pad = " ";

  dtype += pad.insert(0, padwidth + 16 - dtype.size(), ' ');

  fwriter->AppendText(dtype);

  pad = " // ";
  pad += (sig.Signed) ? " [-]" : "    ";

  fwriter->AppendText(pad);

  fwriter->AppendText(StrPrint(" Bits=%2d", sig.LengthBit));

  if (sig.IsDoubleSig)
  {
    if (sig.Offset != 0)
    {
      fwriter->AppendText(StrPrint(" Offset= %-18f", sig.Offset));
    }

    if (sig.Factor != 1)
    {
      fwriter->AppendText(StrPrint(" Factor= %-15f", sig.Factor));
    }
  }
  else if (sig.IsSimpleSig == false)
  {
    // 2 type of signal
    if (sig.Offset != 0)
    {
      fwriter->AppendText(StrPrint(" Offset= %-18d", (int)sig.Offset));
    }

    if (sig.Factor != 1)
    {
      fwriter->AppendText(StrPrint(" Factor= %-15d", (int)sig.Factor));
    }
  }

  if (sig.Unit.size() > 0)
  {
    fwriter->AppendText(StrPrint(" Unit:'%s'", sig.Unit.c_str()));
  }

  fwriter->AppendLine("", 2);

  if (sig.IsDoubleSig)
  {
    // this code only required be d-signals (floating point values based)
    // it placed additional signals to struct for conversion
    // to/from physical values. For non-simple and non-double signal
    // there is no necessity to create addition fields
    // @sigfloat_t must be typedefed by user (e.g. double / float)
    fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usesigfloat_def.c_str()));

    fwriter->AppendLine(StrPrint("  sigfloat_t %s_phys;", sig.Name.c_str()));

    fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usesigfloat_def.c_str()), 2);

  }
}

void CiMainGenerator::WriteUnpackBody(const CiExpr_t* sgs)
{
  for (size_t num = 0; num < sgs->to_signals.size(); num++)
  {
    auto expr = sgs->to_signals[num];

    // for code shortening
    const char* sname = sgs->msg.Signals[num].Name.c_str();

    fwriter->AppendLine(StrPrint("  _m->%s = %s;", sname, expr.c_str()));

    // print sigfloat conversion
    if (sgs->msg.Signals[num].IsDoubleSig)
    {
      fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usesigfloat_def.c_str()));
      fwriter->AppendLine(StrPrint("  _m->%s_phys = (sigfloat_t)(%s_%s_fromS(_m->%s));", sname, fdesc->DRVNAME.c_str(), sname,
                                   sname));
      fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usesigfloat_def.c_str()), 2);
    }

    else if (!sgs->msg.Signals[num].IsSimpleSig)
    {
      // print unpack conversion for non-simple and non-double signals
      // for this case conversion fromS is performed to signal itself
      // without (sigfloat_t) type casting
      fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usesigfloat_def.c_str()));
      fwriter->AppendLine(StrPrint("  _m->%s = (%s_%s_fromS(_m->%s));", sname, fdesc->DRVNAME.c_str(), sname, sname));
      fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usesigfloat_def.c_str()), 2);
    }
    else if (num + 1 == sgs->to_signals.size())
    {
      // last signal without phys part, put \n manually
      fwriter->AppendLine("");
    }
  }

  fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usemon_def.c_str()));
  fwriter->AppendLine(StrPrint("  _m->mon1.dlc_error = (dlc_ < %s_DLC);", sgs->msg.Name.c_str()));
  fwriter->AppendLine("  _m->mon1.last_cycle = GetSysTick();");
  fwriter->AppendLine("  _m->mon1.frame_cnt++;", 2);

  if (sgs->msg.RollSig != nullptr)
  {
    // Put rolling monitor here
    fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->useroll_def.c_str()));
    fwriter->AppendLine(StrPrint("  _m->mon1.roll_error = (_m->%s != _m->%s_expt);",
                                 sgs->msg.RollSig->Name.c_str(), sgs->msg.RollSig->Name.c_str()));
    fwriter->AppendLine(StrPrint("  _m->%s_expt = (_m->%s + 1) & (0x%02XU);", sgs->msg.RollSig->Name.c_str(),
                                 sgs->msg.RollSig->Name.c_str(), (1 << sgs->msg.RollSig->LengthBit) - 1));
    // Put rolling monitor here
    fwriter->AppendLine(StrPrint("#ifdef // %s", fdesc->useroll_def.c_str()), 2);
  }

  if (sgs->msg.CsmSig != nullptr)
  {
    // Put checksum check function call here
    fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usecsm_def.c_str()));
    fwriter->AppendLine(StrPrint("  _m->mon1.csm_error = ((uint8_t)GetFrameCRC(_d, %s_DLC, %s_CANID, %s, %d)) != (_m->%s))",
                                 sgs->msg.Name.c_str(), sgs->msg.Name.c_str(), sgs->msg.CsmMethod.c_str(),
                                 sgs->msg.CsmOp, sgs->msg.CsmSig->Name.c_str()));
    fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usecsm_def.c_str()), 2);
  }

  auto Fmon_func = "FMon_" + sgs->msg.Name + "_" + fdesc->drvname;

  fwriter->AppendLine(StrPrint("  %s(&_m->mon1);", Fmon_func.c_str()));

  fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usemon_def.c_str()), 2);

  fwriter->AppendLine(StrPrint(" return %s_CANID;", sgs->msg.Name.c_str()));
}

void CiMainGenerator::WritePackStructBody(const CiExpr_t* sgs)
{
  fwriter->AppendLine("{");
  PrintPackCommonText("cframe->Data", sgs);
  fwriter->AppendLine(StrPrint("  cframe->MsgId = %s_CANID;", sgs->msg.Name.c_str()));
  fwriter->AppendLine(StrPrint("  cframe->DLC = %s_DLC;", sgs->msg.Name.c_str()));
  fwriter->AppendLine(StrPrint("  cframe->IDE = %s_IDE;", sgs->msg.Name.c_str(), 2));
  fwriter->AppendLine(StrPrint("  return %s_CANID;", sgs->msg.Name.c_str()));
  fwriter->AppendLine("}", 2);
}

void CiMainGenerator::WritePackArrayBody(const CiExpr_t* sgs)
{
  fwriter->AppendLine("{");
  PrintPackCommonText("_d", sgs);
  fwriter->AppendLine(StrPrint("  *_len = %s_DLC;", sgs->msg.Name.c_str()));
  fwriter->AppendLine(StrPrint("  *_ide = %s_IDE;", sgs->msg.Name.c_str(), 2));
  fwriter->AppendLine(StrPrint("  return %s_CANID;", sgs->msg.Name.c_str()));
  fwriter->AppendLine("}", 2);
}

void CiMainGenerator::PrintPackCommonText(const std::string& arrtxt, const CiExpr_t* sgs)
{
  // this function will print part of pack function
  // which is differs only by arra var name

  // pring array content clearin loop
  fwriter->AppendLine(StrPrint("  uint8_t i; for (i = 0; (i < %s_DLC) && (i < 8); %s[i++] = 0);",
                               sgs->msg.Name.c_str(), arrtxt.c_str()), 2);

  if (sgs->msg.RollSig != nullptr)
  {
    fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->useroll_def.c_str()));
    fwriter->AppendLine(StrPrint("  _m->%s = (_m->%s + 1) & (0x%02XU)", sgs->msg.RollSig->Name.c_str(),
                                 sgs->msg.RollSig->Name.c_str(), (1 << sgs->msg.RollSig->LengthBit) - 1));
    fwriter->AppendLine(StrPrint("#ifdef // %s", fdesc->useroll_def.c_str()), 2);
  }

  if (sgs->msg.CsmSig != nullptr)
  {
    // code for clearing checksum
    fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usecsm_def.c_str()));
    fwriter->AppendLine(StrPrint("  _m->%s = 0U;", sgs->msg.CsmSig->Name.c_str()));
    fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usecsm_def.c_str()), 2);
  }

  if (sgs->msg.hasPhys)
  {
    // first step is to put code for sigfloat conversion, before
    // sigint packing to bytes.
    fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usesigfloat_def.c_str()));

    for (size_t n = 0; n < sgs->to_signals.size(); n++)
    {
      if (sgs->msg.Signals[n].IsSimpleSig == false)
      {
        if (sgs->msg.Signals[n].IsDoubleSig)
        {
          // print toS from *_phys to original named sigint (integer duplicate of signal)
          fwriter->AppendLine(StrPrint("  _m->%s = %s_%s_fromS(_m->%s_phys);",
                                       sgs->msg.Signals[n].Name.c_str(), fdesc->DRVNAME.c_str(),
                                       sgs->msg.Signals[n].Name.c_str(), sgs->msg.Signals[n].Name.c_str()));
        }
        else
        {
          // print toS from original named signal to itself (because this signal
          // has enough space for scaling by factor and proper sign
          fwriter->AppendLine(StrPrint("  _m->%s = %s_%s_fromS(_m->%s);",
                                       sgs->msg.Signals[n].Name.c_str(), fdesc->DRVNAME.c_str(),
                                       sgs->msg.Signals[n].Name.c_str(), sgs->msg.Signals[n].Name.c_str()));
        }
      }
    }

    fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usesigfloat_def.c_str()), 2);
  }

  for (size_t i = 0; i < sgs->to_bytes.size(); i++)
  {
    if (sgs->to_bytes[i].size() < 2)
      continue;

    fwriter->AppendLine(StrPrint("  %s[%d] |= %s;", arrtxt.c_str(), i, sgs->to_bytes[i].c_str()));
  }

  fwriter->AppendLine("");

  if (sgs->msg.CsmSig != nullptr)
  {
    // code for getting checksum value and putting it in array
    fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usecsm_def.c_str()));

    fwriter->AppendLine(StrPrint("  _m->%s = ((uint8_t)GetFrameCRC(_d, %s_DLC, %s_CANID, %s, %d));",
                                 sgs->msg.CsmSig->Name.c_str(), sgs->msg.Name.c_str(),
                                 sgs->msg.Name.c_str(), sgs->msg.CsmMethod.c_str(), sgs->msg.CsmOp));

    fwriter->AppendLine(StrPrint("  %s[%d] = %s;", arrtxt.c_str(), sgs->msg.CsmByteNum, sgs->msg.CsmToByteExpr.c_str()));

    fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usecsm_def.c_str()), 2);
  }
}
