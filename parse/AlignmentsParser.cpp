#include "Parse.h"

#include "ParseImpl.h"

#include "../Empire/Empire.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<Alignment>&) { return os; }
    inline ostream& operator<<(ostream& os, const Alignment&) { return os; }
}
#endif

namespace {
    std::vector<std::shared_ptr<Effect::EffectsGroup>>* g_effects_groups = nullptr;

    struct rules {
        rules() {
            const parse::lexer& tok = parse::lexer::instance();

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_r1_type _r1;
            using phoenix::construct;
            using phoenix::push_back;

            alignment
                =    tok.Alignment_
                >    parse::detail::label(Name_token)        > tok.string [ _a = _1 ]
                >    parse::detail::label(Description_token) > tok.string [ _b = _1 ]
                >    parse::detail::label(Graphic_token)     > tok.string [ push_back(_r1, construct<Alignment>(_a, _b, _1)) ]
                ;

            start
                =   +(
                            alignment(_r1)
                        >  -(
                                tok.AlignmentEffects_
                            >   parse::detail::label(EffectsGroups_token) > parse::detail::effects_group_parser() [ phoenix::ref(*g_effects_groups) = _1 ]
                            )
                     )
                ;

            alignment.name("Alignment");

#if DEBUG_PARSERS
            debug(alignment);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef parse::detail::rule<
            void (std::vector<Alignment>&),
            qi::locals<
                std::string,
                std::string
            >
        > alignment_rule;

        typedef parse::detail::rule<
            void (std::vector<Alignment>&)
        > start_rule;

        alignment_rule alignment;
        start_rule start;
    };
}

namespace parse {
    bool alignments(std::vector<Alignment>& alignments_,
                    std::vector<std::shared_ptr<Effect::EffectsGroup>>& effects_groups)
    {
        bool result = true;

        g_effects_groups = &effects_groups;

        for (const boost::filesystem::path& file : ListScripts("scripting/alignments")) {
            result &= detail::parse_file<rules, std::vector<Alignment> >(file, alignments_);
        }

        return result;
    }
}
