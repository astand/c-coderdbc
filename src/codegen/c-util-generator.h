#pragma once

#include "types/message.h"
#include "fs-creator.h"

class CiUtilGenerator {
 public:
  CiUtilGenerator();

  void Clear();

  // msgs - the whole list of messages in CAN matrix
  // fsd - file and names descriptor
  // groups - collection of msg IDs assigned to available groups (rx, tx, both)

  // the output of this function source files which contain:
  // - global TX and RX typedefs message struct
  // - function to Unpack incoming frame to dedicated RX message struct field
  // - optional (through define in global "dbccodeconf.h") variable allocation in source files
  //
  void Generate(std::vector<MessageDescriptor_t*>& msgs, const FsDescriptor_t& fsd, const MsgsClassification& groups);

 private:
  void PrintHeader();
  void PrintSource();

 private:

  std::vector<MessageDescriptor_t*> tx;
  std::vector<MessageDescriptor_t*> rx;
  std::vector<MessageDescriptor_t*> both;

  const FsDescriptor_t* fdesc;
};
