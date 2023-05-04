#pragma once

#include <stdlib.h>
#include <string>
#include <memory>
#include <vector>

typedef struct
{
  std::string value;
  bool ok{false};
} StrParam_t;

typedef struct
{
  StrParam_t dbc;
  StrParam_t outdir;
  StrParam_t drvname;
  bool is_rewrite{false};
  bool is_nodeutils{false};
  bool is_noconfig{false};
  bool is_nocanmon{false};
  bool is_nofmon{false};
  bool is_help{false};
} ParamConfig_t;

class OptionsParser {
 public:

  using OnePair = std::pair<std::string, std::string>;
  using Pairs = ParamConfig_t;

  ParamConfig_t GetOptions(int argc, char** argv);

};


