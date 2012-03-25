// -*- C++ -*-
#ifndef _ReportParseError_h_
#define _ReportParseError_h_

#include "Lexer.h"

#include <boost/tuple/tuple.hpp>
#include <boost/xpressive/xpressive.hpp>


namespace parse {
    namespace detail {
        struct info_visitor {
            typedef void result_type;
            typedef boost::spirit::utf8_string string;

            info_visitor(std::ostream& os, const string& tag, std::size_t indent);

            void indent() const;
            std::string prepare(const string& s) const;
            void print(const string& str) const;
            void operator()(boost::spirit::info::nil) const;
            void operator()(const string& str) const;
            void operator()(const boost::spirit::info& what) const;
            void operator()(const std::pair<boost::spirit::info, boost::spirit::info>& pair) const;
            void operator()(const std::list<boost::spirit::info>& l) const;
            template <typename Iter>
            void multi_info(Iter first, const Iter last) const;

            std::ostream& m_os;
            const string& m_tag;
            int m_indent;
        };

        void pretty_print(std::ostream& os, boost::spirit::info const& what);

        void default_send_error_string(const std::string& str);

        extern const char*      s_filename;
        extern text_iterator*   s_text_it;
        extern text_iterator    s_begin;
        extern text_iterator    s_end;
    }

    struct report_error_ {
        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
        struct result
        { typedef void type; };

        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
        void operator()(Arg1 first, Arg2, Arg3 it, Arg4 rule_name) const
            {
                std::string error_string;
                generate_error_string(first, it, rule_name, error_string);
                send_error_string(error_string);
            }

        static boost::function<void (const std::string&)> send_error_string;

    private:
        std::pair<text_iterator, unsigned int> line_start_and_line_number(text_iterator error_position) const;
        std::string get_line(text_iterator line_start) const;
        std::string get_lines_before(text_iterator line_start) const;
        std::string get_lines_after(text_iterator line_start) const;
        void generate_error_string(const token_iterator& first,
                                   const token_iterator& it,
                                   const boost::spirit::info& rule_name,
                                   std::string& str) const;
    };

    extern const boost::phoenix::function<report_error_> report_error;
}

#endif
