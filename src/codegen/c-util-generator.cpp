#include "c-util-generator.h"
#include "helpers/formatter.h"
#include <algorithm>
#include <regex>

static const std::string openguard = "#ifdef __cplusplus\n\
extern \"C\" {\n\
#endif";

static const std::string closeguard = "#ifdef __cplusplus\n\
}\n\
#endif";

CiUtilGenerator::CiUtilGenerator()
{
  Clear();
}

void CiUtilGenerator::Clear()
{
  // clear all groups of messages
  tx.clear();
  rx.clear();
  both.clear();
}


void CiUtilGenerator::Generate(DbcMessageList_t& dlist, const AppSettings_t& fsd,
  const MsgsClassification& groups, const std::string& drvname)
{
  Clear();

  p_dlist = &dlist;

  code_drvname = drvname;
  file_drvname = str_tolower(drvname);

  // 1 step is to prepare vectors of message's groups
  for (auto m : dlist.msgs)
  {
    if (!m->frameNotEmpty)
    {
      // skip frame when it is empty
      continue;
    }

    auto v = std::find_if(groups.Both.begin(), groups.Both.end(), [&](const uint32_t& msgid)
    {
      return msgid == m->MsgID;
    });

    if (v != std::end(groups.Both))
    {
      // message is in Both group, so put it to rx and tx containers
      tx.push_back(m);
      rx.push_back(m);
      continue;
    }

    v = std::find_if(groups.Rx.begin(), groups.Rx.end(), [&](const uint32_t& msgid)
    {
      return msgid == m->MsgID;
    });

    if (v != std::end(groups.Rx))
    {
      rx.push_back(m);
    }

    v = std::find_if(groups.Tx.begin(), groups.Tx.end(), [&](const uint32_t& msgid)
    {
      return msgid == m->MsgID;
    });

    if (v != std::end(groups.Tx))
    {
      tx.push_back(m);
    }
  }

  std::sort(rx.begin(), rx.end(), [&](const MessageDescriptor_t* a, const MessageDescriptor_t* b)
  {
    return a->MsgID < b->MsgID;
  });

  std::sort(tx.begin(), tx.end(), [&](const MessageDescriptor_t* a, const MessageDescriptor_t* b)
  {
    return a->MsgID < b->MsgID;
  });

  fdesc = &fsd.file;
  gdesc = &fsd.gen;

  // print header for util code
  PrintHeader();

  // print main source for util code
  PrintSource();
}

void CiUtilGenerator::PrintHeader()
{
  tof.Flush();

  if (gdesc->start_info.size() > 0)
  {
    tof.Append("// " + std::regex_replace(gdesc->start_info, std::regex("\n"), "\n// "));
  }

  tof.Append("#pragma once");
  tof.Append();

  tof.Append(openguard);
  tof.Append();

  // include common dbc code config header
  tof.Append("#include \"dbccodeconf.h\"");
  tof.Append();

  // include c-main driver header
  tof.Append("#include \"%s.h\"", file_drvname.c_str());
  tof.Append();


  if (rx.size() == 0)
  {
    tof.Append("// There is no any RX mapped massage.");
    tof.Append();
  }
  else
  {
    // print the typedef
    tof.Append("typedef struct\n{");

    for (auto m : rx)
    {
      tof.Append("  %s_t %s;", m->Name.c_str(), m->Name.c_str());
    }

    tof.Append("} %s_rx_t;", gdesc->drvname.c_str());
    tof.Append();
  }

  if (tx.size() == 0)
  {
    tof.Append("// There is no any TX mapped massage.");
    tof.Append();
  }
  else
  {
    // print the typedef
    tof.Append("typedef struct\n{");

    for (auto m : tx)
    {
      tof.Append("  %s_t %s;", m->Name.c_str(), m->Name.c_str());
    }

    tof.Append("} %s_tx_t;", gdesc->drvname.c_str());
    tof.Append();
  }

  if (rx.size() > 0)
  {
    // receive function necessary only when more than 0 rx messages were mapped
    tof.Append("uint32_t %s_Receive(%s_rx_t* m, const uint8_t* d, uint32_t msgid, uint8_t dlc);",
      gdesc->drvname.c_str(), gdesc->drvname.c_str());
    tof.Append();
  }

  // print extern for super structs
  if (rx.size() > 0 || tx.size() > 0)
  {
    tof.Append("#ifdef __DEF_%s__", gdesc->DRVNAME.c_str());
    tof.Append();

    if (rx.size() > 0)
    {
      tof.Append("extern %s_rx_t %s_rx;", gdesc->drvname.c_str(), gdesc->drvname.c_str());
      tof.Append();
    }

    if (tx.size() > 0)
    {
      tof.Append("extern %s_tx_t %s_tx;", gdesc->drvname.c_str(), gdesc->drvname.c_str());
      tof.Append();
    }

    tof.Append("#endif // __DEF_%s__", gdesc->DRVNAME.c_str());
    tof.Append();
  }

  tof.Append(closeguard);

  tof.Flush(fdesc->util_h.fpath);
}

void CiUtilGenerator::PrintSource()
{
  if (gdesc->start_info.size() > 0)
  {
    tof.Append("// " + std::regex_replace(gdesc->start_info, std::regex("\n"), "\n// "));
  }

  tof.Append("#include \"%s\"", fdesc->util_h.fname.c_str());
  tof.Append();

  tof.Append("// DBC file version");
  tof.Append("#if (%s != (%uU)) || (%s != (%uU))",
    gdesc->verhigh_def.c_str(), p_dlist->ver.hi, gdesc->verlow_def.c_str(), p_dlist->ver.low);

  tof.Append("#error The %s binutil source file has inconsistency with core dbc lib!",
    gdesc->DRVNAME.c_str());
  tof.Append("#endif");
  tof.Append();

  // optional RX and TX struct allocations
  if (rx.size() > 0 || tx.size() > 0)
  {
    tof.Append("#ifdef __DEF_%s__", gdesc->DRVNAME.c_str());
    tof.Append();

    if (rx.size() > 0)
    {
      tof.Append("%s_rx_t %s_rx;", gdesc->drvname.c_str(), gdesc->drvname.c_str());
      tof.Append();
    }

    if (tx.size() > 0)
    {
      tof.Append("%s_tx_t %s_tx;", gdesc->drvname.c_str(), gdesc->drvname.c_str());
      tof.Append();
    }

    tof.Append("#endif // __DEF_%s__", gdesc->DRVNAME.c_str());
    tof.Append();
  }

  if (rx.size() > 0)
  {
    // tree will be created inside (in dynamic memory) so this
    // scope is responsible for deletion these resources
    // tree is the struct tree-view which is used to execute
    // binary search on FrameID for selecting unpacking function
    auto tree = FillTreeLevel(rx, 0, static_cast<int32_t>(rx.size()));

    tof.Append("uint32_t %s_Receive(%s_rx_t* _m, const uint8_t* _d, uint32_t _id, uint8_t dlc_)",
      gdesc->drvname.c_str(), gdesc->drvname.c_str());

    tof.Append("{");
    tof.Append(" uint32_t recid = 0;");

    // put tree-view struct on code (in treestr variable)
    std::string treestr;
    condtree.Clear();
    tof.Append(condtree.WriteCode(tree, treestr, 1));
    tof.Append();
    tof.Append(" return recid;");
    tof.Append("}");
    tof.Append();

    // clear tree after using
    condtree.DeleteTree(tree);
  }

  tof.Flush(fdesc->util_c.fpath);
}

ConditionalTree_t* CiUtilGenerator::FillTreeLevel(std::vector<MessageDescriptor_t*>& list,
  int32_t l,
  int32_t h,
  bool started)
{
  int32_t span = h - l;
  int32_t lowhalf = span / 2;

  treestarted = started;

  if (h < 1)
  {
    return nullptr;
  }

  ConditionalTree_t* ret = new ConditionalTree_t;

  if (!treestarted && h == 1)
  {
    ret->Type = ConditionalType::Cond;
    auto msg = list[l];
    ret->ConditionExpresion = StrPrint("_id == 0x%XU", msg->MsgID);
    ret->High = new ConditionalTree_t;
    ret->High->Type = ConditionalType::Single;
    ret->High->UtilCodeBody = StrPrint("recid = Unpack_%s_%s(&(_m->%s), _d, dlc_);",
      msg->Name.c_str(), code_drvname.c_str(), msg->Name.c_str());
    return ret;
  }

  if (span > 1)
  {
    ret->Type = ConditionalType::Cond;

    if (lowhalf > 1)
    {
      ret->ConditionExpresion = StrPrint("(_id >= 0x%XU) && (_id < 0x%XU)", list[l]->MsgID, list[(l + lowhalf)]->MsgID);
    }
    else
    {
      ret->ConditionExpresion = StrPrint("_id == 0x%XU", list[l]->MsgID);
    }

    ret->High = FillTreeLevel(list, l, l + lowhalf, true);
    ret->Low = FillTreeLevel(list, l + lowhalf, h, true);
  }
  else
  {
    ret->Type = ConditionalType::Express;
    auto msg = list[l];
    ret->ConditionExpresion = StrPrint("_id == 0x%XU", msg->MsgID);
    ret->UtilCodeBody = StrPrint("recid = Unpack_%s_%s(&(_m->%s), _d, dlc_);",
      msg->Name.c_str(), code_drvname.c_str(), msg->Name.c_str());
  }

  return ret;
}
