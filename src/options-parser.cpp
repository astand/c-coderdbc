#include "options-parser.h"
#include "helpers/formatter.h"

OptionsParser::GenOptions OptionsParser::GetOptions(int argc, char** argv)
{
  using onepair_t = std::pair<std::string, std::string>;

  std::vector<onepair_t> temppairs{};
  onepair_t pair{};
  GenOptions retpairs{};

  // collect arguments/values to vector
  for (int i = 0; i < argc; i++)
  {
    // key found (must start with '-' (e.g. '-dbc'))
    if (argv[i][0] == '-')
    {
      pair.first = std::string(argv[i]);
      pair.second.clear();

      size_t valindex = i + 1;

      if ((valindex < argc) && (argv[valindex][0] != '-'))
      {
        // key param
        pair.second = std::string(argv[valindex]);
        // shift position to next key
        ++i;
      }

      temppairs.push_back(pair);
    }
  }

  for (size_t i = 0; i < temppairs.size(); i++)
  {
    if (temppairs[i].first.compare("-dbc") == 0)
    {
      retpairs.dbc.first = temppairs[i].second;
      retpairs.dbc.second = true;
    }
    else if (temppairs[i].first.compare("-out") == 0)
    {
      retpairs.outdir.first = temppairs[i].second;
      retpairs.outdir.second = true;
    }
    else if (temppairs[i].first.compare("-drvname") == 0)
    {
      retpairs.drvname.first = make_c_name(temppairs[i].second);
      retpairs.drvname.second = true;

      if (retpairs.drvname.first.length() == 0)
      {
        retpairs.drvname.second = false;
      }
    }
    else if (temppairs[i].first.compare("-rw") == 0)
    {
      retpairs.is_rewrite = true;
    }
    else if (temppairs[i].first.compare("-nodeutils") == 0)
    {
      retpairs.is_nodeutils = true;
    }
    else if (temppairs[i].first.compare("-help") == 0)
    {
      retpairs.is_help = true;
    }
    else if (temppairs[i].first.compare("-noinc") == 0)
    {
      retpairs.is_nocanmon = true;
    }
    else if (temppairs[i].first.compare("-noconfig") == 0)
    {
      retpairs.is_noconfig = true;
    }
    else if (temppairs[i].first.compare("-nofmon") == 0)
    {
      retpairs.is_nofmon = true;
    }
    else if (temppairs[i].first.compare("-driverdir") == 0)
    {
      retpairs.is_driver_dir = true;
    }
    else if (temppairs[i].first.compare("-gendate") == 0)
    {
      retpairs.add_gen_date = true;
    }
    else if (temppairs[i].first.compare("-multiplex") == 0 || temppairs[i].first.compare("-muxgen") == 0)
    {
      retpairs.is_multiplex_enabled = true;
    }
  }

  return retpairs;
}
