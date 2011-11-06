// -*- C++ -*-
#ifndef _ParseImpl_h_
#define _ParseImpl_h_

#include "ConditionParserImpl.h"
#include "EnumParser.h"
#include "Parse.h"
#include "ReportParseError.h"
#include "../util/MultiplayerCommon.h"

#include <boost/filesystem/fstream.hpp>
#include <boost/spirit/home/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>


namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse { namespace detail {

    typedef boost::spirit::qi::rule<
        token_iterator,
        std::vector<boost::shared_ptr<const Effect::EffectsGroup> > (),
        skipper_type
    > effects_group_rule;

    effects_group_rule& effects_group_parser();

    typedef boost::spirit::qi::rule<
        token_iterator,
        GG::Clr (),
        qi::locals<
            unsigned int,
            unsigned int,
            unsigned int
        >,
        skipper_type
    > color_parser_rule;

    color_parser_rule& color_parser();

    typedef boost::spirit::qi::rule<
        parse::token_iterator,
        ItemSpec (),
        qi::locals<UnlockableItemType>,
        parse::skipper_type
    > item_spec_parser_rule;

    item_spec_parser_rule& item_spec_parser();

    void parse_file_common(const boost::filesystem::path& path,
                           const lexer& l,
                           std::string& filename,
                           std::string& file_contents,
                           text_iterator& first,
                           token_iterator& it);

    template <typename Rules, typename Arg1>
    bool parse_file(const boost::filesystem::path& path, Arg1& arg1)
    {
        std::string filename;
        std::string file_contents;
        text_iterator first;
        token_iterator it;

        const lexer& l = lexer::instance();

        parse_file_common(path, l, filename, file_contents, first, it);

        boost::spirit::qi::in_state_type in_state;

        static Rules rules;

        bool success = boost::spirit::qi::phrase_parse(it, l.end(), rules.start(boost::phoenix::ref(arg1)), in_state("WS")[l.self]);

        std::ptrdiff_t distance = std::distance(first, parse::detail::s_end);

        return success && (!distance || distance == 1 && *first == '\n');
    }

} }

#endif
