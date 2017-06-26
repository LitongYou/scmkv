#include <cmath>
#include <cstdint>
#include "gen.h"

using std::pow;

const double ZipfianGenerator::kZpfConst = 0.99;
const uint64_t ZipfianGenerator::zetalist_double[] = {0,
    UINT64_C(0x4040437dd948c1d9), UINT64_C(0x4040b8f8009bce85),
    UINT64_C(0x4040fe1121e564d6), UINT64_C(0x40412f435698cdf5),
    UINT64_C(0x404155852507a510), UINT64_C(0x404174d7818477a7),
    UINT64_C(0x40418f5e593bd5a9), UINT64_C(0x4041a6614fb930fd),
    UINT64_C(0x4041bab40ad5ec98), UINT64_C(0x4041cce73d363e24),
    UINT64_C(0x4041dd6239ebabc3), UINT64_C(0x4041ec715f5c47be),
    UINT64_C(0x4041fa4eba083897), UINT64_C(0x4042072772fe12bd),
    UINT64_C(0x4042131f5e380b72), UINT64_C(0x40421e53630da013),
};


uint64_t ZipfianGenerator::next() {
  const double u = _dist(_gi);
  const double uz = u * _zetan;
  if (uz < 1.0) {
    return _base + 0lu;
  } else if (uz < (1.0 + pow(0.5, _theta))) {
    return _base + 1lu;
  }
  const double x = ((double)_items) * pow(_eta * (u - 1.0) + 1.0, _alpha);
  const uint64_t ret = _base + (uint64_t)x;
  return ret;
}

double ZipfianGenerator::zeta_range(const uint64_t start, const uint64_t count, const double theta)
{
  double sum = 0.0;
  if (count > 0x10000000) {
    fprintf(stderr, "zeta_range would take a long time... kill me our wait\n");
  }
  for (uint64_t i = 0lu; i < count; i++) {
    sum += (1.0 / pow((double)(start + i + 1lu), theta));
  }
  return sum;
}

double ZipfianGenerator::zeta(const uint64_t n, const double theta)
{
  //assert(theta == zetalist_theta);
  const uint64_t zlid0 = n / zetalist_step;
  const uint64_t zlid = (zlid0 > zetalist_count) ? zetalist_count : zlid0;
  const double sum0 = zetalist_double[zlid];
  const uint64_t start = zlid * zetalist_step;
  const uint64_t count = n - start;
  //assert(n > start);
  const double sum1 = zeta_range(start, count, theta);
  return sum0 + sum1;
}

ZipfianGenerator::ZipfianGenerator(uint64_t min, uint64_t max):
    _base(min), _items(max - min + 1), _zpf_const(kZpfConst),
    _alpha(1 / (1.0 - kZpfConst)), _theta(kZpfConst)
{
    _zetan = zeta(_items, kZpfConst);
    _zeta2theta = zeta(2, kZpfConst);
    _eta = (1 - pow(2 / _items, 1 - kZpfConst)) /
            (1 - (_zeta2theta / _zetan));
    _countforzeta =  _items;
}
