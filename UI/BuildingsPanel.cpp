#include "BuildingsPanel.h"

#include <GG/Button.h>
#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>

#include "../util/Directories.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/Order.h"
#include "../universe/Building.h"
#include "../universe/Effect.h"
#include "../universe/Planet.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "CUIControls.h"
#include "IconTextBrowseWnd.h"
#include "MapWnd.h"
#include "MultiIconValueIndicator.h"
#include "MultiMeterStatusBar.h"
#include "ShaderProgram.h"

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
        const OrderSet& orders = app->Orders();
        for (OrderSet::const_iterator it = orders.begin(); it != orders.end(); ++it) {
            if (boost::shared_ptr<ScrapOrder> order = boost::dynamic_pointer_cast<ScrapOrder>(it->second)) {
                retval[order->ObjectID()] = it->first;
            }
        }
        return retval;
    }

    bool ClientPlayerIsModerator()
    { return HumanClientApp::GetApp()->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR; }
}

BuildingsPanel::BuildingsPanel(GG::X w, int columns, int planet_id) :
    AccordionPanel(w),
    m_planet_id(planet_id),
    m_columns(columns),
    m_building_indicators()
{
    SetName("BuildingsPanel");

    if (m_columns < 1) {
        ErrorLogger() << "Attempted to create a BuidingsPanel with less than 1 column";
        m_columns = 1;
    }

    GG::Connect(m_expand_button->LeftClickedSignal, &BuildingsPanel::ExpandCollapseButtonPressed, this);

    // get owner, connect its production queue changed signal to update this panel
    TemporaryPtr<const UniverseObject> planet = GetUniverseObject(m_planet_id);
    if (planet) {
        if (const Empire* empire = GetEmpire(planet->Owner())) {
            const ProductionQueue& queue = empire->GetProductionQueue();
            GG::Connect(queue.ProductionQueueChangedSignal, &BuildingsPanel::Refresh, this);
        }
    }

    Refresh();
}

BuildingsPanel::~BuildingsPanel() {
    // delete building indicators
    for (std::vector<BuildingIndicator*>::iterator it = m_building_indicators.begin(); it != m_building_indicators.end(); ++it)
        delete *it;
    m_building_indicators.clear();
}

void BuildingsPanel::ExpandCollapse(bool expanded) {
    if (expanded == s_expanded_map[m_planet_id]) return; // nothing to do
    s_expanded_map[m_planet_id] = expanded;

    DoLayout();
}

void BuildingsPanel::Update() {
    //std::cout << "BuildingsPanel::Update" << std::endl;

    // remove old indicators
    for (std::vector<BuildingIndicator*>::iterator it = m_building_indicators.begin(); it != m_building_indicators.end(); ++it) {
        DetachChild(*it);
        delete (*it);
    }
    m_building_indicators.clear();

    TemporaryPtr<const Planet> planet = GetPlanet(m_planet_id);
    if (!planet) {
        ErrorLogger() << "BuildingsPanel::Update couldn't get planet with id " << m_planet_id;
        return;
    }
    const std::set<int>& buildings = planet->BuildingIDs();
    int system_id = planet->SystemID();

    const int indicator_size = static_cast<int>(Value(Width() * 1.0 / m_columns));

    int this_client_empire_id = HumanClientApp::GetApp()->EmpireID();
    const std::set<int>& this_client_known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(this_client_empire_id);
    const std::set<int>& this_client_stale_object_info = GetUniverse().EmpireStaleKnowledgeObjectIDs(this_client_empire_id);

    // get existing / finished buildings and use them to create building indicators
    for (std::set<int>::const_iterator it = buildings.begin(); it != buildings.end(); ++it) {
        int object_id = *it;

        // skip known destroyed and stale info objects
        if (this_client_known_destroyed_objects.find(object_id) != this_client_known_destroyed_objects.end())
            continue;
        if (this_client_stale_object_info.find(object_id) != this_client_stale_object_info.end())
            continue;

        TemporaryPtr<const Building> building = GetBuilding(object_id);
        if (!building) {
            ErrorLogger() << "BuildingsPanel::Update couldn't get building with id: " << object_id << " on planet " << planet->Name();
            continue;
        }

        if (building->SystemID() != system_id || building->PlanetID() != m_planet_id)
            continue;

        BuildingIndicator* ind = new BuildingIndicator(GG::X(indicator_size), object_id);
        m_building_indicators.push_back(ind);

        GG::Connect(ind->RightClickedSignal,    BuildingRightClickedSignal);
    }

    // get in-progress buildings
    const Empire* empire = GetEmpire(planet->Owner());
    if (!empire)
        return;

    const ProductionQueue& queue = empire->GetProductionQueue();

    int queue_index = 0;
    for (ProductionQueue::const_iterator queue_it = queue.begin(); queue_it != queue.end(); ++queue_it, ++queue_index) {
        const ProductionQueue::Element elem = *queue_it;

        if (elem.item.build_type != BT_BUILDING) continue;  // don't show in-progress ships in BuildingsPanel...
        if (elem.location != m_planet_id) continue;         // don't show buildings located elsewhere

        double total_cost;
        int total_turns;
        boost::tie(total_cost, total_turns) = empire->ProductionCostAndTime(elem);

        double progress = std::max(0.0f, empire->ProductionStatus(queue_index));
        double turns_completed = total_turns * std::min<double>(1.0, progress / total_cost);
        BuildingIndicator* ind = new BuildingIndicator(GG::X(indicator_size), elem.item.name,
                                                       turns_completed, total_turns);
        m_building_indicators.push_back(ind);
    }
}

void BuildingsPanel::Refresh() {
    Update();
    DoLayout();
}

void BuildingsPanel::EnableOrderIssuing(bool enable/* = true*/) {
    for (std::vector<BuildingIndicator*>::iterator it = m_building_indicators.begin();
         it != m_building_indicators.end(); ++it)
    { (*it)->EnableOrderIssuing(enable); }
}

void BuildingsPanel::ExpandCollapseButtonPressed()
{ ExpandCollapse(!s_expanded_map[m_planet_id]); }

void BuildingsPanel::DoLayout() {
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
        for (std::vector<BuildingIndicator*>::iterator it = m_building_indicators.begin(); it != m_building_indicators.end(); ++it) {
            BuildingIndicator* ind = *it;

            const GG::Pt ul = GG::Pt(MeterIconSize().x * n, GG::Y0);
            const GG::Pt lr = ul + MeterIconSize();

            if (lr.x < Width() - m_expand_button->Width()) {
                ind->SizeMove(ul, lr);
                AttachChild(ind);
            } else {
                DetachChild(ind);
            }
            ++n;
        }

        height = m_expand_button->Height();
    } else {
        for (std::vector<BuildingIndicator*>::iterator it = m_building_indicators.begin(); it != m_building_indicators.end(); ++it) {
            BuildingIndicator* ind = *it;

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
        DetachChild(m_expand_button);
    } else {
        AttachChild(m_expand_button);
        m_expand_button->Show();
        if (height < MeterIconSize().y)
            height = MeterIconSize().y;
    }

    Resize(GG::Pt(Width(), height));

    Wnd::MoveChildUp(m_expand_button);

    m_expand_button->MoveTo(GG::Pt(Width() - m_expand_button->Width(), GG::Y0));

    SetCollapsed(!s_expanded_map[m_planet_id]);
}

std::map<int, bool> BuildingsPanel::s_expanded_map;

/////////////////////////////////////
//       BuildingIndicator         //
/////////////////////////////////////
boost::shared_ptr<ShaderProgram> BuildingIndicator::s_scanline_shader;

BuildingIndicator::BuildingIndicator(GG::X w, int building_id) :
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y(Value(w)), GG::INTERACTIVE),
    m_graphic(0),
    m_scrap_indicator(0),
    m_progress_bar(0),
    m_building_id(building_id),
    m_order_issuing_enabled(true)
{
    if (TemporaryPtr<const Building> building = GetBuilding(m_building_id))
        GG::Connect(building->StateChangedSignal,   &BuildingIndicator::Refresh,     this);
    Refresh();
}

BuildingIndicator::BuildingIndicator(GG::X w, const std::string& building_type,
                                     double turns_completed, double total_turns) :
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y(Value(w)), GG::INTERACTIVE),
    m_graphic(0),
    m_scrap_indicator(0),
    m_progress_bar(0),
    m_building_id(INVALID_OBJECT_ID),
    m_order_issuing_enabled(true)
{
    boost::shared_ptr<GG::Texture> texture = ClientUI::BuildingIcon(building_type);

    const BuildingType* type = GetBuildingType(building_type);
    const std::string& desc = type ? type->Description() : "";

    SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
        new IconTextBrowseWnd(texture, UserString(building_type), UserString(desc))));

    m_graphic = new GG::StaticGraphic(texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    AttachChild(m_graphic);

    m_progress_bar = new MultiTurnProgressBar(total_turns, turns_completed, GG::LightColor(ClientUI::TechWndProgressBarBackgroundColor()),
                                              ClientUI::TechWndProgressBarColor(), GG::LightColor(ClientUI::ResearchableTechFillColor()));
    AttachChild(m_progress_bar);

    Refresh();
}

void BuildingIndicator::Render() {
    // copied from CUIWnd
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // Draw outline and background...
    GG::FlatRectangle(ul, lr, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);

    // Scanlines for not currently-visible objects?
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    if (!s_scanline_shader || empire_id == ALL_EMPIRES || !GetOptionsDB().Get<bool>("UI.system-fog-of-war"))
        return;
    if (m_building_id == INVALID_OBJECT_ID)
        return;
    if (GetUniverse().GetObjectVisibilityByEmpire(m_building_id, empire_id) >= VIS_BASIC_VISIBILITY)
        return;

    float fog_scanline_spacing = static_cast<float>(GetOptionsDB().Get<double>("UI.system-fog-of-war-spacing"));
    s_scanline_shader->Use();
    s_scanline_shader->Bind("scanline_spacing", fog_scanline_spacing);
    glBegin(GL_POLYGON);
        glColor(GG::CLR_WHITE);
        glVertex(ul.x, ul.y);
        glVertex(lr.x, ul.y);
        glVertex(lr.x, lr.y);
        glVertex(ul.x, lr.y);
        glVertex(ul.x, ul.y);
    glEnd();
    s_scanline_shader->stopUse();
}

void BuildingIndicator::Refresh() {
    if (!s_scanline_shader && GetOptionsDB().Get<bool>("UI.system-fog-of-war")) {
        boost::filesystem::path shader_path = GetRootDataDir() / "default" / "shaders" / "scanlines.frag";
        std::string shader_text;
        ReadFile(shader_path, shader_text);
        s_scanline_shader = boost::shared_ptr<ShaderProgram>(
            ShaderProgram::shaderProgramFactory("", shader_text));
    }

    SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));

    TemporaryPtr<const Building> building = GetBuilding(m_building_id);
    if (!building)
        return;

    ClearBrowseInfoWnd();

    if (m_graphic) {
        delete m_graphic;
        m_graphic = 0;
    }
    if (m_scrap_indicator) {
        delete m_scrap_indicator;
        m_scrap_indicator = 0;
    }

    if (const BuildingType* type = GetBuildingType(building->BuildingTypeName())) {
        boost::shared_ptr<GG::Texture> texture = ClientUI::BuildingIcon(type->Name());
        m_graphic = new GG::StaticGraphic(texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        AttachChild(m_graphic);

        std::string desc = UserString(type->Description());
        if (building->GetMeter(METER_STEALTH))
            desc = UserString("METER_STEALTH") + boost::io::str(boost::format(": %3.1f\n\n") % building->GetMeter(METER_STEALTH)->Current()) + desc;
        if (GetOptionsDB().Get<bool>("UI.autogenerated-effects-descriptions") && !type->Effects().empty())
            desc += boost::io::str(FlexibleFormat(UserString("ENC_EFFECTS_STR")) % EffectsDescription(type->Effects()));

        SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
            new IconTextBrowseWnd(texture, UserString(type->Name()), desc)));
    }

    if (building && building->OrderedScrapped()) {
        boost::shared_ptr<GG::Texture> scrap_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "scrapped.png", true);
        m_scrap_indicator = new GG::StaticGraphic(scrap_texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
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
    TemporaryPtr<Building> building = GetBuilding(m_building_id);
    if (!building)
        return;

    const MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    if (ClientPlayerIsModerator() && map_wnd->GetModeratorActionSetting() != MAS_NoAction) {
        RightClickedSignal(m_building_id);  // response handled in MapWnd
        return;
    }

    if (!building->OwnedBy(empire_id) || !m_order_issuing_enabled) {
        return;
    }

    GG::MenuItem menu_contents;

    if (!building->OrderedScrapped()) {
        // create popup menu with "Scrap" option
        menu_contents.next_level.push_back(GG::MenuItem(UserString("ORDER_BUIDLING_SCRAP"), 3, false, false));
    } else {
        // create popup menu with "Cancel Scrap" option
        menu_contents.next_level.push_back(GG::MenuItem(UserString("ORDER_CANCEL_BUIDLING_SCRAP"), 4, false, false));
    }

    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor(),
                        ClientUI::WndOuterBorderColor(), ClientUI::WndColor(), ClientUI::EditHiliteColor());
    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 3: { // scrap building
            if (m_order_issuing_enabled)
                HumanClientApp::GetApp()->Orders().IssueOrder(
                    OrderPtr(new ScrapOrder(empire_id, m_building_id)));
            break;
        }

        case 4: { // un-scrap building
            if (!m_order_issuing_enabled)
                break;

            // find order to scrap this building, and recind it
            std::map<int, int> pending_scrap_orders = PendingScrapOrders();
            std::map<int, int>::const_iterator it = pending_scrap_orders.find(building->ID());
            if (it != pending_scrap_orders.end()) {
                HumanClientApp::GetApp()->Orders().RecindOrder(it->second);
            break;
            }
        }

        default:
            break;
        }
    }
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
