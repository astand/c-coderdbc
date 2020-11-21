#include "dbclineparser.h"
#include <regex>

// Message line definitions
static const std::string regMessage = "[^A-Za-z0-9_.-]";
static const std::string MessageLineStart = "BO_ ";

// Signal line definitions
static const std::string kRegSigReceivers = "[^A-Za-z0-9_.+-]+";
static const std::string kRegSigSplit0 = "[^\\w]+";
static const std::string kRegSigSplit1 = "(\\s+:\\s+)";
static const std::string kregSigSplit2 = "(\")";

// This reg splits line to parts delimited by (")
static const std::string kRegCommMain = "\"";
static const std::string kRegCommMeta = "[ ]+";

// This reg splits line to parts (for attributes)
static const std::string kRegAttrMain = "[^A-Za-z0-9_\.]+";


static uint64_t __maxvalues[] =
{
  UCHAR_MAX,
  USHRT_MAX,
  UINT_MAX,
  ULLONG_MAX
};

static int __typeslen[] = { 8, 16, 32, 64 };

std::vector<std::string> resplit(const std::string& s, const std::string& rgx_str)
{
  std::vector<std::string> elems;
  std::regex rgx(rgx_str);
  std::sregex_token_iterator iter(s.begin(), s.end(), rgx, -1);
  std::sregex_token_iterator end;

  while (iter != end)
  {
    //std::cout << "S43:" << *iter << std::endl;
    elems.push_back(*iter);
    ++iter;
  }

  return elems;
}

std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
  str.erase(0, str.find_first_not_of(chars));
  return str;
}

std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
  str.erase(str.find_last_not_of(chars) + 1);
  return str;
}

std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
  return ltrim(rtrim(str, chars), chars);
}

static uint32_t clear_msgid(uint32_t messageid)
{
  return messageid & 0x1FFFFFFFU;
}

DbcLineParser::DbcLineParser()
{
}

bool DbcLineParser::IsMessageLine(const std::string& line)
{
  if (line.find(MessageLineStart) == 0)
    return true;

  return false;
}

bool DbcLineParser::ParseMessageLine(MessageDescriptor_t* msg, const std::string& line)
{
  // Parse DBC message line
  auto items = resplit(line, regMessage);

  if (items.size() < 5)
    return false;

  msg->Transmitter = (items.size() >= 6) ? (items[5]) : ("");

  msg->Name = items[2];

  msg->MsgID = static_cast<uint32_t>(atoi(items[1].c_str()));

  msg->DLC = atoi(items[4].c_str());

  if ((msg->MsgID & 0x60000000) != 0 || msg->DLC == 0 || msg->DLC > 8)
  {
    return false;
  }

  msg->IsExt = ((uint8_t)(msg->MsgID >> 29) & 0x04) == 0x04 ? 1 : 0;

  msg->MsgID = clear_msgid(msg->MsgID);

  return true;
}

bool DbcLineParser::IsSignalLine(const std::string& line)
{
  const std::regex sigMatch("\\s+SG_");
  bool ret = std::regex_search(line, sigMatch);
  return ret;
}

bool DbcLineParser::ParseSignalLine(SignalDescriptor_t* sig, const std::string& line)
{
  // split line in two parts
  auto halfs = resplit(line, kRegSigSplit1);

  if (halfs.size() < 2)
    return false;

  // split tail
  auto tailpart = resplit(halfs[1], kregSigSplit2);

  // split middle part on dedicated values
  auto valpart = resplit(trim(tailpart[0]), kRegSigReceivers);

  halfs = resplit(halfs[0], kRegSigSplit0);

  if (halfs.size() > 1)
  {
    sig->Name = halfs[halfs.size() - 1];
  }
  else
  {
    // TODO: handle wrong split
    return false;
  }

  if (valpart.size() < 7)
  {
    return false;
  }
  else
  {
    sig->StartBit = atoi(valpart[0].c_str());
    sig->LengthBit = atoi(valpart[1].c_str());

    //  factor = double;
    //  offset = double;
    //The factorand offset define the linear conversion rule to convert the signals raw
    //value into the signal's physical value and vice versa:
    //  physical_value = raw_value * factor + offset
    //  raw_value = (physical_value � offset) / factor
    sig->Factor = atof(valpart[3].c_str());
    sig->Offset = atof(valpart[4].c_str());

    sig->RawOffset = static_cast<int32_t>(sig->Offset / sig->Factor);

    sig->MinValue = atof(valpart[5].c_str());
    sig->MaxValue = atof(valpart[6].c_str());


    //The signal_size specifies the size of the signal in bits
    //   byte_order = '0' | '1'; (*0 = little endian, 1 = big endian*)
    sig->Order = (valpart[2].find('1') == -1) ? BitLayout::kMotorolla : BitLayout::kIntel;

    //The byte_format is 0 if the signal's byte order is Intel (little endian) or 1 if the byte
    //order is Motorola(big endian).
    //   value_type = '+' | '-'; (*+= unsigned, -=signed*)
    sig->Signed = (valpart[2].find('-') == -1) ? 0 : 1;

    sig->Type = GetSigType(sig);
  }

  if (tailpart.size() == 3)
  {
    // part 1 went on valpart
    // part 2 is the measure unit
    sig->Unit = tailpart[1];
    // part 3 is the list of RX modules
    auto recs = resplit(trim(tailpart[2]), kRegSigReceivers);

    for (size_t i = 0; i < recs.size(); i++)
    {
      sig->RecS.push_back(recs[i]);
    }
  }
  else
  {
    return false;
  }

  return true;
}

// TODO: refactor algorythm detection type of signal
SigType DbcLineParser::GetSigType(SignalDescriptor_t* sig)
{
  SigType ret = SigType::u64;

  auto len = sig->LengthBit;

  uint8_t is_unsigned = 0;

  int64_t roffset = (int64_t)(sig->Offset / sig->Factor);

  if (!sig->Signed)
  {
    uint64_t maxval = 0;

    if (sig->LengthBit <= 32)
    {
      if (roffset >= 0)
      {
        // this only unsinged case
        maxval = (uint64_t)(std::pow(2, sig->LengthBit) - 1 + roffset);
        is_unsigned = 1;
      }
      else
      {
        uint64_t maxpos = (uint64_t)(std::pow(2, sig->LengthBit + 1) - 1);
        uint64_t maxneg = (uint64_t)(std::abs(roffset * 2));
        maxval = std::max(maxpos, maxneg);
      }

      for (uint8_t i = 0; i < 4; i++)
      {
        if (maxval <= __maxvalues[i])
        {
          ret = (SigType)(i + (is_unsigned * 4));
          break;
        }
      }
    }
    else
    {
      is_unsigned = 1;
    }
  }
  else
  {
    for (uint8_t i = 0; i < 4; i++)
    {
      if (len <= __typeslen[i])
      {
        ret = (SigType)(i + (is_unsigned * 4));
        break;
      }
    }
  }

  return ret;
}

bool DbcLineParser::ParseCommentLine(Comment_t* cm, const std::string& line)
{
  bool ret = false;

  if (line.size() > 0)
  {
    if (line.find("CM_") == 0)
    {
      commentline.clear();
      commentline = line;
    }
    else if (commentline.size() > 0)
    {
      // next part of comment line, add to previously saved
      commentline += '\n' + line;
    }

    // check if the current line last
    if (commentline.size() > 0 && line.back() == ';')
    {
      // set invalid target type as default return value
      cm->ca_target = CommentTarget::Undefined;

      // comment line must have 3 part: meta, text, semicolon
      auto items = resplit(commentline, kRegCommMain);

      if (items.size() == 3)
      {
        // part 1 (meta) contains service fields
        auto meta = resplit(items[0], kRegCommMeta);

        if (meta.size() >= 3)
        {
          // 1 CM_ marker
          // 2 target (message or signal)
          // 3 msg id
          uint32_t id = ((uint32_t)(atoi(meta[2].c_str())));

          // clear message id from high 3 bits
          cm->MsgId = clear_msgid(id);

          if (meta[1] == "SG_" && meta.size() == 4)
          {
            // signal comment
            cm->ca_target = CommentTarget::Signal;
            cm->SigName = meta[3];
          }
          else if (meta[1] == "BO_")
          {
            // message comment
            cm->ca_target = CommentTarget::Message;
          }

          // copy comment text
          cm->Text = items[1];
        }

        ret = true;
      }

      commentline.clear();
    }
  }

  return ret;
}

bool DbcLineParser::ParseAttributeLine(AttributeDescriptor_t* attr, const std::string& line)
{
  bool ret = false;

  if (line.size() > 0)
  {
    if (line.find("BA_ ") == 0)
    {
      attribline.clear();
      attribline = line;
    }
    else if (attribline.size() > 0)
    {
      attribline += line;
    }

    // check if the current line is last
    if (attribline.size() > 0 && line.back() == ';')
    {
      attr->Type = AttributeType::Undefined;
      // raw line is ready
      auto items = resplit(attribline, kRegAttrMain);

      if (items.size() > 4 && items[1] == "GenMsgCycleTime"
          && items[2] == "BO_")
      {
        attr->Type = AttributeType::CycleTime;
        attr->MsgId = clear_msgid(atoi(items[3].c_str()));
        // read value of ms of cycle time for the current message
        attr->Value = atoi(items[4].c_str());
        ret = true;
      }

      attribline.clear();
    }
  }

  return ret;
}
