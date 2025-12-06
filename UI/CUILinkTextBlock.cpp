#include "CUILinkTextBlock.h"

#include "../util/VarText.h"
#include "CUIControls.h"

//////////////////////
// CUILinkTextBlock //
//////////////////////
CUILinkTextBlock::CUILinkTextBlock(std::string str, GG::Clr color, GG::Flags<GG::WndFlag> flags) :
    GG::BlockControl(GG::X0, GG::Y0, GG::X1, flags | GG::INTERACTIVE),
    m_link_text(GG::Wnd::Create<CUILinkTextMultiEdit>(
        std::move(str),
        GG::MULTI_WORDBREAK | GG::MULTI_READ_ONLY | GG::MULTI_LEFT | GG::MULTI_LINEWRAP |
        GG::MULTI_TOP | GG::MULTI_NO_HSCROLL | GG::MULTI_NO_VSCROLL))
{}

void CUILinkTextBlock::CompleteConstruction() {
    GG::BlockControl::CompleteConstruction();

    AttachChild(m_link_text);

    m_link_text->SetColor(GG::CLR_ZERO);
    m_link_text->SetInteriorColor(GG::CLR_ZERO);

    SetName("CUILinkTextBlock");
}

GG::Pt CUILinkTextBlock::SetMaxWidth(GG::X width) {
    m_link_text->Resize(GG::Pt(width, GG::Y1));
    // Resize to have enough place to show the whole text.
    GG::Pt size = m_link_text->FullSize();

    // Only resize when changed.
    if (size != m_link_text->Size())
        m_link_text->Resize(size);

    if (size != Size())
        Resize(size);

    return size;
}

std::shared_ptr<GG::BlockControl> CUILinkTextBlock::Factory::CreateFromTag(
    const GG::RichText::TAG_PARAMS&, std::string content,
    std::shared_ptr<const GG::Font>, GG::Clr color, GG::Flags<GG::TextFormat>) const
{
    auto block = GG::Wnd::Create<CUILinkTextBlock>(std::move(content), color, GG::NO_WND_FLAGS);

    // Wire the block's signals to come through us.
    block->m_link_text->LinkClickedSignal.connect(this->LinkClickedSignal);
    block->m_link_text->LinkDoubleClickedSignal.connect(this->LinkDoubleClickedSignal);
    block->m_link_text->LinkRightClickedSignal.connect(this->LinkRightClickedSignal);

    // Color ships and planets by their owner empires.
    block->m_link_text->SetDecorator(VarText::SHIP_ID_TAG, TextLinker::DecoratorType::ColorByOwner);
    block->m_link_text->SetDecorator(VarText::PLANET_ID_TAG, TextLinker::DecoratorType::ColorByOwner);
    block->m_link_text->SetDecorator(TextLinker::BROWSE_PATH_TAG, TextLinker::DecoratorType::PathType);
    block->m_link_text->SetDecorator(VarText::FOCS_VALUE_TAG, TextLinker::DecoratorType::ValueRef);

    return block;
}


//////////////////
// CUITextBlock //
//////////////////
CUITextBlock::CUITextBlock(std::string str, GG::Clr color, GG::Flags<GG::WndFlag> flags) :
    GG::BlockControl(GG::X0, GG::Y0, GG::X1, flags | GG::INTERACTIVE),
    m_text(GG::Wnd::Create<CUIMultiEdit>(
        std::move(str),
        GG::MULTI_WORDBREAK | GG::MULTI_READ_ONLY | GG::MULTI_LEFT | GG::MULTI_LINEWRAP |
        GG::MULTI_TOP | GG::MULTI_NO_HSCROLL | GG::MULTI_NO_VSCROLL))
{}

void CUITextBlock::CompleteConstruction() {
    GG::BlockControl::CompleteConstruction();

    AttachChild(m_text);

    m_text->SetColor(GG::CLR_ZERO);
    m_text->SetInteriorColor(GG::CLR_ZERO);

    SetName("CUITextBlock");
}

GG::Pt CUITextBlock::SetMaxWidth(GG::X width) {
    m_text->Resize(GG::Pt(width, GG::Y1));
    // Resize to have enough place to show the whole text.
    GG::Pt size = m_text->FullSize();

    // Only resize when changed.
    if (size != m_text->Size())
        m_text->Resize(size);

    if (size != Size())
        Resize(size);

    return size;
}

std::shared_ptr<GG::BlockControl> CUITextBlock::Factory::CreateFromTag(
    const GG::RichText::TAG_PARAMS&, std::string content,
    std::shared_ptr<const GG::Font>, GG::Clr color, GG::Flags<GG::TextFormat>) const
{
    return GG::Wnd::Create<CUITextBlock>(std::move(content), color, GG::NO_WND_FLAGS);
}