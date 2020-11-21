#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include "types/message.h"
#include "dbclineparser.h"

using namespace std;

class DbcScanner {
 public:
  DbcScanner();
  ~DbcScanner();

  std::vector<MessageDescriptor_t*> msgs;

  // Trim makes dbc source data analyze and returns count of
  // found CAN messages.
  int32_t TrimDbcText(istream& instrm);

 private:

  void ParseMessageInfo(istream& instrm);
  void ParseOtherInfo(istream& instrm);

  void AddMessage(MessageDescriptor_t* message);

  void SetDefualtMessage(MessageDescriptor_t* message);

 private:

  DbcLineParser lparser;

};
