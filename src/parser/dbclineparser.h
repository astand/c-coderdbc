#pragma once

#include <stdint.h>
#include "../types/message.h"

//Information from official vector DBC format spec:
//
//The keywords used in DBC files o identify the type of an object are given in the
//following table :
//Keyword Object Type
//	BU_ Network Node
//	BO_ Message
//	SG_ Signal
//	EV_ Environment Variable
//
//The signal_size specifies the size of the signal in bits
//byte_order = '0' | '1'; (*0 = little endian, 1 = big endian*)
//
//The byte_format is 0 if the signal's byte order is Intel (little endian) or 1 if the byte
//order is Motorola(big endian).
//value_type = '+' | '-'; (*+= unsigned, -=signed*)
//
//
//The value_type defines the signal as being of type unsigned(-) or signed(-).
//factor = double;
//offset = double;
//The factorand offset define the linear conversion rule to convert the signals raw
//value into the signal's physical value and vice versa:
//physical_value = raw_value * factor + offset
//raw_value = (physical_value - offset) / factor
//As can be seen in the conversion rule formulas the factor must not be 0.
//minimum = double;
//maximum = double;

//The comment section contains the object comments.For each object having a
//comment, an entry with the object's type identification is defined in this section.
//comments = { comment };
//comment = 'CM_' (char_string |
//	'BU_' node_name char_string |
//	'BO_' message_id char_string |
//	'SG_' message_id signal_name char_string |
//	'EV_' env_var_name char_string)
//	';';

class DbcLineParser {
 public:
  DbcLineParser();

  // checks if the line is message description
  bool IsMessageLine(const std::string& line);
  // parses message line
  bool ParseMessageLine(MessageDescriptor_t* msg, const std::string& line = "");

  // checks if the line is signal description
  bool IsSignalLine(const std::string& line);
  // parses signal line
  bool ParseSignalLine(SignalDescriptor_t* sig, const std::string& line);

  // tries to parse attribute line (or a few lines) and
  // loads result in attr struct, return @true if parsed ok
  bool ParseAttributeLine(AttributeDescriptor_t* attr, const std::string& line);

  // tries to parse comment information in line (or a few lines)
  // and loads result to cm struct, return @true if parsed ok
  bool ParseCommentLine(Comment_t* cm, const std::string& line);

  // tries to parse value table string in line
  // saves result as comment text in @cm object, and as
  // pairs of items (definition / value) in @vtab
  bool ParseValTableLine(Comment_t* cm, const std::string& line, ValTable_t& vtab);

  /**
   * @brief tries to find string with information about frame which has
   * multiple TX nodes
   * @param outnodes vector to load names of TX nodes
   * @param str line to parse from DBC file
   * @retval MsgId if parsed successfully or zero
   */
  uint32_t ParseMultiTrans(std::vector<std::string>& outnodes, std::string& str);

 private:
  // defines the type for the message struct member
  SigType GetSigType(SignalDescriptor_t* sig);

  // string to collect comment dbc line
  std::string commentline;

  // string to collect attribute dbc line
  std::string attribline;

  // strign to collect value line
  std::string valueline;
};

