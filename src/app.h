#pragma once

#include <stdint.h>
#include <vector>
#include <string>

class CoderApp {
 public:
  CoderApp(const std::vector<std::pair<std::string, std::string>>& params) : Params(params) {}

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

  const std::vector<std::pair<std::string, std::string>>& Params;

  StrParam_t dbc{};
  StrParam_t outdir{};
  StrParam_t drvname{};

  bool rewrite_src{false};
  bool gen_nodeutils{false};
};
