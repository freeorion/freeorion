// Use the same generator for every source-file, otherwise we get problems
// This is an ugly hack, and I hate it, but it works.
#include "Random.h"

#include "MultiplayerCommon.h"


namespace {
    bool temp_header_bool = RecordHeaderFile(RandomRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


namespace Util_Random {
    GeneratorType gen;
    boost::uniform_01<GeneratorType> zero_to_one_gen(gen);
}
