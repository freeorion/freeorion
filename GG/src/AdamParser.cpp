/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@gmail.com */

#include <GG/AdamParser.h>

#include <GG/ExpressionParser.h>
#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/adam_parser.hpp>
#include <GG/adobe/implementation/token.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>


using namespace GG;

// requirements of spirit::qi::debug()
namespace adobe {

    std::ostream& operator<<(std::ostream& os, const adam_callback_suite_t::relation_t& relation)
    {
        return os << "("
                  << relation.name_m << " "
                  << relation.position_m << " "
                  << relation.expression_m << " "
                  << relation.detailed_m << " "
                  << relation.brief_m << " "
                  << ")";
    }

    std::ostream& operator<<(std::ostream& os, const std::vector<adam_callback_suite_t::relation_t>& relation_set)
    {
        return os; // TODO
    }

}

namespace {

    adobe::aggregate_name_t input_k      = { "input" };
    adobe::aggregate_name_t output_k     = { "output" };
    adobe::aggregate_name_t interface_k  = { "interface" };
    adobe::aggregate_name_t logic_k      = { "logic" };
    adobe::aggregate_name_t constant_k   = { "constant" };
    adobe::aggregate_name_t invariant_k  = { "invariant" };
    adobe::aggregate_name_t sheet_k      = { "sheet" };
    adobe::aggregate_name_t unlink_k     = { "unlink" };
    adobe::aggregate_name_t when_k       = { "when" };
    adobe::aggregate_name_t relate_k     = { "relate" };

    const lexer& AdamLexer()
    {
        static const adobe::name_t s_keywords[] = {
            input_k,
            output_k,
            interface_k,
            logic_k,
            constant_k,
            invariant_k,
            sheet_k,
            unlink_k,
            when_k,
            relate_k
        };
        static const std::size_t s_num_keywords = sizeof(s_keywords) / sizeof(s_keywords[0]);

        static lexer s_lexer(s_keywords, s_keywords + s_num_keywords);

        return s_lexer;    
    }

    const expression_parser_rules::expression_rule& AdamExpressionParser()
    {
        using boost::spirit::qi::token;
        using boost::spirit::qi::_1;
        using boost::spirit::qi::_val;

        lexer& tok = const_cast<lexer&>(AdamLexer());
        assert(tok.keywords.size() == 10u);
        const boost::spirit::lex::token_def<adobe::name_t>& input = tok.keywords[input_k];
        const boost::spirit::lex::token_def<adobe::name_t>& output = tok.keywords[output_k];
        const boost::spirit::lex::token_def<adobe::name_t>& interface = tok.keywords[interface_k];
        const boost::spirit::lex::token_def<adobe::name_t>& logic = tok.keywords[logic_k];
        const boost::spirit::lex::token_def<adobe::name_t>& constant = tok.keywords[constant_k];
        const boost::spirit::lex::token_def<adobe::name_t>& invariant = tok.keywords[invariant_k];
        const boost::spirit::lex::token_def<adobe::name_t>& sheet = tok.keywords[sheet_k];
        const boost::spirit::lex::token_def<adobe::name_t>& unlink = tok.keywords[unlink_k];
        const boost::spirit::lex::token_def<adobe::name_t>& when = tok.keywords[when_k];
        const boost::spirit::lex::token_def<adobe::name_t>& relate = tok.keywords[relate_k];
        assert(tok.keywords.size() == 10u);

        static expression_parser_rules::keyword_rule adam_keywords =
              input[_val = _1]
            | output[_val = _1]
            | interface[_val = _1]
            | logic[_val = _1]
            | constant[_val = _1]
            | invariant[_val = _1]
            | sheet[_val = _1]
            | unlink[_val = _1]
            | when[_val = _1]
            | relate[_val = _1]
            ;
        adam_keywords.name("keyword");

        static const expression_parser_rules s_parser(AdamLexer(), adam_keywords);

        return s_parser.expression;
    }

#define GET_REF(type_, name)                                            \
    struct get_##name##_                                                \
    {                                                                   \
        template <typename Arg1>                                        \
        struct result                                                   \
        { typedef type_ type; };                                        \
                                                                        \
        type_ operator()(adobe::adam_callback_suite_t::relation_t& relation) const \
            { return relation.name##_m; }                               \
    };                                                                  \
    const boost::phoenix::function<get_##name##_> get_##name

    GET_REF(adobe::name_t&, name);
    GET_REF(adobe::line_position_t&, position);
    GET_REF(adobe::array_t&, expression);
    GET_REF(std::string&, detailed);
    GET_REF(std::string&, brief);

#undef GET_REF

    struct add_cell_
    {
        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
        struct result
        { typedef void type; };

        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
        void operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7) const
            { arg1.add_cell_proc_m(arg2, arg3, arg4, arg5, arg6, arg7); }
    };

    const boost::phoenix::function<add_cell_> add_cell;

    struct add_relation_
    {
        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
        struct result
        { typedef void type; };

        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
        void operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6) const
            { arg1.add_relation_proc_m(arg2, arg3, &arg4.front(), &arg4.front() + arg4.size(), arg5, arg6); }
    };

    const boost::phoenix::function<add_relation_> add_relation;

    struct add_interface_
    {
        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
        struct result
        { typedef void type; };

        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
        void operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9) const
            { arg1.add_interface_proc_m(arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9); }
    };

    const boost::phoenix::function<add_interface_> add_interface;

    struct strip_c_comment_
    {
        template <typename Arg>
        struct result
        { typedef std::string type; };

        std::string operator()(const std::string& arg1) const
            { return arg1.substr(2, arg1.size() - 4); }
    };

    const boost::phoenix::function<strip_c_comment_> strip_c_comment;

    struct strip_cpp_comment_
    {
        template <typename Arg>
        struct result
        { typedef std::string type; };

        std::string operator()(const std::string& arg1) const
            { return arg1.substr(2, arg1.size() - 2); }
    };

    const boost::phoenix::function<strip_cpp_comment_> strip_cpp_comment;

    struct adam_parser_rules
    {
        adam_parser_rules(const adobe::adam_callback_suite_t& callbacks_) :
            callbacks(callbacks_)
        {
            namespace ascii = boost::spirit::ascii;
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;
            using ascii::char_;
            using phoenix::clear;
            using phoenix::construct;
            using phoenix::cref;
            using phoenix::if_;
            using phoenix::static_cast_;
            using phoenix::push_back;
            using phoenix::val;
            using qi::_1;
            using qi::_2;
            using qi::_3;
            using qi::_4;
            using qi::_a;
            using qi::_b;
            using qi::_c;
            using qi::_d;
            using qi::_e;
            using qi::_f;
            using qi::_g;
            using qi::_r1;
            using qi::_r2;
            using qi::_r3;
            using qi::_r4;
            using qi::_val;
            using qi::alpha;
            using qi::bool_;
            using qi::digit;
            using qi::double_;
            using qi::eol;
            using qi::eps;
            using qi::lexeme;
            using qi::lit;

            lexer& tok = const_cast<lexer&>(AdamLexer());
            assert(tok.keywords.size() == 10u);
            const boost::spirit::lex::token_def<adobe::name_t>& input = tok.keywords[input_k];
            const boost::spirit::lex::token_def<adobe::name_t>& output = tok.keywords[output_k];
            const boost::spirit::lex::token_def<adobe::name_t>& interface = tok.keywords[interface_k];
            const boost::spirit::lex::token_def<adobe::name_t>& logic = tok.keywords[logic_k];
            const boost::spirit::lex::token_def<adobe::name_t>& constant = tok.keywords[constant_k];
            const boost::spirit::lex::token_def<adobe::name_t>& invariant = tok.keywords[invariant_k];
            const boost::spirit::lex::token_def<adobe::name_t>& sheet = tok.keywords[sheet_k];
            const boost::spirit::lex::token_def<adobe::name_t>& unlink = tok.keywords[unlink_k];
            const boost::spirit::lex::token_def<adobe::name_t>& when = tok.keywords[when_k];
            const boost::spirit::lex::token_def<adobe::name_t>& relate = tok.keywords[relate_k];
            assert(tok.keywords.size() == 10u);

            const expression_parser_rules::expression_rule& expression = AdamExpressionParser();

            using GG::detail::next_pos;

            // note that the lead comment, sheet name, and trail comment are currently all ignored
            sheet_specifier
                =    - lead_comment
                >>     sheet
                >      tok.identifier
                >      '{'
                >>   * qualified_cell_decl
                >      '}'
                >>   - trail_comment
                ;

            qualified_cell_decl
                =      interface_set_decl
                |      input_set_decl
                |      output_set_decl
                |      constant_set_decl
                |      logic_set_decl
                |      invariant_set_decl
                ;

#define SET_DECL(name)                                          \
            name##_set_decl                                     \
                =     name                                      \
                >     ':'                                       \
                >   * (                                         \
                           (                                    \
                               lead_comment [_a = _1]           \
                             | eps [clear(_a)]                  \
                           )                                    \
                        >> name##_cell_decl(_a)                 \
                      )

            SET_DECL(interface);
            SET_DECL(input);
            SET_DECL(output);
            SET_DECL(constant);
            SET_DECL(logic);
            SET_DECL(invariant);

#undef SET_DECL

            interface_cell_decl
                =
                (
                    (
                          tok.identifier
                          [
                              _a = _1,
                              _b = val(true)
                          ]
                     |    (
                               unlink [_b = val(false)]
                           >   tok.identifier [_a = _1]
                          )
                    )
                    >>  - initializer(_e, _c)
                    >>  - define_expression(_f, _d)
                    >     end_statement(_g)
                )
                [
                    add_interface(callbacks, _a, _b, _e, _c, _f, _d, _g, _r1)
                ]
                ;

            input_cell_decl
                =     tok.identifier [_a = _1]
                >>  - initializer(_c, _b)
                >     end_statement(_d)
                      [
                          add_cell(callbacks, adobe::adam_callback_suite_t::input_k, _a, _c, _b, _d, _r1)
                      ]
                ;

            output_cell_decl
                =    named_decl(_a, _c, _b, _d)
                     [
                         add_cell(callbacks, adobe::adam_callback_suite_t::output_k, _a, _c, _b, _d, _r1)
                     ]
                ;

            constant_cell_decl
                =     tok.identifier [_a = _1]
                >     initializer(_c, _b)
                >     end_statement(_d)
                      [
                          add_cell(callbacks, adobe::adam_callback_suite_t::constant_k, _a, _c, _b, _d, _r1)
                      ]
                ;

            logic_cell_decl
                =     named_decl(_a, _c, _b, _d)
                      [
                          add_cell(callbacks, adobe::adam_callback_suite_t::logic_k, _a, _c, _b, _d, _r1)
                      ]
                |     relate_decl(_c, _b, _e, _d)
                      [
                          add_relation(callbacks, _c, _b, _e, _d, _r1)
                      ]
                ;

            invariant_cell_decl
                =     named_decl(_a, _c, _b, _d)
                      [
                          add_cell(callbacks, adobe::adam_callback_suite_t::invariant_k, _a, _c, _b, _d, _r1)
                      ]
                ;

            relate_decl
                =    (
                           (relate >> next_pos [_r1 = _1])
                      |    (conditional(_r1, _r2) > relate)
                     )
                >    '{'
                >    relate_expression(_a)
                >    relate_expression(_b)
                     [
                         push_back(_r3, _a),
                         push_back(_r3, _b),
                         clear(get_expression(_a))
                     ]
                >> * relate_expression(_a)
                     [
                         push_back(_r3, _a),
                         clear(get_expression(_a))
                     ]
                >    '}'
                >> - trail_comment [_r4 = _1]
                ;

            relate_expression
                =    - lead_comment [get_detailed(_r1) = _1]
                >>     named_decl(get_name(_r1), get_position(_r1), get_expression(_r1), get_brief(_r1))
                ;

            named_decl
                =     tok.identifier [_r1 = _1]
                >     define_expression(_r2, _r3)
                >     end_statement(_r4)
                ;

            initializer
                =    ':'
                >>   next_pos [_r1 = _1]
                >    expression(_r2)
                ;

            define_expression
                =     tok.define
                >>    next_pos [_r1 = _1]
                >     expression(_r2)
                ;

            conditional
                =     when
                >     '('
                >>    next_pos [_r1 = _1]
                >     expression(_r2)
                >     ')'
                ;

            end_statement = ';' >> - trail_comment [_r1 = _1] ;

            // convenience rules
            lead_comment = tok.lead_comment [_val = strip_c_comment(_1)] ;
            trail_comment = tok.trail_comment [_val = strip_cpp_comment(_1)] ;

            // define names for rules, to be used in error reporting
#define NAME(x) x.name(#x)
            NAME(sheet_specifier);
            NAME(qualified_cell_decl);
            NAME(interface_set_decl);
            NAME(input_set_decl);
            NAME(output_set_decl);
            NAME(constant_set_decl);
            NAME(logic_set_decl);
            NAME(invariant_set_decl);
            NAME(interface_cell_decl);
            NAME(input_cell_decl);
            NAME(output_cell_decl);
            NAME(constant_cell_decl);
            NAME(logic_cell_decl);
            NAME(invariant_cell_decl);
            NAME(relate_decl);
            NAME(relate_expression);
            NAME(named_decl);
            NAME(initializer);
            NAME(define_expression);
            NAME(conditional);
            NAME(end_statement);
#undef NAME

            qi::on_error<qi::fail>(sheet_specifier, GG::report_error(_1, _2, _3, _4));
        }

        typedef adobe::adam_callback_suite_t::relation_t relation;
        typedef std::vector<relation> relation_set;

        typedef boost::spirit::qi::rule<token_iterator, void(), skipper_type> void_rule;
        typedef boost::spirit::qi::rule<
            token_iterator,
            void(),
            boost::spirit::qi::locals<std::string>,
            skipper_type
        > cell_set_rule;
        typedef boost::spirit::qi::rule<
            token_iterator,
            void(const std::string&),
            boost::spirit::qi::locals<
                adobe::name_t,
                adobe::array_t,
                adobe::line_position_t,
                std::string
            >,
            skipper_type
        > cell_decl_rule;

        // Adam grammar
        void_rule sheet_specifier;
        void_rule qualified_cell_decl;

        cell_set_rule interface_set_decl;
        cell_set_rule input_set_decl;
        cell_set_rule output_set_decl;
        cell_set_rule constant_set_decl;
        cell_set_rule logic_set_decl;
        cell_set_rule invariant_set_decl;

        boost::spirit::qi::rule<
            token_iterator,
            void(const std::string&),
            boost::spirit::qi::locals<
                adobe::name_t,
                bool,
                adobe::array_t,
                adobe::array_t,
                adobe::line_position_t,
                adobe::line_position_t,
                std::string
            >,
            skipper_type
        > interface_cell_decl;

        cell_decl_rule input_cell_decl;
        cell_decl_rule output_cell_decl;
        cell_decl_rule constant_cell_decl;

        boost::spirit::qi::rule<
            token_iterator,
            void(const std::string&),
            boost::spirit::qi::locals<
                adobe::name_t,
                adobe::array_t,
                adobe::line_position_t,
                std::string,
                relation_set
            >,
            skipper_type
        > logic_cell_decl;

        cell_decl_rule invariant_cell_decl;

        boost::spirit::qi::rule<
            token_iterator,
            void(adobe::line_position_t&, adobe::array_t&, relation_set&, std::string&),
            boost::spirit::qi::locals<relation, relation>,
            skipper_type
        > relate_decl;

        boost::spirit::qi::rule<token_iterator, void(relation&), skipper_type> relate_expression;

        boost::spirit::qi::rule<
            token_iterator,
            void(adobe::name_t&, adobe::line_position_t&, adobe::array_t&, std::string&),
            skipper_type
        > named_decl;

        boost::spirit::qi::rule<
            token_iterator,
            void(adobe::line_position_t&, adobe::array_t&),
            skipper_type
        > initializer;
        boost::spirit::qi::rule<
            token_iterator,
            void(adobe::line_position_t&, adobe::array_t&),
            skipper_type
        > define_expression;
        boost::spirit::qi::rule<
            token_iterator,
            void(adobe::line_position_t&, adobe::array_t&),
            skipper_type
        > conditional;
        boost::spirit::qi::rule<token_iterator, void(std::string&), skipper_type> end_statement;

        boost::spirit::qi::rule<token_iterator, std::string(), skipper_type> lead_comment;
        boost::spirit::qi::rule<token_iterator, std::string(), skipper_type> trail_comment;

        const adobe::adam_callback_suite_t& callbacks;
    };

}

bool GG::Parse(const std::string& sheet,
               const std::string& filename,
               const adobe::adam_callback_suite_t& callbacks)
{
    using boost::spirit::qi::phrase_parse;
    text_iterator it(sheet.begin());
    detail::s_text_it = &it;
    detail::s_begin = it;
    detail::s_end = text_iterator(sheet.end());
    detail::s_filename = filename.c_str();
    token_iterator iter = AdamLexer().begin(it, detail::s_end);
    token_iterator end = AdamLexer().end();
    adam_parser_rules adam_rules(callbacks);
    return phrase_parse(iter,
                        end,
                        adam_rules.sheet_specifier,
                        boost::spirit::qi::in_state("WS")[AdamLexer().self]);
}
