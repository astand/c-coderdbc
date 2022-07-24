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

  const OptionsParser::Pairs& Params;
};
