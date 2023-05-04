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

const char* ext_sig_func_name = "__ext_sig__";

const char* extend_func_body =
  "// This function performs extension of sign for the signals\n"
  "// which have non-aligned to power of 2 bit's width.\n"
  "// The types 'bitext_t' and 'ubitext_t' define maximal bit width which\n"
  "// can be correctly handled. You need to select type which can contain\n"
  "// n+1 bits where n is the largest signed signal width. For example if\n"
  "// the most wide signed signal has a width of 31 bits you need to set\n"
  "// bitext_t as int32_t and ubitext_t as uint32_t\n"
  "// Defined these typedefs in @dbccodeconf.h or locally in 'dbcdrvname'-config.h\n"
  "static bitext_t %s(ubitext_t val, uint8_t bits)\n"
  "{\n"
  "  ubitext_t const m = 1u << (bits - 1);\n"
  "  return (val ^ m) - m;\n"
  "}\n";

CiMainGenerator::CiMainGenerator()
{
  sigprt = new CSigPrinter;
  fwriter = new FileWriter;
}

void CiMainGenerator::Generate(DbcMessageList_t& dlist, const FsDescriptor_t& fsd)
{
  p_dlist = &dlist;
  // Load income messages to sig printer
  sigprt->LoadMessages(dlist.msgs);

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

  // 6 step is to print template for drv-config.h
  Gen_ConfigHeader();

  // 7 step is to print canmonitorutil.h template code
  Gen_CanMonUtil();

  // 8 step is to print dbccodeconf.h template
  Gen_DbcCodeConf();
}

void CiMainGenerator::Gen_MainHeader()
{
  // write comment start text
  if (fdesc->start_info.size() > 0)
  {
    // replace all '\n' on "\n //" for c code comment text
    fwriter->AppendLine("// " + std::regex_replace(fdesc->start_info, std::regex("\n"), "\n// "));
  }

  fwriter->AppendLine("#pragma once", 2);
  fwriter->AppendLine("#ifdef __cplusplus\nextern \"C\" {\n#endif", 2);
  fwriter->AppendLine("#include <stdint.h>", 2);

  fwriter->AppendLine("// DBC file version");
  fwriter->AppendLine(StrPrint("#define %s (%uU)", fdesc->verhigh_def.c_str(), p_dlist->ver.hi));
  fwriter->AppendLine(StrPrint("#define %s (%uU)", fdesc->verlow_def.c_str(), p_dlist->ver.low), 2);

  fwriter->AppendLine("// include current dbc-driver compilation config");
  fwriter->AppendLine(StrPrint("#include <%s-config.h>", fdesc->drvname.c_str()), 2);

  fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usemon_def.c_str()));

  fwriter->AppendText(
    "// This file must define:\n"
    "// base monitor struct\n"
    "// function signature for HASH calculation: (@GetFrameHash)\n"
    "// function signature for getting system tick value: (@GetSystemTick)\n"
    "#include <canmonitorutil.h>\n"
    "\n"
  );

  fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usemon_def.c_str()), 3);

  for (size_t num = 0; num < sigprt->sigs_expr.size(); num++)
  {
    // write message typedef s and additional expressions
    MessageDescriptor_t& m = sigprt->sigs_expr[num]->msg;

    if (m.CommentText.size() > 0)
    {
      // replace all '\n' on "\n //" for c code comment text
      fwriter->AppendLine("// " + std::regex_replace(m.CommentText, std::regex("\n"), "\n// "));
    }

    fwriter->AppendLine(StrPrint("// def @%s CAN Message (%-4d %#x)", m.Name.c_str(), m.MsgID, m.MsgID));
    fwriter->AppendLine(StrPrint("#define %s_IDE (%uU)", m.Name.c_str(), m.IsExt));
    fwriter->AppendLine(StrPrint("#define %s_DLC (%uU)", m.Name.c_str(), m.DLC));
    fwriter->AppendLine(StrPrint("#define %s_CANID (%#x)", m.Name.c_str(), m.MsgID));

    if (m.Cycle > 0)
    {
      fwriter->AppendLine(StrPrint("#define %s_CYC (%dU)", m.Name.c_str(), m.Cycle));
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
      {
        max_sig_name_len = s.Name.size();
      }

      // For each signal in current message print value tables definitions
      if (s.ValDefs.vpairs.size() > 0)
      {
        fwriter->AppendLine(StrPrint("\n// Value tables for @%s signal", s.Name.c_str()), 2);

        for (auto i = 0; i < s.ValDefs.vpairs.size(); i++)
        {
          // The value table definition consists of 'signal name + message name + value definition'
          // This provides reliable way of avoiding issues with same macros names
          std::string defname = StrPrint("%s_%s_%s", s.Name.c_str(), m.Name.c_str(), s.ValDefs.vpairs[i].first.c_str());

          // @ifndef guard for the case when different values of table have
          // the same name (it is valid for DBC file format)
          // For this case only one of same named values will be available as macro
          fwriter->AppendLine(StrPrint("#ifndef %s", defname.c_str()));

          fwriter->AppendLine(StrPrint("#define %s_%s_%s (%d)",
              s.Name.c_str(), m.Name.c_str(), s.ValDefs.vpairs[i].first.c_str(),
              s.ValDefs.vpairs[i].second));

          fwriter->AppendLine(StrPrint("#endif"), 2);
        }
      }
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
  if (fdesc->start_info.size() > 0)
  {
    // replace all '\n' on "\n //" for c code comment text
    fwriter->AppendLine("// " + std::regex_replace(fdesc->start_info, std::regex("\n"), "\n// "));
  }

  // include main header file
  fwriter->AppendLine(StrPrint("#include \"%s\"", fdesc->core_h.fname.c_str()), 3);

  fwriter->AppendLine("// DBC file version");
  fwriter->AppendLine(StrPrint("#if (%s != (%uU)) || (%s != (%uU))",
      fdesc->verhigh_def.c_str(), p_dlist->ver.hi, fdesc->verlow_def.c_str(), p_dlist->ver.low));

  fwriter->AppendLine(StrPrint("#error The %s dbc source files have different versions", fdesc->DRVNAME.c_str()));
  fwriter->AppendLine("#endif", 2);

  // put diagmonitor ifdef selection for including @drv-fmon header
  // with FMon_* signatures to call from unpack function
  fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usemon_def.c_str()));

  fwriter->AppendText(
    "// Function prototypes to be called each time CAN frame is unpacked\n"
    "// FMon function may detect RC, CRC or DLC violation\n");

  fwriter->AppendLine(StrPrint("#include <%s-fmon.h>", fdesc->drvname.c_str()), 2);

  fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usemon_def.c_str()), 3);

  fwriter->AppendLine(StrPrint(extend_func_body, ext_sig_func_name), 1);

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

void CiMainGenerator::Gen_ConfigHeader()
{
  if (fdesc->start_info.size() > 0)
  {
    // replace all '\n' on "\n //" for c code comment text
    fwriter->AppendLine("// " + std::regex_replace(fdesc->start_info, std::regex("\n"), "\n// "));
  }

  fwriter->AppendLine("#pragma once");
  fwriter->AppendLine("");
  fwriter->AppendLine("/* include common dbccode configurations */");
  fwriter->AppendLine("#include <dbccodeconf.h>");
  fwriter->AppendLine("");
  fwriter->AppendLine("");
  fwriter->AppendLine("/* ------------------------------------------------------------------------- *");
  fwriter->AppendLine("  This define enables using CAN message structs with bit-fielded signals");
  fwriter->AppendLine("  layout.");
  fwriter->AppendLine("");
  fwriter->AppendLine("  Note(!): bit-feild was not tested properly. */");
  fwriter->AppendLine("");
  fwriter->AppendLine(StrPrint("/* #define %s */", fdesc->usebits_def.c_str()), 3);

  fwriter->AppendLine("/* ------------------------------------------------------------------------- *");
  fwriter->AppendLine("  This macro enables using CAN message descriptive struct packing functions");
  fwriter->AppendLine("  (by default signature of pack function intakes a few simple typed params");
  fwriter->AppendLine("  for loading data, len, etc). To compile you need to define the struct");
  fwriter->AppendLine("  __CoderDbcCanFrame_t__ which must have fields:");
  fwriter->AppendLine("");
  fwriter->AppendLine("    u32 MsgId (CAN Frame message ID)");
  fwriter->AppendLine("    u8 DLC (CAN Frame payload length field)");
  fwriter->AppendLine("    u8 Data[8] (CAN Frame payload data)");
  fwriter->AppendLine("    u8 IDE (CAN Frame Extended (1) / Standard (0) ID type)");
  fwriter->AppendLine("");
  fwriter->AppendLine("  This struct definition have to be placed (or be included) in dbccodeconf.h */");
  fwriter->AppendLine("");
  fwriter->AppendLine(StrPrint("/* #define %s */", fdesc->usesruct_def.c_str()), 3);

  fwriter->AppendLine("/* ------------------------------------------------------------------------- *");
  fwriter->AppendLine("  All the signals which have values of factor != 1 or offset != 0");
  fwriter->AppendLine("  will be named in message struct with posfix '_ro'. Pack to payload");
  fwriter->AppendLine("  operations will be made on this signal value as well as unpack from payload.");
  fwriter->AppendLine("");
  fwriter->AppendLine("  USE_SIGFLOAT macro makes some difference:");
  fwriter->AppendLine("");
  fwriter->AppendLine("  1. All the '_ro' fields will have a pair field with '_phys' postfix.");
  fwriter->AppendLine("  If only offset != 0 is true then the type of '_phys' signal is the same");
  fwriter->AppendLine("  as '_ro' signal. In other case the type will be @sigfloat_t which");
  fwriter->AppendLine("  have to be defined in user dbccodeconf.h");
  fwriter->AppendLine("");
  fwriter->AppendLine("  2. In pack function '_ro' signal will be rewritten by '_phys' signal, which");
  fwriter->AppendLine("  requires from user to use ONLY '_phys' signal for packing frame");
  fwriter->AppendLine("");
  fwriter->AppendLine("  3. In unpack function '_phys' signal will be written by '_ro' signal.");
  fwriter->AppendLine("  User have to use '_phys' signal to read physical value. */");
  fwriter->AppendLine("");
  fwriter->AppendLine(StrPrint("/* #define %s */", fdesc->usesigfloat_def.c_str()), 3);

  fwriter->AppendLine("/* ------------------------------------------------------------------------- *");
  fwriter->AppendLine("  Note(!) that the \"canmonitorutil.h\" must be accessed in include path:");
  fwriter->AppendLine("");
  fwriter->AppendLine("  This macro adds:");
  fwriter->AppendLine("");
  fwriter->AppendLine("  - monitor field @mon1 to message struct");
  fwriter->AppendLine("");
  fwriter->AppendLine("  - capture system tick in unpack function and save value to mon1 field");
  fwriter->AppendLine("  to provide to user better missing frame detection code. For this case");
  fwriter->AppendLine("  user must provide function declared in canmonitorutil.h - GetSysTick()");
  fwriter->AppendLine("  which may return 1ms uptime.");
  fwriter->AppendLine("");
  fwriter->AppendLine("  - calling function FMon_***  (from 'fmon' driver) inside unpack function");
  fwriter->AppendLine("  which is empty by default and have to be filled by user if");
  fwriter->AppendLine("  tests for DLC, rolling, checksum are necessary */");
  fwriter->AppendLine("");
  fwriter->AppendLine(StrPrint("/* #define %s */", fdesc->usemon_def.c_str()), 3);

  fwriter->AppendLine("/* ------------------------------------------------------------------------- *");
  fwriter->AppendLine(StrPrint("  When monitor using is enabled (%s) and define below", fdesc->usemon_def.c_str()));
  fwriter->AppendLine("  uncommented, additional signal will be added to message struct. ***_expt:");
  fwriter->AppendLine("  expected rolling counter, to perform monitoring rolling counter sequence");
  fwriter->AppendLine("  automatically (result may be tested in dedicated Fmon_*** function) */");
  fwriter->AppendLine("");
  fwriter->AppendLine(StrPrint("/* #define %s */", fdesc->useroll_def.c_str()), 3);

  fwriter->AppendLine("/* ------------------------------------------------------------------------- *");
  fwriter->AppendLine(StrPrint("  When monitor using is enabled (%s) and define below", fdesc->usemon_def.c_str()));
  fwriter->AppendLine("  uncommented, frame checksum signal may be handled automatically.");
  fwriter->AppendLine("");
  fwriter->AppendLine("  The signal which may be marked as checksum signal must have substring");
  fwriter->AppendLine("  with next format:");
  fwriter->AppendLine("    <Checksum:XOR8:3>");
  fwriter->AppendLine("");
  fwriter->AppendLine("  where:");
  fwriter->AppendLine("");
  fwriter->AppendLine("  - \"Checksum\": constant marker word");
  fwriter->AppendLine("");
  fwriter->AppendLine("  - \"XOR8\": type of method, this text will be passed to GetFrameHash");
  fwriter->AppendLine("  (canmonitorutil.h) function as is, the best use case is to define 'enum");
  fwriter->AppendLine("  DbcCanCrcMethods' in canmonitorutil.h file with all possible");
  fwriter->AppendLine("  checksum algorithms (e.g. XOR8, XOR4 etc)");
  fwriter->AppendLine("");
  fwriter->AppendLine("  - \"3\": optional value that will be passed to GetFrameHash as integer value");
  fwriter->AppendLine("");
  fwriter->AppendLine("  Function GetFrameHash have to be implemented by user");
  fwriter->AppendLine("");
  fwriter->AppendLine("  In pack function checksum signal will be calculated automatically");
  fwriter->AppendLine("  and loaded to payload");
  fwriter->AppendLine("");
  fwriter->AppendLine("  In unpack function checksum signal is checked with calculated.");
  fwriter->AppendLine("  (result may be tested in dedicated Fmon_*** function). */");
  fwriter->AppendLine("");
  fwriter->AppendLine(StrPrint("/* #define %s */", fdesc->usecsm_def.c_str()), 2);

  fwriter->Flush(fdesc->confdir + '/' + fdesc->drvname + "-config.h");
}

void CiMainGenerator::Gen_FMonHeader()
{
  if (fdesc->start_info.size() > 0)
  {
    // replace all '\n' on "\n //" for c code comment text
    fwriter->AppendLine("// " + std::regex_replace(fdesc->start_info, std::regex("\n"), "\n// "));
  }

  fwriter->AppendLine("#pragma once", 2);

  fwriter->AppendLine("#ifdef __cplusplus\nextern \"C\" {\n#endif", 2);

  fwriter->AppendLine("// DBC file version");
  fwriter->AppendLine(StrPrint("#define %s_FMON (%uU)", fdesc->verhigh_def.c_str(), p_dlist->ver.hi));
  fwriter->AppendLine(StrPrint("#define %s_FMON (%uU)", fdesc->verlow_def.c_str(), p_dlist->ver.low), 2);

  fwriter->AppendLine(StrPrint("#include <%s-config.h>", fdesc->drvname.c_str()), 2);

  // put diagmonitor ifdef selection for including @drv-fmon header
  // with FMon_* signatures to call from unpack function
  fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usemon_def.c_str()), 2);
  fwriter->AppendLine("#include <canmonitorutil.h>");
  fwriter->AppendLine("/*\n\
This file contains the prototypes of all the functions that will be called\n\
from each Unpack_*name* function to detect DBC related errors\n\
It is the user responsibility to defined these functions in the\n\
separated .c file. If it won't be done the linkage error will happen\n*/", 2);

  for (size_t num = 0; num < sigprt->sigs_expr.size(); num++)
  {
    auto msg = &(sigprt->sigs_expr[num]->msg);
    fwriter->AppendLine(StrPrint("void FMon_%s_%s(FrameMonitor_t* _mon, uint32_t msgid);",
        msg->Name.c_str(), fdesc->drvname.c_str()));
  }

  fwriter->AppendLine(StrPrint("\n#endif // %s", fdesc->usemon_def.c_str()), 2);

  fwriter->AppendLine("#ifdef __cplusplus\n}\n#endif");

  fwriter->Flush(fdesc->fmon_h.fpath);
}

void CiMainGenerator::Gen_FMonSource()
{
  if (fdesc->start_info.size() > 0)
  {
    // replace all '\n' on "\n //" for c code comment text
    fwriter->AppendLine("// " + std::regex_replace(fdesc->start_info, std::regex("\n"), "\n// "));
  }

  fwriter->AppendLine(StrPrint("#include <%s>", fdesc->fmon_h.fname.c_str()), 2);
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
    fwriter->AppendLine(
      StrPrint("void FMon_%s_%s(FrameMonitor_t* _mon, uint32_t msgid)\n{\n  (void)_mon;\n  (void)msgid;\n}\n",
        msg->Name.c_str(), fdesc->drvname.c_str()));
  }

  fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usemon_def.c_str()));

  fwriter->Flush(fdesc->fmon_c.fpath);
}

void CiMainGenerator::Gen_CanMonUtil()
{
  fwriter->AppendLine("#pragma once");
  fwriter->AppendLine("");
  fwriter->AppendLine("#include <stdint.h>");
  fwriter->AppendLine("");
  fwriter->AppendLine("#ifdef __cplusplus");
  fwriter->AppendLine("extern \"C\" {");
  fwriter->AppendLine("#endif");
  fwriter->AppendLine("");
  fwriter->AppendLine("// declare here all availible checksum algorithms");
  fwriter->AppendLine("typedef enum");
  fwriter->AppendLine("{");
  fwriter->AppendLine("  // XOR8 = 0,");
  fwriter->AppendLine("  // XOR4 = 1,");
  fwriter->AppendLine("  // etc");
  fwriter->AppendLine("} DbcCanCrcMethods;");
  fwriter->AppendLine("");
  fwriter->AppendLine("typedef struct");
  fwriter->AppendLine("{");
  fwriter->AppendLine("  // @last_cycle keeps tick-value when last frame was received");
  fwriter->AppendLine("  uint32_t last_cycle;");
  fwriter->AppendLine("");
  fwriter->AppendLine("  // @timeout_cycle keeps maximum timeout for frame, user responsibility");
  fwriter->AppendLine("  // to init this field and use it in missing frame monitoring function");
  fwriter->AppendLine("  uint32_t timeout_cycle;");
  fwriter->AppendLine("");
  fwriter->AppendLine("  // @frame_cnt keeps count of all the received frames");
  fwriter->AppendLine("  uint32_t frame_cnt;");
  fwriter->AppendLine("");
  fwriter->AppendLine("  // setting up @roll_error bit indicates roll counting fail.");
  fwriter->AppendLine("  // Bit is not clearing automatically!");
  fwriter->AppendLine("  uint32_t roll_error : 1;");
  fwriter->AppendLine("");
  fwriter->AppendLine("  // setting up @checksum_error bit indicates checksum checking failure.");
  fwriter->AppendLine("  // Bit is not clearing automatically!");
  fwriter->AppendLine("  uint32_t csm_error : 1;");
  fwriter->AppendLine("");
  fwriter->AppendLine("  // setting up @cycle_error bit indicates that time was overrunned.");
  fwriter->AppendLine("  // Bit is not clearing automatically!");
  fwriter->AppendLine("  uint32_t cycle_error : 1;");
  fwriter->AppendLine("");
  fwriter->AppendLine("  // setting up @dlc_error bit indicates that the actual length of");
  fwriter->AppendLine("  // CAN frame is less then defined by CAN matrix!");
  fwriter->AppendLine("  uint32_t dlc_error : 1;");
  fwriter->AppendLine("");
  fwriter->AppendLine("} FrameMonitor_t;");
  fwriter->AppendLine("");
  fwriter->AppendLine("/* ----------------------------------------------------------------------------- */");
  fwriter->AppendLine("// @d - buff for hash calculation");
  fwriter->AppendLine("// @len - number of bytes for hash calculation");
  fwriter->AppendLine("// @method - hash algorythm.");
  fwriter->AppendLine("// @op - optional value");
  fwriter->AppendLine("uint8_t GetFrameHash(const uint8_t* data_ptr, uint8_t len, uint32_t msgid, DbcCanCrcMethods type, uint32_t option);");
  fwriter->AppendLine("");
  fwriter->AppendLine("/* ----------------------------------------------------------------------------- */");
  fwriter->AppendLine("// this function will be called when unpacking is performing. Value will be saved");
  fwriter->AppendLine("// in @last_cycle variable");
  fwriter->AppendLine("uint32_t GetSystemTick(void);");
  fwriter->AppendLine("");
  fwriter->AppendLine("");
  fwriter->AppendLine("#ifdef __cplusplus");
  fwriter->AppendLine("}");
  fwriter->AppendLine("#endif");
  fwriter->AppendLine("");

  fwriter->Flush(fdesc->incdir + '/' + "canmonitorutil.h");
}

void CiMainGenerator::Gen_DbcCodeConf()
{
  fwriter->AppendLine("#pragma once");
  fwriter->AppendLine("");
  fwriter->AppendLine("#include <stdint.h>");
  fwriter->AppendLine("");
  fwriter->AppendLine("// when USE_SIGFLOAT enabed the sigfloat_t must be defined");
  fwriter->AppendLine("// typedef double sigfloat_t;");
  fwriter->AppendLine("");
  fwriter->AppendLine("// when USE_CANSTRUCT enabled __CoderDbcCanFrame_t__ must be defined");
  fwriter->AppendLine("// #include \"{header_with_can_struct}\"");
  fwriter->AppendLine("// typedef {can_struct} __CoderDbcCanFrame_t__;");
  fwriter->AppendLine("");
  fwriter->AppendLine("// if you need to allocate rx and tx messages structs put the allocation macro here");
  fwriter->AppendLine("// #define __DEF_{your_driver_name}__");
  fwriter->AppendLine("");

  fwriter->Flush(fdesc->confdir + '/' + "dbccodeconf.h");
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

  if (sig.Multiplex == MultiplexType::kMulValue)
  {
    fwriter->AppendLine("  // multiplex variable");
  }
  else if (sig.Multiplex == MultiplexType::kMaster)
  {
    fwriter->AppendLine("  // MULTIPLEX master signal");
  }

  std::string dtype = "";

  dtype += "  " + PrintType((int)sig.TypeRo) + " " + sig.Name;

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

  size_t offset = 0;
  std::string infocmnt{};

  if (sig.IsDoubleSig)
  {
    if (sig.Offset != 0)
    {
      infocmnt = IndentedString(offset, infocmnt);
      offset += 27;
      infocmnt += StrPrint(" Offset= %f", sig.Offset);
    }

    if (sig.Factor != 1)
    {
      infocmnt = IndentedString(offset, infocmnt);
      offset += 24;
      infocmnt += StrPrint(" Factor= %f", sig.Factor);
    }
  }
  else if (sig.IsSimpleSig == false)
  {
    // 2 type of signal
    if (sig.Offset != 0)
    {
      infocmnt = IndentedString(offset, infocmnt);
      offset += 27;
      infocmnt += StrPrint(" Offset= %d", (int)sig.Offset);
    }

    if (sig.Factor != 1)
    {
      infocmnt = IndentedString(offset, infocmnt);
      offset += 24;
      infocmnt += StrPrint(" Factor= %d", (int)sig.Factor);
    }
  }

  if (sig.Unit.size() > 0)
  {
    infocmnt = IndentedString(offset, infocmnt);
    infocmnt += StrPrint(" Unit:'%s'", sig.Unit.c_str());
  }

  fwriter->AppendText(infocmnt);

  fwriter->AppendLine("", 2);

  if (!sig.IsSimpleSig)
  {
    // this code only required be d-signals (floating point values based)
    // it placed additional signals to struct for conversion
    // to/from physical values. For non-simple and non-double signal
    // there is no necessity to create addition fields
    // @sigfloat_t must be typedefed by user (e.g. double / float)

    // UPD: from this commit, all non-Simple signals has it's
    // own 'shadow' (_phys) copies, the problem with intermediate type (not simpe and
    // not double) is that the x = ***_toS(x) takes place in each Pack_* call
    // the signals which are not changing from Pack_* to Pack_* will change its values (!)
    fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usesigfloat_def.c_str()));

    if (sig.IsDoubleSig)
    {
      fwriter->AppendLine(StrPrint("  sigfloat_t %s;", sig.NameFloat.c_str()));
    }
    else
    {
      fwriter->AppendLine(StrPrint("  %s %s;", PrintType((int)sig.TypePhys).c_str(), sig.NameFloat.c_str()));
    }

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

    if (sgs->msg.Signals[num].Signed)
    {
      fwriter->AppendLine(StrPrint("  _m->%s = %s(( %s ), %d);",
          sname, ext_sig_func_name, expr.c_str(), (int32_t)sgs->msg.Signals[num].LengthBit));
    }
    else
    {
      fwriter->AppendLine(StrPrint("  _m->%s = %s;", sname, expr.c_str()));
    }

    // print sigfloat conversion
    if (!sgs->msg.Signals[num].IsSimpleSig)
    {
      fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usesigfloat_def.c_str()));

      if (sgs->msg.Signals[num].IsDoubleSig)
      {
        // for double signals (sigfloat_t) type cast
        fwriter->AppendLine(StrPrint("  _m->%s = (sigfloat_t)(%s_%s_fromS(_m->%s));",
            sgs->msg.Signals[num].NameFloat.c_str(), fdesc->DRVNAME.c_str(), sname, sname));
      }
      else
      {
        fwriter->AppendLine(StrPrint("  _m->%s = %s_%s_fromS(_m->%s);",
            sgs->msg.Signals[num].NameFloat.c_str(), fdesc->DRVNAME.c_str(), sname, sname));
      }

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
  fwriter->AppendLine("  _m->mon1.last_cycle = GetSystemTick();");
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
    fwriter->AppendLine(StrPrint("#endif // %s", fdesc->useroll_def.c_str()), 2);
  }

  if (sgs->msg.CsmSig != nullptr)
  {
    // Put checksum check function call here
    fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usecsm_def.c_str()));
    fwriter->AppendLine(
      StrPrint("  _m->mon1.csm_error = (((uint8_t)GetFrameHash(_d, %s_DLC, %s_CANID, %s, %d)) != (_m->%s));",
        sgs->msg.Name.c_str(), sgs->msg.Name.c_str(), sgs->msg.CsmMethod.c_str(),
        sgs->msg.CsmOp, sgs->msg.CsmSig->Name.c_str()));
    fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usecsm_def.c_str()), 2);
  }

  auto Fmon_func = "FMon_" + sgs->msg.Name + "_" + fdesc->drvname;

  fwriter->AppendLine(StrPrint("  %s(&_m->mon1, %s_CANID);", Fmon_func.c_str(), sgs->msg.Name.c_str()));

  fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usemon_def.c_str()), 2);

  fwriter->AppendLine(StrPrint("  return %s_CANID;", sgs->msg.Name.c_str()));
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
    fwriter->AppendLine(StrPrint("  _m->%s = (_m->%s + 1) & (0x%02XU);", sgs->msg.RollSig->Name.c_str(),
        sgs->msg.RollSig->Name.c_str(), (1 << sgs->msg.RollSig->LengthBit) - 1));
    fwriter->AppendLine(StrPrint("#endif // %s", fdesc->useroll_def.c_str()), 2);
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
        // print toS from *_phys to original named sigint (integer duplicate of signal)
        fwriter->AppendLine(StrPrint("  _m->%s = %s_%s_toS(_m->%s);",
            sgs->msg.Signals[n].Name.c_str(), fdesc->DRVNAME.c_str(),
            sgs->msg.Signals[n].Name.c_str(), sgs->msg.Signals[n].NameFloat.c_str()));
      }
    }

    fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usesigfloat_def.c_str()), 2);
  }

  for (size_t i = 0; i < sgs->to_bytes.size(); i++)
  {
    if (sgs->to_bytes[i].size() < 2)
    {
      continue;
    }

    fwriter->AppendLine(StrPrint("  %s[%d] |= %s;", arrtxt.c_str(), i, sgs->to_bytes[i].c_str()));
  }

  fwriter->AppendLine("");

  if (sgs->msg.CsmSig != nullptr)
  {
    // code for getting checksum value and putting it in array
    fwriter->AppendLine(StrPrint("#ifdef %s", fdesc->usecsm_def.c_str()));

    fwriter->AppendLine(StrPrint("  _m->%s = ((uint8_t)GetFrameHash(%s, %s_DLC, %s_CANID, %s, %d));",
        sgs->msg.CsmSig->Name.c_str(), arrtxt.c_str(), sgs->msg.Name.c_str(),
        sgs->msg.Name.c_str(), sgs->msg.CsmMethod.c_str(), sgs->msg.CsmOp));

    fwriter->AppendLine(StrPrint("  %s[%d] |= %s;", arrtxt.c_str(), sgs->msg.CsmByteNum, sgs->msg.CsmToByteExpr.c_str()));

    fwriter->AppendLine(StrPrint("#endif // %s", fdesc->usecsm_def.c_str()), 2);
  }
}

