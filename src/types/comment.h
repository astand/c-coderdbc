#pragma once

#include <stdint.h>
#include <string>

enum class CommentTarget
{
  Message,
  Signal,
  Undefined
};

typedef struct
{

  // CAN ID to which comment is belongs (in case of message targeting)
  uint32_t MsgId;

  // name of signal of the CAN Frame to whick comment is belong (in case
  // of signal targeting
  std::string SigName;

  // type of the comment in attribute line (message or signal)
  CommentTarget ca_target;

  // value of the comment from line
  std::string Text;

} Comment_t;
