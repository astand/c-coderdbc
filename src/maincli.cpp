
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "parser/dbcscanner.h"
#include "codegen/c-main-generator.h"
#include "codegen/c-util-generator.h"
#include "codegen/fs-creator.h"
#include "codegen/version.h"

#define GEN_UTIL_CODE

#define MIN_ARGC_NUM 4
#define MAX_ARGC_NUM 5

char verstr[128] = {0};

const char* helptext =
  "https://github.com/astand/c-coderdbc        (project source code)\n\n"
  "https://coderdbc.com                        (web application)\n\n"
  "To use utility you need to provide 3 arguments:\n\n"
  "1. dbc file path\n"
  "2. directory for generated source files (existable)\n"
  "3. prefix (driver name) which will be used for naming dirver parts\n"
  "4. (optional) --node-utils will generate specific pairs of binutils drivers for each node"
  "   which is either transmitter or receiver of at least one frame from dbc.\n\n"
  "Usage example:\n\n./dbccoder /home/user/docs/driveshaft.dbc /home/user/docs/gen/ drivedb --node-utils\n\n";

DbcScanner* scanner;
CiMainGenerator* cigen;
CiUtilGenerator* ciugen;
FsCreator* fscreator;

std::string source_files_out_path;
std::string dbc_file_path;
std::string dbc_driver_name;

int main(int argc, char* argv[])
{
  scanner = new DbcScanner;
  cigen = new CiMainGenerator;
  ciugen = new CiUtilGenerator;
  fscreator = new FsCreator;

  std::snprintf(verstr, 128, "\nDbccoder v%u.%u\n\n", CODEGEN_LIB_VERSION_MAJ, CODEGEN_LIB_VERSION_MIN);
  std::cout << verstr;

  if (argc >= MIN_ARGC_NUM)
  {
    std::ifstream reader;
    // copy dbc file name to string variable
    dbc_file_path = argv[1];
    source_files_out_path = argv[2];
    dbc_driver_name = argv[3];

    std::cout << "dbc file : " << argv[1] << std::endl;
    std::cout << "gen path : " << argv[2] << std::endl;
    std::cout << "drv name : " << argv[3] << std::endl;

    if (std::filesystem::exists(dbc_file_path) == false)
    {
      std::cout << "DBC file is not exists!" << std::endl;
      return -1;
    }

    reader.open(dbc_file_path);

    std::istream& s = reader;

    scanner->TrimDbcText(s);

    std::string info("");

    auto ret = fscreator->PrepareDirectory(dbc_driver_name.c_str(), source_files_out_path.c_str(), true, info);

    if (ret)
    {
      cigen->Generate(scanner->dblist, fscreator->FS);
    }
    else
    {
      std::cout << "One or both are invalid\n";
    }

#if defined (GEN_UTIL_CODE)

    std::string node_util = "--node-utils";

    // check if option --node-utils is requested, when requested binutil generation
    // wiil be performed on each node from DBC file in accordance to its RX / TX subscription
    if (argc == MAX_ARGC_NUM && node_util.compare(argv[4]) == 0)
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

        ret = fscreator->PrepareDirectory(util_name.c_str(), source_files_out_path.c_str(), true, info);

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
      ret = fscreator->PrepareDirectory(dbc_driver_name.c_str(), (source_files_out_path).c_str(), true, info);

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
    std::cout << helptext;
  }
}
