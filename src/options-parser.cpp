#include "options-parser.h"
#include "helpers/formatter.h"

OptionsParser::Pairs OptionsParser::GetOptions(int argc, char** argv)
{
  std::vector<OnePair> ret{};

  OnePair pair{};

  Pairs pairs;

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

  for (size_t i = 0; i < ret.size(); i++)
  {
    if (ret[i].first.compare("-dbc") == 0)
    {
      pairs.dbc.value = ret[i].second;
      pairs.dbc.ok = true;
    }
    else if (ret[i].first.compare("-out") == 0)
    {
      pairs.outdir.value = ret[i].second;
      pairs.outdir.ok = true;
    }
    else if (ret[i].first.compare("-drvname") == 0)
    {
      pairs.drvname.value = make_c_name(ret[i].second);
      pairs.drvname.ok = true;

      if (pairs.drvname.value.length() == 0)
      {
        pairs.drvname.ok = false;
      }
    }
    else if (ret[i].first.compare("-rw") == 0)
    {
      pairs.is_rewrite = true;
    }
    else if (ret[i].first.compare("-nodeutils") == 0)
    {
      pairs.is_nodeutils = true;
    }
    else if (ret[i].first.compare("-help") == 0)
    {
      pairs.is_help = true;
    }
  }

  return pairs;
}
