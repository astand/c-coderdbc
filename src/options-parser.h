#pragma once

#include <stdlib.h>
#include <string>
#include <memory>
#include <vector>

class OptionsParser {
 public:

  using OnePair = std::pair<std::string, std::string>;
  using Pairs = std::vector<OnePair>;

  Pairs GetOptions(int argc, char** argv);

};


