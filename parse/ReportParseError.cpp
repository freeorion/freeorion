#include "ReportParseError.h"


parse::detail::info_visitor::info_visitor(std::ostream& os, const string& tag, std::size_t indent) :
    m_os(os),
    m_tag(tag),
    m_indent(indent)
{}

void parse::detail::info_visitor::indent() const
{
    if (m_indent)
        m_os << std::string(m_indent, ' ');
}

std::string parse::detail::info_visitor::prepare(const string& s) const
{
    std::string str = s;
    if (str == parse::lexer::bool_regex)
        str = "boolean (true or false)";
    else if (str == parse::lexer::string_regex)
        str = "string";
    else if (str == parse::lexer::int_regex)
        str = "integer";
    else if (str == parse::lexer::double_regex)
        str = "real number";
    else if (str.find("(?i:") == 0)
        str = str.substr(4, str.size() - 5);
    return str;
}

void parse::detail::info_visitor::print(const string& str) const
{ m_os << prepare(str); }

void parse::detail::info_visitor::operator()(boost::spirit::info::nil) const
{
    indent();
    print(m_tag);
}

void parse::detail::info_visitor::operator()(const string& str) const
{
    indent();
    print(str);
}

void parse::detail::info_visitor::operator()(const boost::spirit::info& what) const
{ boost::apply_visitor(info_visitor(m_os, what.tag, m_indent), what.value); }

void parse::detail::info_visitor::operator()(const std::pair<boost::spirit::info, boost::spirit::info>& pair) const
{
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
            for (; first != last; ++first) {
                boost::apply_visitor(info_visitor(m_os, first->tag, 1), first->value);
            }
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

void parse::detail::pretty_print(std::ostream& os, boost::spirit::info const& what)
{
    info_visitor v(os, what.tag, 1);
    boost::apply_visitor(v, what.value);
}

void parse::detail::default_send_error_string(const std::string& str)
{ std::cerr << str; }

const char* parse::detail::s_filename = 0;
parse::text_iterator* parse::detail::s_text_it = 0;
parse::text_iterator parse::detail::s_begin;
parse::text_iterator parse::detail::s_end;

boost::function<void (const std::string&)> parse::report_error_::send_error_string =
    &detail::default_send_error_string;

std::pair<parse::text_iterator, unsigned int> parse::report_error_::line_start_and_line_number(text_iterator error_position) const
{
    unsigned int line = 1;
    text_iterator it = detail::s_begin;
    text_iterator line_start = detail::s_begin;
    while (it != error_position) {
        bool eol = false;
        if (it != error_position && *it == '\r') {
            eol = true;
            line_start = ++it;
        }
        if (it != error_position && *it == '\n') {
            eol = true;
            line_start = ++it;
        }
        if (eol)
            ++line;
        else
            ++it;
    }
    return std::pair<text_iterator, unsigned int>(line_start, line);
}

std::string parse::report_error_::get_line(text_iterator line_start) const
{
    text_iterator line_end = line_start;
    while (line_end != detail::s_end && *line_end != '\r' && *line_end != '\n') {
        ++line_end;
    }
    return std::string(line_start, line_end);
}

void parse::report_error_::generate_error_string(const token_iterator& first,
                                                 const token_iterator& it,
                                                 const boost::spirit::info& rule_name,
                                                 std::string& str) const
{
    std::stringstream is;

    text_iterator line_start;
    unsigned int line_number;
    text_iterator text_it = it->matched().begin();
    if (it->matched().begin() == it->matched().end()) {
        text_it = *detail::s_text_it;
        if (text_it != detail::s_end)
            ++text_it;
    }
    boost::tie(line_start, line_number) = line_start_and_line_number(text_it);
    std::size_t column_number = std::distance(line_start, text_it);

    is << detail::s_filename << ":" << line_number << ":" << column_number << ": "
       << "Parse error.  Expected";

    {
        std::stringstream os;
        detail::pretty_print(os, rule_name);
        using namespace boost::xpressive;
        sregex regex = sregex::compile("(?<=\\[ ).+(?= \\])");
        is << regex_replace(os.str(), regex, "$&, ...");
    }

    if (text_it == detail::s_end) {
        is << " before end of input.\n";
    } else {
        is << " here:\n"
           << "  " << get_line(line_start) << "\n"
           << "  " << std::string(column_number, ' ') << '^' << std::endl;
    }

    str = is.str();
}

const boost::phoenix::function<parse::report_error_> parse::report_error;
