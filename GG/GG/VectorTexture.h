// -*- C++ -*-

/** \file VectorTexture.h \brief Contains the VectorTexture class, which
    encapsulates a vector texture object, which can be rendered like a
    raster texture. */

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
    /** \name Structors */ ///@{
    VectorTexture();
    //@}

    /** \name Accessors */ ///@{
    const boost::filesystem::path& Path() const;    ///< returns the file path from which this vector texture was loaded (default / empty if this vector texture was not loaded from a file)

    /** Renders to region between \a pt1 and \a pt2 */
    void Render(const Pt& pt1, const Pt& pt2) const;
    //@}

    /** \name Mutators */ ///@{
    // intialization functions
    /** Frees any currently-held memory and loads a texture from file \a
        path.  \throw GG::Texture::BadFile Throws if the texture creation
        fails. */
    void Load(const boost::filesystem::path& path);
    //@}

    /** \name Exceptions */ ///@{
    /** The base class for VectorTexture exceptions. */
    GG_ABSTRACT_EXCEPTION(Exception);

    /** Thrown when valid vector image data cannot be read from a file. */
    GG_CONCRETE_EXCEPTION(BadFile, GG::VectorTexture, Exception);
    //@}

private:
    boost::filesystem::path             m_path; ///< file path from which this Texture was constructed
    std::shared_ptr<VectorTextureImpl>  m_impl;
};

} // namespace GG

#endif
