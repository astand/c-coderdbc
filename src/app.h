#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <options-parser.h>

class CoderApp {
 public:
  CoderApp(const OptionsParser::Pairs& params) : Params(params) {}

  void Run();

 private:
  bool ParseParams();
  void GenerateCode();
  void PrintHelp();

  typedef struct app
  {
    std::string value;
    bool ok{false};
  } StrParam_t;

  const OptionsParser::Pairs& Params;

  StrParam_t dbc{};
  StrParam_t outdir{};
  StrParam_t drvname{};

  bool rewrite_src{false};
  bool gen_nodeutils{false};
};
