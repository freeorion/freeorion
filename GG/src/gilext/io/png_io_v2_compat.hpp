#ifndef GILEXT_PNG_IO_V2_COMPAT_H
#define GILEXT_PNG_IO_V2_COMPAT_H

namespace boost { namespace gil {

struct png_tag {};

template< typename Tag>
struct image_read_settings {};

template< typename String
        , typename Image
        , typename FormatTag
        >
inline
void read_image( const String&    file_name
               , Image&           image
               , const FormatTag& tag
               )
{ boost::gil::png_read_image(file_name, image); }

template< typename String
        , typename Image
        , typename FormatTag
        >
inline
void read_and_convert_image( const String&    file_name
                           , Image&           image
                           , const FormatTag& tag
                           )
{ boost::gil::png_read_and_convert_image(file_name, image); }
} }

template< typename String
        , typename View
        , typename FormatTag
        >
inline
void write_view( const String&    file_name
               , const View&      view
               , const FormatTag& tag
               )
{ boost::gil::png_write_view( file_name, view); }

#endif
