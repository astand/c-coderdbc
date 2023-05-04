#pragma once

#include <string>

enum class ConditionalType { Cond, Express, Single };

struct ConditionalTree_t
{
  ConditionalTree_t* High;

  ConditionalTree_t* Low;

  ConditionalType Type;

  std::string ConditionExpresion;

  std::string UtilCodeBody;

  ConditionalTree_t() {
    High = nullptr;
    Low = nullptr;
    Type = ConditionalType::Single;
  }
};

class ConditionalTree {
 public:
  ConditionalTree();

  void Clear() {
    codestr.clear();
  }

  std::string WriteCode(const ConditionalTree_t* tree, std::string& outstr, uint8_t intend);

  // deletes all memory allocated to tree members
  void DeleteTree(ConditionalTree_t* tree);

 private:

  void PrintCode(std::string& str, uint8_t indent);

  std::string codestr;
};
