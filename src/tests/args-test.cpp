#include "testapi.h"
#include <options-parser.h>

TEST(ArgParserTest, BasicAssert)
{
  static char* testchunks[] =
  {
    (char*)"appname",
    (char*)"-out",
    (char*)"path/to/out",
    (char*)"-dbc",
    (char*)"path/to/test.dbc",
    (char*)"-drvname",
    (char*)"testdbc",
    (char*)"-rw"
  };

  OptionsParser parser;
  auto ret = parser.GetOptions(9, testchunks);

  expect_true(ret.dbc.ok);
  expect_true(ret.dbc.value.compare("path/to/test.dbc") == 0);

  expect_true(ret.outdir.ok);
  expect_true(ret.outdir.value.compare("path/to/out") == 0);

  expect_true(ret.drvname.ok);
  expect_true(ret.drvname.value.compare("testdbc") == 0);
  expect_true(ret.is_rewrite);

  expect_false(ret.is_help);
  expect_false(ret.is_noconfig);
  expect_false(ret.is_nodeutils);
  expect_false(ret.is_nofmon);
  expect_false(ret.is_nocanmon);

}
