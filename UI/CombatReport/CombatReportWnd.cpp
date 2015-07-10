#include "CombatReportWnd.h"
#include "../ClientUI.h"
#include "../CUIControls.h"
#include "../../util/i18n.h"
#include "../../util/Logger.h"
#include "../../universe/ShipDesign.h"

#include "CombatReportData.h"
#include "GraphicalSummary.h"
#include "CombatLogWnd.h"

#include <GG/TabWnd.h>
#include <GG/Layout.h>

// The implementation class for CombatReportWnd
class CombatReportWnd::CombatReportPrivate {
public:
    CombatReportPrivate(CombatReportWnd& wnd):
        m_wnd(wnd),
        m_tabs(new GG::TabWnd(GG::X0, GG::Y0, GG::X1, GG::Y1, ClientUI::GetFont(), ClientUI::CtrlColor(), ClientUI::TextColor())),
        m_graphical(new GraphicalSummaryWnd()),
        m_log(new CombatLogWnd()),
        m_min_size(GG::X0, GG::Y0)
    {
        m_tabs->AddWnd(m_graphical, UserString("COMBAT_SUMMARY"));
        m_tabs->AddWnd(m_log, UserString("COMBAT_LOG"));
        m_wnd.AttachChild(m_tabs);

        GG::Connect(m_log->LinkClickedSignal,       &CombatReportPrivate::HandleLinkClick,          this);
        GG::Connect(m_log->LinkDoubleClickedSignal, &CombatReportPrivate::HandleLinkDoubleClick,    this);
        GG::Connect(m_log->LinkRightClickedSignal,  &CombatReportPrivate::HandleLinkDoubleClick,    this);

        // Catch the window-changed signal from the tab bar so that layout
        // updates can be performed for the newly-selected window.
        GG::Connect(m_tabs->WndChangedSignal,
                    boost::bind(&CombatReportPrivate::HandleTabChanged, this));

        // This can be called whether m_graphical is the selected window or
        // not, but it will still only use the min size of the selected window.
        GG::Connect(m_graphical->MinSizeChangedSignal,
                    boost::bind(&CombatReportPrivate::UpdateMinSize, this));
    }

    void SetLog(int log_id) {
        m_graphical->SetLog(log_id);
        m_log->SetLog(log_id);
    }

    void DoLayout() {
        // Leave space for the resize tab.
        m_tabs->SizeMove(GG::Pt(GG::X0, GG::Y0),
                         GG::Pt(m_wnd.ClientWidth(),
                                m_wnd.ClientHeight() - GG::Y(INNER_BORDER_ANGLE_OFFSET)) );

        // Only update the selected window.
        if (GraphicalSummaryWnd* graphical_wnd =
               dynamic_cast<GraphicalSummaryWnd*>(m_tabs->CurrentWnd())) {
            graphical_wnd->DoLayout();
        }
    }

    void HandleLinkClick(const std::string& link_type, const std::string& data) {
        using boost::lexical_cast;
        try {
            if (link_type == VarText::PLANET_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToPlanet(lexical_cast<int>(data));

            } else if (link_type == VarText::SYSTEM_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToSystem(lexical_cast<int>(data));
            } else if (link_type == VarText::FLEET_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToFleet(lexical_cast<int>(data));
            } else if (link_type == VarText::SHIP_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToShip(lexical_cast<int>(data));
            } else if (link_type == VarText::BUILDING_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToBuilding(lexical_cast<int>(data));
            } else if (link_type == VarText::FIELD_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToField(lexical_cast<int>(data));

            } else if (link_type == VarText::COMBAT_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToCombatLog(lexical_cast<int>(data));

            } else if (link_type == VarText::EMPIRE_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToEmpire(lexical_cast<int>(data));
            } else if (link_type == VarText::DESIGN_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToShipDesign(lexical_cast<int>(data));
            } else if (link_type == VarText::PREDEFINED_DESIGN_TAG) {
                if (const ShipDesign* design = GetPredefinedShipDesign(data))
                    ClientUI::GetClientUI()->ZoomToShipDesign(design->ID());

            } else if (link_type == VarText::TECH_TAG) {
                ClientUI::GetClientUI()->ZoomToTech(data);
            } else if (link_type == VarText::BUILDING_TYPE_TAG) {
                ClientUI::GetClientUI()->ZoomToBuildingType(data);
            } else if (link_type == VarText::FIELD_TYPE_TAG) {
                ClientUI::GetClientUI()->ZoomToFieldType(data);
            } else if (link_type == VarText::SPECIAL_TAG) {
                ClientUI::GetClientUI()->ZoomToSpecial(data);
            } else if (link_type == VarText::SHIP_HULL_TAG) {
                ClientUI::GetClientUI()->ZoomToShipHull(data);
            } else if (link_type == VarText::SHIP_PART_TAG) {
                ClientUI::GetClientUI()->ZoomToShipPart(data);
            } else if (link_type == VarText::SPECIES_TAG) {
                ClientUI::GetClientUI()->ZoomToSpecies(data);
            } else if (link_type == TextLinker::ENCYCLOPEDIA_TAG) {
                ClientUI::GetClientUI()->ZoomToEncyclopediaEntry(data);
            } else if (link_type == TextLinker::GRAPH_TAG) {
                // todo: this maybe?
            }

        } catch (const boost::bad_lexical_cast&) {
            ErrorLogger() << "CombatReportPrivate::HandleLinkClick caught lexical cast exception for link type: " << link_type << " and data: " << data;
        }

    }

    void HandleLinkDoubleClick(const std::string& link_type, const std::string& data) {
        HandleLinkClick(link_type, data);
    }

    GG::Pt GetMinSize() const
    { return m_min_size; }

private:
    CombatReportWnd&        m_wnd;
    GG::TabWnd*             m_tabs;
    GraphicalSummaryWnd*    m_graphical;//< Graphical summary
    CombatLogWnd*           m_log;      //< Detailed log
    GG::Pt                  m_min_size; //< Minimum size according to the contents, is not constrained by the app window size

    void SetFocus(int id)
    { DebugLogger() << "SetFocus " << id; }

    void RectangleEnter(int data) {
        DebugLogger() << "RectangleHover " << data;
        SetFocus(data);
    }

    void UpdateMinSize() {
        m_min_size = GG::Pt(GG::X0, GG::Y0);

        m_min_size += m_wnd.Size() - m_wnd.ClientSize();

        // The rest of this function could use m_tabs->MinUsableSize instead of
        // dealing with the children of m_tabs directly, but that checks the
        // MinUsableSize of _all_ child windows, not just the currently
        // selected one.
        m_min_size += m_tabs->CurrentWnd()->MinUsableSize();

        std::list<GG::Wnd*>::const_iterator layout_begin =
            m_tabs->GetLayout()->Children().begin();
        // First object in the layout should be the tab bar.
        if (layout_begin != m_tabs->GetLayout()->Children().end()) {
            GG::Pt tab_min_size = (*layout_begin)->MinUsableSize();
            m_min_size.x = std::max(tab_min_size.x, m_min_size.x);
            // TabBar::MinUsableSize does not seem to return the correct
            // height, so extra height is added here.
            m_min_size.y += tab_min_size.y + GG::Y(14);
        }

        // Leave space for the resize tab.
        m_min_size.y += GG::Y(INNER_BORDER_ANGLE_OFFSET);

        // If the window is currently too small, re-validate its size.
        if (m_wnd.Width() < m_min_size.x || m_wnd.Height() < m_min_size.y) {
            m_wnd.Resize(m_wnd.Size());
        }
    }

    void HandleTabChanged() {
        // Use the minimum size of the newly selected window.
        UpdateMinSize();

        // Make sure that the newly selected window gets an update.
        DoLayout();
    }
};

CombatReportWnd::CombatReportWnd(GG::X default_x, GG::Y default_y,
                                 GG::X default_w, GG::Y default_h,
                                 const std::string& config_name) :
    CUIWnd(UserString("COMBAT_REPORT_TITLE"),
           default_x, default_y, default_w, default_h,
           GG::INTERACTIVE | GG::RESIZABLE | GG::DRAGABLE | GG::ONTOP | CLOSABLE,
           config_name),
    m_impl(0)
{ m_impl.reset(new CombatReportPrivate(*this)); }

CombatReportWnd::~CombatReportWnd()
{}

void CombatReportWnd::SetLog(int log_id)
{ m_impl->SetLog(log_id); }

void CombatReportWnd::CloseClicked()
{ Hide(); }

void CombatReportWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt new_size = GG::Pt(std::max(lr.x - ul.x, m_impl->GetMinSize().x),
                             std::max(lr.y - ul.y, m_impl->GetMinSize().y) );

    CUIWnd::SizeMove(ul, ul + new_size);
    m_impl->DoLayout();
}
