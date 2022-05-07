#include "dbcscanner.h"
#include <cstring>
#include <algorithm>
#include <math.h>
#include "../helpers/formatter.h"

#define MAX_LINE 4096 * 4

char line[MAX_LINE] = { 0 };

MessageDescriptor_t* find_message(vector<MessageDescriptor_t*> msgs, uint32_t ID)
{
  MessageDescriptor_t* ret = nullptr;

  if (msgs.size() == 0)
  {
    return ret;
  }

  for (size_t i = 0; i < msgs.size(); i++)
  {
    ret = msgs[i];

    if (ret->MsgID == ID)
    {
      // Frame found
      break;
    }
  }

  return ret;
}

DbcScanner::DbcScanner()
{
}

DbcScanner::~DbcScanner()
{
}

int32_t DbcScanner::TrimDbcText(istream& readstrm)
{
  dblist.msgs.clear();
  dblist.ver.hi = dblist.ver.low = 0;

  readstrm.clear();
  readstrm.seekg(0);
  // Search each message and signals in dbc source data,
  // and fill @msgs collection
  ParseMessageInfo(readstrm);

  // all the messages and signals were read from source
  // scan it one more time to define all attributes and
  // more additional information
  readstrm.clear();
  readstrm.seekg(0);

  // Search all attributes and additional information
  // and update specific fields in messages
  ParseOtherInfo(readstrm);

  return 0;
}


void DbcScanner::ParseMessageInfo(istream& readstrm)
{
  std::string sline;

  MessageDescriptor_t* pMsg = nullptr;

  while (readstrm.eof() == false)
  {
    readstrm.getline(line, MAX_LINE);

    sline = str_trim(line);

    FindVersion(sline);

    // New message line has been found
    if (lparser.IsMessageLine(sline))
    {
      // check if the pMsg takes previous message
      AddMessage(pMsg);

      // create instance for the detected message
      pMsg = new MessageDescriptor_t;

      SetDefualtMessage(pMsg);

      if (!lparser.ParseMessageLine(pMsg, sline))
      {
        // the message has invalid format so drop it and wait next one
        delete pMsg;
        pMsg = nullptr;
      }
    }

    if (pMsg != nullptr && lparser.IsSignalLine(sline))
    {
      SignalDescriptor_t sig;

      // parse signal line
      if (lparser.ParseSignalLine(&sig, sline))
      {
        // put successfully parsed  signal to the message signals
        pMsg->Signals.push_back(sig);

        if (sig.IsDoubleSig || sig.IsSimpleSig != true)
        {
          pMsg->hasPhys = true;
        }
      }
    }

    std::vector<std::string> tx_nodes;
    tx_nodes.clear();

    uint32_t msgid = lparser.ParseMultiTrans(tx_nodes, sline);

    if (msgid != 0)
    {
      // In this place no messages will captured after,
      // so put temp pMsg as last message and null it
      AddMessage(pMsg);
      pMsg = nullptr;

      // Multi TXs line detected, expand information
      auto msg = find_message(dblist.msgs, msgid);

      if (msg != nullptr)
      {
        for (size_t i = 0; i < tx_nodes.size(); i++)
        {
          if (std::find(msg->TranS.begin(), msg->TranS.end(), tx_nodes[i]) == msg->TranS.end())
          {
            // add another one RX node
            msg->TranS.push_back(tx_nodes[i]);
          }
        }
      }
    }
  }

  // check if the pMsg takes previous message
  AddMessage(pMsg);
}


void DbcScanner::ParseOtherInfo(istream& readstrm)
{
  std::string sline;

  Comment_t cmmnt;

  ValTable_t vals;

  AttributeDescriptor_t attr;

  while (!readstrm.eof())
  {
    readstrm.getline(line, MAX_LINE);

    sline = str_trim(line);

    if (lparser.ParseCommentLine(&cmmnt, sline))
    {
      // update message comment field
      auto msg = find_message(dblist.msgs, cmmnt.MsgId);

      if (msg != nullptr)
      {
        // comment line was found
        if (cmmnt.ca_target == CommentTarget::Message)
        {
          // put comment to message descriptor
          msg->CommentText = cmmnt.Text;
        }
        else if (cmmnt.ca_target == CommentTarget::Signal)
        {
          for (size_t i = 0; i < msg->Signals.size(); i++)
          {
            if (cmmnt.SigName == msg->Signals[i].Name)
            {
              SignalDescriptor_t& sig = msg->Signals[i];
              // signal has been found, update commnet text
              msg->Signals[i].CommentText = cmmnt.Text;

              // 1 test if signal is rolling
              if (cmmnt.Text.find("<RollingCounter>") != std::string::npos)
              {
                // set the RollSig to generate necessary code
                msg->RollSig = &msg->Signals[i];
              }

              extern std::vector<std::string> resplit(const std::string & s, const std::string & rgx_str, int32_t submatch);

              size_t openpos = cmmnt.Text.find('<');

              if (openpos != std::string::npos)
              {
                size_t closepos = cmmnt.Text.find('>', openpos);

                if ((closepos != std::string::npos) && (closepos > (openpos + 1)))
                {
                  auto substr = cmmnt.Text.substr(openpos + 1, closepos - 1);

                  auto meta = resplit(substr, "(\\:)", -1);

                  if (meta.size() == 3 && meta[0] == "Checksum")
                  {
                    // the signal can be CSM, but additional settings must be
                    // checked: size, boundary, signal type
                    bool boundary_ok = (sig.Order == BitLayout::kIntel) ?
                      ((sig.StartBit / 8) == ((sig.StartBit + sig.LengthBit - 1) / 8)) :
                      ((sig.StartBit / 8) == ((sig.StartBit - sig.LengthBit + 1) / 8));

                    if (sig.IsSimpleSig && boundary_ok && sig.Signed == false)
                    {
                      msg->CsmSig = &sig;
                      msg->CsmMethod = meta[1];
                      msg->CsmOp = atoi(meta[2].c_str());
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    if (lparser.ParseValTableLine(&cmmnt, sline, vals))
    {
      // update message comment field
      auto msg = find_message(dblist.msgs, cmmnt.MsgId);

      if (msg != nullptr)
      {
        // comment line was found
        if (cmmnt.ca_target == CommentTarget::Message)
        {
          // put comment to message descriptor
          msg->CommentText = cmmnt.Text;
        }
        else if (cmmnt.ca_target == CommentTarget::Signal)
        {
          for (size_t i = 0; i < msg->Signals.size(); i++)
          {
            if (cmmnt.SigName == msg->Signals[i].Name)
            {
              // signal has been found, update commnet text
              msg->Signals[i].ValueText = cmmnt.Text;
              // save collected value table's definitions to signal
              msg->Signals[i].ValDefs = vals;
            }
          }
        }
      }
    }

    if (lparser.ParseAttributeLine(&attr, sline))
    {
      auto msg = find_message(dblist.msgs, attr.MsgId);

      if (msg != nullptr)
      {
        // message was found, set attribute value
        if (attr.Type == AttributeType::CycleTime)
        {
          msg->Cycle = attr.Value;
        }
      }
    }
  }
}


void DbcScanner::AddMessage(MessageDescriptor_t* message)
{
  if (message != nullptr)
  {
    if (message->Signals.size() > 0)
    {
      // sort signals by start bit
      std::sort(message->Signals.begin(), message->Signals.end(),
        [](const SignalDescriptor_t& a, const SignalDescriptor_t& b) -> bool
      {
        return a.StartBit < b.StartBit;
      });

      for (size_t i = 0; i < message->Signals.size(); i++)
      {
        for (size_t j = 0; j < message->Signals[i].RecS.size(); j++)
        {
          string val = message->Signals[i].RecS[j];

          if (std::find(message->RecS.begin(), message->RecS.end(), val) == message->RecS.end())
          {
            // add another one RX node
            message->RecS.push_back(val);
          }
        }
      }
    }

    // save pointer on message
    dblist.msgs.push_back(message);
  }
}


void DbcScanner::SetDefualtMessage(MessageDescriptor_t* message)
{
  message->CommentText = "";
  message->Cycle = 0;
  message->DLC = 0;
  message->IsExt = false;
  message->MsgID = 0;
  message->Name = "";
  message->RecS.clear();
  message->Signals.clear();
  message->TranS.clear();
  message->hasPhys = false;
  message->RollSig = nullptr;
  message->CsmSig = nullptr;
  message->CsmMethod = "";
  message->CsmOp = 0;
  message->CsmToByteExpr = "";
}


void DbcScanner::FindVersion(const std::string& instr)
{
  // try to find version string which looks like: VERSION "x.x"
  uint32_t h = 0, l = 0;
  char marker[9];

  if (instr[0] != 'V' && instr[1] != 'E')
  {
    return;
  }

  int32_t ret = std::sscanf(instr.c_str(), "%8s \"%u.%u\"", marker, &h, &l);

  if ((ret == 3) && (std::strcmp(marker, "VERSION") == 0))
  {
    // versions have been found, save numeric values
    dblist.ver.hi = h;
    dblist.ver.low = l;
  }
}
