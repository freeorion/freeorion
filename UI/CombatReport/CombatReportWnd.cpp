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

namespace {
    GG::X COMBAT_LOG_WIDTH(400);
    GG::Y COMBAT_LOG_HEIGHT(300);
}

// The implementation class for CombatReportWnd
class CombatReportWnd::CombatReportPrivate {
public:
    CombatReportPrivate(CombatReportWnd& wnd):
        m_wnd(wnd),
        m_tabs(new GG::TabWnd(GG::X0, GG::Y0, GG::X1, GG::Y1, ClientUI::GetFont(), ClientUI::CtrlColor(), ClientUI::TextColor())),
        m_graphical(new GraphicalSummaryWnd()),
        m_log(new CombatLogWnd())
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
        if(GraphicalSummaryWnd* graphical_wnd =
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

private:
    CombatReportWnd&        m_wnd;
    GG::TabWnd*             m_tabs;
    GraphicalSummaryWnd*    m_graphical;//< Graphical summary
    CombatLogWnd*           m_log;      //< Detailed log

    void SetFocus(int id)
    { DebugLogger() << "SetFocus " << id; }

    void RectangleEnter(int data) {
        DebugLogger() << "RectangleHover " << data;
        SetFocus(data);
    }

    void HandleTabChanged() {
        // Make sure that the newly selected window gets an update.
        DoLayout();
     }
};

CombatReportWnd::CombatReportWnd() :
    CUIWnd(UserString("COMBAT_REPORT_TITLE"), GG::X(150), GG::Y(50), COMBAT_LOG_WIDTH, COMBAT_LOG_HEIGHT,
           GG::INTERACTIVE | GG::RESIZABLE | GG::DRAGABLE | GG::ONTOP | CLOSABLE),
    m_impl(0)
{ m_impl.reset(new CombatReportPrivate(*this)); }

CombatReportWnd::~CombatReportWnd()
{ }

void CombatReportWnd::SetLog(int log_id)
{ m_impl->SetLog(log_id); }

void CombatReportWnd::CloseClicked()
{ Hide(); }

void CombatReportWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    CUIWnd::SizeMove(ul, lr);
    m_impl->DoLayout();
}
