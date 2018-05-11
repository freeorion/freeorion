#include "BuildingsPanel.h"

#include <GG/Button.h>
#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/Order.h"
#include "../universe/Building.h"
#include "../universe/Effect.h"
#include "../universe/Planet.h"
#include "../universe/Enums.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "CUIControls.h"
#include "IconTextBrowseWnd.h"
#include "MapWnd.h"
#include "MultiIconValueIndicator.h"
#include "MultiMeterStatusBar.h"

namespace {
    /** How big we want meter icons with respect to the current UI font size.
      * Meters should scale along font size, but not below the size for the
      * default 12 points font. */
    GG::Pt MeterIconSize() {
        const int icon_size = std::max(ClientUI::Pts(), 12) * 4/3;
        return GG::Pt(GG::X(icon_size), GG::Y(icon_size));
    }

    /** Returns map from object ID to issued colonize orders affecting it. */
    std::map<int, int> PendingScrapOrders() {
        std::map<int, int> retval;
        const ClientApp* app = ClientApp::GetApp();
        if (!app)
            return retval;
        for (const auto& id_and_order : app->Orders()) {
            if (auto order = std::dynamic_pointer_cast<ScrapOrder>(id_and_order.second)) {
                retval[order->ObjectID()] = id_and_order.first;
            }
        }
        return retval;
    }

    bool ClientPlayerIsModerator()
    { return HumanClientApp::GetApp()->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR; }
}

BuildingsPanel::BuildingsPanel(GG::X w, int columns, int planet_id) :
    AccordionPanel(w, GG::Y(ClientUI::Pts()*2)),
    m_planet_id(planet_id),
    m_columns(columns),
    m_building_indicators()
{}

void BuildingsPanel::CompleteConstruction() {
    AccordionPanel::CompleteConstruction();

    SetName("BuildingsPanel");

    if (m_columns < 1) {
        ErrorLogger() << "Attempted to create a BuidingsPanel with less than 1 column";
        m_columns = 1;
    }

    m_expand_button->LeftPressedSignal.connect(
        boost::bind(&BuildingsPanel::ExpandCollapseButtonPressed, this));

    // get owner, connect its production queue changed signal to update this panel
    auto planet = GetUniverseObject(m_planet_id);
    if (planet) {
        if (const Empire* empire = GetEmpire(planet->Owner())) {
            const ProductionQueue& queue = empire->GetProductionQueue();
            queue.ProductionQueueChangedSignal.connect(
                boost::bind(&BuildingsPanel::RequirePreRender, this));
        }
    }

    RequirePreRender();
}

BuildingsPanel::~BuildingsPanel()
{}

void BuildingsPanel::ExpandCollapse(bool expanded) {
    if (expanded == s_expanded_map[m_planet_id]) return; // nothing to do
    s_expanded_map[m_planet_id] = expanded;

    DoLayout();
}

void BuildingsPanel::Update() {
    //std::cout << "BuildingsPanel::Update" << std::endl;

    // remove old indicators
    for (auto& indicator : m_building_indicators)
        DetachChild(indicator.get());
    m_building_indicators.clear();

    auto planet = GetPlanet(m_planet_id);
    if (!planet) {
        ErrorLogger() << "BuildingsPanel::Update couldn't get planet with id " << m_planet_id;
        return;
    }
    int system_id = planet->SystemID();

    const int indicator_size = static_cast<int>(Value(Width() * 1.0 / m_columns));

    int this_client_empire_id = HumanClientApp::GetApp()->EmpireID();
    const auto& this_client_known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(this_client_empire_id);
    const auto& this_client_stale_object_info = GetUniverse().EmpireStaleKnowledgeObjectIDs(this_client_empire_id);

    // get existing / finished buildings and use them to create building indicators
    for (int object_id : planet->BuildingIDs()) {
        // skip known destroyed and stale info objects
        if (this_client_known_destroyed_objects.count(object_id))
            continue;
        if (this_client_stale_object_info.count(object_id))
            continue;

        auto building = GetBuilding(object_id);
        if (!building) {
            ErrorLogger() << "BuildingsPanel::Update couldn't get building with id: " << object_id << " on planet " << planet->Name();
            continue;
        }

        if (building->SystemID() != system_id || building->PlanetID() != m_planet_id)
            continue;

        auto ind = GG::Wnd::Create<BuildingIndicator>(GG::X(indicator_size), object_id);
        m_building_indicators.push_back(ind);

        ind->RightClickedSignal.connect(BuildingRightClickedSignal);
    }

    // get in-progress buildings
    const Empire* empire = GetEmpire(planet->Owner());
    if (!empire)
        return;

    int queue_index = -1;
    for (const auto& elem : empire->GetProductionQueue()) {
        ++queue_index;
        //std::cout << "queue index: " << queue_index << " elem: " << elem.Dump() << std::endl;
        if (elem.item.build_type != BT_BUILDING) continue;  // don't show in-progress ships in BuildingsPanel...
        if (elem.location != m_planet_id) continue;         // don't show buildings located elsewhere

        double total_cost;
        int total_turns;
        double turn_spending = elem.allocated_pp;
        std::tie(total_cost, total_turns) = empire->ProductionCostAndTime(elem);

        double progress = std::max(0.0f, empire->ProductionStatus(queue_index));
        double turns_completed = progress / std::max(total_cost, 1.0);
        auto ind = GG::Wnd::Create<BuildingIndicator>(GG::X(indicator_size), elem.item.name,
                                                      turns_completed, total_turns, total_cost, turn_spending);

        m_building_indicators.push_back(ind);
    }
}

void BuildingsPanel::PreRender() {
    AccordionPanel::PreRender();
    RefreshImpl();
}

void BuildingsPanel::Refresh() {
    RequirePreRender();
}

void BuildingsPanel::RefreshImpl() {
    Update();
    DoLayout();
}

void BuildingsPanel::EnableOrderIssuing(bool enable/* = true*/) {
    for (auto& indicator : m_building_indicators)
    { indicator->EnableOrderIssuing(enable); }
}

void BuildingsPanel::ExpandCollapseButtonPressed()
{ ExpandCollapse(!s_expanded_map[m_planet_id]); }

void BuildingsPanel::DoLayout() {
    auto old_size = Size();
    AccordionPanel::DoLayout();

    int row = 0;
    int column = 0;
    const int padding = 5;      // space around and between adjacent indicators
    const GG::X effective_width = Width() - padding * (m_columns + 1);  // padding on either side and between
    const int indicator_size = static_cast<int>(Value(effective_width * 1.0 / m_columns));
    GG::Y height;

    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_planet_id]) {
        int n = 0;
        for (auto& ind : m_building_indicators) {
            const GG::Pt ul = GG::Pt(MeterIconSize().x * n, GG::Y0);
            const GG::Pt lr = ul + MeterIconSize();

            if (lr.x < Width() - m_expand_button->Width()) {
                ind->SizeMove(ul, lr);
                AttachChild(ind);
            } else {
                DetachChild(ind.get());
            }
            ++n;
        }

        height = m_expand_button->Height();
    } else {
        for (auto& ind : m_building_indicators) {
            const GG::Pt ul = GG::Pt(GG::X(padding * (column + 1) + indicator_size * column), GG::Y(padding * (row + 1) + indicator_size * row));
            const GG::Pt lr = ul + GG::Pt(GG::X(indicator_size), GG::Y(indicator_size));

            ind->SizeMove(ul, lr);
            AttachChild(ind);
            ind->Show();

            ++column;
            if (column >= m_columns) {
                column = 0;
                ++row;
            }
        }

        if (column == 0)
            height = GG::Y(padding * (row + 1) + row * indicator_size);        // if column is 0, then there are no buildings in the next row
        else
            height = GG::Y(padding * (row + 2) + (row + 1) * indicator_size);  // if column != 0, there are buildings in the next row, so need to make space
    }

    if (m_building_indicators.empty()) {
        height = GG::Y(0);  // hide if empty
        DetachChild(m_expand_button.get());
    } else {
        AttachChild(m_expand_button);
        m_expand_button->Show();
        if (height < MeterIconSize().y)
            height = MeterIconSize().y;
    }

    Resize(GG::Pt(Width(), height));

    SetCollapsed(!s_expanded_map[m_planet_id]);

    if (old_size != Size())
        if (auto&& parent = Parent())
            parent->RequirePreRender();
}

std::map<int, bool> BuildingsPanel::s_expanded_map;

/////////////////////////////////////
//       BuildingIndicator         //
/////////////////////////////////////
ScanlineRenderer BuildingIndicator::s_scanline_shader;

BuildingIndicator::BuildingIndicator(GG::X w, int building_id) :
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y(Value(w)), GG::INTERACTIVE),
    m_building_id(building_id)
{
    if (auto building = GetBuilding(m_building_id))
        building->StateChangedSignal.connect(
            boost::bind(&BuildingIndicator::RequirePreRender, this));
}

BuildingIndicator::BuildingIndicator(GG::X w, const std::string& building_type,
                                     double turns_completed, double total_turns, double total_cost, double turn_spending) :
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y(Value(w)), GG::INTERACTIVE),
    m_building_id(INVALID_OBJECT_ID)
{
    auto texture = ClientUI::BuildingIcon(building_type);

    const BuildingType* type = GetBuildingType(building_type);
    const std::string& desc = type ? type->Description() : "";

    SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
        texture, UserString(building_type), UserString(desc)));

    m_graphic = GG::Wnd::Create<GG::StaticGraphic>(texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);

    float next_progress = turn_spending / std::max(1.0, total_cost);

    m_progress_bar = GG::Wnd::Create<MultiTurnProgressBar>(total_turns,
                                                           turns_completed,
                                                           next_progress,
                                                           GG::LightColor(ClientUI::TechWndProgressBarBackgroundColor()),
                                                           ClientUI::TechWndProgressBarColor(),
                                                           GG::LightColor(ClientUI::ResearchableTechFillColor()));

}

void BuildingIndicator::CompleteConstruction() {
    GG::Wnd::CompleteConstruction();

    if (m_building_id == INVALID_OBJECT_ID) {
        AttachChild(m_graphic);
        AttachChild(m_progress_bar);
        RequirePreRender();
    } else {
        Refresh();
    }
}

void BuildingIndicator::Render() {
    // copied from CUIWnd
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // Draw outline and background...
    GG::FlatRectangle(ul, lr, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);

    // Scanlines for not currently-visible objects?
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    if (empire_id == ALL_EMPIRES || !GetOptionsDB().Get<bool>("ui.map.scanlines.shown"))
        return;
    if (m_building_id == INVALID_OBJECT_ID)
        return;
    if (GetUniverse().GetObjectVisibilityByEmpire(m_building_id, empire_id) >= VIS_BASIC_VISIBILITY)
        return;

    s_scanline_shader.StartUsing();

    GLfloat verts[8];
    verts[0] = Value(ul.x); verts[1] = Value(ul.y);
    verts[2] = Value(lr.x); verts[3] = Value(ul.y);
    verts[4] = Value(lr.x); verts[5] = Value(lr.y);
    verts[6] = Value(ul.x); verts[7] = Value(lr.y);

    glDisable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, verts);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glPopClientAttrib();
    glEnable(GL_TEXTURE_2D);

    s_scanline_shader.StopUsing();
}

void BuildingIndicator::PreRender() {
    GG::Wnd::PreRender();
    Refresh();
}

void BuildingIndicator::Refresh() {
    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    auto building = GetBuilding(m_building_id);
    if (!building)
        return;

    ClearBrowseInfoWnd();

    DetachChildAndReset(m_graphic);
    DetachChildAndReset(m_scrap_indicator);

    if (const BuildingType* type = GetBuildingType(building->BuildingTypeName())) {
        auto texture = ClientUI::BuildingIcon(type->Name());
        m_graphic = GG::Wnd::Create<GG::StaticGraphic>(texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        AttachChild(m_graphic);

        std::string desc = UserString(type->Description());
        if (building->GetMeter(METER_STEALTH))
            desc = UserString("METER_STEALTH") + boost::io::str(boost::format(": %3.1f\n\n") % building->GetMeter(METER_STEALTH)->Current()) + desc;
        if (GetOptionsDB().Get<bool>("resource.effects.description.shown") && !type->Effects().empty())
            desc += "\n" + Dump(type->Effects());

        SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
            texture, UserString(type->Name()), desc));
    }

    if (building && building->OrderedScrapped()) {
        auto scrap_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "scrapped.png", true);
        m_scrap_indicator = GG::Wnd::Create<GG::StaticGraphic>(scrap_texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        AttachChild(m_scrap_indicator);
    }

    DoLayout();
}

void BuildingIndicator::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = Size();

    GG::Wnd::SizeMove(ul, lr);

    if (old_size != Size())
        DoLayout();
}

void BuildingIndicator::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void BuildingIndicator::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    // verify that this indicator represents an existing building, and not a
    // queued production item, and that the owner of the building is this
    // client's player's empire
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    auto building = GetBuilding(m_building_id);
    if (!building)
        return;

    const auto& map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    if (ClientPlayerIsModerator() && map_wnd->GetModeratorActionSetting() != MAS_NoAction) {
        RightClickedSignal(m_building_id);  // response handled in MapWnd
        return;
    }

    auto scrap_building_action = [this, empire_id]() {
        HumanClientApp::GetApp()->Orders().IssueOrder(std::make_shared<ScrapOrder>(empire_id, m_building_id));};
    auto un_scrap_building_action = [building]() {
        // find order to scrap this building, and recind it
        auto pending_scrap_orders = PendingScrapOrders();
        auto it = pending_scrap_orders.find(building->ID());
        if (it != pending_scrap_orders.end())
            HumanClientApp::GetApp()->Orders().RescindOrder(it->second);
    };

    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

    if (m_order_issuing_enabled) {
        if (!building->OrderedScrapped()) {
            // create popup menu with "Scrap" option
            popup->AddMenuItem(GG::MenuItem(UserString("ORDER_BUIDLING_SCRAP"), false, false, scrap_building_action));
        } else {
            // create popup menu with "Cancel Scrap" option
            popup->AddMenuItem(GG::MenuItem(UserString("ORDER_CANCEL_BUIDLING_SCRAP"), false, false, un_scrap_building_action));
        }
    }

    const std::string& building_type = building->BuildingTypeName();
    const BuildingType* bt = GetBuildingType(building_type);
    if (bt) {
        auto pedia_lookup_building_type_action = [building_type]() {
            ClientUI::GetClientUI()->ZoomToBuildingType(building_type);
        };
        std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % UserString(building_type));
        popup->AddMenuItem(GG::MenuItem(popup_label, false, false, pedia_lookup_building_type_action));
    }

    popup->Run();
}

void BuildingIndicator::EnableOrderIssuing(bool enable/* = true*/)
{ m_order_issuing_enabled = enable; }

void BuildingIndicator::DoLayout() {
    GG::Pt child_lr = Size() - GG::Pt(GG::X1, GG::Y1);   // extra pixel prevents graphic from overflowing border box

    if (m_graphic)
        m_graphic->SizeMove(GG::Pt(GG::X0, GG::Y0), child_lr);

    if (m_scrap_indicator)
        m_scrap_indicator->SizeMove(GG::Pt(GG::X0, GG::Y0), child_lr);

    GG::Y bar_top = Height() * 4 / 5;
    if (m_progress_bar)
        m_progress_bar->SizeMove(GG::Pt(GG::X0, bar_top), child_lr);
}
