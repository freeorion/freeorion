/*
    Copyright 2006-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_LAYOUT_ATTRIBUTES_HPP
#define ADOBE_LAYOUT_ATTRIBUTES_HPP

#include <GG/adobe/config.hpp>
#include <GG/adobe/extents.hpp>

/*!
\defgroup layout_attributes Layout Attributes
\ingroup layout_library
*/


namespace adobe {

    /*************************************************************************************************/

/*!
\ingroup layout_attributes
*/
struct layout_attributes_alignment_t {   
    enum alignment_t
    {
        align_forward = 0, // Used as index for guide array.
        align_reverse = 1, // Used as index for guide array.
        align_center,
        align_proportional,
        align_forward_fill,
        align_reverse_fill,
        
        align_default,
        
        align_fill          = align_forward_fill,
        align_left_fill     = align_forward_fill,
        align_right_fill    = align_reverse_fill,
        align_top_fill      = align_forward_fill,
        align_bottom_fill   = align_reverse_fill,
        align_left          = align_forward,
        align_right         = align_reverse,
        align_top           = align_forward,
        align_bottom        = align_reverse
    };
};

/*!
\ingroup layout_attributes
*/
struct layout_attributes_placement_t {   
    enum placement_t
    {
        place_leaf,
        place_column,
        place_row,
        place_overlay
    };
};

/*!
\ingroup layout_attributes
*/
struct layout_attributes_t
#if !defined(ADOBE_NO_DOCUMENTATION)
    : public extents_slices_t, 
      public layout_attributes_alignment_t, 
      public layout_attributes_placement_t
#endif
{
    layout_attributes_t() :
        indent_m(0),
        create_m(true),
        spacing_m(2, 0),
        placement_m(place_leaf)
    {
        spacing_m[1] = 10; /* REVISIT FIXED VALUE container_spacing */
    }

    typedef std::vector<long>            spacing_t;
    struct slice_t
    {
        slice_t() : alignment_m(align_default),
                    suppress_m(false),
                    balance_m(false),
                    child_alignment_m(align_forward) {}

        alignment_t             alignment_m;
        bool                    suppress_m;
        bool                    balance_m;

        // containers only
        pair_long_t             margin_m;
        alignment_t             child_alignment_m;
    };
    
    extents_t                   extents_m;
    long                        indent_m;
    bool                        create_m;
    spacing_t                   spacing_m;
    boost::array<slice_t, 2>    slice_m;

    // containers only
    placement_t                 placement_m;

    slice_t&                    vertical()          { return slice_m[extents_slices_t::vertical]; }
    slice_t&                    horizontal()        { return slice_m[extents_slices_t::horizontal]; }

    const slice_t&              vertical() const    { return slice_m[extents_slices_t::vertical]; }
    const slice_t&              horizontal() const  { return slice_m[extents_slices_t::horizontal]; }


    long&                       height()            { return extents_m.height(); }
    long&                       width()             { return extents_m.width(); }

    const long&                 height() const      { return extents_m.height(); }
    const long&                 width() const       { return extents_m.width(); }
};

/*************************************************************************************************/

/*!
\ingroup layout_attributes
*/
struct place_data_t
#if !defined(ADOBE_NO_DOCUMENTATION)
    : extents_slices_t
#endif
{
    struct slice_t
    {
#if !defined(ADOBE_NO_DOCUMENTATION)
        slice_t();
#endif

        long        length_m;
        long        position_m;
        pair_long_t outset_m;
        guide_set_t guide_set_m;
    };

    boost::array<slice_t, 2>    slice_m;

    slice_t&                    vertical()          { return slice_m[extents_slices_t::vertical]; }
    slice_t&                    horizontal()        { return slice_m[extents_slices_t::horizontal]; }

    const slice_t&              vertical() const    { return slice_m[extents_slices_t::vertical]; }
    const slice_t&              horizontal() const  { return slice_m[extents_slices_t::horizontal]; }
};


/*!
\ingroup layout_attributes
*/
inline long top(const place_data_t& place_data)    { return place_data.vertical().position_m; }

/*!
\ingroup layout_attributes
*/
inline long left(const place_data_t& place_data)   { return place_data.horizontal().position_m; }

/*!
\ingroup layout_attributes
*/
inline long& top(place_data_t& place_data)         { return place_data.vertical().position_m; }

/*!
\ingroup layout_attributes
*/
inline long& left(place_data_t& place_data)        { return place_data.horizontal().position_m; }

/*!
\ingroup layout_attributes
*/
inline long height(const place_data_t& place_data) { return place_data.vertical().length_m; }

/*!
\ingroup layout_attributes
*/
inline long width(const place_data_t& place_data)  { return place_data.horizontal().length_m; }

/*!
\ingroup layout_attributes
*/
inline long& height(place_data_t& place_data)      { return place_data.vertical().length_m; }

/*!
\ingroup layout_attributes
*/
inline long& width(place_data_t& place_data)       { return place_data.horizontal().length_m; }

/*!
\ingroup layout_attributes
*/
inline long bottom(const place_data_t& place_data) { return top(place_data) + height(place_data); }

/*!
\ingroup layout_attributes
*/
inline long right(const place_data_t& place_data)  { return left(place_data) + width(place_data); }

/*************************************************************************************************/
}


#endif
