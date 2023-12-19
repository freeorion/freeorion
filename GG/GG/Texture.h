//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/Texture.h
//!
//! Contains the Texture class, which encapsulates an OpenGL texture object;
//! the SubTexture class, which represents a portion of an OpenGL texture
//! object; and the TextureManager class, which provides GUI-wide management
//! of Texture objects.

#ifndef _GG_Texture_h_
#define _GG_Texture_h_


#include <boost/filesystem/path.hpp>
#include <GG/Base.h>
#include <GG/Exception.h>
#include <mutex>


namespace GG {
    class GL2DVertexBuffer;
    class GLTexCoordBuffer;

/** \brief This class encapsulates an OpenGL texture object.

    If the dimensions of the image used to initialize the texture are not both
    powers of two, the texture is created with dimensions of the next largest
    (or equal) powers of two.  The original image occupies the region near the
    texture's origin, and the rest is zero-initialized.  This is done to
    prevent the image from being scaled, since textures used in a GUI almost
    always must maintain pixel accuracy.  The original image size and
    corresponding texture coords are saved, and can be accessed through
    DefaultWidth(), DefaultHeight(), and DefaultTexCoords(), respectively.
    These are kept so that only the originally-loaded-image part of the
    texture can be used, if desired.  All initialization functions first free
    the OpenGL texture currently in use by the texture (if any) and create a
    new one.  When the load filename is "" or the image parameter is 0, all
    initialization functions fail silently, performing no initialization and
    allocating no memory or OpenGL texture.  Serialized Textures save the
    filename associated with the texture when available, so the originally
    loaded file can be reloaded again later.  If no such file exists, such as
    when a Texture is created from in-memory image data, the contents of the
    Texture are read from video memory and saved as binary data.  A
    default-constructed Texture will have niether a filename nor raw image
    data. */
class GG_API Texture
{
public:
    Texture();

    virtual ~Texture();

    auto& Path() const noexcept { return m_path; } ///< file path from which this texture was loaded (default / empty if this texture was not loaded from a file)

    GLenum           WrapS() const noexcept { return m_wrap_s; }                ///< returns S-wrap mode associated with this opengl texture
    GLenum           WrapT() const noexcept { return m_wrap_t; }                ///< returns T-wrap mode associated with this opengl texture
    GLenum           MinFilter() const noexcept { return m_min_filter; }        ///< returns minimization filter modes associated with this opengl texture
    GLenum           MagFilter() const noexcept { return m_mag_filter; }        ///< returns maximization filter modes associated with this opengl texture
    unsigned int     BytesPP() const noexcept { return m_bytes_pp; }            ///< returns the image's color depth in bytes
    X                Width() const noexcept { return m_width; }                 ///< returns width of entire texture
    Y                Height() const noexcept { return m_height; }               ///< returns height of entire texture
    bool             MipMapped() const noexcept { return m_mipmaps; }           ///< returns true if the texture has mipmaps
    GLuint           OpenGLId() const noexcept { return m_opengl_id; }          ///< GLuint "name" of the opengl texture object associated with this object
    X                DefaultWidth() const noexcept { return m_default_width; }  ///< returns width in pixels, based on initial image (0 if texture was not loaded)
    Y                DefaultHeight() const noexcept { return m_default_height; }///< returns height in pixels, based on initial image (0 if texture was not loaded)

    std::array<GLfloat, 4> DefaultTexCoords() const noexcept { return m_tex_coords; }///< texture coordinates to use by default when blitting this texture

    /** Blit any portion of texture to any place on screen, scaling as
        necessary*/
    void OrthoBlit(Pt pt1, Pt pt2, std::array<GLfloat, 4> tex_coords) const;
    void OrthoBlit(Pt pt1, Pt pt2) const;
    void Blit(const GL2DVertexBuffer& vertex_buffer, const GLTexCoordBuffer& tex_coord_buffer,
              bool render_scaled = true) const;

    /** Fill \a vertex_buffer and with vertex data for the quad spanning between
        \a pt1 and \a pt2 */
    static void InitBuffer(GL2DVertexBuffer& vertex_buffer, Pt pt1, Pt pt2);
    static void InitBuffer(GL2DVertexBuffer& vertex_buffer, float x1, float y1, float x2, float y2);

    /** Fill \a tex_coord_buff with texture coordinate data for the texture
      * coords specified by \a tex_coords */
    static void InitBuffer(GLTexCoordBuffer& tex_coord_buffer,
                           std::array<GLfloat, 4> tex_coords = {0.0, 0.0, 1.0, 1.0});

    /** Blit default portion of texture unscaled to \a pt (upper left
        corner)*/
    void OrthoBlit(Pt pt) const;

    // intialization functions
    /** Frees any currently-held memory and loads a texture from file \a
        path.  \throw GG::Texture::BadFile Throws if the texture creation
        fails. */
    void Load(const boost::filesystem::path& path, bool mipmap = false);

    /** Frees any currently-held memory and creates a texture from supplied
        array \a image.  \throw GG::Texture::Exception Throws applicable
        subclass if the texture creation fails in one of the specified
        ways. */
    void Init(X width, Y height, const uint8_t* image, GLenum format, GLenum type,
              unsigned int bytes_per_pixel, bool mipmap = false);

    void SetFilters(GLenum min, GLenum mag);  ///< sets the opengl min/mag filter modes associated with opengl texture m_opengl_id
    void Clear();  ///< frees the opengl texture object associated with this object

    /** The base class for Texture exceptions. */
    GG_ABSTRACT_EXCEPTION(Exception);

    /** Thrown when valid texture data cannot be read from a file. */
    GG_CONCRETE_EXCEPTION(BadFile, GG::Texture, Exception);

    /** Thrown when an unsupported number of color channels is requested. */
    GG_CONCRETE_EXCEPTION(InvalidColorChannels, GG::Texture, Exception);

    /** Thrown when GL fails to provide a requested texture object. */
    GG_CONCRETE_EXCEPTION(InsufficientResources, GG::Texture, Exception);

private:
    Texture(const Texture& rhs) = delete;
    Texture& operator=(const Texture& rhs) = delete;

    void InitFromRawData(X width, Y height, const uint8_t* image, GLenum format, GLenum type,
                         unsigned int bytes_per_pixel, bool mipmap);
    [[nodiscard]] std::vector<uint8_t> GetRawBytes();

    boost::filesystem::path m_path;     ///< file path from which this Texture was constructed

    unsigned int m_bytes_pp = 0;
    X            m_width = GG::X0;
    Y            m_height = GG::Y0;

    GLenum       m_wrap_s = GL_REPEAT;
    GLenum       m_wrap_t = GL_REPEAT;
    GLenum       m_min_filter = GL_LINEAR_MIPMAP_LINEAR;
    GLenum       m_mag_filter = GL_LINEAR;

    bool         m_mipmaps = false;
    GLuint       m_opengl_id = 0;   ///< OpenGL texture ID
    GLenum       m_format = GL_INVALID_ENUM;
    GLenum       m_type = GL_INVALID_ENUM;

    /// each of these is used for a non-power-of-two-sized graphic loaded into a power-of-two-sized texture
    std::array<GLfloat, 4> m_tex_coords = {0.0f, 0.0f, 0.0f, 0.0f};    ///< the texture coords used to blit from this texture by default (reflecting original image width and height)
    X            m_default_width = GG::X0;                      ///< the original width and height of this texture to be used in blitting 
    Y            m_default_height = GG::Y0;
};

/** \brief This class is a convenient way to store the info needed to use a
    portion of an OpenGL texture. */
class GG_API SubTexture
{
public:
    SubTexture() = default;

    /** Creates a SubTexture from a GG::Texture and coordinates into it.
        \throw GG::SubTexture::BadTexture Throws if the given Texture is null.
        \throw GG::SubTexture::InvalidTextureCoordinates Throws if the texture
        coordinates are not well formed.*/
    SubTexture(std::shared_ptr<const Texture> texture, X x1, Y y1, X x2, Y y2);

    /** Creates a SubTexture from a GG::Texture and uses coordinates to cover
        the whole texture.
        \throw GG::SubTexture::BadTexture Throws if the given Texture is null.*/
    SubTexture(std::shared_ptr<const Texture> texture);
    SubTexture(const SubTexture& rhs);

    SubTexture& operator=(const SubTexture& rhs);
    SubTexture& operator=(SubTexture&& rhs) noexcept;

    virtual ~SubTexture() = default;

    bool                    Empty() const noexcept { return !m_texture; }           ///< true if this object has no associated GG::Texture
    std::array<GLfloat, 4>  TexCoords() const noexcept { return m_tex_coords; }     ///< texture coordinates to use when blitting this sub-texture
    X                       Width() const noexcept { return m_width; }              ///< width of sub-texture in pixels
    Y                       Height() const noexcept { return m_height; }            ///< height of sub-texture in pixels
    const Texture*          GetTexture() const noexcept { return m_texture.get(); } ///< the texture the SubTexture is a part of

    /** Blit sub-texture to any place on screen, scaling as necessary \see
        GG::Texture::OrthoBlit*/
    void OrthoBlit(Pt pt1, Pt pt2) const;

    /** Blit sub-texture unscaled to \a pt (upper left corner) \see
        GG::Texture::OrthoBlit*/
    void OrthoBlit(Pt pt) const;

    void Clear();

    /** The base class for SubTexture exceptions. */
    GG_ABSTRACT_EXCEPTION(Exception);

    /** Thrown when an attempt is made to create a SubTexture using a null
        texture. */
    GG_CONCRETE_EXCEPTION(BadTexture, GG::SubTexture, Exception);

    /** Thrown when invalid or out-of-order texture coordinates are
        supplied.*/
    GG_CONCRETE_EXCEPTION(InvalidTextureCoordinates, GG::SubTexture, Exception);

private:
    /** shared_ptr to texture object with entire image. */
    std::shared_ptr<const Texture>  m_texture;
    X                               m_width = GG::X0;
    Y                               m_height = GG::Y0;
    std::array<GLfloat, 4>          m_tex_coords = {0.0f, 0.0f, 1.0f, 1.0f}; ///< position of element within containing texture 
};

/** \brief A singleton that loads and stores textures for use by GG.

    This class is essentially a very thin wrapper around a map of Texture
    smart pointers, keyed on std::string texture names.  The user need only
    request a texture through GetTexture(); if the texture is not already
    resident, it will be loaded.  If the user would like to create her own
    images and store them in the manager, that can be accomplished via
    StoreTexture() calls.*/
class GG_API TextureManager
{
public:
    std::map<std::string_view, std::shared_ptr<const Texture>> Textures() const;

    /** Stores a pre-existing GG::Texture in the manager's texture pool, and
        returns a shared_ptr to it. \warning Calling code <b>must not</b>
        delete \a texture; \a texture becomes the property of the manager,
        which will eventually delete it. */
    std::shared_ptr<Texture> StoreTexture(Texture* texture, std::string texture_name);

    /** Stores a pre-existing GG::Texture in the manager's texture pool, and
        returns a shared_ptr to it. \warning Calling code <b>must not</b>
        delete \a texture; \a texture becomes the property of the manager,
        which will eventually delete it. */
    std::shared_ptr<Texture> StoreTexture(std::shared_ptr<Texture> texture, std::string texture_name);

    /** Returns a shared_ptr to the texture created from image file \a path.
        If the texture is not present in the manager's pool, it will be loaded
        from disk. */
    std::shared_ptr<Texture> GetTexture(const boost::filesystem::path& path, bool mipmap = false);

    /** Removes the manager's shared_ptr to the texture created from image
        file \a path, if it exists.  \note Due to shared_ptr semantics, the
        texture may not be deleted until much later. */
    void                     FreeTexture(const boost::filesystem::path& path);

    /** Removes the manager's shared_ptr to the texture stored with the name
        \a name, if it exists.  \note Due to shared_ptr semantics, the
        texture may not be deleted until much later. */
    void                     FreeTexture(const std::string& name);

private:
    TextureManager();
    std::shared_ptr<Texture> LoadTexture(const boost::filesystem::path& path, bool mipmap);

    /** Indexed by string, not path, because some textures may be stored by a
        name and not loaded from a path. */
    std::map<std::string, std::shared_ptr<Texture>> m_textures;

    mutable std::mutex m_texture_access_guard;

    friend GG_API TextureManager& GetTextureManager();
};

/** Returns the singleton TextureManager instance. */
GG_API TextureManager& GetTextureManager();

}


#endif
