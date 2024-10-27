#include "Random.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/uniform_smallint.hpp>

#include <mutex>
#include <utility>
#if !defined(__cpp_lib_integer_comparison_functions)
namespace std {
    constexpr auto cmp_less(auto&& lhs, auto&& rhs) { return lhs < rhs; }
}
#endif

using GeneratorType = std::mt19937;

namespace {
    GeneratorType gen{2462343}; // the one random number generator driving the distributions below. arbitrarily chosen default seed
    static std::mutex s_prng_mutex;
}

namespace StaticTests {
    using namespace RandomImpl;

    constexpr auto test_num_hash0 = hash(uint8_t{0});
    static_assert(test_num_hash0 == 220086046u);
    constexpr auto test_num_hash1 = hash(uint8_t{1});
    static_assert(test_num_hash1 == 555565661u);

    constexpr std::array<char, 9> zt{"00:00:00"};
    constexpr std::array test_nums{CxPRNG(0, zt), CxPRNG(1, zt) % 20, CxPRNG(2, zt) % 20, CxPRNG(3, zt) % 20,
        CxPRNG(4, zt) % 20, CxPRNG(5, zt) % 20, CxPRNG(6, zt) % 20, CxPRNG(7, zt) % 20};
    static_assert(test_nums == std::array{0u, 6u, 15u, 19u, 7u, 5u, 8u, 16u});

    constexpr std::array more_test_nums{RandIntCx(0, 10000, 0, zt), RandIntCx(0, 10000, 1, zt),
                                        RandIntCx(0, 10000, 2, zt), RandIntCx(0, 10000, 3, zt)};
    static_assert(more_test_nums == std::array{0, 4040, 6150, 9697});

    consteval auto GetTestDoubles() {
        std::array<double, 32> doubles{};
        for (int16_t i = 0; i < static_cast<int16_t>(doubles.size()); ++i)
            doubles[static_cast<std::size_t>(i)] = RandDoubleCx(10.0, 1000.0, i);
        return doubles;
    }
    consteval bool CheckTestDoublesInRange() {
        const auto doubles = GetTestDoubles();
        const auto [mind, maxd] = std::minmax_element(doubles.begin(), doubles.end());
        return *mind >= 10.0 && *maxd <= 1000.0 && *mind < *maxd;
    }

#if !defined(_MSC_VER) || (_MSC_VER >= 1936)  // https://developercommunity.visualstudio.com/t/constexpr-constructorstdnullptr-t-causes-error-c21/41791
    static_assert(TimeToInt(std::array<char, 9>{"23:59:59"}) + 1 == 86400);
    static_assert(RandBoolCx() == RandBoolCx()); // same result for multiple calls in single translation unit
    static_assert(CheckTestDoublesInRange());
#endif
}

void Seed(unsigned int seed) {
    std::scoped_lock lock(s_prng_mutex);
    gen.seed(static_cast<GeneratorType::result_type>(seed));
}

void ClockSeed() {
    std::scoped_lock lock(s_prng_mutex);
    boost::posix_time::time_duration diff = boost::posix_time::microsec_clock::local_time().time_of_day();
    gen.seed(static_cast<GeneratorType::result_type>(diff.total_milliseconds()));
}

int RandInt(int min, int max) {
    if (min >= max)
        return min;
    {
        std::scoped_lock lock(s_prng_mutex);
        static const boost::random::uniform_smallint<> dis;
        return dis(gen, decltype(dis)::param_type{min, max});
    }
}

double RandZeroToOne() {
    std::scoped_lock lock(s_prng_mutex);
    static boost::random::uniform_01<> dis;
    return dis(gen);
}

double RandDouble(double min, double max) {
    if (min >= max)
        return min;
    {
        std::scoped_lock lock(s_prng_mutex);
        static const boost::random::uniform_real_distribution<> dis;
        return dis(gen, decltype(dis)::param_type{min, max});
    }
}

double RandGaussian(double mean, double sigma) {
    if (sigma <= 0.0)
        return mean;
    {
        std::scoped_lock lock(s_prng_mutex);
        static boost::random::normal_distribution<> dis;
        return dis(gen, decltype(dis)::param_type{mean, sigma});
    }
}

void RandomShuffle(std::vector<uint8_t>& c) {
    std::scoped_lock lock(s_prng_mutex);
    std::shuffle(c.begin(), c.end(), gen);
}

void RandomShuffle(std::vector<int>& c) {
    std::scoped_lock lock(s_prng_mutex);
    std::shuffle(c.begin(), c.end(), gen);
}
