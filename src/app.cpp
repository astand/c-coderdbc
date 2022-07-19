#include "app.h"
#include <iostream>
#include <memory>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <stdlib.h>
#include "app.h"
#include "parser/dbcscanner.h"
#include "codegen/c-main-generator.h"
#include "codegen/c-util-generator.h"
#include "codegen/fs-creator.h"
#include "codegen/version.h"
#include <codegen/version.h>
#include <helpers/formatter.h>

void CoderApp::Run()
{
  if (ParseParams())
  {
    GenerateCode();
  }
  else
  {
    PrintHelp();
  }
}

bool CoderApp::ParseParams()
{
  for (size_t i = 0; i < Params.size(); i++)
  {
    if (Params[i].first.compare("-dbc") == 0)
    {
      dbc.value = Params[i].second;
      dbc.ok = true;
    }
    else if (Params[i].first.compare("-out") == 0)
    {
      outdir.value = Params[i].second;
      outdir.ok = true;
    }
    else if (Params[i].first.compare("-drvname") == 0)
    {
      drvname.value = make_c_name(Params[i].second);
      drvname.ok = true;

      if (drvname.value.length() == 0)
      {
        drvname.ok = false;
      }
    }
    else if (Params[i].first.compare("-rw") == 0)
    {
      rewrite_src = true;
    }
    else if (Params[i].first.compare("-nodeutils") == 0)
    {
      gen_nodeutils = true;
    }
    else if (Params[i].first.compare("-help") == 0)
    {
      return false;
    }
  }

  return (dbc.ok && outdir.ok && drvname.ok);
}

void CoderApp::GenerateCode()
{
  auto scanner = std::make_unique<DbcScanner>();
  auto cigen = std::make_unique<CiMainGenerator>();
  auto ciugen = std::make_unique<CiUtilGenerator>();
  auto fscreator = std::make_unique<FsCreator>();

  std::ifstream reader;

  std::cout << "dbc file : " << dbc.value << std::endl;
  std::cout << "gen path : " << outdir.value << std::endl;
  std::cout << "drv name : " << drvname.value << std::endl;

  if (std::filesystem::exists(dbc.value) == false)
  {
    std::cout << "DBC file is not exists!" << std::endl;
    return;
  }

  reader.open(dbc.value);

  std::istream& s = reader;

  scanner->TrimDbcText(s);

  std::string info("");

  // create main destination directory
  auto ret = fscreator->PrepareDirectory(drvname.value.c_str(), outdir.value.c_str(), rewrite_src, info);

  if (ret)
  {
    cigen->Generate(scanner->dblist, fscreator->FS);
  }
  else
  {
    std::cout << "One or both are invalid\n";
  }

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
      std::string util_name = nodes[node] + "_" + drvname.value;

      // set new driver name for current node
      fscreator->FS.gen.drvname = str_tolower(util_name);
      fscreator->FS.gen.DRVNAME = str_toupper(fscreator->FS.gen.drvname);
      fscreator->FS.file.util_c.dir = fscreator->FS.file.utildir;
      fscreator->FS.file.util_h.dir = fscreator->FS.file.utildir;

      fscreator->FS.file.util_h.fname = str_tolower(fscreator->FS.gen.drvname + "-binutil.h");
      fscreator->FS.file.util_h.fpath = fscreator->FS.file.utildir + "/" + fscreator->FS.file.util_h.fname;

      fscreator->FS.file.util_c.fname = str_tolower(fscreator->FS.gen.drvname + "-binutil.c");
      fscreator->FS.file.util_c.fpath = fscreator->FS.file.utildir + "/" + fscreator->FS.file.util_c.fname;

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
        ciugen->Generate(scanner->dblist, fscreator->FS, groups, drvname.value);
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
      ciugen->Generate(scanner->dblist, fscreator->FS, groups, drvname.value);
    }
  }
}


void CoderApp::PrintHelp()
{
  std::cout << "coderdbc v" << CODEGEN_LIB_VERSION_MAJ << "." << CODEGEN_LIB_VERSION_MIN << std::endl << std::endl;
  std::cout << "project source code:\thttps://github.com/astand/c-coderdbc\t\t" << std::endl;
  std::cout << "free web application:\thttps://coderdbc.com" << std::endl;
  std::cout << std::endl;
  std::cout << "required parameters:" << std::endl;

  std::cout << "   -dbc\t\t path to dbc file" << std::endl;
  std::cout << "   -out\t\t directory for generated source files (must be pre-created)" << std::endl;
  std::cout << "   -drvname\t driver name - will be used for naming driver parts" << std::endl;
  std::cout << std::endl;
  std::cout << "optional parameters:" << std::endl;
  std::cout << "   -nodeutils\t will generate specific pairs of binutils drivers for each node" << std::endl;
  std::cout << "   -rw\t\t by default each new generation with previously used params" << std::endl;
  std::cout << "   \t\t will create new sud-directory with source files (000, 001, ... etc)" << std::endl;
  std::cout << "   \t\t '-rw' option enables rewriting: all source files previously generated" << std::endl;
  std::cout << "   \t\t will be replaced by new ones" << std::endl;
  std::cout << std::endl;

  std::cout << "examples:" << std::endl;
  std::cout << std::endl;

  std::cout <<
    "./dbccoder -dbc /home/user/docs/driveshaft.dbc -out /home/user/docs/gen/ -drvname drivedb -nodeutils -rw" << std::endl;

  std::cout <<
    "./dbccoder -dbc /home/user/docs/driveshaft.dbc -out /home/user/docs/gen/ -drvname drivedb -nodeutils" << std::endl;

  std::cout << "./dbccoder -dbc /home/user/docs/driveshaft.dbc -out /home/user/docs/gen/ -drvname drivedb" << std::endl;
  std::cout << std::endl;
}
