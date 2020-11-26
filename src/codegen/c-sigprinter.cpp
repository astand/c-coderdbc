#include "c-sigprinter.h"


CSigPrinter::CSigPrinter()
{
}

CSigPrinter::~CSigPrinter()
{
}

void CSigPrinter::AddMessage(const std::vector<MessageDescriptor_t*> message)
{
  sigs_expr.clear();

  for (size_t i = 0; i < message.size(); i++)
  {
    AddMessage(*(message[i]));
  }
}

void CSigPrinter::AddMessage(const MessageDescriptor_t& message)
{
  CiExpr_t* nexpr = new CiExpr_t;

  nexpr->msg = message;

  // do for this new expr to_byte and to_field expression building,
  // add them to dedicated members, set signal stdint type
  // and push it to vector

  sigs_expr.push_back(nexpr);
}

std::string CSigPrinter::GetSignalType(const SignalDescriptor_t& signal)
{
  std::string ret = "";

  return ret;
}
