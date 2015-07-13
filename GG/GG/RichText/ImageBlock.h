#ifndef IMAGEBLOCK_H
#define IMAGEBLOCK_H

#include <GG/RichText/BlockControl.h>
#include <GG/RichText/RichText.h>
#include <GG/StaticGraphic.h>

namespace GG {


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
class ImageBlock : public BlockControl
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
    ImageBlock(const std::string& image_path, X x, Y y, X w, GG::Flags< GG::WndFlag > flags);

    //! Implement from BlockControl sets the maximum width, returns the actual size based on that.
    virtual Pt SetMaxWidth(X width);

    virtual void Render();

    //! Set the root path from which to look for images with the factory.
    static bool SetImagePath(
        RichText::IBlockControlFactory* factory, //!< The factory to set the path for. Should be an image block factory.
        const std::string& path); //!< The base path to look for images from.

    //! Set the root path from which to look for images with the factory.
    static bool SetDefaultImagePath(
        const std::string& path); //!< The base path to look for images from.
private:
    StaticGraphic* m_graphic; //! The StaticGraphic used to render the image.
};

}

#endif // IMAGEBLOCK_H
