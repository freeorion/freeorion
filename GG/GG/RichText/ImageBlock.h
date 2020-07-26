//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2015 Mitten-O
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef _GG_RichText_ImageBlock_h_
#define _GG_RichText_ImageBlock_h_


#include <boost/filesystem/path.hpp>
#include <GG/RichText/BlockControl.h>
#include <GG/RichText/RichText.h>
#include <GG/StaticGraphic.h>


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


#endif
