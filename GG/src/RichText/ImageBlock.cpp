#include <GG/RichText/ImageBlock.h>

#include <GG/Texture.h>
#include <GG/RichText/RichText.h>
#include <GG/TextControl.h>
#include <GG/DrawUtil.h>

#include <boost/filesystem.hpp>
#include <algorithm>

namespace GG {

const std::string ImageBlock::IMAGE_TAG("img");

ImageBlock::ImageBlock(const std::string& image_path, X x, Y y, X w, GG::Flags< GG::WndFlag > flags):
    BlockControl(x, y, w, flags),
    m_graphic(0)
{
    try{
        boost::shared_ptr<Texture> texture = GetTextureManager().GetTexture(image_path);
        m_graphic = new StaticGraphic(texture, GRAPHIC_PROPSCALE | GRAPHIC_SHRINKFIT | GRAPHIC_CENTER );
        AttachChild(m_graphic);
    } catch( GG::Texture::BadFile& errBadFile ){
        // No can do inside GiGi.
    }
}

Pt ImageBlock::SetMaxWidth(X width)
{

    if( m_graphic != 0 ){
        // Give the graphic the set width and give it liberty with the height.
        m_graphic->Resize( Pt(width, Y(INT_MAX) ) );

        // Get the actual space the graphic decided to use.
        Rect rect = m_graphic->RenderedArea();
        Pt size = rect.LowerRight() - rect.UpperLeft();

        // Take up the full width to center the image.
        size.x = width;

        // Don't take the extra vertical space.
        m_graphic->Resize( size );

        // Update our size to match the graphic we are wrapping.
        Resize(size);

        // Return the size we decided to be.
        return size;
    } else {
        // We don't have an image. Take a quarter of width to show an X.
        Pt size(width, Y(Value(width)/4));
        Resize(size);
        return size;
    }
}

void ImageBlock::Render()
{
    if( m_graphic == 0 ){
        // Error: no image. Draw red x.
        Pt ul = UpperLeft();
        Pt lr = LowerRight();
        Pt size = lr - ul;
        ul.x += size.x / 2 - X(Value(size.y)) / 2;
        lr.x -= size.x / 2 - X(Value(size.y)) / 2;
        FlatX(ul, lr, CLR_RED);
    }
}

// A factory for creating image blocks from tags.
class ImageBlockFactory: public RichText::IBlockControlFactory {
public:
    ImageBlockFactory():
    m_rootPath(".")
    {}

    //! Create a Text block from a plain text tag.
    virtual BlockControl* CreateFromTag(const std::string& tag,
                                        const RichText::TAG_PARAMS& params,
                                        const std::string& content,
                                        const boost::shared_ptr<Font>& font,
                                        Clr color,
                                        Flags<TextFormat> format
                                       )
    {
        // Get the path from the parameters.
        std::string path = ExtractPath(params);

        // Create a new image block, basing the path on the root path.
        return new ImageBlock( (m_rootPath / path).string() , X0, Y0, X1, Flags<WndFlag>());
    }

    // Sets the root of image search path.
    void SetRootPath(const std::string& path){
        m_rootPath = path;
    }
private:
    boost::filesystem::path m_rootPath;

    // Extracts the path from the given params.
    static std::string ExtractPath( const RichText::TAG_PARAMS& params ){

        // Find the src.
        RichText::TAG_PARAMS::const_iterator src_param = params.find("src");

        // If src not found, error out.
        if(src_param == params.end()){
            return std::string("ERR:invalid params: missing src.");
        }else{
            // Return the path.
            return src_param->second;
        }
    }
};

// Register image block as the image tag handler.
static int dummy = RichText::RegisterDefaultBlock(ImageBlock::IMAGE_TAG, new ImageBlockFactory());

//! Set the root path from which to look for images with the factory.
bool ImageBlock::SetImagePath(
    RichText::IBlockControlFactory* factory, //!< The factory to set the path for. Should be an image block factory.
    const std::string& path){ //!< The base path to look for images from.

    // Try to convert the factory to an image factory.
    ImageBlockFactory* image_factory = dynamic_cast<ImageBlockFactory*>(factory);

    // If successful, set the root path.
    if( image_factory != 0 ){
        image_factory->SetRootPath(path);
        return true;
    } else {
        return false;
    }
}

//! Set the root path from which to look for images with the factory.
bool ImageBlock::SetDefaultImagePath(
    const std::string& path){
    // Find the image block factory from the default map and give it the path.
    RichText::BLOCK_FACTORY_MAP::const_iterator factory_it = RichText::DefaultBlockFactoryMap()->find(IMAGE_TAG);
    if( factory_it != RichText::DefaultBlockFactoryMap()->end() ){
        ImageBlockFactory* factory = dynamic_cast<ImageBlockFactory*>(factory_it->second);
        if( factory != 0 ){
            return SetImagePath(factory, path);
        }
    }

    return false;
}

}
