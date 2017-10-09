#ifndef _ParseImpl_h_
#define _ParseImpl_h_

#include "ReportParseError.h"
#include "../universe/Tech.h"
#include "../util/Logger.h"

#include <boost/filesystem/path.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/timer.hpp>

#include <GG/Clr.h>


namespace Condition {
    struct ConditionBase;
}

namespace Effect {
    class EffectBase;
}

namespace parse { namespace detail {

    /// A functor to determine if \p key will be unique in \p map of \p type, and log an error otherwise.
    struct is_unique {
        typedef bool result_type;

        template <typename Map>
        result_type operator() (const Map& map, const char* const type, const std::string& key) const {
            // Will this key be unique?
            auto will_be_unique = (map.count(key) == 0);
            if (!will_be_unique)
                ErrorLogger() << "More than one " <<  type << " has the same name, " << key << ".";
            return will_be_unique;
        }
    };

    /// A functor to insert a \p value with key \p key into \p map.
    struct insert {
        typedef void result_type;

        template <typename Map, typename Value>
        result_type operator() (Map& map, const std::string& key, Value* value) const
        { map.insert(std::make_pair(key, value)); }
    };

    template <
        typename signature = boost::spirit::qi::unused_type,
        typename locals = boost::spirit::qi::unused_type
    >
    using rule = boost::spirit::qi::rule<
        parse::token_iterator,
        parse::skipper_type,
        signature,
        locals
    >;

    template <
        typename signature = boost::spirit::qi::unused_type,
        typename locals = boost::spirit::qi::unused_type
    >
    using grammar = boost::spirit::qi::grammar<
        parse::token_iterator,
        parse::skipper_type,
        signature,
        locals
    >;

    using double_rule = detail::rule<
        double ()
    >;
    extern double_rule double_;


    using int_rule = detail::rule<
        int ()
    >;
    extern int_rule int_;


    using label_rule = detail::rule<>;
    /** Store label_rules. */
    class Labeller {
        public:
        Labeller(const parse::lexer& tok_);

        /** Retrieve or create a label rule for \p name.*/
        label_rule& rule(const char* name);
        private:
        const parse::lexer& m_tok;
        std::unordered_map<const char*, label_rule> m_rules;
    };

    using tags_rule_type    = rule<void (std::set<std::string>&)>;
    using tags_grammar_type = grammar<void (std::set<std::string>&)>;

    struct tags_grammar : public tags_grammar_type {
        tags_grammar(const parse::lexer& tok,
                     Labeller& labeller);
        tags_rule_type start;
    };

    typedef rule<
        GG::Clr (),
        boost::spirit::qi::locals<
            unsigned int,
            unsigned int,
            unsigned int
        >
    > color_parser_rule;
    color_parser_rule& color_parser();

    using item_spec_rule_type = rule<ItemSpec (), boost::spirit::qi::locals<UnlockableItemType>>;
    using item_spec_grammar_type = grammar<ItemSpec (), boost::spirit::qi::locals<UnlockableItemType>>;

    struct item_spec_grammar : public item_spec_grammar_type {
        item_spec_grammar(const parse::lexer& tok,
                          Labeller& labeller);
        item_spec_rule_type start;
    };

    void parse_file_common(const boost::filesystem::path& path,
                           const lexer& lexer,
                           std::string& filename,
                           std::string& file_contents,
                           text_iterator& first,
                           text_iterator& last,
                           token_iterator& it);

    template <typename Rules, typename Arg1>
    bool parse_file(const boost::filesystem::path& path, Arg1& arg1)
    {
        std::string filename;
        std::string file_contents;
        text_iterator first;
        text_iterator last;
        token_iterator it;

        boost::timer timer;

        const lexer& lexer = lexer::instance();

        parse_file_common(path, lexer, filename, file_contents, first, last, it);

        //TraceLogger() << "Parse: parsed contents for " << path.string() << " : \n" << file_contents;

        boost::spirit::qi::in_state_type in_state;

        detail::Labeller labeller(lexer);
        Rules rules(lexer, labeller, filename, first, last);

        bool success = boost::spirit::qi::phrase_parse(it, lexer.end(), rules.start(boost::phoenix::ref(arg1)), in_state("WS")[lexer.self]);

        TraceLogger() << "Parse: Elapsed time to parse " << path.string() << " = " << (timer.elapsed() * 1000.0);

        // s_end is global and static.  It is wrong when multiple files are concurrently or
        // recursively parsed.  This check is meaningless and was removed May 2017.
        /* std::ptrdiff_t length_of_unparsed_file = std::distance(first, parse::detail::s_end); */
        /* bool parse_length_good = ((length_of_unparsed_file == 0) */
        /*                           || (length_of_unparsed_file == 1 && *first == '\n')); */

        return success /*&& parse_length_good*/;
    }
} }

#endif
