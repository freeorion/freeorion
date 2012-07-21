/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_LEX_SHARED_HPP
#define ADOBE_LEX_SHARED_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/implementation/lex_shared_fwd.hpp>
#include <GG/adobe/string.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/istream.hpp>
#include <GG/adobe/circular_queue.hpp>
#include <GG/adobe/implementation/parser_shared.hpp>

#include <iostream>
#include <cstdio>

#include <boost/function.hpp>
#include <boost/array.hpp>

/*************************************************************************************************/

#ifdef BOOST_MSVC
namespace std
{
    using ::isspace;
    using ::isdigit;
    using ::isalnum;
    using ::isalpha;
}
#endif

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

template <  std::size_t S,
            typename    E>  // E models Enumeration
struct lex_base_t
{
 public:
    typedef std::istream::pos_type      pos_type;
    typedef boost::function<void ()>    parse_token_proc_t;
    typedef lex_token_t<E>              token_type;

    lex_base_t(uchar_ptr_t first, uchar_ptr_t last, const line_position_t& position);

    virtual ~lex_base_t();

    const token_type&       get_token();
    void                    putback_token();
    void                    put_token(const token_type& token);

    void                    init_token(token_type& token)
        { token.range_m.first = first_m; }
    void                    finalize_token(token_type& token, E enumeration)
        { token.range_m.second = first_m; token.enum_m = enumeration; }
    void                    reset_lex(const token_type& token)
        { first_m = token.range_m.first; }

    void                    advance_lex()
        { ++first_m; }
    bool                    peek_char(char& c)
        { if (first_m == last_m) return false; c = *first_m; return true; }
    bool                    get_char(char& c)
        { bool result(peek_char(c)); if (result) advance_lex(); return result; }

    void                    set_lex_position(uchar_ptr_t p) { first_m = p; }

    void                    throw_exception(E expected, E found);
    void                    throw_parser_exception(const char* error_string);

    void                    set_skip_white_space(bool skip);

    void                    skip_white_space();

    const line_position_t&  next_position();

    bool                    is_eof();
    bool                    is_line_end();

    void                    set_parse_token_proc(parse_token_proc_t proc);

private:
    struct lex_fragment_t
    {
        lex_fragment_t( const token_type&       token = token_type(),
                        const line_position_t&  line_position = line_position_t()) :
            token_value_m(token), line_position_m(line_position)
        { }

        token_type      token_value_m;
        line_position_t line_position_m;
    };

    uchar_ptr_t                         first_m;
    uchar_ptr_t                         last_m;
    std::streampos                      streampos_m;
    line_position_t                     line_position_m;
    parse_token_proc_t                  parse_proc_m;
    bool                                skip_white_m;
    boost::array<char, 8>               putback_m; // stack-based is faster
    std::size_t                         index_m; // for putback_m

#if !defined(ADOBE_NO_DOCUMENTATION)
    circular_queue<lex_fragment_t>  last_token_m; // N token lookahead
#endif // !defined(ADOBE_NO_DOCUMENTATION)
};

/*************************************************************************************************/

template <std::size_t S, typename E>
lex_base_t<S, E>::lex_base_t(uchar_ptr_t first, uchar_ptr_t last, const line_position_t& position) :
    first_m(first),
    last_m(last),
    streampos_m(0),
    line_position_m(position),
    skip_white_m(true),
    index_m(0),
    last_token_m(S)
{ }

/*************************************************************************************************/

template <std::size_t S, typename E>
lex_base_t<S, E>::~lex_base_t()
{ }

/*************************************************************************************************/
    
template <std::size_t S, typename E>
const typename lex_base_t<S, E>::token_type& lex_base_t<S, E>::get_token()
{
    assert(parse_proc_m);

    if (last_token_m.empty())
    {
        if (skip_white_m)
            skip_white_space();

        line_position_m.position_m = streampos_m; // remember the start of the token position

        if (!is_eof())
            parse_proc_m();
    }

    token_type& result(last_token_m.front().token_value_m);

    last_token_m.pop_front();

    return result;
}

/*************************************************************************************************/
    
template <std::size_t S, typename E>
void lex_base_t<S, E>::put_token(const token_type& token)
{
    last_token_m.push_back(lex_fragment_t(token, line_position_m));
}

/*************************************************************************************************/

template <std::size_t S, typename E>
void lex_base_t<S, E>::putback_token()
{
    last_token_m.putback(); // REVISIT (sparent) : Check for overflow
}
    
/*************************************************************************************************/

template <std::size_t S, typename E>
const line_position_t& lex_base_t<S, E>::next_position()
{
/*
    REVISIT (sparent) : Clean this up - this primes the ring buffer so we can get the next position
*/
    get_token(); putback_token();

    return last_token_m.front().line_position_m;
}

/*************************************************************************************************/

template <std::size_t S, typename E>
bool lex_base_t<S, E>::is_line_end()
{
    using adobe::is_line_end;

    uchar_ptr_t old_first(first_m);

    bool result(is_line_end(first_m, last_m));

    if (result)
    {
        ++line_position_m.line_number_m;

        streampos_m += static_cast<std::streamoff>(std::distance(old_first, first_m));

        line_position_m.line_start_m = streampos_m;
    }

    return result;
}

/*************************************************************************************************/

template <std::size_t S, typename E>
bool lex_base_t<S, E>::is_eof()
{
    if (first_m != last_m) return false;

    put_token(token_type(eof_token<E>(), last_m, last_m));

    return true;
}

/*************************************************************************************************/

template <std::size_t S, typename E>
void lex_base_t<S, E>::throw_parser_exception(const char* error_string)
{
    using adobe::throw_parser_exception;

    throw_parser_exception(error_string, line_position_m);
}

/*************************************************************************************************/

template <std::size_t S, typename E>
void lex_base_t<S, E>::set_parse_token_proc(parse_token_proc_t proc)
{
    parse_proc_m = proc;
}

/*************************************************************************************************/

template <std::size_t S, typename E>
void lex_base_t<S, E>::set_skip_white_space(bool skip)
{
    skip_white_m = skip;
}

/*************************************************************************************************/

template <std::size_t S, typename E>
void lex_base_t<S, E>::skip_white_space()
{
    while (true)
    {
        (void)is_line_end();

        if (first_m == last_m) break;

        char c;

        if (peek_char(c) && std::isspace(static_cast<unsigned char>(c)))
            advance_lex();
        else
            break;
    }
}

/*************************************************************************************************/

#if 0
#pragma mark -
#endif

/*************************************************************************************************/
namespace implementation {

struct lex_fragment_t
{
    lex_fragment_t( stream_lex_token_t      token = stream_lex_token_t(),
                    const line_position_t&  line_position = line_position_t()) :
        token_value_m(::adobe::move(token)), line_position_m(line_position)
    { }

    lex_fragment_t(move_from<lex_fragment_t> x) :
        token_value_m(::adobe::move(x.source.token_value_m)), line_position_m(::adobe::move(x.source.line_position_m))
    { }
    
    lex_fragment_t& operator=(lex_fragment_t x)
    {
        token_value_m = ::adobe::move(x.token_value_m);
        line_position_m = ::adobe::move(x.line_position_m);
        return *this;
    }

    stream_lex_token_t  token_value_m;
    line_position_t     line_position_m;
};

} // namespace implementation

template <  std::size_t S,
            typename    I>  // I models InputIterator
struct stream_lex_base_t
{
 public:
    typedef std::istream::pos_type                          pos_type;
    typedef boost::function<void (char)>                    parse_token_proc_t;

    stream_lex_base_t(I first, I last, const line_position_t& position);

    virtual ~stream_lex_base_t();

    const stream_lex_token_t&   get_token();
    void                        putback_token();
    void                        put_token(stream_lex_token_t token);

    bool                    get_char(char& c);
    void                    putback_char(char c);
    int                     peek_char();
    void                    ignore_char();

    void                    throw_exception(const name_t& expected, const name_t& found);
    void                    throw_parser_exception(const char* error_string);

    void                    set_skip_white_space(bool skip);

    void                    skip_white_space();

    const line_position_t&  next_position();

    bool                    is_line_end(char c);

    void                    set_parse_token_proc(parse_token_proc_t proc);

    std::vector<char>       identifier_buffer_m;

private:
    I                                   first_m;
    I                                   last_m;
    std::streampos                      streampos_m;
    line_position_t                     line_position_m;
    parse_token_proc_t                  parse_proc_m;
    bool                                skip_white_m;
    boost::array<char, 8>               putback_m; // stack-based is faster
    std::size_t                         index_m; // for putback_m

#if !defined(ADOBE_NO_DOCUMENTATION)
    circular_queue<implementation::lex_fragment_t>  last_token_m; // N token lookahead
#endif // !defined(ADOBE_NO_DOCUMENTATION)
};

/*************************************************************************************************/

template <std::size_t S, typename I>
stream_lex_base_t<S, I>::stream_lex_base_t(I first, I last, const line_position_t& position) :
    identifier_buffer_m(128),
    first_m(first),
    last_m(last),
    streampos_m(1),
    line_position_m(position),
    skip_white_m(true),
    index_m(0),
    last_token_m(S)
{ }

/*************************************************************************************************/

template <std::size_t S, typename I>
stream_lex_base_t<S, I>::~stream_lex_base_t()
{ }

/*************************************************************************************************/

template <std::size_t S, typename I>
bool stream_lex_base_t<S, I>::get_char(char& c)
{
    if (index_m)
    {
        c = static_cast<char>(putback_m[index_m]);

        --index_m;

        streampos_m += 1;

        return true;
    }

    if (first_m == last_m) return false;

    c = static_cast<char>(*first_m);

    ++first_m;

    streampos_m += 1;

    return true;
}

/*************************************************************************************************/

template <std::size_t S, typename I>
void stream_lex_base_t<S, I>::putback_char(char c)
{
    putback_m[++index_m] = c;

    streampos_m -= 1;
}

/*************************************************************************************************/

template <std::size_t S, typename I>
int stream_lex_base_t<S, I>::peek_char()
{
    if (index_m)
        return putback_m[index_m];
    else if (first_m == last_m)
        return EOF;
    else
        return *first_m;
}

/*************************************************************************************************/

template <std::size_t S, typename I>
void stream_lex_base_t<S, I>::ignore_char()
{
    if (index_m)
        --index_m;
    else if (first_m == last_m)
        return;
    else
        ++first_m;

    streampos_m += 1;
}

/*************************************************************************************************/
    
template <std::size_t S, typename I>
const stream_lex_token_t& stream_lex_base_t<S, I>::get_token()
{
    assert(parse_proc_m);

    if (last_token_m.empty())
    {
        char c;

    /*
        REVISIT (sparent) : You can't get(c), tellg(), and then putback(c) on a stream - the 
        tellg() in the middle will cause the putback(c) to move the stream to an invalid state.
        So instead of calling skip_space(c) we break up the call with the tellg() in the middle,
        prior to the callback.
    */

        if (skip_white_m)
            skip_white_space();

        line_position_m.position_m = streampos_m; // remember the start of the token position

    /*
        REVISIT (sparent) : I don't like that eof is not handled as the other tokens are handled
        there should be a way to make this logic consistant.
    */

        if (!get_char(c)) // eof
            put_token(stream_lex_token_t(eof_k, any_regular_t()));
        else
            parse_proc_m(c);
    }

    stream_lex_token_t& result(last_token_m.front().token_value_m);

    last_token_m.pop_front();

    return result;
}

/*************************************************************************************************/
    
template <std::size_t S, typename I>
void stream_lex_base_t<S, I>::put_token(stream_lex_token_t token)
{
    last_token_m.push_back(implementation::lex_fragment_t(::adobe::move(token), line_position_m));
}

/*************************************************************************************************/

template <std::size_t S, typename I>
void stream_lex_base_t<S, I>::putback_token()
{
    last_token_m.putback(); // REVISIT (sparent) : Check for overflow
}
    
/*************************************************************************************************/

template <std::size_t S, typename I>
const line_position_t& stream_lex_base_t<S, I>::next_position()
{
/*
    REVISIT (sparent) : Clean this up - this primes the ring buffer so we can get the next position
*/
    get_token(); putback_token();

    return last_token_m.front().line_position_m;
}

/*************************************************************************************************/

template <std::size_t S, typename I>
bool stream_lex_base_t<S, I>::is_line_end(char c)
{
    using adobe::is_line_end;

    std::size_t num_chars_eaten(is_line_end(first_m, last_m, c));

    if (num_chars_eaten != 0)
    {
        ++line_position_m.line_number_m;

        if (num_chars_eaten == 2)
            streampos_m += 1;

        line_position_m.line_start_m = streampos_m;
    }

    return num_chars_eaten != 0;
}

/*************************************************************************************************/

template <std::size_t S, typename I>
void stream_lex_base_t<S, I>::throw_parser_exception(const char* error_string)
{
    using adobe::throw_parser_exception;

    throw_parser_exception(error_string, line_position_m);
}

/*************************************************************************************************/

template <std::size_t S, typename I>
void stream_lex_base_t<S, I>::set_parse_token_proc(parse_token_proc_t proc)
{
    parse_proc_m = proc;
}

/*************************************************************************************************/

template <std::size_t S, typename I>
void stream_lex_base_t<S, I>::set_skip_white_space(bool skip)
{
    skip_white_m = skip;
}

/*************************************************************************************************/

template <std::size_t S, typename I>
void stream_lex_base_t<S, I>::skip_white_space()
{
    char c;

    while (get_char(c))
    {
        // Handle any type of line ending.
        if (!is_line_end(c) && !std::isspace(c))
        {
            putback_char(c);

            break;
        }
    }
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
