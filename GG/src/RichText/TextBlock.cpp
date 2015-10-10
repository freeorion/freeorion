#include "TextBlock.h"

#include <GG/RichText/RichText.h>

#include <limits.h>

namespace GG {

    TextBlock::TextBlock(X x, Y y, X w, const std::string& str, const boost::shared_ptr<Font>& font,
                         Clr color, Flags<TextFormat> format, Flags<WndFlag> flags)
    : BlockControl(x, y, w, flags)
    {
        // Construct the text control. Activatee full text wrapping features, and make it stick to the top.
        // With these setting the height is largely ignored, so we set it to one.
        m_text = new TextControl(GG::X0, GG::Y0, w, Y1, str, font, color,
                                 format | FORMAT_WORDBREAK | FORMAT_LINEWRAP | FORMAT_TOP, flags);
        AttachChild(m_text);
    }

    Pt TextBlock::SetMaxWidth(X width)
    {
        // Reflow the text to the given width.
        // We will actually listen to the height
        // the text wants to be instead of forcing it to a given height,
        // but the text layout code seems to get confused if you give it a height
        // less than the height of the font so we use that.
        m_text->Resize(GG::Pt(width, m_text->GetFont()->Height()));

        // Use the size the text requires.
        Pt text_size = m_text->MinUsableSize();
        Resize(text_size);
        return text_size;
    }

    // A factory for creating text blocks from tags.
    class TextBlockFactory: public RichText::IBlockControlFactory {
        public:
            //! Create a Text block from a plain text tag.
          virtual BlockControl* CreateFromTag(const std::string& tag,
                                              const RichText::TAG_PARAMS& params,
                                              const std::string& content,
                                              const boost::shared_ptr<Font>& font, const Clr& color,
                                              Flags<TextFormat> format)
          {
              return new TextBlock(X0, Y0, X1, content, font, color, format, Flags<WndFlag>());
            }
    };

    // Register text block as the default plaintext handler.
    static int dummy =
        RichText::RegisterDefaultBlock(RichText::PLAINTEXT_TAG, new TextBlockFactory());
}
