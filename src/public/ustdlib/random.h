#ifndef USTDLIB_RANDOM_H
#define USTDLIB_RANDOM_H
#include <stdint.h>

namespace ustd
{
    class IRandomStream {
    public:
        virtual void init(uint64_t seed) = 0;
        virtual uint64_t getInt(void) = 0;
        virtual uint64_t getInt(uint64_t min, uint64_t max) = 0;
        virtual double getDouble(void) = 0;
        virtual double getDouble(double min, double max) = 0;
    };

    static const uint64_t MT_NN = 312;
    static const uint64_t MT_MM = 156;
    static const uint64_t MT_UM = 0xFFFFFFFF80000000;
    static const uint64_t MT_LM = 0x000000007FFFFFFF;
    static const uint64_t MT_MA = 0xB5026F5AA96619E9;

    class MT19937_RandomStream : public IRandomStream {
    public:
        MT19937_RandomStream(void);
        MT19937_RandomStream(uint64_t seed);
        void init(uint64_t seed = 5489);
        void twist(void);

        uint64_t getInt(void);
        uint64_t getInt(uint64_t min, uint64_t max);
        double getDouble(void);
        double getDouble(double min, double max);

    private:
        uint64_t state[MT_NN];
        uint64_t index = 0;
    };

    static const uint64_t LC_A = 0x5851F42D4C957F2D;
    static const uint64_t LC_C = 1;

    class LCG_RandomStream : public IRandomStream {
    public:
        LCG_RandomStream(void);
        LCG_RandomStream(uint64_t seed);
        void init(uint64_t seed);

        uint64_t getInt(void);
        uint64_t getInt(uint64_t min, uint64_t max);
        double getDouble(void);
        double getDouble(double min, double max);

    private:
        uint64_t state = 0;
    };
}

#endif
