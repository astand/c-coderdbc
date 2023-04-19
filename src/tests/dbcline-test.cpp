#include <stdlib.h>
#include "testapi.h"
#include "parser/dbclineparser.h"
#include "helpers/formatter.h"


TEST(TestSigLineParsing, test1)
{
  DbcLineParser parser;

  std::string sigMultiplexMasterName;

  const std::string t2 = " SG_ FLT4_TEST_1 : 39|4@0+ (2.01,1E-002) [-0.01|30.14] \"\"  BCM";

  expect_true(parser.IsSignalLine(t2));

  SignalDescriptor_t dsc;

  parser.ParseSignalLine(&dsc, t2, sigMultiplexMasterName);

  expect_true(dsc.IsDoubleSig);
  expect_true(dsc.Offset == 0.01);

  const std::string t3 = " SG_ FLT4_TEST_1 : 39|4@0+ (2, 1) [-0.01|30.14] \"\"  BCM";

  parser.ParseSignalLine(&dsc, t3, sigMultiplexMasterName);

  expect_false(dsc.IsDoubleSig);
  expect_true(dsc.Offset == 1.0);

  // last supported double values
  const std::string t2_ok_norm = " SG_ FLT4_TEST_1 : 39|4@0+ (2, 0.000000001) [-0.01|30.14] \"\"  BCM";
  const std::string t2_ok_scie = " SG_ FLT4_TEST_1 : 39|4@0+ (2, 1e-9) [-0.01|30.14] \"\"  BCM";
  // next values (and less than) are not currently supported
  const std::string t2_nok_norm = " SG_ FLT4_TEST_1 : 39|4@0+ (2, 0.00000000099) [-0.01|30.14] \"\"  BCM";
  const std::string t2_nok_scie = " SG_ FLT4_TEST_1 : 39|4@0+ (2, 1e-10) [-0.01|30.14] \"\"  BCM";
  const std::string t3_nok_scie = " SG_ FLT4_TEST_1 : 39|4@0+ (1e-10, 44) [-0.01|30.14] \"\"  BCM";
  const std::string t3_nok_norm = " SG_ FLT4_TEST_1 : 39|4@0+ (0.00000000099, 2) [-0.01|30.14] \"\"  BCM";

  parser.ParseSignalLine(&dsc, t2_ok_norm, sigMultiplexMasterName);

  expect_true(dsc.IsDoubleSig);
  expect_eq(dsc.Offset, 0.000000001);
  expect_eq(dsc.Factor, 2.0);

  parser.ParseSignalLine(&dsc, t2_ok_scie, sigMultiplexMasterName);

  expect_true(dsc.IsDoubleSig);
  expect_eq(dsc.Offset, 0.000000001);
  expect_eq(dsc.Factor, 2.0);

  parser.ParseSignalLine(&dsc, t3_nok_norm, sigMultiplexMasterName);

  expect_false(dsc.IsDoubleSig);
  expect_eq(dsc.Offset, 0.0);
  expect_eq(dsc.Factor, 1.0);

  parser.ParseSignalLine(&dsc, t3_nok_scie, sigMultiplexMasterName);

  expect_false(dsc.IsDoubleSig);
  expect_eq(dsc.Offset, 0.0);
  expect_eq(dsc.Factor, 1.0);

}

TEST(TestSigLineParsing, test_02)
{
  const std::string t3_ok = " SG_ FLT4_TEST_1 : 39|4@0+ (0.99, 0) [-0.01|30.14] \"\"  BCM";
  const std::string t4_ok = " SG_ FLT4_TEST_1 : 39|4@0+ (0, -0.11) [-0.01|30.14] \"\"  BCM";
  const std::string t5_notok = " SG_ FLT4_TEST_1 : 39|4@0+ (0.00000000099, 0) [-0.01|30.14] \"\"  BCM";
  const std::string t6_ok = " SG_ FLT4_TEST_1 : 39|4@0+ (0, 0.000000000000) [-0.01|30.14] \"\"  BCM";

  DbcLineParser parser;

  SignalDescriptor_t dsc;

  std::string sigMultiplexMasterName;

  parser.ParseSignalLine(&dsc, t3_ok, sigMultiplexMasterName);

  expect_true(dsc.IsDoubleSig);

  parser.ParseSignalLine(&dsc, t4_ok, sigMultiplexMasterName);

  expect_true(dsc.IsDoubleSig);
  expect_eq(dsc.Factor, 0.0);
  expect_eq(dsc.Offset, -0.11);

  parser.ParseSignalLine(&dsc, t5_notok, sigMultiplexMasterName);

  expect_false(dsc.IsDoubleSig);
  expect_eq(dsc.Factor, 1.0);
  expect_eq(dsc.Offset, 0.0);

  parser.ParseSignalLine(&dsc, t6_ok, sigMultiplexMasterName);

  expect_true(dsc.IsDoubleSig);
  expect_eq(dsc.Factor, 0.0);
  expect_eq(dsc.Offset, 0.0);

}

TEST(TestSigLineParsing, test_prt_double)
{
  constexpr double v = -124.10001110002220;

  expect_eq(prt_double(v, 0, false), "-124");
  expect_eq(prt_double(v, 1, false), "-124.1");
  expect_eq(prt_double(v, 2, false), "-124.1");
  expect_eq(prt_double(v, 3, false), "-124.1");
  expect_eq(prt_double(v, 4, false), "-124.1");
  expect_eq(prt_double(v, 5, false), "-124.10001");
  expect_eq(prt_double(v, 6, false), "-124.100011");
  expect_eq(prt_double(v, 7, false), "-124.1000111");
  expect_eq(prt_double(v, 8, false), "-124.1000111");
  expect_eq(prt_double(v, 9, false), "-124.1000111");

  constexpr double vint = 123.0000;

  expect_eq(prt_double(vint, 3), "123.0");
  expect_eq(prt_double(vint, 2), "123.0");
  expect_eq(prt_double(vint, 1), "123.0");
  expect_eq(prt_double(vint, 0), "123.0");
  expect_eq(prt_double(vint, 0, false), "123");
  expect_eq(prt_double(vint, 1, false), "123");
  expect_eq(prt_double(vint, 100), "123.0");
  expect_eq(prt_double(vint, 1000), "123.0");

  constexpr double v2 = 0.0110022;

  expect_eq(prt_double(v2, 0), "0.0");

  constexpr double v3 = -20.47;

  expect_eq(prt_double(v3, 2), "-20.47");
  expect_eq(prt_double(v3, 10), "-20.47");

  constexpr double v4 = 20.4699999999;

  expect_eq(prt_double(v4, 9), "20.469999999");
  expect_eq(prt_double(v4, 8), "20.46999999");
  expect_eq(prt_double(v4, 7), "20.4699999");
  expect_eq(prt_double(v4, 3), "20.469");

  constexpr double v5 = -20.4699999999;

  expect_eq(prt_double(v5, 8), "-20.46999999");
  expect_eq(prt_double(v5, 7), "-20.4699999");
  expect_eq(prt_double(v5, 3), "-20.469");
  expect_eq(prt_double(v5, 10), "-20.4699999999");
  expect_eq(prt_double(v5, 11), "-20.4699999999");
  expect_eq(prt_double(v5, 15), "-20.4699999999");

  constexpr double v6 = 123.012345678900000;
  expect_eq(prt_double(v6, 10), "123.0123456789");
  expect_eq(prt_double(v6, 11), "123.0123456789");
  expect_eq(prt_double(v6, 15), "123.0123456789");
}

TEST(TestMuxParsing, test_mux)
{
  const std::string msg_mux = "SBO_ 546 FOO_MUXED_M: 6 FOO_NODE";

  std::vector<std::string> sigLines;
  sigLines.push_back("SG_ FOO_Mux1 M : 0|8@1+ (1,0) [0|4] \"\" BCM");
  sigLines.push_back("SG_ FOO_Val_S8_L8_G0 m0 : 8|8@1+ (1,0) [0|255] "" BCM");
  sigLines.push_back("SG_ FOO_Val_S8_L8_G1 m1 : 8|8@1+ (1,0) [0|255] "" BCM");
  sigLines.push_back("SG_ FOO_Mux2 M : 0|8@1+ (1,0) [0|4] \"\" BCM");
  sigLines.push_back("SG_ FOO_Val_S24_L8_G0 m0 : 8|8@1+ (1,0) [0|255] "" BCM");
  sigLines.push_back("SG_ FOO_Val_S24_L8_G1 m1 : 8|8@1+ (1,0) [0|255] "" BCM");

  DbcLineParser parser;

  MessageDescriptor_t msg;
  
  // parse message
  parser.ParseMessageLine(&msg, msg_mux);

  // parse signals
  std::string sigMultiplexMasterName;
  for (auto & element : sigLines) 
  {
    SignalDescriptor_t sig;

    parser.ParseSignalLine(&sig, element, sigMultiplexMasterName);
    msg.Signals.push_back(sig);
  }

  expect_true(msg.Signals[0].Multiplex == MultiplexType::kMaster);
  expect_true(msg.Signals[1].Multiplex == MultiplexType::kMulValue);
  expect_true(msg.Signals[2].Multiplex == MultiplexType::kMulValue);
  
  expect_true(msg.Signals[1].MultiplexGroup == 0);
  expect_true(msg.Signals[2].MultiplexGroup == 1);

  expect_eq(msg.Signals[1].MultiplexName, "FOO_Mux1");
  expect_eq(msg.Signals[2].MultiplexName, "FOO_Mux1");

  expect_true(msg.Signals[3].Multiplex == MultiplexType::kMaster);
  expect_true(msg.Signals[4].Multiplex == MultiplexType::kMulValue);
  expect_true(msg.Signals[5].Multiplex == MultiplexType::kMulValue);
  
  expect_true(msg.Signals[4].MultiplexGroup == 0);
  expect_true(msg.Signals[5].MultiplexGroup == 1);

  expect_eq(msg.Signals[4].MultiplexName, "FOO_Mux2");
  expect_eq(msg.Signals[5].MultiplexName, "FOO_Mux2");
}