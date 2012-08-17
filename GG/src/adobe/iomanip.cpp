/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifdef ADOBE_STD_SERIALIZATION

/*************************************************************************************************/

#include <GG/adobe/iomanip.hpp>

#include <GG/adobe/string.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

int format_base_idx()
{
    static const int idx(std::ios_base::xalloc());
    return idx;
}

format_base* get_formatter(std::ostream& os)
{
    return static_cast<format_base*>(os.pword(format_base_idx()));
}

/*************************************************************************************************/
/*
    ADOBE_FORMAT_MANIP_BOILERPLATE is a standard iostream manipulator
    boilerplate that will get the adobe iomanip format object currently
    installed and call the relative function in that object.
*/
#define ADOBE_GLUE(x) x

#define ADOBE_FORMAT_MANIP_BOILERPLATE(x) \
format_base::stream_type& ADOBE_GLUE(x)(format_base::stream_type& os) \
    { \
    format_base* format(get_formatter(os)); \
    if (format) format->ADOBE_GLUE(x)(os); \
    return os; \
    }

/*************************************************************************************************/

ADOBE_FORMAT_MANIP_BOILERPLATE(begin_format)
ADOBE_FORMAT_MANIP_BOILERPLATE(end_format)

ADOBE_FORMAT_MANIP_BOILERPLATE(end_bag)

ADOBE_FORMAT_MANIP_BOILERPLATE(begin_sequence)
ADOBE_FORMAT_MANIP_BOILERPLATE(end_sequence)

ADOBE_FORMAT_MANIP_BOILERPLATE(begin_alternate)
ADOBE_FORMAT_MANIP_BOILERPLATE(end_alternate)

ADOBE_FORMAT_MANIP_BOILERPLATE(end_atom)

#undef ADOBE_FORMAT_MANIP_BOILERPLATE
#undef ADOBE_GLUE

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
