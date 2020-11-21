#pragma once

#include <stdint.h>
#include <vector>
#include <string>

enum class AttributeType
{
  CycleTime,
  Undefined
};

typedef struct
{
  // message id of the attribute
  uint32_t MsgId;

  // value of the comment from line
  AttributeType Type;

  // attribute value
  int32_t Value;

} AttributeDescriptor_t;
