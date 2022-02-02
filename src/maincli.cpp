
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <stdlib.h>
#include "parser/dbcscanner.h"
#include "codegen/c-main-generator.h"
#include "codegen/c-util-generator.h"
#include "codegen/fs-creator.h"
#include "codegen/version.h"

#define GEN_UTIL_CODE

#define MIN_ARGC_NUM 4
#define MAX_ARGC_NUM 5

char verstr[128] = {0};

DbcScanner* scanner;
CiMainGenerator* cigen;
CiUtilGenerator* ciugen;
FsCreator* fscreator;

std::string source_files_out_path;
std::string dbc_file_path;
std::string dbc_driver_name;

typedef struct
{
  std::string arg;
  std::string param;
} ParamPair_t;

void PrintUsage();

std::vector<ParamPair_t> getoptions(int argc, char** argv)
{
  std::vector<ParamPair_t> ret;

  ParamPair_t pair;

  if (argc <= 0)
  {
    return ret;
  }

  for (int i = 0; i < argc; i++)
  {
    // key found (must start with '-' (e.g. '-dbc'))
    if (argv[i][0] == '-')
    {
      pair.arg = std::string(argv[i]);
      pair.param.clear();

      if (i + 1 < argc && argv[i + 1][0] != '-')
      {
        // key param
        pair.param = std::string(argv[i + 1]);
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
  const std::string arg_ops = "dbc:out:rxnodes:rw";

  bool err = false;
  bool dbc_ok = false;
  bool path_ok = false;
  bool drvname_ok = false;
  bool rewrite_src = false;
  bool gen_nodeutils = false;
  bool help = false;

  std::vector<ParamPair_t> opts = getoptions(argc, argv);

  if (opts.size() < 3)
  {
    PrintUsage();
  }

  for (size_t i = 0; i < opts.size(); i++)
  {
    if (opts[i].arg == "-dbc")
    {
      dbc_file_path = opts[i].param;
      dbc_ok = true;
    }
    else if (opts[i].arg == "-out")
    {
      source_files_out_path = opts[i].param;
      path_ok = true;
    }
    else if (opts[i].arg == "-drvname")
    {
      dbc_driver_name = opts[i].param;
      drvname_ok = true;
    }
    else if (opts[i].arg == "-rw")
    {
      rewrite_src = true;
    }
    else if (opts[i].arg == "-nodeutils")
    {
      gen_nodeutils = true;
    }
    else if (opts[i].arg == "-help")
    {
      help = true;
    }
  }

  if (help)
  {
    PrintUsage();
    return 0;
  }

  if (drvname_ok && path_ok && dbc_ok)
  {

    scanner = new DbcScanner;
    cigen = new CiMainGenerator;
    ciugen = new CiUtilGenerator;
    fscreator = new FsCreator;

    std::ifstream reader;

    std::cout << "dbc file : " << dbc_file_path << std::endl;
    std::cout << "gen path : " << source_files_out_path << std::endl;
    std::cout << "drv name : " << dbc_driver_name << std::endl;

    if (std::filesystem::exists(dbc_file_path) == false)
    {
      std::cout << "DBC file is not exists!" << std::endl;
      return -1;
    }

    reader.open(dbc_file_path);

    std::istream& s = reader;

    scanner->TrimDbcText(s);

    std::string info("");

    // create main destination directory
    auto ret = fscreator->PrepareDirectory(dbc_driver_name.c_str(), source_files_out_path.c_str(), rewrite_src, info);

    if (ret)
    {
      cigen->Generate(scanner->dblist, fscreator->FS);
    }
    else
    {
      std::cout << "One or both are invalid\n";
    }

#if defined (GEN_UTIL_CODE)

    // check if option --node-utils is requested, when requested binutil generation
    // wiil be performed on each node from DBC file in accordance to its RX / TX subscription
    if (gen_nodeutils)
    {
      std::vector<std::string> nodes;

      for (size_t num = 0; num < scanner->dblist.msgs.size(); num++)
      {
        // iterate all messages and collect All nodes assign to at least one message
        auto m = scanner->dblist.msgs[num];

        for (size_t txs = 0; txs < m->TranS.size(); txs++)
        {
          std::string tx_node_name = m->TranS[txs];

          if (std::find(nodes.begin(), nodes.end(), tx_node_name) == nodes.end())
          {
            // New node name. put it in the node collection
            nodes.push_back(tx_node_name);
          }
        }

        for (size_t recs = 0; recs < m->RecS.size(); recs++)
        {
          std::string rx_node_name = m->RecS[recs];

          // test all recs
          if (std::find(nodes.begin(), nodes.end(), rx_node_name) == nodes.end())
          {
            // New node name, put it in the node collection
            nodes.push_back(rx_node_name);
          }
        }
      }

      // for each node in collection perform specific bin-util generation
      for (size_t node = 0; node < nodes.size(); node++)
      {
        std::string util_name = nodes[node] + "_" + dbc_driver_name;

        fscreator->FS.util_c.dir = fscreator->FS.utildir;
        fscreator->FS.util_h.dir = fscreator->FS.utildir;

        fscreator->FS.util_h.fname = util_name + ".h";
        fscreator->FS.util_h.fpath = fscreator->FS.utildir + "/" + fscreator->FS.util_h.fname;

        fscreator->FS.util_c.fname = util_name + ".c";
        fscreator->FS.util_c.fpath = fscreator->FS.utildir + "/" + fscreator->FS.util_c.fname;

        MsgsClassification groups;

        for (size_t i = 0; i < scanner->dblist.msgs.size(); i++)
        {
          auto m = scanner->dblist.msgs[i];

          bool found = (std::find(m->TranS.begin(), m->TranS.end(), nodes[node]) != m->TranS.end());

          if (found)
          {
            // Message is in Tx array of current node
            groups.Tx.push_back(m->MsgID);
          }

          found = (std::find(m->RecS.begin(), m->RecS.end(), nodes[node]) != m->RecS.end());

          if (found)
          {
            // Message is in Rx array of current node
            groups.Rx.push_back(m->MsgID);
          }
        }

        if (ret)
        {
          ciugen->Generate(scanner->dblist, fscreator->FS, groups, dbc_driver_name);
        }
      }
    }
    else
    {
      MsgsClassification groups;

      for (size_t i = 0; i < scanner->dblist.msgs.size(); i++)
      {
        groups.Rx.push_back(scanner->dblist.msgs[i]->MsgID);
      }

      if (ret)
      {
        ciugen->Generate(scanner->dblist, fscreator->FS, groups, dbc_driver_name);
      }
    }

#endif

  }
  else
  {
    PrintUsage();
  }
}

void PrintUsage()
{
  std::cout << "C-CoderDbc v" << CODEGEN_LIB_VERSION_MAJ << "." << CODEGEN_LIB_VERSION_MIN << std::endl << std::endl;
  std::cout << "Project source code:\thttps://github.com/astand/c-coderdbc\t\t" << std::endl;
  std::cout << "Free web application:\thttps://coderdbc.com" << std::endl;
  std::cout << std::endl;
  std::cout << "Usage rules." << std::endl;
  std::cout << "To use code generator you need to provide three obligatory arguments:" << std::endl;

  std::cout << "   -dbc\t\t path to dbc file" << std::endl;
  std::cout << "   -out\t\t directory for generated source files." << std::endl;
  std::cout << "   \t\t (directory have to be created in advance)" << std::endl;
  std::cout << "   -drvname\t driver name - will be used for naming driver parts" << std::endl;
  std::cout << std::endl;
  std::cout << "Optional parameters:" << std::endl;
  std::cout << "   -nodeutils\t will generate specific pairs of binutils drivers for each node" << std::endl;
  std::cout << "   -rw\t\t by default each new generation with previously used params" << std::endl;
  std::cout << "   \t\t will create new directory with source files (000, 001, 002, ... etc)" << std::endl;
  std::cout << "   \t\t '-rw' option enables rewriting: all source files previously generated" << std::endl;
  std::cout << "   \t\t will be replaced by new ones." << std::endl;
  std::cout << std::endl;

  std::cout << "Examples:" << std::endl;
  std::cout << std::endl;

  std::cout <<
    "./dbccoder -dbc /home/user/docs/driveshaft.dbc -out /home/user/docs/gen/ -drvname drivedb -nodeutils -rw" << std::endl;

  std::cout << "./dbccoder -dbc /home/user/docs/driveshaft.dbc -out /home/user/docs/gen/ -drvname drivedb" << std::endl;
  std::cout << std::endl;
}
