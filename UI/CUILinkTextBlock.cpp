#include "CUILinkTextBlock.h"

#include "util/VarText.h"

CUILinkTextBlock::CUILinkTextBlock(const std::string& str,
                                   const boost::shared_ptr<GG::Font>& font,
                                   GG::Flags<GG::TextFormat> format,
                                   const GG::Clr& color,
                                   GG::Flags< GG::WndFlag > flags):
GG::BlockControl(GG::X0, GG::Y0, GG::X(1), flags | GG::INTERACTIVE),
m_linkText( new LinkText(GG::X0, GG::Y0, GG::X(1), str, font, format, color) )
{
    AttachChild(m_linkText);
}

GG::Pt CUILinkTextBlock::SetMaxWidth(GG::X width)
{
    m_linkText->Resize(GG::Pt(width, GG::Y(1)));
    GG::Pt size = m_linkText->TextLowerRight() - m_linkText->TextUpperLeft();

    // Only resize when changed.
    if (size != m_linkText->Size() )
        m_linkText->Resize(size);
    if( size != Size() )
        Resize(size);
    return size;
}

LinkText& CUILinkTextBlock::Text(){
    return *m_linkText;
}

GG::BlockControl* CUILinkTextBlock::Factory::CreateFromTag(
    const std::string& tag,
    const std::string& params,
    const std::string& content,
    const boost::shared_ptr<GG::Font>& font,
    GG::Clr color,
    GG::Flags<GG::TextFormat> format)
{
    CUILinkTextBlock* block = new CUILinkTextBlock(content,
                                font,
                                format,
                                color,
                                GG::NO_WND_FLAGS);

    // Wire the block's signals to come through us.
    GG::Connect(block->m_linkText->LinkClickedSignal,        this->LinkClickedSignal);
    GG::Connect(block->m_linkText->LinkDoubleClickedSignal,  this->LinkDoubleClickedSignal);
    GG::Connect(block->m_linkText->LinkRightClickedSignal,   this->LinkRightClickedSignal);

    // Color ships and planets by their owner empires.
    block->m_linkText->SetDecorator(VarText::SHIP_ID_TAG, new ColorByOwner());
    block->m_linkText->SetDecorator(VarText::PLANET_ID_TAG, new ColorByOwner());

    return block;
}