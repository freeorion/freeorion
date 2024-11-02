//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/Config.h>
#include <iomanip>
#include <iostream>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/gil/extension/dynamic_image/any_image.hpp>
#if GG_HAVE_LIBTIFF
# include <boost/gil/extension/io/tiff_dynamic_io.hpp>
#endif
#if GG_HAVE_LIBPNG
# include <boost/gil/extension/io/png.hpp>
#endif
#include <GG/GLClientAndServerBuffer.h>
#include <GG/Texture.h>
#include <GG/utf8/checked.h>


using namespace GG;

namespace {
    template <typename T>
    constexpr T PowerOfTwo(T input)
    {
        T value{1};
        while (value < input)
            value *= 2;
        return value;
    }
}

///////////////////////////////////////
// class GG::Texture
///////////////////////////////////////
Texture::Texture()
{ Clear(); }

Texture::~Texture()
{ Clear(); }

void Texture::Blit(const GL2DVertexBuffer& vertex_buffer,
                   const GLTexCoordBuffer& tex_coord_buffer,
                   bool render_scaled) const
{
    if (m_opengl_id == 0)
        return;

    // HACK! This code ensures that unscaled textures are reproduced exactly, even
    // though they theoretically should be even when using non-GL_NEAREST* scaling.
    bool need_min_filter_change = !render_scaled && m_min_filter != GL_NEAREST;
    bool need_mag_filter_change = !render_scaled && m_mag_filter != GL_NEAREST;
    if (need_min_filter_change)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    if (need_mag_filter_change)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_2D);

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindTexture(GL_TEXTURE_2D, m_opengl_id);
    vertex_buffer.activate();
    tex_coord_buffer.activate();
    glDrawArrays(GL_QUADS, 0, static_cast<GLsizei>(vertex_buffer.size()));

    if (need_min_filter_change)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_min_filter);
    if (need_mag_filter_change)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_mag_filter);

    glPopClientAttrib();

    glPopAttrib();
}

void Texture::OrthoBlit(Pt pt1, Pt pt2, std::array<GLfloat, 4> tex_coords) const
{
    if (m_opengl_id == 0)
        return;

    bool render_scaled = (pt2.x - pt1.x) != m_default_width || (pt2.y - pt1.y) != m_default_height;

    GL2DVertexBuffer vertex_buffer;
    vertex_buffer.reserve(4);
    GLTexCoordBuffer tex_coord_buffer;
    tex_coord_buffer.reserve(4);
    InitBuffer(vertex_buffer, pt1, pt2);
    InitBuffer(tex_coord_buffer, tex_coords);

    Blit(vertex_buffer, tex_coord_buffer, render_scaled);
}

void Texture::OrthoBlit(Pt pt1, Pt pt2) const
{ OrthoBlit(pt1, pt2, m_tex_coords); }

void Texture::OrthoBlit(Pt pt) const
{ OrthoBlit(pt, pt + Pt(m_default_width, m_default_height), m_tex_coords); }

void Texture::InitBuffer(GL2DVertexBuffer& vertex_buffer, Pt pt1, Pt pt2)
{
    vertex_buffer.store(pt2.x, pt1.y);
    vertex_buffer.store(pt1.x, pt1.y);
    vertex_buffer.store(pt1.x, pt2.y);
    vertex_buffer.store(pt2.x, pt2.y);
}

void Texture::InitBuffer(GL2DVertexBuffer& vertex_buffer, float x1, float y1, float x2, float y2)
{
    vertex_buffer.store(x2, y1);
    vertex_buffer.store(x1, y1);
    vertex_buffer.store(x1, y2);
    vertex_buffer.store(x2, y2);
}

void Texture::InitBuffer(GLTexCoordBuffer& tex_coord_buffer, std::array<GLfloat, 4> tex_coords)
{
    tex_coord_buffer.store(tex_coords[2], tex_coords[1]);
    tex_coord_buffer.store(tex_coords[0], tex_coords[1]);
    tex_coord_buffer.store(tex_coords[0], tex_coords[3]);
    tex_coord_buffer.store(tex_coords[2], tex_coords[3]);
}

void Texture::Load(const boost::filesystem::path& path, bool mipmap)
{
    namespace gil = boost::gil;
    namespace fs = boost::filesystem;

    if (m_opengl_id)
        Clear();

    // convert path into UTF-8 format filename string for potential error reporting
    // but do the work only if actually throwing error to log
    constexpr auto loggable_path = [](const fs::path& p) {
#if defined (_WIN32)
        boost::filesystem::path::string_type path_native = p.native();
        std::string filename;
        utf8::utf16to8(path_native.begin(), path_native.end(), std::back_inserter(filename));
        return filename;
#else
        return p.generic_string();
#endif
    };

    if (!fs::exists(path)) {
        std::cerr << "Texture::Load passed non-existant path: " << path.generic_string() << std::endl;
        throw BadFile("Texture file \"" + loggable_path(path) + "\" does not exist");
    }
    if (!fs::is_regular_file(path)) {
        std::cerr << "Texture::Load passed non-file path: " << path.generic_string() << std::endl;
        throw BadFile("Texture \"file\" \"" + loggable_path(path) + "\" is not a file");
    }

    static_assert(sizeof(gil::gray8_pixel_t) == 1, "gray8 pixel type does not match expected type size");
    static_assert(sizeof(gil::gray_alpha8_pixel_t) == 2, "gray_alpha8 pixel type does not match expected type size");
    static_assert(sizeof(gil::rgb8_pixel_t) == 3, "rgb8 pixel type does not match expected type size");
    static_assert(sizeof(gil::rgba8_pixel_t) == 4, "rgba8 pixel type does not match expected type size");

#if BOOST_VERSION >= 107400
    typedef gil::any_image<gil::gray8_image_t,
        gil::gray_alpha8_image_t,
        gil::rgb8_image_t,
        gil::rgba8_image_t> ImageType;
#else
# ifdef BOOST_GIL_USES_MP11
    typedef boost::mp11::mp_list<
# else
    typedef boost::mpl::vector4<
# endif
        gil::gray8_image_t,
        gil::gray_alpha8_image_t,
        gil::rgb8_image_t,
        gil::rgba8_image_t
    > ImageTypes;
    typedef gil::any_image<ImageTypes> ImageType;
#endif

    std::string extension = boost::algorithm::to_lower_copy(path.extension().string());
    ImageType image;
    try {
        // First attempt -- try just to read the file in one of the default formats above.
        // using ifstream version instead of file name version of read_image goes around unicode paths translation issues
#if GG_HAVE_LIBPNG
        if (extension == ".png") {
            boost::filesystem::ifstream in(path, std::ios::binary);
            gil::read_image(in, image, gil::image_read_settings<gil::png_tag>());
        }
        else
#endif
#if GG_HAVE_LIBTIFF
        if (extension == ".tif" || extension == ".tiff") {
            boost::filesystem::ifstream in(path, std::ios::binary);
            gil::read_image(in, image, gil::image_read_settings<gil::tiff_tag>());
        }
        else
#endif
            throw BadFile("Texture file \"" + loggable_path(path) + "\" does not have a supported file extension");
    } catch (const std::ios_base::failure&) {
        // Second attempt -- If *_read_image() throws, see if we can convert
        // the image to RGBA.  This is needed for color-indexed images.
#if GG_HAVE_LIBPNG
        if (extension == ".png") {
            gil::rgba8_image_t rgba_image;
            boost::filesystem::ifstream in(path, std::ios::binary);
            gil::read_and_convert_image(in, rgba_image, gil::image_read_settings<gil::png_tag>());
            image = std::move(rgba_image);
        }
#endif
#if GG_HAVE_LIBTIFF
        if (extension == ".tif" || extension == ".tiff") {
            gil::rgba8_image_t rgba_image;
            boost::filesystem::ifstream in(path, std::ios::binary);
            gil::read_and_convert_image(in, rgba_image, gil::image_read_settings<gil::tiff_tag>());
            image = std::move(rgba_image);
        }
#endif
    }

    m_path = path;
    m_default_width = X(image.width());
    m_default_height = Y(image.height());
    m_type = GL_UNSIGNED_BYTE;

#if BOOST_VERSION >= 107400
#define IF_IMAGE_TYPE_IS(image_prefix)                                  \
    if (boost::variant2::get_if<image_prefix ## _image_t>(&image)) {    \
        m_bytes_pp = sizeof(image_prefix ## _pixel_t);                  \
        image_data = interleaved_view_get_raw_data(                     \
            const_view(boost::variant2::get<image_prefix ## _image_t>(image))); \
    }
#elif BOOST_VERSION >= 107000
#define IF_IMAGE_TYPE_IS(image_prefix)                                  \
    if (boost::get<image_prefix ## _image_t>(&image)) {                 \
        m_bytes_pp = sizeof(image_prefix ## _pixel_t);                  \
        image_data = interleaved_view_get_raw_data(                     \
            const_view(boost::get<image_prefix ## _image_t>(image)));   \
    }
#else
#define IF_IMAGE_TYPE_IS(image_prefix)                                  \
    if (image.current_type_is<image_prefix ## _image_t>()) {            \
        m_bytes_pp = sizeof(image_prefix ## _pixel_t);                  \
        image_data = interleaved_view_get_raw_data(                     \
            const_view(image._dynamic_cast<image_prefix ## _image_t>())); \
    }
#endif

    const uint8_t* image_data = nullptr;

    IF_IMAGE_TYPE_IS(gil::gray8)
    else IF_IMAGE_TYPE_IS(gil::gray_alpha8)
    else IF_IMAGE_TYPE_IS(gil::rgb8)
    else IF_IMAGE_TYPE_IS(gil::rgba8)

#undef IF_IMAGE_TYPE_IS

    switch (m_bytes_pp) {
    case 1:  m_format = GL_LUMINANCE; break;
    case 2:  m_format = GL_LUMINANCE_ALPHA; break;
    case 3:  m_format = GL_RGB; break;
    case 4:  m_format = GL_RGBA; break;
    default: throw BadFile("Texture file \"" + loggable_path(path) + "\" does not have a supported number of color channels (1-4)");
    }

    assert(image_data);
    Init(m_default_width, m_default_height, image_data, m_format, m_type, m_bytes_pp, mipmap);
}

void Texture::Init(X width, Y height, const uint8_t* image, GLenum format, GLenum type,
                   unsigned int bytes_per_pixel, bool mipmap)
{
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    glPixelStorei(GL_UNPACK_SWAP_BYTES, false);
    glPixelStorei(GL_UNPACK_LSB_FIRST, false);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    try {
        InitFromRawData(width, height, image, format, type, bytes_per_pixel, mipmap);
    } catch (...) {
        glPopClientAttrib();
        throw;
    }

    glPopClientAttrib();
}

void Texture::SetFilters(GLenum min, GLenum mag)
{
    m_min_filter = min;
    m_mag_filter = mag;
    if (m_opengl_id) {
        glBindTexture(GL_TEXTURE_2D, m_opengl_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_mag_filter);
    }
}

void Texture::Clear()
{
    if (m_opengl_id)
        glDeleteTextures(1, &m_opengl_id);

    m_path.clear();

    m_bytes_pp = 4;
    m_default_width = m_width = X0;
    m_default_height = m_height = Y0;

    m_wrap_s = m_wrap_t = GL_REPEAT;
    m_min_filter = GL_LINEAR_MIPMAP_LINEAR;
    m_mag_filter = GL_LINEAR;

    m_mipmaps = false;
    m_opengl_id = 0;
    m_format = GL_INVALID_ENUM;
    m_type = GL_INVALID_ENUM;

    m_tex_coords[0] = m_tex_coords[1] = 0.0f;   // min x, y
    m_tex_coords[2] = m_tex_coords[3] = 1.0f;   // max x, y
}

void Texture::InitFromRawData(X width, Y height, const uint8_t* image, GLenum format, GLenum type,
                              unsigned int bytes_per_pixel, bool mipmap)
{
    if (!image)
        return;

    if (m_opengl_id)
        Clear();

    X GL_texture_width = PowerOfTwo(width);
    Y GL_texture_height = PowerOfTwo(height);

    glGenTextures(1, &m_opengl_id);
    glBindTexture(GL_TEXTURE_2D, m_opengl_id);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrap_t);

    if (mipmap) {
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    }

    glTexImage2D(GL_PROXY_TEXTURE_2D, 0, format, Value(GL_texture_width), Value(GL_texture_height), 0, format, type, nullptr);
    GLint checked_format;
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &checked_format);
    if (!checked_format)
        throw InsufficientResources("Insufficient resources to create requested OpenGL texture");
    bool image_is_power_of_two = width == GL_texture_width && height == GL_texture_height;
    if (image_is_power_of_two) {
        glTexImage2D(GL_TEXTURE_2D, 0, format, Value(width), Value(height), 0, format, type, image);
    } else {
        std::vector<uint8_t> zero_data(bytes_per_pixel * Value(GL_texture_width) * Value(GL_texture_height));
        glTexImage2D(GL_TEXTURE_2D, 0, format, Value(GL_texture_width), Value(GL_texture_height), 0, format, type, zero_data.data());
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Value(width), Value(height), format, type, image);
    }

    m_mipmaps = mipmap;
    m_default_width = width;
    m_default_height = height;
    m_bytes_pp = bytes_per_pixel;
    {
        GLint w, h;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
        m_width = X(w);
        m_height = Y(h);
    }
    m_tex_coords[2] = static_cast<GLfloat>(m_default_width) / Value(m_width);
    m_tex_coords[3] = static_cast<GLfloat>(m_default_height) / Value(m_height);
}

std::vector<uint8_t> Texture::GetRawBytes()
{
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    glPixelStorei(GL_PACK_SWAP_BYTES, false);
    glPixelStorei(GL_PACK_LSB_FIRST, false);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // get pixel data
    const auto num_bytes = Value(m_width) * Value(m_height) * m_bytes_pp;
    std::vector<uint8_t> retval(num_bytes, 0u);
    glGetTexImage(GL_TEXTURE_2D, 0, m_format, m_type, retval.data());
    glPopClientAttrib();
    return retval;
}


///////////////////////////////////////
// class GG::SubTexture
///////////////////////////////////////
SubTexture::SubTexture(std::shared_ptr<const Texture> texture, X x1, Y y1, X x2, Y y2) :
    m_texture(std::move(texture)),
    m_width(x2 - x1),
    m_height(y2 - y1)
{
    if (!m_texture) throw BadTexture("Attempted to construct subtexture from invalid texture");
    if (x2 < x1 || y2 < y1) throw InvalidTextureCoordinates("Attempted to construct subtexture from invalid coordinates");

    m_tex_coords[0] = static_cast<GLfloat>(x1) / Value(m_texture->Width());
    m_tex_coords[1] = static_cast<GLfloat>(y1) / Value(m_texture->Height());
    m_tex_coords[2] = static_cast<GLfloat>(x2) / Value(m_texture->Width());
    m_tex_coords[3] = static_cast<GLfloat>(y2) / Value(m_texture->Height());
}

SubTexture::SubTexture(std::shared_ptr<const Texture> texture) :
    m_texture(std::move(texture)),
    m_width(GG::X1),
    m_height(GG::Y1)
{
    if (!m_texture) throw BadTexture("Attempted to construct subtexture from invalid texture");

    m_width = m_texture->Width();
    m_height = m_texture->Height();

    m_tex_coords[0] = 0.0f;
    m_tex_coords[1] = 0.0f;
    m_tex_coords[2] = 1.0f;
    m_tex_coords[3] = 1.0f;
}

SubTexture::SubTexture(const SubTexture& rhs)
{ *this = rhs; }

SubTexture& SubTexture::operator=(const SubTexture& rhs)
{
    if (this != &rhs) {
        m_texture = rhs.m_texture;
        m_width = rhs.m_width;
        m_height = rhs.m_height;
        m_tex_coords[0] = rhs.m_tex_coords[0];
        m_tex_coords[1] = rhs.m_tex_coords[1];
        m_tex_coords[2] = rhs.m_tex_coords[2];
        m_tex_coords[3] = rhs.m_tex_coords[3];
    }
    return *this;
}

SubTexture& SubTexture::operator=(SubTexture&& rhs) noexcept
{
    if (this != &rhs) {
        m_texture = std::move(rhs.m_texture);
        m_width = rhs.m_width;
        m_height = rhs.m_height;
        m_tex_coords[0] = rhs.m_tex_coords[0];
        m_tex_coords[1] = rhs.m_tex_coords[1];
        m_tex_coords[2] = rhs.m_tex_coords[2];
        m_tex_coords[3] = rhs.m_tex_coords[3];
    }
    return *this;
}

void SubTexture::OrthoBlit(Pt pt1, Pt pt2) const
{ if (m_texture) m_texture->OrthoBlit(pt1, pt2, m_tex_coords); }

void SubTexture::OrthoBlit(Pt pt) const
{ if (m_texture) m_texture->OrthoBlit(pt, pt + Pt(m_width, m_height), m_tex_coords); }

void SubTexture::Clear()
{
    m_texture.reset();
    m_width = X0;
    m_height = Y0;
    m_tex_coords[0] = 0.0f;
    m_tex_coords[1] = 0.0f;
    m_tex_coords[2] = 1.0f;
    m_tex_coords[3] = 1.0f;
}

///////////////////////////////////////
// class GG::TextureManager
///////////////////////////////////////
TextureManager::TextureManager()
{}

std::map<std::string_view, std::shared_ptr<const Texture>> TextureManager::Textures() const
{
    std::scoped_lock lock(m_texture_access_guard);
    return {m_textures.begin(), m_textures.end()};
}

std::shared_ptr<Texture> TextureManager::StoreTexture(Texture* texture, std::string texture_name)
{ return StoreTexture(std::shared_ptr<Texture>(texture), std::move(texture_name)); }

std::shared_ptr<Texture> TextureManager::StoreTexture(std::shared_ptr<Texture> texture, std::string texture_name)
{
    std::scoped_lock lock(m_texture_access_guard);
    m_textures[std::move(texture_name)] = texture;
    return texture;
}

std::shared_ptr<Texture> TextureManager::GetTexture(const boost::filesystem::path& path, bool mipmap)
{
    std::scoped_lock lock(m_texture_access_guard);
    auto it = m_textures.find(path.generic_string());
    if (it == m_textures.end()) { // if no such texture was found, attempt to load it now, using name as the filename
        //std::cout << "TextureManager::GetTexture storing new texture under name: " << path.generic_string();
        return LoadTexture(path, mipmap);
    } else { // otherwise, just return the found texture
        return it->second;
    }
}

void TextureManager::FreeTexture(const boost::filesystem::path& path)
{ FreeTexture(path.generic_string()); }

void TextureManager::FreeTexture(const std::string& name)
{
    std::scoped_lock lock(m_texture_access_guard);
    auto it = m_textures.find(name);
    if (it != m_textures.end())
        m_textures.erase(it);
}

std::shared_ptr<Texture> TextureManager::LoadTexture(const boost::filesystem::path& path, bool mipmap)
{
    // only called from other TextureManager functions that should already have locked m_texture_access_guard
    auto temp = std::make_shared<Texture>();
    temp->Load(path, mipmap);
    m_textures[path.generic_string()] = temp;
    return temp;
}

TextureManager& GG::GetTextureManager()
{
    static TextureManager manager;
    return manager;
}
