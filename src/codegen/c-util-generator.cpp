#include "c-util-generator.h"
#include <algorithm>

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


void CiUtilGenerator::Generate(std::vector<MessageDescriptor_t*>& msgs, const FsDescriptor_t& fsd,
  const MsgsClassification& groups)
{
  Clear();

  // 1 step is to prepare vectors of message's groups
  for (auto m : msgs)
  {
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
      continue;
    }

    v = std::find_if(groups.Tx.begin(), groups.Tx.end(), [&](const uint32_t& msgid)
    {
      return msgid == m->MsgID;
    });

    if (v != std::end(groups.Tx))
    {
      tx.push_back(m);
      continue;
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

  fdesc = &fsd;

}

