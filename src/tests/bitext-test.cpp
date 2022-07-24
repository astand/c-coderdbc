#include <stdlib.h>
#include "testapi.h"

template<typename iT, typename uT>
static iT bitext(uT val, uint8_t bits)
{
  uT const m = 1u << (bits - 1);
  return (val ^ m) - m;
}

TEST(TestBitExt, FullTest)
{
  // 1 test: 3 bits signal
  static const int32_t cmp[] {0, 1, 2, 3, -4, -3, -2, -1};

  for (auto val = 0; val < 8; val++)
  {
    auto ret = bitext<int32_t, uint32_t>(val, 3);
    expect_eq(ret, cmp[val]);
  }

  static const uint8_t val2[] { 126, 127,  128,  129, 255, 0, 1};
  static const int32_t cmp2[] { 126, 127, -128, -127,  -1, 0, 1};

  for (uint8_t i = 0; i < 7; i++)
  {
    auto ret = bitext<int32_t, uint32_t>(val2[i], 8);
    expect_eq(ret, cmp2[i]);
  }
}
