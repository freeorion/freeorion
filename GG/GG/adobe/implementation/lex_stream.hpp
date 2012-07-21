/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_LEX_STREAM_HPP
#define ADOBE_LEX_STREAM_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/implementation/lex_stream_fwd.hpp>
#include <GG/adobe/istream.hpp>

#include <iosfwd>

/*************************************************************************************************/

void swap(adobe::lex_stream_t&, adobe::lex_stream_t&);

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

class lex_stream_t
{
public:
    lex_stream_t(std::istream& in, const line_position_t& position);

#if !defined(ADOBE_NO_DOCUMENTATION)
    lex_stream_t(const lex_stream_t& rhs);

    ~lex_stream_t();

    lex_stream_t& operator = (const lex_stream_t& rhs);
#endif // !defined(ADOBE_NO_DOCUMENTATION)

    const stream_lex_token_t&   get();

    void                        putback();

    const line_position_t&      next_position();

    void                        set_keyword_extension_lookup(const keyword_extension_lookup_proc_t& proc);

#if !defined(ADOBE_NO_DOCUMENTATION)
private:
    friend void ::swap(adobe::lex_stream_t&, adobe::lex_stream_t&);

    struct implementation_t;

    implementation_t* object_m;
#endif // !defined(ADOBE_NO_DOCUMENTATION)
};

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

inline void swap(adobe::lex_stream_t& x, adobe::lex_stream_t& y)
{
    std::swap(x.object_m, y.object_m);
}

/*************************************************************************************************/

#endif

/*************************************************************************************************/
