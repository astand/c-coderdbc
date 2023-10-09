#pragma once

#include <stdint.h>
#include <vector>
#include <string>

/// @brief Message attributes
enum class AttributeType
{
  /// @brief Message cycle time attribute
  CycleTime,

  /// @brief Undefined attribute
  Undefined
};

struct AttributeDescriptor_t
{
  /// @brief Attribute message ID
  uint32_t MsgId;

  /// @brief Attribute type
  AttributeType Type;

  /// @brief Attribute value
  int32_t Value;
};
