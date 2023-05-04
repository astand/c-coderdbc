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

enum class MultiplexType
{
  kNone,
  kMaster,
  kMulValue
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
  // Signal float name
  std::string NameFloat;
  // Unit
  std::string Unit;

  uint32_t StartBit;

  uint8_t LengthBit;

  // By next two fields any signal can be strictly related to one of
  // 3 signal type:
  // 1 double based scaled value (IsDoubleSig == true)
  // 2 integer based scaled value (IsDoubleSig == false && IsSimpleSig == false)
  // 3 simple (IsDoubleSig == false && IsSimpleSig == true)

  // this flag shows when factor (or offset) is double
  // it is used when *_from_S and _to_S macros is generated
  bool IsDoubleSig;

  // this flag shows if the signal has factor = 1 and offset = 0
  // to reject any sigfloat or "toS"/"fromS" operations
  // SimpleSig is true when: IsDoubleSig == false && Factor == 1 && Offset == 0
  bool IsSimpleSig;

  double Factor;

  double Offset;

  double RawOffset;

  BitLayout Order;

  bool Signed;

  SigType TypeRo;

  SigType TypePhys;

  std::vector<std::string> SigToByte;

  double MinValue;

  double MaxValue;

  std::vector<std::string> RecS;

  ValTable_t ValDefs;

  std::string CommentText;

  std::string ValueText;

  MultiplexType Multiplex;

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
  std::vector<std::string> TranS;

  // List of ECUs to receive frame
  std::vector<std::string> RecS;

  // List of Message signals
  std::vector<SignalDescriptor_t> Signals;

  // flag about having sigfloat fields
  bool hasPhys;

  // pointer to rolling counter signal
  SignalDescriptor_t* RollSig;

  // pointer to checksum signal
  SignalDescriptor_t* CsmSig;

  // keeps the method or crc algorythm (will be passed to CRC calc function)
  std::string CsmMethod;

  // option value (will be passed to CRC calc function)
  uint32_t CsmOp;

  // expression to load CSM signal to byte
  std::string CsmToByteExpr;

  // byte number in payload which keeps CS value
  uint8_t CsmByteNum;

  // Message comment
  std::string CommentText;

} MessageDescriptor_t;

typedef struct
{
  std::vector<uint32_t> Rx;

  std::vector<uint32_t> Tx;

  std::vector<uint32_t> Both;

} MsgsClassification;

typedef struct
{
  uint32_t hi;
  uint32_t low;
} DbcFileVersion_t;

typedef struct
{
  std::vector<MessageDescriptor_t*> msgs;
  DbcFileVersion_t ver;
} DbcMessageList_t;
