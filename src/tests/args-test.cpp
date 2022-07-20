#include <gtest/gtest.h>
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
    (char*)"-rw"
  };

  OptionsParser parser;
  auto ret = parser.GetOptions(6, testchunks);
  EXPECT_TRUE(ret.size() == 3);

  EXPECT_EQ(ret[0].first.compare("-out"), 0);
  EXPECT_EQ(ret[0].second.compare("path/to/out"), 0);
  EXPECT_EQ(ret[1].first.compare("-dbc"), 0);
  EXPECT_EQ(ret[1].second.compare("path/to/test.dbc"), 0);
  EXPECT_EQ(ret[2].first.compare("-rw"), 0);
  EXPECT_EQ(ret[2].second.size(), 0);

  EXPECT_EQ(1, 1);
}