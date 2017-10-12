#ifndef _ParseImpl_h_
#define _ParseImpl_h_

#include "ReportParseError.h"
#include "../universe/Tech.h"
#include "../util/Logger.h"
#include "../util/ScopedTimer.h"

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


    struct double_grammar : public grammar<double()> {
        double_grammar(const parse::lexer& tok);
        rule<double()> double_;
    };

    struct int_grammar : public grammar<int()> {
        int_grammar(const parse::lexer& tok);
        rule<int()> int_;
    };

    using label_rule = rule<>;
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

    template <typename T>
    using enum_rule = rule<T ()>;
    template <typename T>
    using enum_grammar = grammar<T ()>;

    using tags_rule_type    = rule<void (std::set<std::string>&)>;
    using tags_grammar_type = grammar<void (std::set<std::string>&)>;

    struct tags_grammar : public tags_grammar_type {
        tags_grammar(const parse::lexer& tok,
                     Labeller& labeller);
        tags_rule_type start;
    };

    using color_parser_signature = GG::Clr ();
    using color_parser_locals = boost::spirit::qi::locals<
        unsigned int,
        unsigned int,
        unsigned int
        >;
    using color_rule_type = rule<color_parser_signature, color_parser_locals>;
    using color_grammar_type = grammar<color_parser_signature, color_parser_locals>;

    struct color_parser_grammar : public color_grammar_type {
        color_parser_grammar(const parse::lexer& tok);
        using channel_rule = rule<unsigned int ()>;

        channel_rule channel;
        color_rule_type start;
    };

    void parse_file_common(const boost::filesystem::path& path,
                           const lexer& lexer,
                           std::string& filename,
                           std::string& file_contents,
                           text_iterator& first,
                           text_iterator& last,
                           token_iterator& it);

    template <typename Grammar, typename Arg1>
    bool parse_file(const lexer& lexer, const boost::filesystem::path& path, Arg1& arg1)
    {
        SectionedScopedTimer timer("parse_file \"" + path.filename().string()  + "\"", std::chrono::microseconds(100));

        std::string filename;
        std::string file_contents;
        text_iterator first;
        text_iterator last;
        token_iterator it;

        parse_file_common(path, lexer, filename, file_contents, first, last, it);

        //TraceLogger() << "Parse: parsed contents for " << path.string() << " : \n" << file_contents;

        boost::spirit::qi::in_state_type in_state;

        Grammar grammar(lexer, filename, first, last);

        bool success = boost::spirit::qi::phrase_parse(
            it, lexer.end(), grammar(boost::phoenix::ref(arg1)), in_state("WS")[lexer.self]);

        if (!success)
            WarnLogger() << "A parser failed while parsing " << path;

        auto length_of_unparsed_file = std::distance(first, last);
        bool parse_length_good = ((length_of_unparsed_file == 0)
                                  || (length_of_unparsed_file == 1 && *first == '\n'));

        if (!parse_length_good
            && length_of_unparsed_file > 0
            && static_cast<std::string::size_type>(length_of_unparsed_file) <= file_contents.size())
        {
            auto unparsed_section = file_contents.substr(file_contents.size() - std::abs(length_of_unparsed_file));
            std::copy(first, last, std::back_inserter(unparsed_section));
            WarnLogger() << "File \"" << path << "\" was incompletely parsed. " << std::endl
                         << "Unparsed section of file, " << length_of_unparsed_file <<" characters:" << std::endl
                         << unparsed_section;
        }

        return success && parse_length_good;
    }

} }

#endif
