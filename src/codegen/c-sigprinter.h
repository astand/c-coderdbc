#pragma once

#include "../types/c-expr.h"

class CSigPrinter {
 public:
  CSigPrinter();
  ~CSigPrinter();

  void LoadMessage(const MessageDescriptor_t& message);
  void LoadMessages(const std::vector<MessageDescriptor_t*> message);

  std::string PrintPhysicalToRaw(const SignalDescriptor_t* msg, const std::string& drvname);

 public:
  std::vector<CiExpr_t*> sigs_expr;

 private:
  int32_t BuildCConvertExprs(CiExpr_t* msg);

  std::string PrintSignalExpr(const SignalDescriptor_t* sig, std::vector<std::string>& to_bytes);

  void AppendToByteLine(std::string& expr, std::string str);

};
