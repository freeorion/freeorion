/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_EXTENTS_HPP
#define ADOBE_EXTENTS_HPP

#include <GG/adobe/config.hpp>

#include <boost/array.hpp>
#include <boost/operators.hpp>

#include <vector>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

template <typename T = long>
struct point_2d : boost::equality_comparable<point_2d<T> >
{
    point_2d() :
        x_m(T()), y_m(T())
    { }

    point_2d(const T& x, const T& y) :
        x_m(x), y_m(y)
    { }

    T   x_m;
    T   y_m;

    friend inline void swap(const point_2d& x, const point_2d& y)
    {
        swap(x.x_m, y.x_m);
        swap(x.y_m, y.y_m);
    }

    friend inline bool operator==(const point_2d& x, const point_2d& y)
    {
        return (x.x_m == y.x_m) && (x.y_m == y.y_m);
    }
};

typedef std::pair<long, long>   pair_long_t;
typedef point_2d<long>          point_2d_t;
typedef std::vector<long>       guide_set_t;

// REVISIT (sparent) : points of interest need to be named entities. This will become:

#if 0
struct guide_set_t
{
    name_t name_m;
    long   offset_m;
};
typedef std::vector<guide_set_t> point_of_interest_set_t;
#endif

// REVISIT (sparent) : Open issue - are there "alignment" attributes on POIs?

struct extents_slices_t
{
    enum slice_select_t { horizontal, vertical };
};

/****************************************************************************************************/

struct extents_t :
#if !defined(ADOBE_NO_DOCUMENTATION)
    private extents_slices_t, boost::equality_comparable<extents_t>
#endif
{
    struct slice_t : boost::equality_comparable<slice_t>
    {
        slice_t() : length_m(0) { };

        long        length_m;
        pair_long_t outset_m;
        pair_long_t frame_m;
        pair_long_t inset_m;
        guide_set_t guide_set_m;
        
        friend bool operator ==(const slice_t& x, const slice_t& y);
    };

    boost::array<slice_t, 2>    slice_m;

    slice_t&                    vertical()          { return slice_m[extents_slices_t::vertical]; }
    slice_t&                    horizontal()        { return slice_m[extents_slices_t::horizontal]; }

    const slice_t&              vertical() const    { return slice_m[extents_slices_t::vertical]; }
    const slice_t&              horizontal() const  { return slice_m[extents_slices_t::horizontal]; }

    long&                       height()            { return vertical().length_m; }
    long&                       width()             { return horizontal().length_m; }

    const long&                 height() const      { return vertical().length_m; }
    const long&                 width() const       { return horizontal().length_m; }
    
    friend  bool operator == (const extents_t& x, const extents_t& y);
};

/****************************************************************************************************/

#ifndef NDEBUG
std::ostream&   operator << (std::ostream& s, const extents_t& x);
#endif

#ifndef NDEBUG
std::ostream&   operator << (std::ostream& s, const extents_t::slice_t& x);
#endif

/*************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

namespace std {

template <> inline void swap(adobe::extents_t::slice_t& x, adobe::extents_t::slice_t& y)
{
    swap(x.length_m, y.length_m);
    swap(x.outset_m, y.outset_m);
    swap(x.frame_m, y.frame_m);
    swap(x.inset_m, y.inset_m);
    swap(x.guide_set_m, y.guide_set_m);
}

template <> inline void swap(adobe::extents_t& x, adobe::extents_t& y)
{ swap(x.slice_m, y.slice_m); }

} // namespace std

/****************************************************************************************************/

#endif

/****************************************************************************************************/
