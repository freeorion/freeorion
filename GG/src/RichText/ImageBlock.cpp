//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2015 Mitten-O
//!  Copyright (C) 2016-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <algorithm>
#include <fstream>
#include <GG/dialogs/FileDlg.h>
#include <GG/DrawUtil.h>
#include <GG/RichText/ImageBlock.h>
#include <GG/RichText/RichText.h>
#include <GG/TextControl.h>
#include <GG/Texture.h>
#include <GG/utf8/utf8.h>


using namespace GG;

namespace fs = std::filesystem;


ImageBlock::ImageBlock(std::shared_ptr<Texture> tex, X x, Y y, X w, GG::Flags<GG::WndFlag> flags) :
    BlockControl(x, y, w, flags),
    m_graphic(Wnd::Create<StaticGraphic>(std::move(tex),
                                         GRAPHIC_PROPSCALE | GRAPHIC_SHRINKFIT | GRAPHIC_CENTER))
{}

void ImageBlock::CompleteConstruction()
{
    if (m_graphic)
        AttachChild(m_graphic);
}

Pt ImageBlock::SetMaxWidth(X width)
{
    if (m_graphic) {
        // Give the graphic the set width and give it liberty with the height.
        m_graphic->Resize(Pt(width, Y(INT_MAX)));

        // Get the actual space the graphic decided to use.
        Rect rect = m_graphic->RenderedArea();
        Pt size = rect.LowerRight() - rect.UpperLeft();

        // Take up the full width to center the image.
        size.x = width;

        // Don't take the extra vertical space.
        m_graphic->Resize(size);

        // Update our size to match the graphic we are wrapping.
        Resize(size);

        // Return the size we decided to be.
        return size;
    } else {
        // We don't have an image. Take a quarter of width to show an X.
        Pt size(width, Y(Value(width) / 4));
        Resize(size);
        return size;
    }
}

void ImageBlock::Render()
{
    if (m_graphic)
        return;

    // Error: no image. Draw red x.
    Pt ul = UpperLeft();
    Pt lr = LowerRight();
    Pt size = lr - ul;
    ul.x += size.x / 2 - X(Value(size.y)) / 2;
    lr.x -= size.x / 2 - X(Value(size.y)) / 2;
    FlatX(ul, lr, CLR_RED);
}

// A factory for creating image blocks from tags.
class ImageBlockFactory : public RichText::IBlockControlFactory {
public:
    //! Create a Text block from a plain text tag.
    std::shared_ptr<BlockControl> CreateFromTag(const RichText::TAG_PARAMS& params, std::string,
                                                std::shared_ptr<const Font>, Clr, Flags<TextFormat>) const override
    {
        // Get texture name from parameters
        auto tex_name = ExtractTextureName(params);
        if (tex_name.empty())
            return nullptr;

        auto& tm = GetTextureManager();

        if (auto tex = tm.GetTextureByName(std::string{tex_name})) // TODO: avoid string construction
            return Wnd::Create<ImageBlock>(std::move(tex), X0, Y0, X1, Flags<WndFlag>());

        // if not stored by name, interpet as a path

        // Get the path from the parameters, with or without prepending root path
        fs::path param_path = NameToPath(tex_name);
        fs::path combined_path = fs::exists(param_path) ? param_path : (m_root_path / param_path);

        try {
            if (auto tex = tm.GetTexture(combined_path, true)) // throws if texture not loaded and path doesn't exist or is invalid
                return Wnd::Create<ImageBlock>(std::move(tex), X0, Y0, X1, Flags<WndFlag>());
        } catch (...) {}

        // no such texture found :(
        return nullptr;
    }

    // Sets the root of image search path.
    void SetRootPath(fs::path path) noexcept
    { m_root_path = std::move(path); }

private:
    fs::path m_root_path;

    // Extracts a texture name from the given params.
    static std::string_view ExtractTextureName(const RichText::TAG_PARAMS& params)
    {
        // Find the src.
        const auto src_param_it = std::find_if(params.begin(), params.end(),
                                               [](const auto p) noexcept { return p.first == "src"; });
        return (src_param_it == params.end()) ? "" : src_param_it->second;
    }

    // converts string to path
    static fs::path NameToPath(std::string_view name)
    {
#if defined(_WIN32)
        // convert UTF-8 path string to UTF-16
        fs::path::string_type str_native;
        str_native.reserve(name.size());
        utf8::utf8to16(name.begin(), name.end(), std::back_inserter(str_native));
        return fs::path(str_native);
#else
        return fs::path(std::string{name});
#endif

    }
};

namespace {
    // Register image block as the image tag handler.
    const auto dummy = RichText::RegisterDefaultBlock(ImageBlock::IMAGE_TAG,
                                                      std::make_shared<ImageBlockFactory>());
}

//! Set the root path from which to look for images with the factory.
bool ImageBlock::SetImagePath(RichText::IBlockControlFactory* factory, fs::path path)
{
    // Try to convert the factory to an image factory.
    ImageBlockFactory* image_factory = dynamic_cast<ImageBlockFactory*>(factory);

    // If successful, set the root path.
    if (image_factory) {
        image_factory->SetRootPath(std::move(path));
        return true;
    } else {
        return false;
    }
}

//! Set the root path from which to look for images with the factory.
bool ImageBlock::SetDefaultImagePath(fs::path path)
{
    // Find the image block factory from the default map and give it the path.
    auto& dbf{RichText::DefaultBlockFactoryMap()};
    const auto factory_it = std::find_if(dbf.begin(), dbf.end(), [](auto f) { return f.first == IMAGE_TAG; });
    if (factory_it != dbf.end()) {
        if (auto factory = dynamic_cast<ImageBlockFactory*>(factory_it->second.get()))
            return SetImagePath(factory, std::move(path));
    }
    return false;
}
