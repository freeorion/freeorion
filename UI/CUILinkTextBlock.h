#ifndef CUILINKTEXTBLOCK_H
#define CUILINKTEXTBLOCK_H

#include <GG/RichText/BlockControl.h>
#include <GG/RichText/RichText.h>
#include "LinkText.h"

class CUILinkTextBlock : public GG::BlockControl
{
public:
    CUILinkTextBlock(const std::string& str,
                     const boost::shared_ptr<GG::Font>& font,
                     GG::Flags<GG::TextFormat> format,
                     const GG::Clr& color,
                     GG::Flags< GG::WndFlag > flags );

    virtual GG::Pt SetMaxWidth( GG::X width );

    LinkText& Text();

    virtual void Render(){}

    class Factory: public GG::RichText::IBlockControlFactory{
    public:
        //! Creates a control from the tag (with unparsed parameters) and the content between the tags.
        //! You own the returned control.
        virtual GG::BlockControl* CreateFromTag(const std::string& tag,
                                                const std::string& params,
                                                const std::string& content,
                                                const boost::shared_ptr<GG::Font>& font,
                                                GG::Clr color,
                                                GG::Flags<GG::TextFormat> format);

        ///< link clicked signals: first string is the link type, second string is the specific item clicked
        mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkClickedSignal;
        mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkDoubleClickedSignal;
        mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkRightClickedSignal;
    private:
    };
private:
    LinkText* m_linkText;
};

#endif // CUILINKTEXTBLOCK_H
