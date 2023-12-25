#ifndef _ParseImpl_h_
#define _ParseImpl_h_

#include "ReportParseError.h"
#include "../util/Logger.h"
#include "../util/ScopedTimer.h"

#include <array>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/spirit/include/qi.hpp>


namespace Condition {
    struct Condition;
}
namespace Effect {
    class Effect;
}

namespace parse::detail {
    template<typename ... Types> struct is_vector : std::false_type {};
    template<typename ... Types> struct is_vector<std::vector<Types...>> : std::true_type {};
    template<typename ... Types> constexpr bool is_vector_v = is_vector<Types...>::value;

    template<typename ... Types> struct is_map : std::false_type {};
    template<typename ... Types> struct is_map<std::map<Types...>> : std::true_type {};
    template<typename ... Types> struct is_map<boost::container::flat_map<Types...>> : std::true_type {};
    template<typename ... Types> constexpr bool is_map_v = is_map<Types...>::value;


    /// A functor to determine if \p key will be unique in \p container of \p type, and log an error otherwise.
    struct is_unique {
        typedef bool result_type;

        template <typename Container>
        result_type operator() (const Container& container, const std::string& type, const std::string& key) const {
            // Will this key be unique?
            bool will_be_unique = false;

            if constexpr (is_map_v<Container>) {
                using key_type = typename Container::key_type;
                static_assert(std::is_same_v<std::string, key_type>);
                will_be_unique = (container.find(key) == container.end());

            } else if constexpr (is_vector_v<Container>) {
                using value_type = typename Container::value_type;
                static_assert(!std::is_pointer_v<value_type>); // doesn't check for unique_ptr
                will_be_unique = container.end() == std::find_if(container.begin(), container.end(),
                                              [&key](const auto& entry) { return entry.Name() == key; });

            } else {
                static_assert(is_map_v<Container>, "unknown container, can't check for uniqueness of parsed content...");
            }

            if (!will_be_unique)
                ErrorLogger() << "More than one parsed " << type << " has the same name: " << key << ".";

            return will_be_unique;
        }
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

    template <typename Signature>
    struct BracketedParserSignature;
    template <typename Synthesized, typename... Inherited>
    struct BracketedParserSignature<Synthesized (Inherited...)> {
        using Synthesized_t = Synthesized;
        using Signature_t = std::vector<Synthesized_t>(Inherited...);
    };

      /** single_or_bracketed_repeat uses \p Parser to parser expressions with
        either a single element or repeated elements within square brackets. */
    template <typename Parser>
    struct single_or_bracketed_repeat : public grammar<
        typename BracketedParserSignature<typename Parser::sig_type>::Signature_t
        >
    {
        single_or_bracketed_repeat(const Parser& repeated_parser)
            : single_or_bracketed_repeat::base_type(start)
        {
            boost::spirit::qi::repeat_type repeat_;
            start
                =    ('[' > +repeated_parser > ']')
                |    repeat_(1)[repeated_parser]
                ;

            this->name("List of " + repeated_parser.name());
        }

        rule<typename BracketedParserSignature<typename Parser::sig_type>::Signature_t> start;
    };

    template <typename SynthesizedContainer = std::vector<std::string>>
    struct single_or_repeated_string : public grammar<SynthesizedContainer ()> {
        single_or_repeated_string(const parse::lexer& tok) : single_or_repeated_string::base_type(start) {
            boost::spirit::qi::repeat_type repeat_;
            start
                =    ('[' > +tok.string > ']')
                |    repeat_(1)[tok.string]
                ;

            this->name("List of strings");
        }

        rule<SynthesizedContainer ()> start;
    };

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
        /** Retrieve or create a label rule for \p name.*/
        label_rule& operator()(const parse::lexer::string_token_def& token);
    private:
        std::unordered_map<const parse::lexer::string_token_def*, label_rule> m_rules;
    };

    template <typename T>
    using enum_rule = rule<T ()>;
    template <typename T>
    using enum_grammar = grammar<T ()>;

    using tags_rule_type    = rule<std::set<std::string> ()>;
    using tags_grammar_type = grammar<std::set<std::string> ()>;

    struct tags_grammar : public tags_grammar_type {
        tags_grammar(const parse::lexer& tok, Labeller& label);
        tags_rule_type start;
        single_or_repeated_string<std::set<std::string>> one_or_more_string_tokens;
    };

    using color_parser_signature = std::array<uint8_t, 4> ();
    using color_rule_type = rule<color_parser_signature>;
    using color_grammar_type = grammar<color_parser_signature>;

    struct color_parser_grammar : public color_grammar_type {
        color_parser_grammar(const parse::lexer& tok);
        using channel_rule = rule<unsigned int ()>;

        channel_rule channel;
        channel_rule alpha;
        color_rule_type start;
    };

    void parse_file_common(const boost::filesystem::path& path,
                           const lexer& lexer,
                           std::string& filename,
                           std::string& file_contents,
                           text_iterator& first,
                           text_iterator& last,
                           token_iterator& it);

    /** Report warnings about unparsed end of file and return true for a good
        parse. */
    bool parse_file_end_of_file_warnings(const boost::filesystem::path& path,
                                         bool parser_success,
                                         const std::string& file_contents,
                                         const text_iterator first,
                                         const text_iterator last);

    template <typename Grammar, typename Arg1>
    bool parse_file(const lexer& lexer, const boost::filesystem::path& path, Arg1& arg1) {
        ScopedTimer timer("parse_file \"" + path.filename().string()  + "\"", std::chrono::milliseconds(100));

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

        return parse_file_end_of_file_warnings(path, success, file_contents, first, last);
    }

    template <typename Grammar, typename Arg1, typename Arg2>
    bool parse_file(const lexer& lexer, const boost::filesystem::path& path, Arg1& arg1, Arg2& arg2) {
        ScopedTimer timer("parse_file \"" + path.filename().string()  + "\"", std::chrono::milliseconds(10));

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
            it, lexer.end(), grammar(boost::phoenix::ref(arg1), boost::phoenix::ref(arg2)), in_state("WS")[lexer.self]);

        return parse_file_end_of_file_warnings(path, success, file_contents, first, last);
    }

}

#endif
