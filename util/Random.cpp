// Use the same generator for every source-file, otherwise we get problems
// This is an ugly hack, and I hate it, but it works.
#include "Random.h"

namespace Util_Random {
    GeneratorType gen;
    boost::uniform_01<GeneratorType> zero_to_one_gen(gen);
}
