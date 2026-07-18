#include "Parse.h"

#include "PythonParserImpl.h"
#include "ValueRefPythonParser.h"
#include "ConditionPythonParser.h"
#include "EnumPythonParser.h"
#include "SourcePythonParser.h"

#include "../universe/NamedValueRefManager.h"
#include "../universe/ValueRefs.h"
#include "../universe/ValueRef.h"
#include "../util/Directories.h"

#include <boost/python/import.hpp>

namespace parse {

    using start_rule_payload = NamedValueRefManager::NamedValueRefParseMap;

    struct py_grammar {
        py_grammar(const PythonParser& parser)
        {
            parser.LoadValueRefsModule();
        }
    };
}

namespace parse {
    start_rule_payload named_value_refs(const PythonParser& parser, const std::filesystem::path& path, bool& success) {
        start_rule_payload named_value_refs;

        ScopedTimer timer("Named ValueRef Parsing");

        bool file_success = true;
        py_grammar p = py_grammar(parser);
        for (const auto& file : ListDir(path, IsFOCPyScript))
            file_success = file_success && py_parse::detail::parse_file<py_grammar>(parser, file, p);

        for (auto& k_v : named_value_refs)
            ErrorLogger() << "Should have not returned anything: named_value_refs : " << k_v.first;

        success = file_success;
        return named_value_refs;
    }
}
