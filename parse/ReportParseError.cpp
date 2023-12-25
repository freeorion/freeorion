#include "ReportParseError.h"

#include "../util/Logger.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/xpressive/xpressive.hpp>

#include <iostream>
#include <tuple>

parse::detail::info_visitor::info_visitor(std::ostream& os, const string& tag, std::size_t indent) :
    m_os(os),
    m_tag(tag),
    m_indent(indent)
{}

void parse::detail::info_visitor::indent() const {
    if (m_indent > 0)
        m_os << std::string(m_indent, ' ');
}

std::string parse::detail::info_visitor::prepare(const string& s) const {
    if (s == parse::lexer::bool_regex)
        return "boolean (true or false)";
    else if (s == parse::lexer::string_regex)
        return "string";
    else if (s == parse::lexer::int_regex)
        return "integer";
    else if (s == parse::lexer::double_regex)
        return "real number";
    else if (s.find("(?i:") == 0)
        return s.substr(4, s.size() - 5);
    return s;
}

void parse::detail::info_visitor::print(const string& str) const
{ m_os << prepare(str); }

void parse::detail::info_visitor::operator()(boost::spirit::info::nil_) const {
    indent();
    print(m_tag);
}

void parse::detail::info_visitor::operator()(const string& str) const {
    indent();
    print(str);
}

void parse::detail::info_visitor::operator()(const boost::spirit::info& what) const
{ boost::apply_visitor(info_visitor(m_os, what.tag, m_indent), what.value); }

void parse::detail::info_visitor::operator()(const std::pair<boost::spirit::info, boost::spirit::info>& pair) const {
    const boost::spirit::info* infos = &pair.first;
    multi_info(infos, infos + 2);
}

void parse::detail::info_visitor::operator()(const std::list<boost::spirit::info>& l) const
{ multi_info(l.begin(), l.end()); }

template <typename Iter>
void parse::detail::info_visitor::multi_info(Iter first, const Iter last) const 
{
    if (m_tag == "sequence" || m_tag == "expect") {
        if (first->tag.find(" =") == first->tag.size() - 2)
            ++first;
        const string* value = boost::get<string>(&first->value);
        if (value && *value == "[") {
            for (; first != last; ++first)
                boost::apply_visitor(info_visitor(m_os, first->tag, 1), first->value);
        } else {
            boost::apply_visitor(info_visitor(m_os, first->tag, 1), first->value);
        }
    } else if (m_tag == "alternative") {
        boost::apply_visitor(info_visitor(m_os, first->tag, 1), first->value);
        indent();
        ++first;
        for (; first != last; ++first) {
            m_os << "-OR-";
            boost::apply_visitor(info_visitor(m_os, first->tag, 1), first->value);
        }
    }
}

void parse::detail::pretty_print(std::ostream& os, boost::spirit::info const& what) {
    info_visitor v(os, what.tag, 1);
    boost::apply_visitor(v, what.value);
}

void parse::detail::default_send_error_string(const std::string& str) {
    ErrorLogger() << str;
    // output to cout also so can be better associated with any output from parser semantic action debug output
    std::cout << str << std::endl;
}

std::function<void (const std::string&)> parse::report_error_::send_error_string =
    &detail::default_send_error_string;

namespace {
    std::vector<parse::text_iterator> LineStarts(const parse::text_iterator begin,
                                                 const parse::text_iterator end)
    {
        //DebugLogger() << "line starts start";
        using namespace parse;

        std::vector<text_iterator> retval;

        text_iterator it = begin;
        retval.push_back(it);   // first line

        // find subsequent lines
        while (it != end) {
            bool eol = false;
            text_iterator temp;

            // if line-ending char is found, store iterator to point after it
            if (*it == '\r') {
                eol = true;
                temp = ++it;
            }
            if (it != end && *it == '\n') {
                eol = true;
                temp = ++it;
            }

            if (eol && temp != end)
                retval.push_back(temp);
            else if (it != end)
                ++it;
        }

        //DebugLogger() << "line starts end.  num lines: " << retval.size();
        //for (unsigned int i = 0; i < retval.size(); ++i) {
        //    text_iterator line_end = retval[i];
        //    while (line_end != end && *line_end != '\r' && *line_end != '\n')
        //        ++line_end;
        //    DebugLogger() << " line " << i+1 << ": " << std::string(retval[i], line_end);
        //}
        return retval;
    }
}

std::pair<parse::text_iterator, unsigned int> parse::report_error_::line_start_and_line_number(
    const parse::text_iterator begin, const parse::text_iterator end, text_iterator error_position) const
{
    //DebugLogger() << "line_start_and_line_number start ... looking for: " << std::string(error_position, error_position + 20);
    if (error_position == begin)
        return std::pair(begin, 1);

    std::vector<parse::text_iterator> line_starts = LineStarts(begin, end);

    // search for the first line where the iterator to the start of the line is
    // at or past the error position
    for (unsigned int index = 0; index < line_starts.size(); ++index) {
        if (std::distance(line_starts[index], error_position) < 0 && index > 0) {
            //DebugLogger() << "line_start_and_line_number early end";
            return std::pair(line_starts[index-1], index); // return start of previous line, which contained the error_position text
        }
        //DebugLogger() << "line: " << index + 1 << " distance: " << std::distance(line_starts[index], error_position) << " : " << get_line(line_starts[index]);
    }

    //DebugLogger() << "line_start_and_line_number end";
    return std::pair(begin, 1);
}

std::string parse::report_error_::get_line(const parse::text_iterator& end, text_iterator line_start) const {
    text_iterator line_end = line_start;
    while (line_end != end && *line_end != '\r' && *line_end != '\n')
        ++line_end;
    return std::string(line_start, line_end);
}

std::string parse::report_error_::get_lines_before(
    const parse::text_iterator& begin, const parse::text_iterator& end, text_iterator line_start) const
{
    //DebugLogger() << "get_lines_before start";

    std::vector<parse::text_iterator> all_line_starts = LineStarts(begin, end);
    unsigned int target_line = 1;
    for (unsigned int line_minus_one = 0; line_minus_one < all_line_starts.size(); ++line_minus_one) {
        if (std::distance(all_line_starts[line_minus_one], line_start) < 0) {
            target_line = line_minus_one;   // want line before line that starts past the requested line_start
            break;
        }
    }
    if (target_line <= 1) {
        //DebugLogger() << "get_lines_before early end";
        return "";
    }
    //DebugLogger() << "get_lines_before line " << target_line;

    static constexpr unsigned int NUM_LINES = 5;
    unsigned int retval_first_line = 1;
    unsigned int retval_last_line = target_line - 1;
    if (retval_last_line > NUM_LINES)
        retval_first_line = retval_last_line - NUM_LINES + 1;

    //DebugLogger() << "get_lines_before showing lines " << retval_first_line << " to " << retval_last_line;
    return std::string(all_line_starts[retval_first_line-1], all_line_starts[retval_last_line]);    // start of first line to start of line after first line
}

std::string parse::report_error_::get_lines_after(
    const parse::text_iterator& begin, const parse::text_iterator& end, text_iterator line_start) const
{
    //DebugLogger() << "get_lines_after start";

    std::vector<parse::text_iterator> all_line_starts = LineStarts(begin, end);
    size_t target_line = 1;
    for (size_t line_minus_one = 0u; line_minus_one < all_line_starts.size(); ++line_minus_one) {
        if (std::distance(all_line_starts[line_minus_one], line_start) < 0) {
            target_line = line_minus_one;   // want line before line that starts past the requested line_start
            break;
        }
    }
    if (target_line >= all_line_starts.size()) {
        //DebugLogger() << "get_lines_after early end";
        return "";
    }
    //DebugLogger() << "get_lines_after line " << target_line;

    static constexpr size_t NUM_LINES = 5;
    const size_t retval_first_line = target_line + 1;
    size_t retval_last_line = all_line_starts.size();
    if (retval_first_line + NUM_LINES < all_line_starts.size())
        retval_last_line = retval_first_line + NUM_LINES - 1;

    text_iterator last_it = end;
    if (retval_last_line < all_line_starts.size())
        last_it = all_line_starts[retval_last_line];

    //DebugLogger() << "get_lines_after showing lines " << retval_first_line << " to " << retval_last_line;
    return std::string(all_line_starts[retval_first_line-1], last_it);
}

void parse::report_error_::generate_error_string(const std::string& filename,
                                                 const text_iterator& begin, const text_iterator& end,
                                                 const token_iterator& first,
                                                 const token_iterator& it,
                                                 const boost::spirit::info& rule_name,
                                                 std::string& str) const
{
    //DebugLogger() << "generate_error_string";
    std::stringstream is;

    text_iterator line_start;
    unsigned int line_number;
    text_iterator text_it = it->matched().begin();
    if (it->matched().begin() == it->matched().end()) {
        text_it = begin;
        if (text_it != end)
            ++text_it;
    }

    {
        text_iterator text_it_copy = text_it;
        while (text_it_copy != end && boost::algorithm::is_space()(*text_it_copy)) {
            ++text_it_copy;
        }
        if (text_it_copy != end)
            text_it = text_it_copy;
    }

    std::tie(line_start, line_number) = line_start_and_line_number(begin, end, text_it);
    std::size_t column_number = std::distance(line_start, text_it);
    //DebugLogger() << "generate_error_string found line number: " << line_number << " column number: " << column_number;

    is << filename << ":" << line_number << ":" << column_number << ": "
       << "Parse error.  Expected";

    {
        std::stringstream os;
        detail::pretty_print(os, rule_name);
        using namespace boost::xpressive;
        sregex regex = sregex::compile("(?<=\\[ ).+(?= \\])");
        is << regex_replace(os.str(), regex, "$&, ...");
    }

    if (text_it == end) {
        is << " before end of input.\n";
    } else {
        is << " here:\n";
        is << get_lines_before(begin, end, line_start);
        is << get_line(end, line_start) << "\n";
        is << std::string(column_number, ' ') << "^\n";
        is << get_lines_after(begin, end, line_start);
    }

    str = is.str();
}

const boost::phoenix::function<parse::report_error_> parse::report_error;
