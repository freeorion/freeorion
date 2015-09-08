#include "CUILinkTextBlock.h"

#include "../util/VarText.h"

CUILinkTextBlock::CUILinkTextBlock(const std::string& str, const boost::shared_ptr<GG::Font>& font,
                                   GG::Flags<GG::TextFormat> format, const GG::Clr& color,
                                   GG::Flags<GG::WndFlag> flags)
: GG::BlockControl(GG::X0, GG::Y0, GG::X1, flags | GG::INTERACTIVE),
  m_link_text(new LinkText(GG::X0, GG::Y0, GG::X1, str, font, format, color))
{
    AttachChild(m_link_text);
}

GG::Pt CUILinkTextBlock::SetMaxWidth(GG::X width)
{
    m_link_text->Resize(GG::Pt(width, GG::Y1));
    GG::Pt size = m_link_text->TextLowerRight() - m_link_text->TextUpperLeft();

    // Only resize when changed.
    if (size != m_link_text->Size())
        m_link_text->Resize(size);

    if (size != Size())
        Resize(size);

    return size;
}

LinkText& CUILinkTextBlock::Text() {
    return *m_link_text;
}

GG::BlockControl* CUILinkTextBlock::Factory::CreateFromTag(
    const std::string& tag, const GG::RichText::TAG_PARAMS& params, const std::string& content,
    const boost::shared_ptr<GG::Font>& font, const GG::Clr& color, GG::Flags<GG::TextFormat> format)
{
    CUILinkTextBlock* block = new CUILinkTextBlock(content, font, format, color, GG::NO_WND_FLAGS);

    // Wire the block's signals to come through us.
    GG::Connect(block->m_link_text->LinkClickedSignal, this->LinkClickedSignal);
    GG::Connect(block->m_link_text->LinkDoubleClickedSignal, this->LinkDoubleClickedSignal);
    GG::Connect(block->m_link_text->LinkRightClickedSignal, this->LinkRightClickedSignal);

    // Color ships and planets by their owner empires.
    block->m_link_text->SetDecorator(VarText::SHIP_ID_TAG, new ColorByOwner());
    block->m_link_text->SetDecorator(VarText::PLANET_ID_TAG, new ColorByOwner());

    return block;
}
