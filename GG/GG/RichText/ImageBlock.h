/* GG is a GUI for SDL and OpenGL.

   Copyright (C) 2015 Mitten-O

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.

   Zach Laine
   whatwasthataddress@gmail.com */

#ifndef IMAGEBLOCK_H
#define IMAGEBLOCK_H

#include <GG/RichText/BlockControl.h>
#include <GG/RichText/RichText.h>
#include <GG/StaticGraphic.h>

#include <boost/filesystem/path.hpp>

namespace GG
{
/**
* @brief A Block control containing an image.
*
* Will be used to show img tags. They take the relative path
* of the image file as a parameter.
*
* Example:
* \code
* Here is image: &lt;img ../art/images/muminmamma.png&gt;&lt;/img&gt; Isn't that beautiful!
* \endcode
*/
class GG_API ImageBlock : public BlockControl
{
public:
    /**
     * @brief The tag to mark images with.
     */
    static const std::string IMAGE_TAG;

    /**
     * @brief Construct image block.
     *
     * @param image_path The path to the image.
     */
    ImageBlock(const boost::filesystem::path& path, X x, Y y, X w, GG::Flags<GG::WndFlag> flags);
    void CompleteConstruction() override;

    //! Implement from BlockControl sets the maximum width, returns the actual size based on that.
    Pt SetMaxWidth(X width) override;

    void Render() override;

    //! Set the root path from which to look for images with the factory.
    static bool SetImagePath(RichText::IBlockControlFactory* factory,   //!< The factory to set the path for. Should be an image block factory.
                             const boost::filesystem::path& path);      //!< The base path to look for images from.

    //! Set the root path from which to look for images with the factory.
    static bool SetDefaultImagePath(const boost::filesystem::path& path); //!< The base path to look for images from.

private:
    std::shared_ptr<StaticGraphic> m_graphic; //! The StaticGraphic used to render the image.
};

}

#endif // IMAGEBLOCK_H
