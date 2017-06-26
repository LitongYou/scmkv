#ifndef _GEN_H

#include <random>


template<typename T>
class Generator {
    public:
      virtual ~Generator() {};
      virtual T next() = 0;
};

template<typename T>
class UniformGenerator : public Generator<T> {
    public:
        UniformGenerator(T range): _dist(0, range) {}
        T next() {
            return _dist(_gi);
        }
    private:
        std::default_random_engine _gi;
        std::uniform_int_distribution<T> _dist;
};


class ZipfianGenerator: public Generator<uint64_t> {
    public:
        ZipfianGenerator(uint64_t min, uint64_t max);
        uint64_t next();
    private:
        double zeta(const uint64_t n, const double theta);
        double zeta_range(const uint64_t start, const uint64_t count, const double theta);

        uint64_t _base;
        uint64_t _items;
        double _zpf_const;
        double _alpha;
        double _zetan;
        double _eta;
        double _theta;
        double _zeta2theta;

        uint64_t _countforzeta;

        std::default_random_engine _gi;
        std::uniform_real_distribution<double> _dist;

        static const double kZpfConst;
        static const uint64_t zetalist_step = UINT64_C(0x10000000000);
        static const uint64_t zetalist_count = 16;

        static const uint64_t zetalist_double[];
};


#endif

