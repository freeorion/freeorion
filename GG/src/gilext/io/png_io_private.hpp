/*
    Copyright 2005-2007 Adobe Systems Incorporated

    Copyright 2008 T. Zachary Laine
   
    Use, modification and distribution are subject to the Boost Software License,
    Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt).

    See http://stlab.adobe.com/gil for most recent version including documentation.
*/
/*************************************************************************************************/

#ifndef GILEXT_PNG_IO_PRIVATE_H
#define GILEXT_PNG_IO_PRIVATE_H

/// \file
/// \brief  Internal support for reading and writing PNG files
/// \author Hailin Jin and Lubomir Bourdev \n
///         Adobe Systems Incorporated
/// \date   2005-2007 \n Last updated August 14, 2007

#include <algorithm>
#include <vector>
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/io_error.hpp>
#include "../color_convert.hpp"
#include "../gray_alpha.hpp"
#include "../typedefs.hpp"

namespace boost { namespace gil {

namespace detail {

static const std::size_t PNG_BYTES_TO_CHECK = 4;

// lbourdev: These can be greatly simplified, for example:
template <typename Cs> struct png_color_type {static int constexpr color_type = 0;};
template<> struct png_color_type<gray_t>       { static int constexpr color_type = PNG_COLOR_TYPE_GRAY; };
template<> struct png_color_type<gray_alpha_t> { static int constexpr color_type = PNG_COLOR_TYPE_GRAY_ALPHA; };
template<> struct png_color_type<rgb_t>        { static int constexpr color_type = PNG_COLOR_TYPE_RGB; };
template<> struct png_color_type<rgba_t>       { static int constexpr color_type = PNG_COLOR_TYPE_RGBA; };

template <typename Channel,typename ColorSpace> struct png_is_supported {static bool constexpr value = false;};
template <> struct png_is_supported<boost::uint8_t,gray_t>        {static bool constexpr value = true;};
template <> struct png_is_supported<boost::uint8_t,gray_alpha_t>  {static bool constexpr value = true;};
template <> struct png_is_supported<boost::uint8_t,rgb_t>         {static bool constexpr value = true;};
template <> struct png_is_supported<boost::uint8_t,rgba_t>        {static bool constexpr value = true;};
template <> struct png_is_supported<boost::uint16_t,gray_t>       {static bool constexpr value = true;};
template <> struct png_is_supported<boost::uint16_t,gray_alpha_t> {static bool constexpr value = true;};
template <> struct png_is_supported<boost::uint16_t,rgb_t>        {static bool constexpr value = true;};
template <> struct png_is_supported<boost::uint16_t,rgba_t>       {static bool constexpr value = true;};

template <typename Channel,typename ColorSpace>
struct png_read_support_private {
    static bool constexpr is_supported = false;
    static int constexpr bit_depth = 0;
    static int constexpr color_type = 0;
};
template <>
struct png_read_support_private<boost::uint8_t,gray_t> {
    static bool constexpr is_supported = true;
    static int constexpr bit_depth = 8;
    static int constexpr color_type = PNG_COLOR_TYPE_GRAY;
};
template <>
struct png_read_support_private<boost::uint8_t,gray_alpha_t> {
    static bool constexpr is_supported = true;
    static int constexpr bit_depth = 8;
    static int constexpr color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
};
template <>
struct png_read_support_private<boost::uint8_t,rgb_t> {
    static bool constexpr is_supported = true;
    static int constexpr bit_depth = 8;
    static int constexpr color_type = PNG_COLOR_TYPE_RGB;
};
template <>
struct png_read_support_private<boost::uint8_t,rgba_t> {
    static bool constexpr is_supported = true;
    static int constexpr bit_depth = 8;
    static int constexpr color_type = PNG_COLOR_TYPE_RGBA;
};
template <>
struct png_read_support_private<boost::uint16_t,gray_t> {
    static bool constexpr is_supported = true;
    static int constexpr bit_depth = 16;
    static int constexpr color_type = PNG_COLOR_TYPE_GRAY;
};
template <>
struct png_read_support_private<boost::uint16_t,gray_alpha_t> {
    static bool constexpr is_supported = true;
    static int constexpr bit_depth = 16;
    static int constexpr color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
};
template <>
struct png_read_support_private<boost::uint16_t,rgb_t> {
    static bool constexpr is_supported = true;
    static int constexpr bit_depth = 16;
    static int constexpr color_type = PNG_COLOR_TYPE_RGB;
};
template <>
struct png_read_support_private<boost::uint16_t,rgba_t> {
    static bool constexpr is_supported = true;
    static int constexpr bit_depth = 16;
    static int constexpr color_type = PNG_COLOR_TYPE_RGBA;
};

template <typename Channel,typename ColorSpace>
struct png_write_support_private {
    static bool constexpr is_supported = false;
    static int constexpr bit_depth = 0;
    static int constexpr color_type = 0;
};
template <>
struct png_write_support_private<boost::uint8_t,gray_t> {
    static bool constexpr is_supported = true;
    static int constexpr bit_depth = 8;
    static int constexpr color_type = PNG_COLOR_TYPE_GRAY;
};
template<>
struct png_write_support_private<boost::uint8_t,gray_alpha_t> {
    static bool constexpr is_supported = true;
    static int constexpr bit_depth = 8;
    static int constexpr color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
};
template <>
struct png_write_support_private<boost::uint8_t,rgb_t> {
    static bool constexpr is_supported = true;
    static int constexpr bit_depth = 8;
    static int constexpr color_type = PNG_COLOR_TYPE_RGB;
};
template <>
struct png_write_support_private<boost::uint8_t,rgba_t> {
    static bool constexpr is_supported = true;
    static int constexpr bit_depth = 8;
    static int constexpr color_type = PNG_COLOR_TYPE_RGBA;
};
template <>
struct png_write_support_private<boost::uint16_t,gray_t> {
    static bool constexpr is_supported = true;
    static int constexpr bit_depth = 16;
    static int constexpr color_type = PNG_COLOR_TYPE_GRAY;
};
template <>
struct png_write_support_private<boost::uint16_t,gray_alpha_t> {
    static bool constexpr is_supported = true;
    static int constexpr bit_depth = 16;
    static int constexpr color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
};
template <>
struct png_write_support_private<boost::uint16_t,rgb_t> {
    static bool constexpr is_supported = true;
    static int constexpr bit_depth = 16;
    static int constexpr color_type = PNG_COLOR_TYPE_RGB;
};
template <>
struct png_write_support_private<boost::uint16_t,rgba_t> {
    static bool constexpr is_supported = true;
    static int constexpr bit_depth = 16;
    static int constexpr color_type = PNG_COLOR_TYPE_RGBA;
};

class png_reader : public file_mgr {
protected:
    png_structp _png_ptr;
    png_infop _info_ptr;

    void init() {
        char buf[PNG_BYTES_TO_CHECK];
        // read in some of the signature bytes
        io_error_if(fread(buf, 1, PNG_BYTES_TO_CHECK, get()) != detail::PNG_BYTES_TO_CHECK,
                    "png_check_validity: fail to read file");
        // compare the first PNG_BYTES_TO_CHECK bytes of the signature.
        io_error_if(png_sig_cmp((png_bytep)buf, (png_size_t)0, detail::PNG_BYTES_TO_CHECK)!=0,
                    "png_check_validity: invalid png file");

        _png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
        io_error_if(_png_ptr==NULL,"png_get_file_size: fail to call png_create_write_struct()");
        // allocate/initialize the image information data
        _info_ptr = png_create_info_struct(_png_ptr);
        if (_info_ptr == NULL) {
            png_destroy_read_struct(&_png_ptr,NULL,NULL);
            io_error("png_get_file_size: fail to call png_create_info_struct()");
        }
        if (setjmp(png_jmpbuf(_png_ptr))) {
            //free all of the memory associated with the png_ptr and info_ptr
            png_destroy_read_struct(&_png_ptr, &_info_ptr, NULL);
            io_error("png_get_file_size: fail to call setjmp()");
        }
        png_init_io(_png_ptr, get());
        png_set_sig_bytes(_png_ptr,PNG_BYTES_TO_CHECK);
        png_read_info(_png_ptr, _info_ptr);
        if (little_endian() && png_get_bit_depth(_png_ptr,_info_ptr)>8)
            png_set_swap(_png_ptr);
    }
public:
    png_reader(FILE* file          ) : file_mgr(file)           { init(); }
    png_reader(const char* filename) : file_mgr(filename, "rb") { init(); }

    ~png_reader() {
        png_destroy_read_struct(&_png_ptr,&_info_ptr,NULL);
    }
    point2<std::ptrdiff_t> get_dimensions() {
        return point2<std::ptrdiff_t>(png_get_image_width(_png_ptr,_info_ptr),
                                      png_get_image_height(_png_ptr,_info_ptr));
    }
    template <typename View>
    void apply(const View& view) {
        png_uint_32 width, height;
        int bit_depth, color_type, interlace_type;
        png_get_IHDR(_png_ptr, _info_ptr,
                     &width, &height,&bit_depth,&color_type,&interlace_type,
                     NULL, NULL);
        io_error_if(((png_uint_32)view.width()!=width || (png_uint_32)view.height()!= height),
                    "png_read_view: input view size does not match PNG file size");
        
        if(png_read_support_private<typename channel_type<View>::type,
                                    typename color_space_type<View>::type>::bit_depth!=bit_depth ||
           png_read_support_private<typename channel_type<View>::type,
                                    typename color_space_type<View>::type>::color_type!=color_type)
            io_error("png_read_view: input view type is incompatible with the image type");

        bool interlaced = interlace_type != PNG_INTERLACE_NONE;
        typedef pixel<
            typename channel_type<View>::type,
            layout<typename color_space_type<View>::type>
        > pixel_type;
        std::vector<pixel_type> image_data(interlaced ? width * height : width);
        if(interlaced) {
            std::vector<pixel_type*> row_ptrs(height);
            for(png_uint_32 y=0;y<height;++y) {
                row_ptrs[y] = &image_data[y * width];
            }
            png_read_image(_png_ptr,(png_bytepp)&row_ptrs.front());
        }
        for(png_uint_32 y=0;y<height;++y) {
            if(!interlaced)
                png_read_row(_png_ptr,(png_bytep)&image_data.front(),NULL);
            pixel_type* row = interlaced ? &image_data[y * width] : &image_data.front();
            std::copy(row,row+width,view.row_begin(y));
        }
        png_read_end(_png_ptr,NULL);
    }

    template <typename Image>
    void read_image(Image& im) {
        im.recreate(get_dimensions());
        apply(view(im));
    }
};

template <typename Pixel, typename PixelRef, typename View, typename CC>
void png_read_and_convert_pixels(const View& view, CC& cc,
                                 png_structp png_ptr, png_uint_32 width, png_uint_32 height,
                                 bool interlaced)
{
    std::vector<Pixel> image_data(interlaced ? width * height : width);
    if(interlaced) {
        std::vector<Pixel*> row_ptrs(height);
        for(png_uint_32 y=0;y<height;++y) {
            row_ptrs[y] = &image_data[y * width];
        }
        png_read_image(png_ptr,(png_bytepp)&row_ptrs.front());
    }
    for(png_uint_32 y=0;y<height;++y) {
        if(!interlaced)
            png_read_row(png_ptr,(png_bytep)&image_data.front(),NULL);
        Pixel* row = interlaced ? &image_data[y * width] : &image_data.front();
        std::transform(row,row+width,view.row_begin(y),color_convert_deref_fn<PixelRef,typename View::value_type,CC>(cc));
    }
}

// This code will be simplified...
template <typename CC>
class png_reader_color_convert : public png_reader {
private:
    CC _cc;
public:
    png_reader_color_convert(FILE* file          ,CC cc_in) : png_reader(file),_cc(cc_in) {}
    png_reader_color_convert(FILE* file          ) : png_reader(file) {}
    png_reader_color_convert(const char* filename,CC cc_in) : png_reader(filename),_cc(cc_in) {}
    png_reader_color_convert(const char* filename) : png_reader(filename) {}
    // the following line has been commented out to fix a compile error on OSX
    // png_reader_color_convert(const boost::filesystem::path& path) : png_reader(path) {}

    template <typename View>
    void apply(const View& view) {
        png_uint_32 width, height;
        int bit_depth, color_type, interlace_type;
        png_get_IHDR(_png_ptr, _info_ptr,
                     &width, &height,&bit_depth,&color_type,&interlace_type,
                     NULL, NULL);
        io_error_if(((png_uint_32)view.width()!=width || (png_uint_32)view.height()!= height),
                    "png_reader_color_convert::apply(): input view size does not match PNG file size");
        bool interlaced = interlace_type != PNG_INTERLACE_NONE;
        if(color_type == PNG_COLOR_TYPE_PALETTE) {
            color_type = PNG_COLOR_TYPE_RGB;
            bit_depth = 8;
            png_set_palette_to_rgb(_png_ptr);
        }
        if(png_get_valid(_png_ptr,_info_ptr,PNG_INFO_tRNS)) 
        {
            if(color_type == PNG_COLOR_TYPE_RGB)
                color_type = PNG_COLOR_TYPE_RGBA;
            else if(color_type == PNG_COLOR_TYPE_GRAY)
                color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
            png_set_tRNS_to_alpha(_png_ptr);
        }
        switch (color_type) {
        case PNG_COLOR_TYPE_GRAY:
            switch (bit_depth) {
            case 1:
            case 2:
            case 4:
            case 8:
                if(bit_depth < 8)
                    png_set_expand_gray_1_2_4_to_8(_png_ptr);
                png_read_and_convert_pixels<gray8_pixel_t,gray8_ref_t>(view,_cc,_png_ptr,width,height,interlaced);
                break;
            case 16:
                png_read_and_convert_pixels<gray16_pixel_t,gray16_ref_t>(view,_cc,_png_ptr,width,height,interlaced);
                break;
            default: io_error("png_reader_color_convert::apply(): unknown combination of color type and bit depth");
            }
            break;
        case PNG_COLOR_TYPE_RGB:
            switch (bit_depth) {
            case 8:
                png_read_and_convert_pixels<rgb8_pixel_t,rgb8_ref_t>(view,_cc,_png_ptr,width,height,interlaced);
                break;
            case 16:
                png_read_and_convert_pixels<rgb16_pixel_t,rgb16_ref_t>(view,_cc,_png_ptr,width,height,interlaced);
                break;
            default: io_error("png_reader_color_convert::apply(): unknown combination of color type and bit depth");
            }
            break;
        case PNG_COLOR_TYPE_RGBA:
            switch (bit_depth) {
            case 8:
                png_read_and_convert_pixels<rgba8_pixel_t,rgba8_ref_t>(view,_cc,_png_ptr,width,height,interlaced);
                break;
            case 16:
                png_read_and_convert_pixels<rgba16_pixel_t,rgba16_ref_t>(view,_cc,_png_ptr,width,height,interlaced);
                break;
            default: io_error("png_reader_color_convert::apply(): unknown combination of color type and bit depth");
            }
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            switch (bit_depth) {
            case 1:
            case 2:
            case 4:
            case 8:
                if(bit_depth < 8)
                    png_set_expand_gray_1_2_4_to_8(_png_ptr);
                png_read_and_convert_pixels<gray_alpha8_pixel_t,gray_alpha8_ref_t>(view,_cc,_png_ptr,width,height,interlaced);
                break;
            case 16:
                png_read_and_convert_pixels<gray_alpha16_pixel_t,gray_alpha16_ref_t>(view,_cc,_png_ptr,width,height,interlaced);
                break;
            default: io_error("png_reader_color_convert::apply(): unknown combination of color type and bit depth");
            }
            break;
        default: io_error("png_reader_color_convert::apply(): unknown color type");
        }
        png_read_end(_png_ptr,NULL);
    }
    template <typename Image>
    void read_image(Image& im) {
        im.recreate(get_dimensions());
        apply(view(im));
    }
};


class png_writer : public file_mgr {
protected:
    png_structp _png_ptr;
    png_infop _info_ptr;

    void init() {
        _png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
        io_error_if(!_png_ptr,"png_write_initialize: fail to call png_create_write_struct()");
        _info_ptr = png_create_info_struct(_png_ptr);
        if (!_info_ptr) {
            png_destroy_write_struct(&_png_ptr,NULL);
            io_error("png_write_initialize: fail to call png_create_info_struct()");
        }
        if (setjmp(png_jmpbuf(_png_ptr))) {
            png_destroy_write_struct(&_png_ptr, &_info_ptr);
            io_error("png_write_initialize: fail to call setjmp(png_jmpbuf())");
        }
        png_init_io(_png_ptr,get());
    }
public:
    png_writer(FILE* file          ) : file_mgr(file)           { init(); }
    png_writer(const char* filename) : file_mgr(filename, "wb") { init(); }

    ~png_writer() {
        png_destroy_write_struct(&_png_ptr,&_info_ptr);
    }
    template <typename View>
    void apply(const View& view) {
        png_set_IHDR(_png_ptr, _info_ptr, view.width(), view.height(),
                     png_write_support_private<typename channel_type<View>::type,
                                               typename color_space_type<View>::type>::bit_depth,
                     png_write_support_private<typename channel_type<View>::type,
                                               typename color_space_type<View>::type>::color_type,
                     PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
        png_write_info(_png_ptr,_info_ptr);
        if (little_endian() &&
            png_write_support_private<typename channel_type<View>::type,
                                      typename color_space_type<View>::type>::bit_depth>8)
            png_set_swap(_png_ptr);
        std::vector<pixel<typename channel_type<View>::type,
                          layout<typename color_space_type<View>::type> > > row(view.width());
        for(int y=0;y<view.height();++y) {
            std::copy(view.row_begin(y),view.row_end(y),row.begin());
            png_write_row(_png_ptr,(png_bytep)&row.front());
        }
        png_write_end(_png_ptr,_info_ptr);
    }
};

} // namespace detail
} }  // namespace boost::gil

#endif
