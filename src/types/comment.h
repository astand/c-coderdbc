#pragma once

#include <stdint.h>
#include <string>
#include <vector>

/// @brief Type of commented value
enum class CommentTarget
{
  /// @brief Comment is for CAN message
  Message,
  /// @brief Comment is for CAN message signal
  Signal,
  /// @brief Invalid type
  Undefined
};


/// @brief Comment descripton
struct Comment_t
{
  /// @brief Message ID for which comment is bound
  uint32_t MsgId;

  /// @brief Signal name for which comment is bound
  std::string SigName;

  /// @brief Comment target type
  CommentTarget ca_target;

  /// @brief Comment text
  std::string Text;
};


/// @brief Value table inforamtion
struct ValTable_t
{
  /// @brief Signal name for which value table is applied
  std::string SigName;

  /// @brief Value table names and values pairs
  std::vector<std::pair<std::string, uint32_t>> vpairs{};

};
