/*
    (c) Copyright 2005-2008 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/
// $Id: $ 
// $DateTime: $ 
// $Change: $ 
// $Author: $ 

/*************************************************************************************************/

#include <iomanip>
#include <sstream>
#include <ostream>

#include <GG/adobe/name.hpp>
#include <GG/adobe/istream.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

std::ostream& operator<<(std::ostream& result, const line_position_t& position)
{
    typedef std::streampos pos_t;

    const char* current_file ("");

    std::string         line_string(position.file_snippet());
    
    // Replace any tabs with spaces
    std::replace(line_string.begin(), line_string.end(), '\t', ' ');
    
    // Count any leading spaces
    std::string::size_type leading_spaces (line_string.find_first_not_of(' '));

    line_string.erase(0, leading_spaces);

    std::string::size_type trailing_nulls (line_string.find_last_not_of((char)0));

    if ( trailing_nulls != std::string::npos )
      line_string.erase(trailing_nulls+1);

    // Determine the carret position

    pos_t width = (position.position_m == pos_t(-1))
            ? pos_t(std::streamoff(line_string.size()))
            : pos_t(position.position_m - position.line_start_m);
        
    width -= std::streamoff(leading_spaces);
    
    int width_int (static_cast<int>(std::streamoff(width))); // shorten to an int
    
    // If the file name has changed then output it
    
    if (position.stream_name() && (std::strcmp(position.stream_name(), current_file) != 0))
    {
        current_file = position.stream_name();
        result << "File: " << current_file << '\n';
    }

    // By capitalizing Line and keeping char lowercase, it allows things to
    // line up in most proportional fonts.  By using the good part of the
    // line instead of spaces or other character constant, we allow the
    // arrows to be under the offending characters, even for a proportional
    // font.

    result << "Line " << std::setw(5) << std::setfill('0') << position.line_number_m;
    result << ": " << line_string << "\nchar ";
    result << std::setw(5) << std::setfill('0') << width_int;

    line_string.erase (width_int);
    result << ": " << line_string << "^^^\n";
    
    return result;
}

/*************************************************************************************************/

line_position_t::line_position_t(   adobe::name_t   file_path,
                                    getline_proc_t  getline_proc,
                                    int             line_number,
                                    std::streampos  line_start,
                                    std::streampos  position) :
        line_number_m(line_number),
        line_start_m(line_start),
        position_m(position),
        file_name_m(file_path),
        getline_proc_m(getline_proc)
    { }

line_position_t::line_position_t(const char* stream_name, int line_index) :
    line_number_m(line_index + 1),
    line_start_m(0),
    position_m(-1),
    file_name_m(adobe::name_t(stream_name))
{ }

#if !defined(ADOBE_NO_DOCUMENTATION)
line_position_t::line_position_t() :
        line_number_m(1),
        line_start_m(0),
        position_m(-1)
    { }
#endif // !defined(ADOBE_NO_DOCUMENTATION)

/*************************************************************************************************/

std::string format_stream_error(const stream_error_t& error)
{
    std::ostringstream  result;

    result << error.what() << '\n';

    for (stream_error_t::position_set_t::const_iterator iter (error.line_position_set().begin()),
            last (error.line_position_set().end()); iter != last; ++iter)
    {
        result << *iter;
    }

    return result.str();
}

/*************************************************************************************************/

std::string format_stream_error(std::istream& , const stream_error_t& error)
{
    // Format the error

    std::ostringstream  result;
    
    result << error.what() << '\n';
    
    for (stream_error_t::position_set_t::const_iterator iter (error.line_position_set().begin()),
            last (error.line_position_set().end()); iter != last; ++iter)
    {
        result << *iter;
    }
    
    return result.str();
}

/*************************************************************************************************/

#if 0
// REVISIT (sparent) : Why doesn't this return the string?
std::istream& getline(std::istream& is, std::string& str)
{
    typedef std::string::size_type  size_type;
    typedef std::char_traits<char>  traits_type;
    
    std::istream::sentry    ok(is, true);
    size_type               count(0);
    
    if (ok)
    {
        str.clear();
        while (true)
        {
            traits_type::int_type i = is.rdbuf()->sbumpc();
            if (i == traits_type::eof())
            {
                is.setstate(std::ios_base::eofbit);
                break;
            }
            char c (traits_type::to_char_type(i));
            ++count;
/*
    REVISIT (sparent) : I'm not real happy with the iterator interface for is_line_end() and in this
    case it is just plain wrong - constructing an istream_iterator consumes one character
    from the stream. Also, istream_iterator uses operator >> which does line ending interpretation.
*/

#if 0
            std::istream_iterator<char> first(is);
            if (is_line_end(first, std::istream_iterator<char>(), c)) break;
#endif
            std::istreambuf_iterator<char> first(is);
            if (is_line_end(first, std::istreambuf_iterator<char>(), c)) break;
            str.append(1, c);
        }
    }
    if (count == 0) is.setstate(std::ios_base::failbit);
    return is;
}
#endif

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

/*
    REVISIT (t_schwinger) : This is needed to disable automatic CR-LF conversion, which seems 
    to be broken in MinGW 3.2 (GCC 3.4).
*/
#ifdef __MINGW32__ 
extern "C"
{
    #include <fcntl.h>
    unsigned int _CRT_fmode = _O_BINARY;
}
#endif

/*************************************************************************************************/
