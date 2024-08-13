#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdarg.h>
#include <filesystem>
#include <algorithm>
#include <set>
#include <regex>
#include "helpers/formatter.h"
#include "mon-generator.h"
#include "config-generator.h"

#include "c-main-generator.h"

const char* ext_sig_func_name = "__ext_sig__";

const char* extend_func_body =
  "// This function performs extension of sign for the signals\n"
  "// whose bit width value is not aligned to one of power of 2 or less than 8.\n"
  "// The types 'bitext_t' and 'ubitext_t' define the biggest bit width which\n"
  "// can be correctly handled. You need to select type which can contain\n"
  "// n+1 bits where n is the largest signed signal width. For example if\n"
  "// the most wide signed signal has a width of 31 bits you need to set\n"
  "// bitext_t as int32_t and ubitext_t as uint32_t\n"
  "// Defined these typedefs in @dbccodeconf.h or locally in 'dbcdrvname'-config.h\n"
  "static bitext_t %s(ubitext_t val, uint8_t bits)\n"
  "{\n"
  "  ubitext_t const m = (ubitext_t) (1u << (bits - 1u));\n"
  "  return ((val ^ m) - m);\n"
  "}\n\n";

void CiMainGenerator::Generate(DbcMessageList_t& dlist, const AppSettings_t& fsd)
{
  // Load income messages to sig printer
  sigprt.LoadMessages(dlist.msgs);

  // save max dlc value from message list for printing on the following generation steps
  val_maxDlcValueFromDbcList = dlist.maxDlcValue;

  // save pointer to output file descriptor struct to
  // enable using this information inside class member functions
  fdesc = &fsd;

  std::sort(sigprt.sigs_expr.begin(), sigprt.sigs_expr.end(),
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

  if (!fsd.gen.no_fmon)
  {
    // 5 step is to print fmon source file
    Gen_FMonSource();
  }

  if (!fsd.gen.no_config)
  {
    // 6 step is to print template for drv-config.h
    Gen_ConfigHeader();

    // 8 step is to print dbccodeconf.h template
    Gen_DbcCodeConf();
  }

  if (!fsd.gen.no_inc)
  {
    // 7 step is to print canmonitorutil.h template code
    Gen_CanMonUtil();
  }
}

void CiMainGenerator::Gen_MainHeader()
{
  std::set<std::string> passed_sigs;
  fwriter.AppendText(fdesc->gen.start_common_info);
  fwriter.AppendText(fdesc->gen.start_driver_info);
  fwriter.Append("#pragma once");
  fwriter.Append();
  fwriter.Append("#ifdef __cplusplus\nextern \"C\" {\n#endif");
  fwriter.Append();
  fwriter.Append("#include <stdint.h>");
  fwriter.Append();

  fwriter.Append("// DBC file version");
  fwriter.Append("#define %s (%uU)", fdesc->gen.verhigh_def.c_str(), fdesc->gen.hiver);
  fwriter.Append("#define %s (%uU)", fdesc->gen.verlow_def.c_str(), fdesc->gen.lowver);
  fwriter.Append();

  fwriter.Append("// include current dbc-driver compilation config");
  fwriter.Append("#include \"%s-config.h\"", fdesc->gen.drvname.c_str());
  fwriter.Append();

  fwriter.Append("#ifdef %s", fdesc->gen.usemon_def.c_str());

  fwriter.Append(
    "// This file must define:\n"
    "// base monitor struct\n"
    "#include \"canmonitorutil.h\"\n"
    "\n"
  );

  fwriter.Append("#endif // %s", fdesc->gen.usemon_def.c_str());
  fwriter.Append(2);

  // set macro name for max dlc value based on driver name
  std::string maxDlcMacroName = fdesc->gen.DRVNAME + "_MAX_DLC_VALUE";

  // set macro name for dlc validation
  prt_dlcValidateMacroName = fdesc->gen.DRVNAME + "_VALIDATE_DLC";

  // set macro name for initial data byte value based on driver name
  prt_initialDataByteValueName = fdesc->gen.DRVNAME + "_INITIAL_BYTE_VALUE";

  // print part with max DLC macro
  fwriter.Append("// DLC maximum value which is used as the limit for frame's data buffer size.");
  fwriter.Append("// Client can set its own value (not sure why) in driver-config");
  fwriter.Append("// or can test it on some limit specified by application");
  fwriter.Append("// e.g.: static_assert(TESTDB_MAX_DLC_VALUE <= APPLICATION_FRAME_DATA_SIZE, \"Max DLC value in the driver is too big\")");
  fwriter.Append("#ifndef %s", maxDlcMacroName.c_str());
  fwriter.Append("// The value which was found out by generator (real max value)");
  fwriter.Append("#define %s %uU", maxDlcMacroName.c_str(), val_maxDlcValueFromDbcList);
  fwriter.Append("#endif");
  fwriter.Append(1);

  // actual macro for final DLC validation in the driver
  fwriter.Append("// The limit is used for setting frame's data bytes");
  fwriter.Append("#define %s(msgDlc) (((msgDlc) <= (%s)) ? (msgDlc) : (%s))",
    prt_dlcValidateMacroName.c_str(), maxDlcMacroName.c_str(), maxDlcMacroName.c_str());

  // print initial data byte section
  fwriter.Append(1);
  fwriter.Append("// Initial byte value to be filles in data bytes of the frame before pack signals");
  fwriter.Append("// User can define its own custom value in driver-config file");
  fwriter.Append("#ifndef %s", prt_initialDataByteValueName.c_str());
  fwriter.Append("#define %s 0U", prt_initialDataByteValueName.c_str());
  fwriter.Append("#endif");
  fwriter.Append(2);

  for (size_t num = 0u; num < sigprt.sigs_expr.size(); num++)
  {
    // write message typedef s and additional expressions
    MessageDescriptor_t& m = sigprt.sigs_expr[num]->msg;

    if (m.CommentText.size() > 0)
    {
      // replace all '\n' on "\n //" for c code comment text
      fwriter.Append("// " + std::regex_replace(m.CommentText, std::regex("\n"), "\n// "));
    }

    fwriter.Append("// def @%s CAN Message (%-4d %#x)", m.Name.c_str(), m.MsgID, m.MsgID);
    fwriter.Append("#define %s_IDE (%uU)", m.Name.c_str(), m.IsExt);
    fwriter.Append("#define %s_DLC (%uU)", m.Name.c_str(), m.DLC);
    fwriter.Append("#define %s_CANID (%#xU)", m.Name.c_str(), m.MsgID);

    if (m.Cycle > 0)
    {
      fwriter.Append("#define %s_CYC (%dU)", m.Name.c_str(), m.Cycle);
    }

    size_t max_sig_name_len = 27;

    if (!m.frameNotEmpty)
    {
      // do nothing with empty frame, leave only id other relevant constants
      fwriter.Append();
      continue;
    }

    for (size_t signum = 0; signum < m.Signals.size(); signum++)
    {
      SignalDescriptor_t& s = m.Signals[signum];

      if (!s.IsSimpleSig)
      {
        if (passed_sigs.find(s.Name) == passed_sigs.end())
        {
          // print signal macroses only it was not printed before
          fwriter.Append(sigprt.PrintPhysicalToRaw(&s, fdesc->gen.DRVNAME));
          passed_sigs.insert(s.Name);
        }
      }

      if (s.Name.size() > max_sig_name_len)
      {
        max_sig_name_len = s.Name.size();
      }

      // For each signal in current message print value tables definitions
      if (s.ValDefs.vpairs.size() > 0)
      {
        fwriter.Append("\n// Value tables for @%s signal", s.Name.c_str());
        fwriter.Append();

        for (auto i = 0; i < s.ValDefs.vpairs.size(); i++)
        {
          // The value table definition consists of 'signal name + message name + value definition'
          // This provides reliable way of avoiding issues with same macros names
          std::string defname = StrPrint("%s_%s_%s", s.Name.c_str(), m.Name.c_str(), s.ValDefs.vpairs[i].first.c_str());

          // @ifndef guard for the case when different values of table have
          // the same name (it is valid for DBC file format)
          // For this case only one of same named values will be available as macro
          fwriter.Append("#ifndef %s", defname.c_str());

          fwriter.Append("#define %s_%s_%s (%d)",
            s.Name.c_str(), m.Name.c_str(), s.ValDefs.vpairs[i].first.c_str(),
            s.ValDefs.vpairs[i].second);

          fwriter.Append("#endif");
          fwriter.Append();
        }
      }
    }

    fwriter.Append();
    fwriter.Append("typedef struct");
    fwriter.Append("{");

    // Write section for bitfielded part
    fwriter.Append("#ifdef %s", fdesc->gen.usebits_def.c_str());
    fwriter.Append();

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
      fwriter.Append("#ifdef %s", fdesc->gen.useroll_def.c_str());
      fwriter.Append();
      WriteSigStructField(rollsig, true, max_sig_name_len);
      fwriter.Append("#endif // %s", fdesc->gen.useroll_def.c_str());
      fwriter.Append();
    }

    // Write clean part
    fwriter.Append("#else");
    fwriter.Append();

    for (size_t signum = 0; signum < m.Signals.size(); signum++)
    {
      SignalDescriptor_t& sig = m.Signals[signum];
      // Write clean signals
      WriteSigStructField(sig, false, max_sig_name_len);
    }

    if (m.RollSig != nullptr)
    {
      fwriter.Append("#ifdef %s", fdesc->gen.useroll_def.c_str());
      fwriter.Append();
      WriteSigStructField(rollsig, false, max_sig_name_len);
      fwriter.Append("#endif // %s", fdesc->gen.useroll_def.c_str());
      fwriter.Append();
    }

    fwriter.Append("#endif // %s", fdesc->gen.usebits_def.c_str());
    fwriter.Append();

    // start mon1 section
    fwriter.Append("#ifdef %s", fdesc->gen.usemon_def.c_str());
    fwriter.Append();
    fwriter.Append("  FrameMonitor_t mon1;");
    fwriter.Append();
    fwriter.Append("#endif // %s", fdesc->gen.usemon_def.c_str());
    fwriter.Append();
    fwriter.Append("} %s_t;", m.Name.c_str());
    fwriter.Append();
  }

  fwriter.Append("// Function signatures");
  fwriter.Append();

  for (size_t num = 0; num < sigprt.sigs_expr.size(); num++)
  {
    // write message typedef s and additional expressions
    MessageDescriptor_t& m = sigprt.sigs_expr[num]->msg;

    if (!m.frameNotEmpty)
    {
      // do nothing with empty frame
      continue;
    }

    fwriter.Append("uint32_t Unpack_%s_%s(%s_t* _m, const uint8_t* _d, uint8_t dlc_);",
      m.Name.c_str(), fdesc->gen.DrvName_orig.c_str(), m.Name.c_str());

    fwriter.Append("#ifdef %s", fdesc->gen.usesruct_def.c_str());

    fwriter.Append("uint32_t Pack_%s_%s(%s_t* _m, __CoderDbcCanFrame_t__* cframe);",
      m.Name.c_str(), fdesc->gen.DrvName_orig.c_str(), m.Name.c_str());

    fwriter.Append("#else");

    fwriter.Append("uint32_t Pack_%s_%s(%s_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide);",
      m.Name.c_str(), fdesc->gen.DrvName_orig.c_str(), m.Name.c_str());

    fwriter.Append("#endif // %s", fdesc->gen.usesruct_def.c_str());
    fwriter.Append();
  }

  fwriter.Append("#ifdef __cplusplus\n}\n#endif");

  // save fwrite cached text to file
  fwriter.Flush(fdesc->file.core_h.fpath);
}

void CiMainGenerator::Gen_MainSource()
{

  fwriter.AppendText(fdesc->gen.start_common_info);
  fwriter.AppendText(fdesc->gen.start_driver_info);
  // include main header file
  fwriter.Append("#include \"%s\"", fdesc->file.core_h.fname.c_str());
  fwriter.Append(2);

  fwriter.Append("// DBC file version");
  fwriter.Append("#if (%s != (%uU)) || (%s != (%uU))",
    fdesc->gen.verhigh_def.c_str(), fdesc->gen.hiver, fdesc->gen.verlow_def.c_str(), fdesc->gen.lowver);

  fwriter.Append("#error The %s dbc source files have different versions", fdesc->gen.DRVNAME.c_str());
  fwriter.Append("#endif");
  fwriter.Append();

  // put diagmonitor ifdef selection for including @drv-fmon header
  // with FMon_* signatures to call from unpack function
  fwriter.Append("#ifdef %s", fdesc->gen.usemon_def.c_str());

  fwriter.Append(
    "// Function prototypes to be called each time CAN frame is unpacked\n"
    "// FMon function may detect RC, CRC or DLC violation\n");

  fwriter.Append("#include \"%s-fmon.h\"", fdesc->gen.drvname.c_str());
  fwriter.Append();

  fwriter.Append("#endif // %s", fdesc->gen.usemon_def.c_str());
  fwriter.Append("");
  fwriter.Append("// This macro guard for the case when you need to enable");
  fwriter.Append("// using diag monitors but there is no necessity in proper");
  fwriter.Append("// SysTick provider. For providing one you need define macro");
  fwriter.Append("// before this line - in dbccodeconf.h");
  fwriter.Append("");
  fwriter.Append("#ifndef GetSystemTick");
  fwriter.Append("#define GetSystemTick() (0u)");
  fwriter.Append("#endif");
  fwriter.Append("");
  fwriter.Append("// This macro guard is for the case when you want to build");
  fwriter.Append("// app with enabled optoin auto CSM, but don't yet have");
  fwriter.Append("// proper getframehash implementation");
  fwriter.Append("");
  fwriter.Append("#ifndef GetFrameHash");
  fwriter.Append("#define GetFrameHash(a,b,c,d,e) (0u)");
  fwriter.Append("#endif");
  fwriter.Append();

  fwriter.Append(extend_func_body, ext_sig_func_name), 1;

  // for each message 3 functions must be defined - 1 unpack function,
  // 2: pack with raw signature
  // 3: pack with canstruct
  for (size_t num = 0; num < sigprt.sigs_expr.size(); num++)
  {
    // write message typedef s and additional expressions
    MessageDescriptor_t& m = sigprt.sigs_expr[num]->msg;

    if (!m.frameNotEmpty)
    {
      // do nothing, no pack and unpack functions for empty frames
      continue;
    }

    // first function
    fwriter.Append("uint32_t Unpack_%s_%s(%s_t* _m, const uint8_t* _d, uint8_t dlc_)\n{",
      m.Name.c_str(), fdesc->gen.DrvName_orig.c_str(), m.Name.c_str());

    // put dirt trick to avoid warning about unusing parameter
    // (dlc) when monitora are disabled. trick is better than
    // selection different signatures because of external API consistency
    fwriter.Append("  (void)dlc_;");

    WriteUnpackBody(sigprt.sigs_expr[num]);

    fwriter.Append("}");
    fwriter.Append();

    // next one is the pack function for using with CANFrame struct
    fwriter.Append("#ifdef %s", fdesc->gen.usesruct_def.c_str());
    fwriter.Append();

    // second function
    fwriter.Append("uint32_t Pack_%s_%s(%s_t* _m, __CoderDbcCanFrame_t__* cframe)",
      m.Name.c_str(), fdesc->gen.DrvName_orig.c_str(), m.Name.c_str());

    WritePackStructBody(sigprt.sigs_expr[num]);

    fwriter.Append("#else");
    fwriter.Append();

    // third function
    fwriter.Append("uint32_t Pack_%s_%s(%s_t* _m, uint8_t* _d, uint8_t* _len, uint8_t* _ide)",
      m.Name.c_str(), fdesc->gen.DrvName_orig.c_str(), m.Name.c_str());

    WritePackArrayBody(sigprt.sigs_expr[num]);

    fwriter.Append("#endif // %s", fdesc->gen.usesruct_def.c_str());
    fwriter.Append();
  }

  fwriter.Flush(fdesc->file.core_c.fpath);
}

void CiMainGenerator::Gen_ConfigHeader()
{
  fwriter.AppendText(fdesc->gen.start_common_info);
  fwriter.AppendText(fdesc->gen.start_driver_info);

  ConfigGenerator confgen;
  confgen.FillHeader(fwriter, fdesc->gen);

  fwriter.Flush(fdesc->file.confdir + '/' + fdesc->gen.drvname + "-config.h");
}

void CiMainGenerator::Gen_FMonHeader()
{
  MonGenerator mongen;
  mongen.FillHeader(fwriter, sigprt.sigs_expr, *fdesc);
  fwriter.Flush(fdesc->file.fmon_h.fpath);
}

void CiMainGenerator::Gen_FMonSource()
{
  MonGenerator mongen;
  mongen.FillSource(fwriter, sigprt.sigs_expr, *fdesc);
  fwriter.Flush(fdesc->file.fmon_c.fpath);
}

void CiMainGenerator::Gen_CanMonUtil()
{
  fwriter.AppendText(fdesc->gen.start_common_info);
  fwriter.Append("#pragma once");
  fwriter.Append("");
  fwriter.Append("#include <stdint.h>");
  fwriter.Append("");
  fwriter.Append("#ifdef __cplusplus");
  fwriter.Append("extern \"C\" {");
  fwriter.Append("#endif");
  fwriter.Append("");
  fwriter.Append("// declare here all availible checksum algorithms");
  fwriter.Append("typedef enum");
  fwriter.Append("{");
  fwriter.Append("  // XOR8 = 0,");
  fwriter.Append("  // XOR4 = 1,");
  fwriter.Append("  // etc");
  fwriter.Append("");
  fwriter.Append("  // it is up to user to have or to skip final enum value - @CRC_ALG_COUNT");
  fwriter.Append("  CRC_ALG_COUNT");
  fwriter.Append("} DbcCanCrcMethods;");
  fwriter.Append("");
  fwriter.Append("typedef struct");
  fwriter.Append("{");
  fwriter.Append("  // @last_cycle keeps tick-value when last frame was received");
  fwriter.Append("  uint32_t last_cycle;");
  fwriter.Append("");
  fwriter.Append("  // @timeout_cycle keeps maximum timeout for frame, user responsibility");
  fwriter.Append("  // to init this field and use it in missing frame monitoring function");
  fwriter.Append("  uint32_t timeout_cycle;");
  fwriter.Append("");
  fwriter.Append("  // @frame_cnt keeps count of all the received frames");
  fwriter.Append("  uint32_t frame_cnt;");
  fwriter.Append("");
  fwriter.Append("  // setting up @roll_error bit indicates roll counting fail.");
  fwriter.Append("  // Bit is not clearing automatically!");
  fwriter.Append("  uint32_t roll_error : 1;");
  fwriter.Append("");
  fwriter.Append("  // setting up @checksum_error bit indicates checksum checking failure.");
  fwriter.Append("  // Bit is not clearing automatically!");
  fwriter.Append("  uint32_t csm_error : 1;");
  fwriter.Append("");
  fwriter.Append("  // setting up @cycle_error bit indicates that time was overrunned.");
  fwriter.Append("  // Bit is not clearing automatically!");
  fwriter.Append("  uint32_t cycle_error : 1;");
  fwriter.Append("");
  fwriter.Append("  // setting up @dlc_error bit indicates that the actual length of");
  fwriter.Append("  // CAN frame is less then defined by CAN matrix!");
  fwriter.Append("  uint32_t dlc_error : 1;");
  fwriter.Append("");
  fwriter.Append("} FrameMonitor_t;");
  fwriter.Append("");
  fwriter.Append("#ifdef __cplusplus");
  fwriter.Append("}");
  fwriter.Append("#endif");
  fwriter.Append("");

  fwriter.Flush(fdesc->file.incdir + '/' + "canmonitorutil.h");
}

void CiMainGenerator::Gen_DbcCodeConf()
{
  fwriter.AppendText(fdesc->gen.start_common_info);
  fwriter.Append("#pragma once");
  fwriter.Append("");
  fwriter.Append("#include <stdint.h>");
  fwriter.Append("");
  fwriter.Append("// when USE_SIGFLOAT enabed the sigfloat_t must be defined");
  fwriter.Append("// typedef double sigfloat_t;");
  fwriter.Append("");
  fwriter.Append("// when USE_CANSTRUCT enabled __CoderDbcCanFrame_t__ must be defined");
  fwriter.Append("// #include \"{header_with_can_struct}\"");
  fwriter.Append("// typedef {can_struct} __CoderDbcCanFrame_t__;");
  fwriter.Append("");
  fwriter.Append("// if you need to allocate rx and tx messages structs put the allocation macro here");
  fwriter.Append("// #define __DEF_{your_driver_name}__");
  fwriter.Append("");

  fwriter.Append("// defualt @__ext_sig__ help types definition");
  fwriter.Append("");
  fwriter.Append("typedef uint32_t ubitext_t;");
  fwriter.Append("typedef int32_t bitext_t;");
  fwriter.Append("");
  fwriter.Append("// To provide a way to make missing control correctly you");
  fwriter.Append("// have to define macro @GetSystemTick() which has to");
  fwriter.Append("// return kind of tick counter (e.g. 1 ms ticker)");
  fwriter.Append("");
  fwriter.Append("// #define GetSystemTick() __get__tick__()");
  fwriter.Append("");
  fwriter.Append("// To provide a way to calculate hash (crc) for CAN");
  fwriter.Append("// frame's data field you have to define macro @GetFrameHash");
  fwriter.Append("");
  fwriter.Append("// #define GetFrameHash(a,b,c,d,e) __get_hash__(a,b,c,d,e)");
  fwriter.Append("");

  fwriter.Flush(fdesc->file.confdir + '/' + "dbccodeconf.h");
}

void CiMainGenerator::WriteSigStructField(const SignalDescriptor_t& sig, bool bits, size_t padwidth)
{
  if (sig.CommentText.size() > 0)
  {
    fwriter.Append("  // " + std::regex_replace(sig.CommentText, std::regex("\n"), "\n  // "));
  }

  if (sig.ValueText.size() > 0)
  {
    fwriter.Append("  // " + std::regex_replace(sig.ValueText, std::regex("\n"), "\n  // "));
  }

  if (sig.Multiplex == MultiplexType::kMulValue)
  {
    fwriter.Append("  // multiplex variable");
  }
  else if (sig.Multiplex == MultiplexType::kMaster)
  {
    fwriter.Append("  // MULTIPLEX master signal");
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

  fwriter.AppendText(dtype);

  pad = " // ";
  pad += (sig.Signed) ? " [-]" : "    ";

  fwriter.AppendText(pad);

  fwriter.AppendText(" Bits=%2d", sig.LengthBit);

  size_t offset = 0;
  std::string infocmnt{};

  if (sig.IsDoubleSig)
  {
    if (sig.Offset != 0)
    {
      infocmnt = IndentedString(offset, infocmnt);
      offset += 27;
      infocmnt += " Offset= " + prt_double(sig.Offset, 9);
    }

    if (sig.Factor != 1)
    {
      infocmnt = IndentedString(offset, infocmnt);
      offset += 24;
      infocmnt += " Factor= " + prt_double(sig.Factor, 9);
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

  fwriter.AppendText(infocmnt);

  fwriter.Append("");
  fwriter.Append();

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
    fwriter.Append("#ifdef %s", fdesc->gen.usesigfloat_def.c_str());

    if (sig.IsDoubleSig)
    {
      fwriter.Append("  sigfloat_t %s;", sig.NameFloat.c_str());
    }
    else
    {
      fwriter.Append("  %s %s;", PrintType((int)sig.TypePhys).c_str(), sig.NameFloat.c_str());
    }

    fwriter.Append("#endif // %s", fdesc->gen.usesigfloat_def.c_str());
    fwriter.Append();
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
      fwriter.Append("  _m->%s = (%s) %s(( %s ), %d);",
        sname, PrintType((int)sgs->msg.Signals[num].TypeRo).c_str(),
        ext_sig_func_name, expr.c_str(), (int32_t)sgs->msg.Signals[num].LengthBit);
    }
    else
    {
      fwriter.Append("  _m->%s = (%s) ( %s );", sname,
        PrintType((int)sgs->msg.Signals[num].TypeRo).c_str(), expr.c_str());
    }

    // print sigfloat conversion
    if (!sgs->msg.Signals[num].IsSimpleSig)
    {
      fwriter.Append("#ifdef %s", fdesc->gen.usesigfloat_def.c_str());

      if (sgs->msg.Signals[num].IsDoubleSig)
      {
        // for double signals (sigfloat_t) type cast
        fwriter.Append("  _m->%s = (sigfloat_t)(%s_%s_fromS(_m->%s));",
          sgs->msg.Signals[num].NameFloat.c_str(), fdesc->gen.DRVNAME.c_str(), sname, sname);
      }
      else
      {
        fwriter.Append("  _m->%s = (%s) %s_%s_fromS(_m->%s);",
          sgs->msg.Signals[num].NameFloat.c_str(),
          PrintType((int)sgs->msg.Signals[num].TypePhys).c_str(),
          fdesc->gen.DRVNAME.c_str(), sname, sname);
      }

      fwriter.Append("#endif // %s", fdesc->gen.usesigfloat_def.c_str());
      fwriter.Append();
    }

    else if (num + 1 == sgs->to_signals.size())
    {
      // last signal without phys part, put \n manually
      fwriter.Append("");
    }
  }

  fwriter.Append("#ifdef %s", fdesc->gen.usemon_def.c_str());
  fwriter.Append("  _m->mon1.dlc_error = (dlc_ < %s_DLC);", sgs->msg.Name.c_str());
  fwriter.Append("  _m->mon1.last_cycle = GetSystemTick();");
  fwriter.Append("  _m->mon1.frame_cnt++;");
  fwriter.Append();

  if (sgs->msg.RollSig != nullptr)
  {
    // Put rolling monitor here
    fwriter.Append("#ifdef %s", fdesc->gen.useroll_def.c_str());
    fwriter.Append("  _m->mon1.roll_error = (_m->%s != _m->%s_expt);",
      sgs->msg.RollSig->Name.c_str(), sgs->msg.RollSig->Name.c_str());
    fwriter.Append("  _m->%s_expt = (_m->%s + 1) & (0x%02XU);", sgs->msg.RollSig->Name.c_str(),
      sgs->msg.RollSig->Name.c_str(), (1 << sgs->msg.RollSig->LengthBit) - 1);
    // Put rolling monitor here
    fwriter.Append("#endif // %s", fdesc->gen.useroll_def.c_str());
    fwriter.Append();
  }

  if (sgs->msg.CsmSig != nullptr)
  {
    // Put checksum check function call here
    fwriter.Append("#ifdef %s", fdesc->gen.usecsm_def.c_str());
    fwriter.Append("  _m->mon1.csm_error = (((uint8_t)GetFrameHash(_d, %s_DLC, %s_CANID, %s, %d)) != (_m->%s));",
      sgs->msg.Name.c_str(), sgs->msg.Name.c_str(), sgs->msg.CsmMethod.c_str(),
      sgs->msg.CsmOp, sgs->msg.CsmSig->Name.c_str());
    fwriter.Append("#endif // %s", fdesc->gen.usecsm_def.c_str());
    fwriter.Append();
  }

  auto Fmon_func = "FMon_" + sgs->msg.Name + "_" + fdesc->gen.drvname;

  fwriter.Append("  %s(&_m->mon1, %s_CANID);", Fmon_func.c_str(), sgs->msg.Name.c_str());

  fwriter.Append("#endif // %s", fdesc->gen.usemon_def.c_str());
  fwriter.Append();

  fwriter.Append("  return %s_CANID;", sgs->msg.Name.c_str());
}

void CiMainGenerator::WritePackStructBody(const CiExpr_t* sgs)
{
  fwriter.Append("{");
  PrintPackCommonText("cframe->Data", sgs);
  fwriter.Append("  cframe->MsgId = (uint32_t) %s_CANID;", sgs->msg.Name.c_str());
  fwriter.Append("  cframe->DLC = (uint8_t) %s_DLC;", sgs->msg.Name.c_str());
  fwriter.Append("  cframe->IDE = (uint8_t) %s_IDE;", sgs->msg.Name.c_str());
  fwriter.Append("  return %s_CANID;", sgs->msg.Name.c_str());
  fwriter.Append("}");
  fwriter.Append();
}

void CiMainGenerator::WritePackArrayBody(const CiExpr_t* sgs)
{
  fwriter.Append("{");
  PrintPackCommonText("_d", sgs);
  fwriter.Append("  *_len = (uint8_t) %s_DLC;", sgs->msg.Name.c_str());
  fwriter.Append("  *_ide = (uint8_t) %s_IDE;", sgs->msg.Name.c_str());
  fwriter.Append("  return %s_CANID;", sgs->msg.Name.c_str());
  fwriter.Append("}");
  fwriter.Append();
}

void CiMainGenerator::PrintPackCommonText(const std::string& arrtxt, const CiExpr_t* sgs)
{
  // this function will print body of packing function

  // print array content clearing loop
  fwriter.Append("  uint8_t i; for (i = 0u; i < %s(%s_DLC); %s[i++] = %s);",
    prt_dlcValidateMacroName.c_str(),
    sgs->msg.Name.c_str(), arrtxt.c_str(),
    prt_initialDataByteValueName.c_str());
  fwriter.Append();

  if (sgs->msg.RollSig != nullptr)
  {
    fwriter.Append("#ifdef %s", fdesc->gen.useroll_def.c_str());
    fwriter.Append("  _m->%s = (_m->%s + 1) & (0x%02XU);", sgs->msg.RollSig->Name.c_str(),
      sgs->msg.RollSig->Name.c_str(), (1 << sgs->msg.RollSig->LengthBit) - 1);
    fwriter.Append("#endif // %s", fdesc->gen.useroll_def.c_str());
    fwriter.Append();
  }

  if (sgs->msg.CsmSig != nullptr)
  {
    // code for clearing checksum
    fwriter.Append("#ifdef %s", fdesc->gen.usecsm_def.c_str());
    fwriter.Append("  _m->%s = (%s) 0;", sgs->msg.CsmSig->Name.c_str(), PrintType((int)sgs->msg.CsmSig->TypeRo).c_str());
    fwriter.Append("#endif // %s", fdesc->gen.usecsm_def.c_str());
    fwriter.Append();
  }

  if (sgs->msg.hasPhys)
  {
    // first step is to put code for sigfloat conversion, before
    // sigint packing to bytes.
    fwriter.Append("#ifdef %s", fdesc->gen.usesigfloat_def.c_str());

    for (size_t n = 0; n < sgs->to_signals.size(); n++)
    {
      if (sgs->msg.Signals[n].IsSimpleSig == false)
      {
        // print toS from *_phys to original named sigint (integer duplicate of signal)
        fwriter.Append("  _m->%s = (%s) %s_%s_toS(_m->%s);",
          sgs->msg.Signals[n].Name.c_str(),
          PrintType((int) sgs->msg.Signals[n].TypeRo).c_str(),
          fdesc->gen.DRVNAME.c_str(),
          sgs->msg.Signals[n].Name.c_str(), sgs->msg.Signals[n].NameFloat.c_str());
      }
    }

    fwriter.Append("#endif // %s", fdesc->gen.usesigfloat_def.c_str());
    fwriter.Append();
  }

  // Find the master multiplex signal (if any)
  const SignalDescriptor_t* masterSignal = nullptr;
  for (const auto& sig : sgs->msg.Signals)
  {
    if (sig.Multiplex == MultiplexType::kMaster)
    {
      masterSignal = &sig;
      break;
    }
  }

  // Generate packing code for each byte in the CAN message
  for (size_t i = 0; i < sgs->to_bytes.size(); i++)
  {

    if (masterSignal)
    {
      bool first = true;
      // Handle the case where only a master multiplexor signal exists and there are no other kMulValue signal types in the CAN msg.
      // There may or may not be other normal signals in the CAN msg.
      if (sgs->mux_values.size() == 0)
      {
        // Filter out any empty bytes in the CAN msg. 
        if ( !sgs->to_bytes[i].empty() )
        {
          fwriter.Append("  %s[%d] |= (uint8_t) ( %s );", arrtxt.c_str(), i, sgs->to_bytes[i].c_str());
        }
      } 
      else 
      {
        // handle the case where there is a mux master and kMulValue (and other normal signals) in the CAN msg.
        for (size_t mux_idx = 0; mux_idx < sgs->mux_values.size(); ++mux_idx)
        {
          int mux_val = sgs->mux_values[mux_idx];
          if (sgs->to_bytes_mux[i].size() > mux_idx && !sgs->to_bytes_mux[i][mux_idx].empty())
          {
            if (first)
            {
              fwriter.Append("  if (_m->%s == %d) {", masterSignal->Name.c_str(), mux_val);
              first = false;
            }
            else
            {
              fwriter.Append("  else if (_m->%s == %d) {", masterSignal->Name.c_str(), mux_val);
            }
            fwriter.Append("    %s[%d] |= (uint8_t) ( %s );", arrtxt.c_str(), i, sgs->to_bytes_mux[i][mux_idx].c_str());
            fwriter.Append("  }");
          }
        }
        // Add the final else block for the case where no expected multiplex value matches. Just encode all signals in the byte.
        if (!first)
        {
            fwriter.Append("  else {");
            if ( !sgs->to_bytes[i].empty() )
            {
                fwriter.Append("    %s[%d] |= (uint8_t) ( %s );", arrtxt.c_str(), i, sgs->to_bytes[i].c_str());
            }
            fwriter.Append("  }");
        }
      }
    }
    else 
    {
      // Handle for when there is no master multiplexor signal. Just pack the signal values from all signals making up this byte.
      if ( !sgs->to_bytes[i].empty() )
      {
        fwriter.Append("  %s[%d] |= (uint8_t) ( %s );", arrtxt.c_str(), i, sgs->to_bytes[i].c_str());
      }
    }
  }

  fwriter.Append("");

  if (sgs->msg.CsmSig != nullptr)
  {
    // code for getting checksum value and putting it in array
    fwriter.Append("#ifdef %s", fdesc->gen.usecsm_def.c_str());

    fwriter.Append("  _m->%s = ((uint8_t)GetFrameHash(%s, %s_DLC, %s_CANID, %s, %d));",
      sgs->msg.CsmSig->Name.c_str(), arrtxt.c_str(), sgs->msg.Name.c_str(),
      sgs->msg.Name.c_str(), sgs->msg.CsmMethod.c_str(), sgs->msg.CsmOp);

    fwriter.Append("  %s[%d] |= (uint8_t) ( %s );", arrtxt.c_str(), sgs->msg.CsmByteNum, sgs->msg.CsmToByteExpr.c_str());

    fwriter.Append("#endif // %s", fdesc->gen.usecsm_def.c_str());
    fwriter.Append();
  }
}