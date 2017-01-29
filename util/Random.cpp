#include "Random.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/mutex.hpp>

namespace {
    GeneratorType gen; // the one random number generator driving the distributions below

    static boost::mutex s_prng_mutex;
}

void Seed(unsigned int seed) {
    boost::mutex::scoped_lock lock(s_prng_mutex);
    gen.seed(static_cast<boost::mt19937::result_type>(seed));
}

void ClockSeed() {
    boost::mutex::scoped_lock lock(s_prng_mutex);
    boost::posix_time::time_duration diff = boost::posix_time::microsec_clock::local_time().time_of_day();
    gen.seed(static_cast<boost::mt19937::result_type>(diff.total_milliseconds()));
}

SmallIntDistType SmallIntDist(int min, int max) {
    boost::mutex::scoped_lock lock(s_prng_mutex);
    return SmallIntDistType(gen, boost::uniform_smallint<>(min, max));
}

IntDistType IntDist(int min, int max) {
    boost::mutex::scoped_lock lock(s_prng_mutex);
    return IntDistType(gen, boost::uniform_int<>(min, max));
}

DoubleDistType DoubleDist(double min, double max) {
    boost::mutex::scoped_lock lock(s_prng_mutex);
    return DoubleDistType(gen, boost::uniform_real<>(min, max));
}

GaussianDistType GaussianDist(double mean, double sigma) {
    boost::mutex::scoped_lock lock(s_prng_mutex);
    return GaussianDistType(gen, boost::normal_distribution<>(mean, sigma));
}

int RandSmallInt(int min, int max)
{ return (min == max ? min : SmallIntDist(min,max)()); }

int RandInt(int min, int max)
{ return (min == max ? min : IntDist(min, max)()); }

double RandZeroToOne()
{ return DoubleDist(0.0, 1.0)(); }

double RandDouble(double min, double max)
{ return (min == max ? min : DoubleDist(min, max)()); }

double RandGaussian(double mean, double sigma)
{ return GaussianDist(mean, sigma)(); }
