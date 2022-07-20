#include "options-parser.h"

OptionsParser::Pairs OptionsParser::GetOptions(int argc, char** argv)
{
  Pairs ret{};

  OnePair pair{};

  if (argc <= 0)
  {
    return ret;
  }

  for (int i = 0; i < argc; i++)
  {
    // key found (must start with '-' (e.g. '-dbc'))
    if (argv[i][0] == '-')
    {
      pair.first = std::string(argv[i]);
      pair.second.clear();

      if ((i + 1) < argc && argv[i + 1][0] != '-')
      {
        // key param
        pair.second = std::string(argv[i + 1]);
        // unlooped i incremention
        ++i;
      }

      ret.push_back(pair);
    }
  }

  return ret;
}
