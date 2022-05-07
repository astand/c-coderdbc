#include "dbclineparser.h"
#include <helpers/formatter.h>
#include <stdlib.h>
#include <clocale>
#include <regex>
#include <math.h>
#include <limits.h>

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
static const std::string kRegAttrMain = "[^A-Za-z0-9_\\.]+";
static const std::string kTransmittersList = "[^a-zA-Z_0-9]+";

// Regex template to split string by spaces BUT NOT what inside quotes OR apostrophes
// [^\s"']+|"([^"]*)"|'([^']*)'

// Reges template to split string by spaces BUT NOT what inside quotes
// [^\s"]+|"([^"]*)"

static const std::string kRegValTable = "[^\\s\"]+|\"([^\"]*)\"";

static uint64_t __maxunsigvalues[] =
{
  UCHAR_MAX,
  USHRT_MAX,
  UINT_MAX,
  ULLONG_MAX
};

static uint64_t __maxsignedvals[] =
{
  CHAR_MAX,
  SHRT_MAX,
  INT_MAX,
  LLONG_MAX
};

//* @param __submatch
// - -1 each enumerated subexpression does NOT
// match the regular expression (aka field
// splitting)
// - 0 the entire string matching the
// subexpression is returned for each match
// within the text.
// - >0 enumerates only the indicated
// subexpression from a match within the text.
std::vector<std::string> resplit(const std::string& s, const std::string& rgx_str, int32_t submatch)
{
  std::vector<std::string> elems;
  std::regex rgx(rgx_str);
  std::sregex_token_iterator iter(s.begin(), s.end(), rgx, submatch);
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
  {
    return true;
  }

  return false;
}

bool DbcLineParser::ParseMessageLine(MessageDescriptor_t* msg, const std::string& line)
{
  // Parse DBC message line
  auto items = resplit(line, regMessage, -1);

  if (items.size() < 5)
  {
    return false;
  }

  std::string txname = (items.size() >= 6) ? (items[5]) : ("");

  if (txname.size() > 1)
  {
    msg->TranS.push_back(txname);
  }

  msg->Name = items[2];

  msg->MsgID = static_cast<uint32_t>(atoll(items[1].c_str()));

  msg->DLC = atoi(items[4].c_str());

  if ((msg->MsgID & 0x60000000) != 0 || msg->DLC == 0 || msg->DLC > 8)
  {
    return false;
  }

  msg->IsExt = ((uint8_t)(msg->MsgID >> 29) & 0x04) == 0x04 ? 1 : 0;

  msg->MsgID = clear_msgid(msg->MsgID);

  return true;
}

uint32_t DbcLineParser::ParseMultiTrans(std::vector<std::string>& outnodes, std::string& line)
{
  uint32_t ret = 0;

  auto chunks = resplit(line, kTransmittersList, -1);

  if (chunks.size() >= 3 && chunks[0] == "BO_TX_BU_")
  {
    ret = clear_msgid(static_cast<uint32_t>(atoll(chunks[1].c_str())));

    if (ret != 0)
    {
      for (size_t i = 2; i < chunks.size(); i++)
      {
        outnodes.push_back(chunks[i]);
      }
    }
  }

  return ret;
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
  auto halfs = resplit(line, kRegSigSplit1, -1);

  if (halfs.size() < 2)
  {
    return false;
  }

  // split tail
  auto tailpart = resplit(halfs[1], kregSigSplit2, -1);

  // split middle part on dedicated values
  auto valpart = resplit(trim(tailpart[0]), kRegSigReceivers, -1);

  halfs = resplit(halfs[0], kRegValTable, 0);

  if (halfs.size() >= 2)
  {
    sig->Name = halfs[1];
    sig->Multiplex = MultiplexType::kNone;

    if (halfs.size() == 3)
    {
      // Multiplex signal, put additional comment
      if (halfs[2] == "M")
      {
        sig->Multiplex = MultiplexType::kMaster;
      }
      else
      {
        sig->Multiplex = MultiplexType::kMulValue;
      }
    }
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

    // get info about factor or offset double nature
    sig->IsDoubleSig = false;
    sig->IsSimpleSig = false;

    // for enabling double conversation the factor or offset
    // substring must have dot ('.') character
    if (valpart[3].find_first_of('.') != std::string::npos ||
      valpart[4].find_first_of('.') != std::string::npos)
    {
      sig->IsDoubleSig = true;
    }

    //  factor = double;
    //  offset = double;
    //The factorand offset define the linear conversion rule to convert the signals raw
    //value into the signal's physical value and vice versa:
    //  physical_value = raw_value * factor + offset
    //  raw_value = (physical_value - offset) / factor
    std::setlocale(LC_ALL, "en_US.UTF-8");

    sig->Factor = atof(valpart[3].c_str());
    sig->Offset = atof(valpart[4].c_str());

    sig->RawOffset = sig->Offset / sig->Factor;

    sig->MinValue = atof(valpart[5].c_str());
    sig->MaxValue = atof(valpart[6].c_str());


    //The signal_size specifies the size of the signal in bits
    //   byte_order = '0' | '1'; (*0 = little endian, 1 = big endian*)
    sig->Order = (valpart[2].find('1') == std::string::npos) ? BitLayout::kMotorolla : BitLayout::kIntel;

    //The byte_format is 0 if the signal's byte order is Intel (little endian) or 1 if the byte
    //order is Motorola(big endian).
    //   value_type = '+' | '-'; (*+= unsigned, -=signed*)
    sig->Signed = (valpart[2].find('-') == std::string::npos) ? 0 : 1;

    GetSigType(sig);

    // mark all simple signals to make using them easier
    if (!sig->IsDoubleSig && (sig->Factor == 1) && (sig->Offset == 0))
    {
      sig->IsSimpleSig = true;
    }

    if (!sig->IsSimpleSig)
    {
      // For this case the name of signal must be marked specially
      // to pay attention that if SIGFLOAT is enabled, this signal
      // must behave as ReadOnly (_ro)
      sig->NameFloat = sig->Name + "_phys";
      sig->Name += "_ro";
    }
  }

  if (tailpart.size() == 3)
  {
    // part 1 went on valpart
    // part 2 is the measure unit
    sig->Unit = tailpart[1];
    // part 3 is the list of RX modules
    auto recs = resplit(trim(tailpart[2]), kRegSigReceivers, -1);

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

  uint8_t is_unsigned = 0;

  int64_t roffset = (int64_t)(sig->Offset / sig->Factor);

  uint64_t max_v = 0;

  // 1 step is to detect type of _ro
  int64_t max_abs, min_abs;

  int64_t addon = 0;

  if (sig->Signed)
  {
    addon = 1;
    max_abs = static_cast<int64_t>((std::pow(2, sig->LengthBit - 1) - 1));
    min_abs = (max_abs + 1) * -1;

    for (size_t i = 0; i < 4; i++)
    {
      sig->TypeRo = (SigType)(i);

      if (max_abs <= __maxsignedvals[i])
      {
        break;
      }
    }
  }
  else
  {
    max_abs = static_cast<int64_t>((std::pow(2, sig->LengthBit) - 1));
    min_abs = 0;

    for (size_t i = 0; i < 4; i++)
    {
      sig->TypeRo = (SigType)(i + 4);

      if (max_abs <= __maxunsigvalues[i])
      {
        break;
      }
    }
  }

  if (sig->IsSimpleSig)
  {
    // the most simple case, TypePhys is the same as TypeRo
    sig->TypePhys = sig->TypeRo;
  }

  else if (sig->IsDoubleSig == false)
  {
    int64_t i_offset = (int64_t)sig->Offset;
    int64_t i_factor = (int64_t)sig->Factor;

    // get max and min values with applied factor and offset (physical values)
    max_abs = max_abs * i_factor + i_offset;
    min_abs = min_abs * i_factor + i_offset;

    if (sig->Signed || max_abs < 0 || min_abs < 0)
    {
      // phys value must be signed
      uint64_t max_v = std::abs(max_abs);
      uint64_t addon = 0;

      if ((max_v + 1) < std::abs(min_abs))
      {
        // low part is main
        addon = 1;
        max_v = std::abs(min_abs);
      }

      for (size_t i = 0; i < 4; i++)
      {
        sig->TypePhys = (SigType)(i);

        if (max_v <= __maxsignedvals[i] + addon)
        {
          break;
        }
      }
    }
    else
    {
      // phys value must be unsigned
      for (uint8_t i = 0; i < 4; i++)
      {
        if ((uint64_t)max_abs <= __maxunsigvalues[i])
        {
          sig->TypePhys = (SigType)(i + 4);
          break;
        }
      }
    }
  }
  else
  {
    // in this case TypePhys will be (sigfloat_t), so
    // there is no necessity to determine physical signal type
  }

  return sig->TypeRo;
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
      auto items = resplit(commentline, kRegCommMain, -1);

      if (items.size() == 3)
      {
        // part 1 (meta) contains service fields
        auto meta = resplit(items[0], kRegCommMeta, -1);

        if (meta.size() >= 3)
        {
          // 1 CM_ marker
          // 2 target (message or signal)
          // 3 msg id
          uint32_t id = static_cast<uint32_t>(atoll(meta[2].c_str()));

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

        if (cm->Text.size() > 0 && cm->Text.back() == '\n')
        {
          // remove last '\n' symbol in the string end
          cm->Text.pop_back();
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
      auto items = resplit(attribline, kRegAttrMain, -1);

      if (items.size() > 4 && items[1] == "GenMsgCycleTime" && items[2] == "BO_")
      {
        attr->Type = AttributeType::CycleTime;
        attr->MsgId = clear_msgid(static_cast<uint32_t>(atoll(items[3].c_str())));
        // read value of ms of cycle time for the current message
        attr->Value = atoi(items[4].c_str());
        ret = true;
      }

      attribline.clear();
    }
  }

  return ret;
}

bool DbcLineParser::ParseValTableLine(Comment_t* comm, const std::string& line, ValTable_t& vtab)
{
  bool ret = false;

  if (line.size() > 0)
  {
    if (line.find("VAL_ ") == 0)
    {
      valueline.clear();
      valueline = line;
    }
    else if (valueline.size() > 0)
    {
      valueline += line;
    }

    // check if the current line is last
    if (valueline.size() > 0 && line.back() == ';')
    {
      // split all items by spaces and inside quotes.
      // after this step proper value items will have count >= 5
      // last item will be ';' and number of items will be even
      auto items = resplit(valueline, kRegValTable, 0);

      if ((items.size() >= 3) && (items.back() == ";") && (items.size() % 2 == 0))
      {
        comm->MsgId = (clear_msgid(static_cast<uint32_t>(atoll(items[1].c_str()))));
        comm->SigName = items[2];
        comm->Text = "";
        comm->ca_target = CommentTarget::Signal;

        // prepare value table container
        vtab.SigName = items[2];
        vtab.vpairs.clear();

        for (size_t valpair = 3; valpair < (items.size() - 1); valpair += 2)
        {
          comm->Text += " " + items[valpair + 0] + " : ";
          comm->Text += items[valpair + 1] + '\n';

          auto valdef = make_c_name(items[valpair + 1]);
          vtab.vpairs.push_back({valdef, (uint32_t)atoll((items[valpair + 0]).c_str())});
        }

        if (comm->Text.size() > 0)
        {
          // remove last '\n' symbol in the string end
          comm->Text.pop_back();
        }

        // value table params were parse successfully
        ret = true;
      }

      valueline.clear();
    }
  }

  return ret;
}
