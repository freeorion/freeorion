//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2019 Geoff Topping
//!  Copyright (C) 2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/VectorTexture.h
//!
//! Contains the VectorTexture class, which encapsulates a vector texture
//! object, which can be rendered like a raster texture.

#ifndef _GG_VectorTexture_h_
#define _GG_VectorTexture_h_

#include <GG/Base.h>
#include <GG/Exception.h>

#include <boost/filesystem/path.hpp>

class VectorTextureImpl;

namespace GG {

/** \brief This class encapsulates an vector image object. */
class GG_API VectorTexture
{
public:
    VectorTexture();

    const boost::filesystem::path& Path() const;    ///< returns the file path from which this vector texture was loaded (default / empty if this vector texture was not loaded from a file)

    /** Renders to region between \a pt1 and \a pt2 */
    void Render(const Pt& ul, const Pt& lr) const;
    bool TextureLoaded() const;
    int NumShapes() const;
    Pt Size() const;

    // intialization functions
    /** Frees any currently-held memory and loads a texture from file \a
        path.  \throw GG::Texture::BadFile Throws if the texture creation
        fails. */
    void Load(const boost::filesystem::path& path);

    /** The base class for VectorTexture exceptions. */
    GG_ABSTRACT_EXCEPTION(Exception);

    /** Thrown when valid vector image data cannot be read from a file. */
    GG_CONCRETE_EXCEPTION(BadFile, GG::VectorTexture, Exception);

private:
    VectorTexture(const VectorTexture& rhs);            ///< disabled
    VectorTexture& operator=(const VectorTexture& rhs); ///< disabled

    boost::filesystem::path             m_path;         ///< file path from which this VectorTexture was constructed
    std::shared_ptr<VectorTextureImpl>  m_impl;
};

/** \brief A singleton that loads and stores vector textures for use by GG.

    This class is essentially a very thin wrapper around a map of VectorTexture
    smart pointers, keyed on std::string texture names.  The user need only
    request a texture through GetVectorTexture(); if the texture is not already
    resident, it will be loaded. */
class GG_API VectorTextureManager
{
public:
    const std::map<std::string, std::shared_ptr<VectorTexture>>& Textures() const;

    /** Returns a shared_ptr to the texture created from image file \a path.
        If the texture is not present in the manager's pool, it will be loaded
        from disk. */
    std::shared_ptr<VectorTexture> GetTexture(const boost::filesystem::path& path);

    /** Removes the manager's shared_ptr to the texture created from image
        file \a path, if it exists.  \note Due to shared_ptr semantics, the
        texture may not be deleted until much later. */
    void                           FreeTexture(const boost::filesystem::path& path);

    /** Removes the manager's shared_ptr to the texture stored with the name
        \a name, if it exists.  \note Due to shared_ptr semantics, the
        texture may not be deleted until much later. */
    void                           FreeTexture(const std::string& name);

private:
    VectorTextureManager();
    std::shared_ptr<VectorTexture> LoadTexture(const boost::filesystem::path& path);

    /** Indexed by string, not path, because some textures may be stored by a
        name and not loaded from a path. */
    std::map<std::string, std::shared_ptr<VectorTexture>> m_textures;

    friend GG_API VectorTextureManager& GetVectorTextureManager();
};

/** Returns the singleton VectorTextureManager instance. */
GG_API VectorTextureManager& GetVectorTextureManager();


}

#endif
