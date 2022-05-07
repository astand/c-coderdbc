#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include "../types/message.h"
#include "dbclineparser.h"

using namespace std;

class DbcScanner {
 public:
  DbcScanner();
  ~DbcScanner();

  DbcMessageList_t dblist;

  // Trim makes dbc source data analyze and returns count of
  // found CAN messages.
  int32_t TrimDbcText(istream& instrm);

 private:

  void ParseMessageInfo(istream& instrm);
  void ParseOtherInfo(istream& instrm);
  void AddMessage(MessageDescriptor_t* message);
  void SetDefualtMessage(MessageDescriptor_t* message);
  void FindVersion(const std::string& instr);

 private:

  DbcLineParser lparser;

  // this variable is used for gathering value table signal's information
  std::pair<std::string, uint32_t> vpairs;

};
