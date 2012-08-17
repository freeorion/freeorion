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

#include <GG/EveParser.h>

#include <GG/ExpressionParser.h>
#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/eve_parser.hpp>
#include <GG/adobe/implementation/token.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>


using namespace GG;

namespace {

    adobe::aggregate_name_t interface_k  = { "interface" };
    adobe::aggregate_name_t constant_k   = { "constant" };
    adobe::aggregate_name_t layout_k     = { "layout" };
    adobe::aggregate_name_t view_k       = { "view" };

    const lexer& EveLexer()
    {
        static const adobe::name_t s_keywords[] = {
            interface_k,
            constant_k,
            layout_k,
            view_k
        };
        static const std::size_t s_num_keywords = sizeof(s_keywords) / sizeof(s_keywords[0]);

        static lexer s_lexer(s_keywords, s_keywords + s_num_keywords);

        return s_lexer;    
    }

    const expression_parser_rules& EveExpressionParserRules()
    {
        using boost::spirit::qi::token;
        using boost::spirit::qi::_1;
        using boost::spirit::qi::_val;

        lexer& tok = const_cast<lexer&>(EveLexer());
        assert(tok.keywords.size() == 4u);
        const boost::spirit::lex::token_def<adobe::name_t>& interface = tok.keywords[interface_k];
        const boost::spirit::lex::token_def<adobe::name_t>& constant = tok.keywords[constant_k];
        const boost::spirit::lex::token_def<adobe::name_t>& layout = tok.keywords[layout_k];
        const boost::spirit::lex::token_def<adobe::name_t>& view = tok.keywords[view_k];
        assert(tok.keywords.size() == 4u);

        static expression_parser_rules::keyword_rule eve_keywords =
              interface[_val = _1]
            | constant[_val = _1]
            | layout[_val = _1]
            | view[_val = _1]
            ;
        eve_keywords.name("keyword");

        static const expression_parser_rules s_parser(EveLexer(), eve_keywords);

        return s_parser;
    }

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

    struct add_view_
    {
        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
        struct result
        { typedef boost::any type; };

        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
        boost::any operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7) const
            { return arg1.add_view_proc_m(arg2, arg3, arg4, arg5, arg6, arg7); }
    };

    const boost::phoenix::function<add_view_> add_view;

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

    struct eve_parser_rules
    {
        eve_parser_rules(const adobe::eve_callback_suite_t& callbacks_) :
            callbacks(callbacks_)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;
            using phoenix::push_back;
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
            using qi::_r1;
            using qi::_r2;
            using qi::_val;
            using qi::eps;

            lexer& tok = const_cast<lexer&>(EveLexer());
            assert(tok.keywords.size() == 4u);
            const boost::spirit::lex::token_def<adobe::name_t>& interface = tok.keywords[interface_k];
            const boost::spirit::lex::token_def<adobe::name_t>& constant = tok.keywords[constant_k];
            const boost::spirit::lex::token_def<adobe::name_t>& layout = tok.keywords[layout_k];
            const boost::spirit::lex::token_def<adobe::name_t>& view = tok.keywords[view_k];
            assert(tok.keywords.size() == 4u);

            const expression_parser_rules::expression_rule& expression = EveExpressionParserRules().expression;
            const expression_parser_rules::local_size_rule& named_argument_list = EveExpressionParserRules().named_argument_list;

            using GG::detail::next_pos;

            // Note that the comments and layout name are currently ignored.
            layout_specifier
                =  - lead_comment
                >>   layout
                >    tok.identifier
                >    '{'
                >> * qualified_cell_decl
                >    view
                >    view_definition(_r1)
                >    '}'
                >> - trail_comment
                ;

            qualified_cell_decl
                =    interface_set_decl
                |    constant_set_decl
                ;

            interface_set_decl
                =    interface
                >    ':'
                >  * cell_decl(adobe::eve_callback_suite_t::interface_k)
                ;

            constant_set_decl
                =    constant
                >    ':'
                >  * cell_decl(adobe::eve_callback_suite_t::constant_k)
                ;

            cell_decl
                =    (
                          - lead_comment [_a = _1]
                       >>   tok.identifier [_c = _1]
                       >    initializer(_d, _e)
                       >    end_statement(_b)
                     )
                     [
                         add_cell(callbacks, _r1, _c, _d, _e, _b, _a)
                     ]
                ;

            initializer
                =    ':'
                >>   next_pos [_r1 = _1]
                >    expression(_r2)
                ;

            view_definition
                =  - lead_comment [_a = _1]
                >>   next_pos [_e = _1]
                >>   view_class_decl(_c, _d)
                >    (
                          end_statement(_b)
                          [
                              _f = add_view(callbacks, _r1, _e, _c, _d, _b, _a)
                          ]
                       |  (
                              - trail_comment [_b = _1]
                            >>  eps
                                [
                                    _f = add_view(callbacks, _r1, _e, _c, _d, _b, _a)
                                ]
                            >   view_statment_list(_f)
                          )
                     )
                ;

            view_statment_sequence
                =  * view_definition(_r1)
                ;

            view_class_decl
                =     tok.identifier [_r1 = _1]
                >     '('
                >>    (
                           named_argument_list(_r2)
                        |  eps [push_back(_r2, adobe::any_regular_t(adobe::dictionary_t()))]
                      )
                >     ')'
                ;

            view_statment_list
                =     '{'
                >>    view_statment_sequence(_r1)
                >     '}'
                ;

            end_statement = ';' >> - trail_comment [_r1 = _1] ;

            // convenience rules
            lead_comment = tok.lead_comment [_val = strip_c_comment(_1)] ;
            trail_comment = tok.trail_comment [_val = strip_cpp_comment(_1)] ;

            // define names for rules, to be used in error reporting
#define NAME(x) x.name(#x)
            NAME(layout_specifier);
            NAME(qualified_cell_decl);
            NAME(interface_set_decl);
            NAME(constant_set_decl);
            NAME(cell_decl);
            NAME(initializer);
            NAME(view_definition);
            NAME(view_statment_sequence);
            NAME(view_class_decl);
            NAME(view_statment_list);
            NAME(end_statement);
#undef NAME

            qi::on_error<qi::fail>(layout_specifier, GG::report_error(_1, _2, _3, _4));
        }

        typedef adobe::eve_callback_suite_t::cell_type_t cell_type_t;

        typedef boost::spirit::qi::rule<token_iterator, void(), skipper_type> void_rule;

        typedef boost::spirit::qi::rule<
            token_iterator,
            void(const boost::any&),
            skipper_type
        > position_rule;

        position_rule layout_specifier;

        void_rule qualified_cell_decl;
        void_rule interface_set_decl;
        void_rule constant_set_decl;

        boost::spirit::qi::rule<
            token_iterator,
            void(cell_type_t),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                adobe::name_t,
                adobe::line_position_t,
                adobe::array_t
            >,
            skipper_type
        > cell_decl;

        boost::spirit::qi::rule<
            token_iterator,
            void(adobe::line_position_t&, adobe::array_t&),
            skipper_type
        > initializer;

        boost::spirit::qi::rule<
            token_iterator,
            void(const boost::any&),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                adobe::name_t,
                adobe::array_t,
                adobe::line_position_t,
                boost::any
            >,
            skipper_type
        > view_definition;

        position_rule view_statment_sequence;

        boost::spirit::qi::rule<
            token_iterator,
            void(adobe::name_t&, adobe::array_t&),
            skipper_type
        > view_class_decl;

        position_rule view_statment_list;

        boost::spirit::qi::rule<token_iterator, void(std::string&), skipper_type> end_statement;

        boost::spirit::qi::rule<token_iterator, std::string(), skipper_type> lead_comment;
        boost::spirit::qi::rule<token_iterator, std::string(), skipper_type> trail_comment;

        const adobe::eve_callback_suite_t& callbacks;
    };

}

bool GG::Parse(const std::string& layout,
               const std::string& filename,
               const boost::any& parent,
               const adobe::eve_callback_suite_t& callbacks)
{
    using boost::spirit::qi::phrase_parse;
    text_iterator it(layout.begin());
    detail::s_text_it = &it;
    detail::s_begin = it;
    detail::s_end = text_iterator(layout.end());
    detail::s_filename = filename.c_str();
    token_iterator iter = EveLexer().begin(it, detail::s_end);
    token_iterator end = EveLexer().end();
    eve_parser_rules eve_rules(callbacks);
    return phrase_parse(iter,
                        end,
                        eve_rules.layout_specifier(boost::phoenix::cref(parent)),
                        boost::spirit::qi::in_state("WS")[EveLexer().self]);
}
