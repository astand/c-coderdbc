
#include <stdlib.h>
#include <memory>
#include "app.h"

std::vector<std::pair<std::string, std::string>> getoptions(int argc, char** argv)
{
  std::vector<std::pair<std::string, std::string>> ret{};

  std::pair<std::string, std::string> pair{};

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

int main(int argc, char* argv[])
{
  auto opts = getoptions(argc, argv);
  auto app = std::make_unique<CoderApp>(opts);
  app->Run();
}
