#include "ParseImpl.h"

#include "Label.h"
#include "../Empire/Empire.h"


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<Alignment>&) { return os; }
    inline ostream& operator<<(ostream& os, const Alignment&) { return os; }
}
#endif

namespace {
    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >* g_effects_groups = 0;

    struct rules
    {
        rules()
        {
            const parse::lexer& tok = parse::lexer::instance();

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_r1_type _r1;
            using phoenix::construct;
            using phoenix::new_;
            using phoenix::push_back;

            alignment
                =    tok.Alignment_
                >    parse::label(Name_name)        > tok.string [ _a = _1 ]
                >    parse::label(Description_name) > tok.string [ _b = _1 ]
                >    parse::label(Graphic_name)     > tok.string [ push_back(_r1, construct<Alignment>(_a, _b, _1)) ]
                ;

            start
                =   +(
                            alignment(_r1)
                        >> -(
                                tok.AlignmentEffects_
                            >   parse::label(EffectsGroups_name) > parse::detail::effects_group_parser() [ phoenix::ref(*g_effects_groups) = _1 ]
                            )
                        )
                ;

            alignment.name("Alignment");

#if DEBUG_PARSERS
            debug(alignment);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::vector<Alignment>&),
            qi::locals<
                std::string,
                std::string
            >,
            parse::skipper_type
        > alignment_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::vector<Alignment>&),
            parse::skipper_type
        > start_rule;

        alignment_rule alignment;
        start_rule start;
    };

}

namespace parse {
    bool alignments(const boost::filesystem::path& path,
                    std::vector<Alignment>& alignments_,
                    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects_groups)
    {
        g_effects_groups = &effects_groups;
        return detail::parse_file<rules, std::vector<Alignment> >(path, alignments_);
    }
}
