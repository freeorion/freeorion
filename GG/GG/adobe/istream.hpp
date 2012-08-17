/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ISTREAM_HPP
#define ADOBE_ISTREAM_HPP

#include <GG/adobe/config.hpp>

#include <ios>
#include <istream>
#include <stdexcept>
#include <vector>

#include <GG/adobe/name_fwd.hpp>
#include <GG/adobe/istream_fwd.hpp>

#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

#if 0
std::istream& getline(std::istream& is, std::string& str);
#endif

/*************************************************************************************************/

/*!
\class adobe::line_position_t
\ingroup asl_xml_parser

\brief A type detailing parser position information.

line_position_t is used to remember a position on a particular line of a file.
*/

/*!
\var adobe::line_position_t::line_number_m

type int to match __LINE__ token
*/

/*!
\var adobe::line_position_t::line_start_m

The stream position for the start of the line
*/

/*!
\var adobe::line_position_t::position_m

The stream position for the current character on the current line
*/

/*!
\fn adobe::line_position_t::line_position_t(adobe::name_t file_path, getline_proc_t getline_proc, int line_number, std::streampos line_start, std::streampos position)

line_number starts at 1.
*/

/*!
\fn adobe::line_position_t::line_position_t(const char*, int line_index)

This constructor is used with __FILE__ and __LINE__; line_index starts at 0
*/

//***************************************************************************//
//***************************************************************************//
//***************************************************************************//

/*!
\class adobe::stream_error_t
\ingroup asl_xml_parser

\brief An exception class thrown during parsing failures.
*/

/*!
\typedef adobe::stream_error_t::position_set_t

Stores a vector of <code>adobe::line_position_t</code>s so the exception can be decoded to trace from where it originated.
*/

/*!
\fn adobe::stream_error_t::stream_error_t(const ExceptionBase& base, const adobe::line_position_t& position)

Constructing from an arbitrary exception. It captures the value in base.what(), and moves on.

\param base The base exception from which the what() string is captured
\param position The stream information detailing position of failure.
*/

/*!
\fn adobe::stream_error_t::stream_error_t(adobe::stream_error_t& base, const adobe::line_position_t& position)

This contstructor will construct a list of positions and retain the exception information. This is used to report back traces of what went wrong.

\param base The base exception from which the what() string and previous position sets are captured
\param position The stream information detailing position of failure.
*/

/*!
\fn adobe::stream_error_t::stream_error_t(const char* what, const adobe::line_position_t& position)

\param what The string that is to become the what() parameter for this exception.
\param position The stream information detailing position of failure.
*/

/*!
\fn adobe::stream_error_t::stream_error_t(const std::string& what, const adobe::line_position_t& position)

\param what The string that is to become the what() parameter for this exception.
\param position The stream information detailing position of failure.
*/

/*!
\fn const adobe::stream_error_t::position_set_t& adobe::stream_error_t::line_position_set() const

\return
    The vector of <code>line_position_t</code>s detailing the trace history of this exception.
*/

//***************************************************************************//
//***************************************************************************//
//***************************************************************************//

/*!
\fn std::string adobe::format_stream_error(std::istream& stream, const adobe::stream_error_t& error);
\relates adobe::stream_error_t

A function used to format data stored in an adobe::stream_error_t into something human-readable.

\param stream The stream containing the parsing information from which the error came.
\param error The error detailing the cause for the parsing failure.

\return
    A string that presents the parsing failure in a human readable form. Note that the string is intended to be displayed on multiple lines with a monospaced font.
*/


// line_position_t is used to remember a position on a particular line of a file.

struct line_position_t
{
public:
    typedef boost::function<std::string (name_t, std::streampos)>    getline_proc_impl_t;
    typedef boost::shared_ptr<getline_proc_impl_t>                          getline_proc_t;

    // line_number starts at 1.
    line_position_t(    name_t   file_path,
                        getline_proc_t  getline_proc,
                        int             line_number = 1,
                        std::streampos  line_start = 0,
                        std::streampos  position = -1);

    // This constructor is used with __FILE__ and __LINE__, line_index starts at 0
    explicit line_position_t(const char*, int line_index = 0);

#if !defined(ADOBE_NO_DOCUMENTATION)
    line_position_t();
#endif // !defined(ADOBE_NO_DOCUMENTATION)

    const char* stream_name() const
        { return file_name_m.c_str(); }

    std::string file_snippet() const
    {
        return getline_proc_m ?
            (*getline_proc_m)(file_name_m, line_start_m) :
            std::string();
    }

    int             line_number_m; // type int to match __LINE__ token
    std::streampos  line_start_m;
    std::streampos  position_m;

#if !defined(ADOBE_NO_DOCUMENTATION)
private:
    name_t   file_name_m;
    getline_proc_t  getline_proc_m;
#endif // !defined(ADOBE_NO_DOCUMENTATION)
};

/*************************************************************************************************/

std::ostream& operator<<(std::ostream&, const line_position_t&);

/*************************************************************************************************/

class stream_error_t : public std::logic_error
{
 public:
    typedef std::vector<line_position_t> position_set_t;
 
    stream_error_t(const std::exception& base, const line_position_t& position) :
        std::logic_error(base.what())
     {
        try {
            const stream_error_t* error = dynamic_cast<const stream_error_t*>(&base);
                
            if (error) line_position_set_m = error->line_position_set_m;
            
            line_position_set_m.push_back(position);
        } catch (...) { }
    }
    
    stream_error_t(const char* what, const line_position_t& position) :
        std::logic_error(what),
        line_position_set_m(1, position)
    { }
    
    stream_error_t(const std::string& what, const line_position_t& position) :
        std::logic_error(what),
        line_position_set_m(1, position)
    { }

    const position_set_t& line_position_set() const
    { return line_position_set_m; }

#if !defined(ADOBE_NO_DOCUMENTATION)
    ~stream_error_t() throw()
    { }
 
private:
    position_set_t line_position_set_m;
#endif // !defined(ADOBE_NO_DOCUMENTATION)
};

/*************************************************************************************************/

/*
    REVISIT (sparent) : I'm not certain this code is correct when used with an istream iterator
    which might do line ending conversions with operator <<. Read up on this.
*/

/*
    REVISIT (sparent) : The interface to is_line_end should take an iterator pair and return
    a pair (first, bool) - not work on a reference to first.
*/

template <typename I> // I models InputIterator
bool is_line_end(I& first, I last)
{
    // Handle any type of line ending.

    typename std::iterator_traits<I>::value_type c(*first);

    if (c != '\n' && c != '\r') return false;

    ++first;

    if (c == '\r' && first != last && *first == '\n') ++first;

    return true;
}

/*************************************************************************************************/

template <typename I> // I models InputIterator
std::size_t is_line_end(I& first, I last, char c)
{
    // Handle any type of line ending.

    if (c == '\n') return 1;

    if (c == '\r')
    {
        if (first != last && *first == '\n')
        {
            ++first;

            return 2;
        }

        return 1;
    }

    return 0;
}

/*************************************************************************************************/

template <typename I> // I models InputIterator
std::pair<I, std::string> get_line(I first, I last)
{
    std::string result;
    
    while (first != last && !is_line_end(first, last))
    {
        result.append(1, *first);
        ++ first;
    }
    
    return std::make_pair(first, result);
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
