#include <stdlib.h>
#include <memory>
#include "c-sigprinter.h"
#include "helpers/formatter.h"

// work buffer for all snprintf operations
static const size_t WBUFF_LEN = 2048;
static char workbuff[WBUFF_LEN] = { 0 };

// additional templates for expression generation
static std::string msk[] = { "0", "0x01U", "0x03U", "0x07U", "0x0FU", "0x1FU", "0x3FU", "0x7FU", "0xFFU" };

static inline int32_t ShiftByte(const SignalDescriptor_t* sig, int32_t bn)
{
  return (sig->Order == BitLayout::kIntel) ? (bn - 1) : (bn + 1);
}

CSigPrinter::CSigPrinter()
{
  sigs_expr.clear();
}

CSigPrinter::~CSigPrinter()
{
  sigs_expr.clear();
}

void CSigPrinter::LoadMessages(const std::vector<MessageDescriptor_t*> message)
{
  sigs_expr.clear();

  for (auto it = message.cbegin(); it != message.cend(); ++it)
  {
    LoadMessage(*(*it));
  }
}

void CSigPrinter::LoadMessage(const MessageDescriptor_t& message)
{
  CiExpr_t* nexpr = new CiExpr_t;

  nexpr->msg = message;

  // do for this new expr to_byte and to_field expression building,
  // add them to dedicated members, set signal stdint type
  // and push it to vector

  BuildCConvertExprs(nexpr);

  sigs_expr.push_back(nexpr);
}

std::string CSigPrinter::PrintPhysicalToRaw(const SignalDescriptor_t* sig, const std::string& drvname)
{
  std::string retstr = "";

  retstr = StrPrint("// signal: @%s\n", sig->Name.c_str());

  if (sig->IsDoubleSig)
  {
    retstr += StrPrint("#define %s_%s_CovFactor (%f)\n", drvname.c_str(), sig->Name.c_str(), sig->Factor);
  }
  else
  {
    retstr += StrPrint("#define %s_%s_CovFactor (%d)\n", drvname.c_str(), sig->Name.c_str(), (int32_t)sig->Factor);
  }

  retstr += StrPrint("#define %s_%s_toS(x) ( (%s) ", drvname.c_str(), sig->Name.c_str(),
      PrintType((uint8_t)sig->TypeRo).c_str());

  if (sig->IsDoubleSig)
  {
    retstr += StrPrint("(((x) - (%f)) / (%f)) )\n", sig->Offset, sig->Factor);
  }
  else
  {
    if (sig->Offset == 0)
    {
      // only factor
      retstr += StrPrint("((x) / (%d)) )\n", (int32_t)sig->Factor);
    }
    else if (sig->Factor == 1)
    {
      // only offset
      retstr += StrPrint("((x) - (%d)) )\n", (int32_t)sig->Offset);
    }
    else
    {
      // full expression
      retstr += StrPrint("(((x) - (%d)) / (%d)) )\n", (int32_t)sig->Offset, (int32_t)sig->Factor);
    }
  }

  retstr += StrPrint("#define %s_%s_fromS(x) ( ", drvname.c_str(), sig->Name.c_str());

  if (sig->IsDoubleSig)
  {
    retstr += StrPrint("(((x) * (%f)) + (%f)) )\n", sig->Factor, sig->Offset);
  }
  else
  {
    if (sig->Offset == 0)
    {
      // only factor
      retstr += StrPrint("((x) * (%d)) )\n", (int32_t)sig->Factor);
    }
    else if (sig->Factor == 1)
    {
      // only offset
      retstr += StrPrint("((x) + (%d)) )\n", (int32_t)sig->Offset);
    }
    else
    {
      // full expression
      retstr += StrPrint("(((x) * (%d)) + (%d)) )\n", (int32_t)sig->Factor, (int32_t)sig->Offset);
    }
  }

  return retstr;
}

int32_t CSigPrinter::BuildCConvertExprs(CiExpr_t* msgprinter)
{
  int32_t ret = 0;
  std::string tmpstr;

  msgprinter->to_bytes.clear();
  msgprinter->to_signals.clear();
  msgprinter->to_bytes.resize(msgprinter->msg.DLC);

  // for each signal specific to_signal expression must be defined,
  // and during all signals processing, for each byte to_byte expression
  // must be collected

  for (size_t i = 0; i < msgprinter->msg.Signals.size(); i++)
  {
    // there are two main goal of this code:
    // 1 - to generate bytes to signal C-expression, (_d - name of array).
    // For each signal one or more bytes can be referenced. It's generated
    // once on each function call for each signal
    //
    // 2 - to generate signals to each byte expression, (_m - name of struct with
    // signals). For each byte a 8 signals can be referenced. It's generated
    // consequently signal after signal (by adding chunks of expressions to @to_bytes
    // collection)
    //
    // signal expression is saved to vector @to_signals, which id must be
    // fully correlated to id of target signal. the final size of
    // @to_signals vector must be equal to size of Signals vector
    //
    // bytes expression is saved to vector @to_bytes, where id is the
    // byte number in frame payload (i.e. to_bytes.size() == frame.DLC)
    msgprinter->to_signals.push_back(PrintSignalExpr(&msgprinter->msg.Signals[i], msgprinter->to_bytes));
  }

  if (msgprinter->msg.CsmSig != nullptr)
  {
    std::vector<std::string> v(8);

    PrintSignalExpr(msgprinter->msg.CsmSig, v);

    for (uint8_t i = 0; i < v.size() && i < 8; i++)
    {
      if (v[i].size() > 0)
      {
        msgprinter->msg.CsmToByteExpr = v[i];
        msgprinter->msg.CsmByteNum = i;
        break;
      }
    }
  }

  return ret;
}

std::string CSigPrinter::PrintSignalExpr(const SignalDescriptor_t* sig, std::vector<std::string>& to_bytes)
{
  // value for collecting expression (to_signal)
  std::string tosigexpr;

  if (to_bytes.size() == 0)
  {
    // return empty line is bytes count somehow equals 0
    return "Error in DBC file !!!! Dlc of this message must be greater.";
  }

  uint16_t startb = (uint16_t)((sig->Order == BitLayout::kIntel) ?
      (sig->StartBit + (sig->LengthBit - 1)) : (sig->StartBit));

  if (startb > 63)
  {
    startb = 63;
  }

  uint32_t bn = (startb / 8);

  if (to_bytes.size() <= bn)
  {
    // DLC from message doesn't fit to signal layout
    // make code uncomplilable
    to_bytes[0] = "Error in DBC file !!!! Dlc of this message must be greater.";
    return to_bytes[0];
  }

  // set valid to_byte prefix
  int32_t bbc = (startb % 8) + 1;
  int32_t slen = sig->LengthBit;

  if (bbc > slen)
  {
    snprintf(workbuff, WBUFF_LEN, "((_d[%d] >> %d) & (%s))", bn, bbc - slen, msk[slen].c_str());
    tosigexpr += workbuff;

    snprintf(workbuff, WBUFF_LEN, "((_m->%s & (%s)) << %d)", sig->Name.c_str(), msk[slen].c_str(), bbc - slen);
    AppendToByteLine(to_bytes[bn], workbuff);
  }
  else if (bbc == slen)
  {
    // no rolling bits
    snprintf(workbuff, WBUFF_LEN, "(_d[%d] & (%s))", bn, msk[slen].c_str());
    tosigexpr += workbuff;

    snprintf(workbuff, WBUFF_LEN, "(_m->%s & (%s))", sig->Name.c_str(), msk[slen].c_str());
    AppendToByteLine(to_bytes[bn], workbuff);
  }
  else
  {
    std::string t64 = "";
    slen -= bbc;

    if (slen > 31)
    {
      t64 = "(uint64_t)";
    }

    snprintf(workbuff, WBUFF_LEN, "(%s(_d[%d] & (%s)) << %d)", t64.c_str(), bn, msk[bbc].c_str(), slen);
    tosigexpr += workbuff;

    snprintf(workbuff, WBUFF_LEN, "((_m->%s >> %d) & (%s))", sig->Name.c_str(), slen, msk[bbc].c_str());
    AppendToByteLine(to_bytes[bn], workbuff);

    while ((slen - 8) >= 0)
    {
      t64.clear();

      slen -= 8;

      bn = ShiftByte(sig, bn);

      if (to_bytes.size() < bn)
      {
        // DLC from message doesn't fit to signal layout
        // make code uncomplilable
        to_bytes[0] = "Error in DBC file !!!! Dlc of this message must be greater.";
        return to_bytes[0];
      }

      tosigexpr += " | ";

      if (slen == 0)
      {
        // last byte is aligned
        snprintf(workbuff, WBUFF_LEN, "(_d[%d] & (%s))", bn, msk[8].c_str());
        tosigexpr += workbuff;

        snprintf(workbuff, WBUFF_LEN, "(_m->%s & (%s))", sig->Name.c_str(), msk[8].c_str());
        AppendToByteLine(to_bytes[bn], workbuff);

      }
      else
      {
        if (slen > 31)
        {
          t64 = "(uint64_t)";
        }

        snprintf(workbuff, WBUFF_LEN, "(%s(_d[%d] & (%s)) << %d)", t64.c_str(), bn, msk[8].c_str(), slen);
        tosigexpr += workbuff;

        snprintf(workbuff, WBUFF_LEN, "((_m->%s >> %d) & (%s))", sig->Name.c_str(), slen, msk[8].c_str());
        AppendToByteLine(to_bytes[bn], workbuff);
      }
    }

    if (slen > 0)
    {
      bn = ShiftByte(sig, bn);

      snprintf(workbuff, WBUFF_LEN, " | ((_d[%d] >> %d) & (%s))", bn, 8 - slen, msk[slen].c_str());
      tosigexpr += workbuff;

      snprintf(workbuff, WBUFF_LEN, "((_m->%s & (%s)) << %d)", sig->Name.c_str(), msk[slen].c_str(),
        8 - slen);
      AppendToByteLine(to_bytes[bn], workbuff);
    }
  }

  return tosigexpr;
}

void CSigPrinter::AppendToByteLine(std::string& expr, std::string str)
{
  if (expr.size() > 0)
  {
    // Not first appendingF
    expr += " | " + str;
  }
  else
  {
    // First appending
    expr = str;
  }
}
