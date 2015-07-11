#ifndef TEXTBLOCK_H
#define TEXTBLOCK_H

#include <GG/TextControl.h>
#include <GG/RichText/BlockControl.h>

namespace GG {

/**
 * @brief A text control embeddable in RichText.
*/
class TextBlock : public GG::BlockControl
{
public:
    //! Create a new TextBlock.
    TextBlock(X x, Y y, X w, const std::string& str,
              const boost::shared_ptr<Font>& font,
              Clr color, Flags<TextFormat> format,
              Flags<WndFlag> flags);

    virtual void Render() {};

    //! Calculate the size based on the width it should take.
    virtual Pt SetMaxWidth(X width);
private:
    TextControl* m_text; //! The text control used to handle the text.
};

}

#endif // TEXTBLOCK_H
