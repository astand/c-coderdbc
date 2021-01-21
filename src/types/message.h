#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "attributes.h"
#include "comment.h"

enum class BitLayout
{
  kIntel = 0,
  kMotorolla
};

enum class SigType
{
  i8 = 0,
  i16,
  i32,
  i64,
  u8,
  u16,
  u32,
  u64
};


typedef struct
{
  // Signal name
  std::string Name;

  // Unit
  std::string Unit;

  uint32_t StartBit;

  uint8_t LengthBit;

  // this flag shows when factor (or offset) is double
  // it is used when *_from_S and _to_S macros is generated
  bool IsDoubleSig;

  double Factor;

  double Offset;

  double RawOffset;

  BitLayout Order;

  bool Signed;

  SigType Type;

  std::vector<std::string> SigToByte;

  double MinValue;

  double MaxValue;

  std::vector<std::string> RecS;

  std::string CommentText;

  std::string ValueText;

} SignalDescriptor_t;

typedef struct
{

  // Pointer on message name
  std::string Name;

  // Value of message length in bytes
  uint8_t DLC;

  // Message CAN identifier
  uint32_t MsgID;

  // Extended frame type mark, if 0 then standart frame
  uint8_t IsExt;

  // Frame cycle time im ms
  uint32_t Cycle;

  // Name of transmitter ECU
  std::string Transmitter;

  // List of ECUs to receive frame
  std::vector<std::string> RecS;

  // List of Message signals
  std::vector<SignalDescriptor_t> Signals;

  // Message comment
  std::string CommentText;

} MessageDescriptor_t;
