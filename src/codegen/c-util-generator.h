#pragma once

#include <memory>
#include "types/message.h"
#include "fs-creator.h"
#include "filewriter.h"
#include "conditional-tree.h"

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
  void Generate(DbcMessageList_t& dlist, const AppSettings_t& fsd,
    const MsgsClassification& groups, const std::string& drvname);

 private:
  void PrintHeader();
  void PrintSource();
  ConditionalTree_t* FillTreeLevel(std::vector<MessageDescriptor_t*>& msgs,
    int32_t l, int32_t h, bool started = false);

 private:

  std::vector<MessageDescriptor_t*> tx;
  std::vector<MessageDescriptor_t*> rx;
  std::vector<MessageDescriptor_t*> both;

  // to file writer
  std::unique_ptr<FileWriter> tof;
  std::unique_ptr<ConditionalTree> condtree;

  std::string code_drvname;
  std::string file_drvname;

  const FsDescriptor_t* fdesc;
  const GenDescriptor_t* gdesc;
  const DbcMessageList_t* p_dlist;

  bool treestarted;
};
