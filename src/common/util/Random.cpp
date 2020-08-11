#include "Random.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/uniform_smallint.hpp>

#include <mutex>


typedef std::lock_guard<std::mutex> lock_guard;
typedef std::mt19937 GeneratorType;


namespace {
    GeneratorType gen{2462343}; // the one random number generator driving the distributions below. arbitrarily chosen default seed
    static std::mutex s_prng_mutex;
}

void Seed(unsigned int seed) {
    lock_guard lock(s_prng_mutex);
    gen.seed(static_cast<GeneratorType::result_type>(seed));
}

void ClockSeed() {
    lock_guard lock(s_prng_mutex);
    boost::posix_time::time_duration diff = boost::posix_time::microsec_clock::local_time().time_of_day();
    gen.seed(static_cast<GeneratorType::result_type>(diff.total_milliseconds()));
}

int RandInt(int min, int max) {
    if (min >= max)
        return min;
    {
        lock_guard lock(s_prng_mutex);
        static boost::random::uniform_smallint<> dis;
        return dis(gen, decltype(dis)::param_type{min, max});
    }
}

double RandZeroToOne() {
    lock_guard lock(s_prng_mutex);
    static boost::random::uniform_01<> dis;
    return dis(gen);
}

double RandDouble(double min, double max) {
    if (min >= max)
        return min;
    {
        lock_guard lock(s_prng_mutex);
        static boost::random::uniform_real_distribution<> dis;
        return dis(gen, decltype(dis)::param_type{min, max});
    }
}

double RandGaussian(double mean, double sigma) {
    if (sigma <= 0.0)
        return mean;
    {
        lock_guard lock(s_prng_mutex);
        static boost::random::normal_distribution<> dis;
        return dis(gen, decltype(dis)::param_type{mean, sigma});
    }
}

void RandomShuffle(std::vector<bool>& c) {
    lock_guard lock(s_prng_mutex);
    std::shuffle(c.begin(), c.end(), gen);
}

void RandomShuffle(std::vector<int>& c) {
    lock_guard lock(s_prng_mutex);
    std::shuffle(c.begin(), c.end(), gen);
}
