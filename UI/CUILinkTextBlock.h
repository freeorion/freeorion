#ifndef CUILINKTEXTBLOCK_H
#define CUILINKTEXTBLOCK_H

#include <GG/RichText/BlockControl.h>
#include <GG/RichText/RichText.h>
#include "LinkText.h"

class CUILinkTextMultiEdit;

class CUILinkTextBlock final : public GG::BlockControl {
public:
    CUILinkTextBlock(std::string str, GG::Flags<GG::TextFormat> format,
                     GG::Clr color, GG::Flags<GG::WndFlag> flags);

    void CompleteConstruction() override;
    GG::Pt SetMaxWidth(GG::X width) override;

    void Render() override {}

    const auto& Text() const noexcept { return *m_link_text; }

    class Factory final : public GG::RichText::IBlockControlFactory {
    public:
        //! Creates a control from the tag (with unparsed parameters) and the content between the tags.
        //! You own the returned control.
        std::shared_ptr<GG::BlockControl> CreateFromTag(const GG::RichText::TAG_PARAMS&, std::string,
                                                        std::shared_ptr<GG::Font>, GG::Clr,
                                                        GG::Flags<GG::TextFormat>) const override;

        ///< link clicked signals: first string is the link type, second string is the specific item clicked
        mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkClickedSignal;
        mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkDoubleClickedSignal;
        mutable boost::signals2::signal<void (const std::string&, const std::string&)> LinkRightClickedSignal;
    };

private:
    std::shared_ptr<CUILinkTextMultiEdit> m_link_text;
};


#endif
