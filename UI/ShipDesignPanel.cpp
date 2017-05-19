#include "ShipDesignPanel.h"

#include <GG/StaticGraphic.h>

#include "../universe/ShipDesign.h"
#include "ClientUI.h"
#include "CUIControls.h"

ShipDesignPanel::ShipDesignPanel(GG::X w, GG::Y h, int design_id) :
    GG::Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
    m_design_id(design_id),
    m_graphic(nullptr),
    m_name(nullptr)
{
    SetChildClippingMode(ClipToClient);
    if (const ShipDesign* design = GetShipDesign(m_design_id)) {
        m_graphic = new GG::StaticGraphic(ClientUI::ShipDesignIcon(design_id), GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC);
        m_graphic->Resize(GG::Pt(w, h));
        AttachChild(m_graphic);
        m_name = new CUILabel(design->Name(), GG::FORMAT_WORDBREAK | GG::FORMAT_CENTER | GG::FORMAT_TOP);
        m_name->SetTextColor(GG::CLR_WHITE);
        AttachChild(m_name);
    }
}

void ShipDesignPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Control::SizeMove(ul, lr);
    if (m_graphic)
        m_graphic->Resize(Size());
    if (m_name)
        m_name->Resize(Size());
}

void ShipDesignPanel::Render()
{}

void ShipDesignPanel::Update() {
    if (const ShipDesign* design = GetShipDesign(m_design_id)) {
        m_name->SetText(design->Name());
        m_name->Resize(GG::Pt(Width(), m_name->Height()));
    }
}

void ShipDesignPanel::SetAvailability(const Availability::Enum type) {
    auto disabled = type != Availability::Available;
    Disable(disabled);
    m_graphic->Disable(disabled);
    m_name->Disable(disabled);
}
