#include "InfoPanels.h"

#include "../universe/UniverseObject.h"
#include "../universe/PopCenter.h"
#include "../universe/ResourceCenter.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../universe/Building.h"
#include "../universe/Ship.h"
#include "../universe/Special.h"
#include "../universe/Effect.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "../util/OptionsDB.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "Sound.h"

#include <GG/DrawUtil.h>
#include <GG/GUI.h>
#include <GG/StaticGraphic.h>
#include <GG/StyleFactory.h>
#include <GG/WndEvent.h>

#include <boost/lexical_cast.hpp>

namespace {
    /** Returns text wrapped in GG RGBA tags for specified colour */
    std::string ColourWrappedtext(const std::string& text, const GG::Clr colour) {
        return GG::RgbaTag(colour) + text + "</rgba>";
    }

    /** Returns text representation of number wrapped in GG RGBA tags for
      * colour depending on whether number is positive, negative or 0.0 */
    std::string ColouredNumber(double number) {
        GG::Clr clr = ClientUI::TextColor();
        if (number > 0.0)
            clr = ClientUI::StatIncrColor();
        else if (number < 0.0)
            clr = ClientUI::StatDecrColor();
        return ColourWrappedtext(DoubleToString(number, 3, true), clr);
    }

    /** Returns GG::Clr with which to display programatically coloured things
      * (such as meter bars) for the indicated \a meter_type */
    GG::Clr MeterColor(MeterType meter_type) {
        switch (meter_type) {
        case METER_GROWTH:
            return GG::CLR_YELLOW;
            break;
        case METER_MINING:
        case METER_TARGET_MINING:
            return GG::CLR_RED;
            break;
        case METER_INDUSTRY:
        case METER_TARGET_INDUSTRY:
            return GG::Clr(0, 100, 255, 255);   // a bit greener / brighter than blue, at pd's suggestion on forums (sort of)
            break;
        case METER_RESEARCH:
        case METER_TARGET_RESEARCH:
            return GG::CLR_GREEN;
            break;
        case METER_TRADE:
        case METER_TARGET_TRADE:
        case METER_TROOPS:
        case METER_MAX_TROOPS:
            return GG::Clr(255, 148, 0, 255);   // orange
            break;
        case METER_CONSTRUCTION:
        case METER_TARGET_CONSTRUCTION:
        case METER_POPULATION:
        case METER_TARGET_POPULATION:
        default:
            return GG::CLR_WHITE;
        }
    }

    /** How big we want meter icons with respect to the current UI font size.
      * Meters should scale along font size, but not below the size for the
      * default 12 points font. */
    GG::Pt MeterIconSize() {
        const int icon_size = std::max(ClientUI::Pts(), 12) * 4/3;
        return GG::Pt(GG::X(icon_size), GG::Y(icon_size));
    }

    /** Returns how much of specified \a resource_type is being consumed by the
      * empire with id \a empire_id at the location of the specified
      * object \a obj. */
    double ObjectResourceConsumption(const UniverseObject* obj, ResourceType resource_type, int empire_id = ALL_EMPIRES) {
        if (!obj) {
            Logger().errorStream() << "ObjectResourceConsumption passed a null object";
            return 0.0;
        }
        if (resource_type == INVALID_RESOURCE_TYPE) {
            Logger().errorStream() << "ObjectResourceConsumption passed a INVALID_RESOURCE_TYPE";
            return 0.0;
        }


        const Empire* empire = 0;

        if (empire_id != ALL_EMPIRES) {
            empire = Empires().Lookup(empire_id);

            if (!empire) {
                Logger().errorStream() << "ObjectResourceConsumption requested consumption for empire " << empire_id << " but this empire was not found";
                return 0.0;     // requested a specific empire, but didn't find it in this client, so production is 0.0
            }

            if (!obj->OwnedBy(empire_id)) {
                Logger().debugStream() << "ObjectResourceConsumption requested consumption for empire " << empire_id << " but this empire doesn't own the object";
                return 0.0;     // if the empire doesn't own the object, assuming it can't be consuming any of the empire's resources.  May need to revisit this assumption later.
            }
        }


        const PopCenter* pc = 0;
        double prod_queue_allocation_sum = 0.0;
        const Building* building = 0;

        switch (resource_type) {
        case RE_MINERALS:
        case RE_INDUSTRY:
            // PP (equal to mineral and industry) cost of objects on production queue at this object's location
            if (empire) {
                // add allocated PP for all production items at this location for this empire
                const ProductionQueue& queue = empire->GetProductionQueue();
                for (ProductionQueue::const_iterator queue_it = queue.begin(); queue_it != queue.end(); ++queue_it)
                    if (queue_it->location == obj->ID())
                        prod_queue_allocation_sum += queue_it->allocated_pp;

            } else {
                // add allocated PP for all production items at this location for all empires
                for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
                    empire = it->second;
                    const ProductionQueue& queue = empire->GetProductionQueue();
                    for (ProductionQueue::const_iterator queue_it = queue.begin(); queue_it != queue.end(); ++queue_it)
                        if (queue_it->location == obj->ID())
                            prod_queue_allocation_sum += queue_it->allocated_pp;
                }
            }
            return prod_queue_allocation_sum;
            break;

        case RE_TRADE:
            // maintenance cost of this object
            if (building = dynamic_cast<const Building*>(obj))
                return building->GetBuildingType()->MaintenanceCost();
            return 0.0; // if not a building, doesn't presently consume trade
            break;

        case RE_RESEARCH:
            // research isn't consumed at a particular location, so none is consumed at any location
        default:
            // for INVALID_RESOURCE_TYPE just return 0.0.  Could throw an exception, I suppose...
            break;
        }
        return 0.0;
    }

    /** Returns height of rows of text in InfoTextBrowseWnd. */
    int IconTextBrowseWndRowHeight() {
        return ClientUI::Pts()*3/2;
    }

    const GG::X     METER_BROWSE_LABEL_WIDTH(300);
    const GG::X     METER_BROWSE_VALUE_WIDTH(50);
    const int       EDGE_PAD(3);
    const int       MULTI_INDICATOR_ICON_SPACING(12);
    const GG::X     MULTI_INDICATOR_ICON_WIDTH(24);
    const GG::Y     MULTI_INDICATOR_ICON_HEIGHT(24);
    const int       BAR_PAD(1);
    const GG::Y     BAR_HEIGHT(10);
    const GG::X     LABEL_WIDTH(240);
    const GG::X     VALUE_WIDTH(60);
    const GG::X     ICON_BROWSE_TEXT_WIDTH(400);
    const GG::X     ICON_BROWSE_ICON_WIDTH(64);
    const GG::Y     ICON_BROWSE_ICON_HEIGHT(64);
    const GG::X     SPECIAL_ICON_WIDTH(24);
    const GG::Y     SPECIAL_ICON_HEIGHT(24);

    const double    MULTI_METER_STATUS_BAR_DISPLAYED_METER_RANGE = 100.0;

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

    class MeterModifiersIndicatorBrowseWnd : public GG::BrowseInfoWnd {
    public:
        MeterModifiersIndicatorBrowseWnd(int object_id, MeterType meter_type);
        virtual bool    WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const;
        virtual void    Render();

    private:
        void            Initialize();
        virtual void    UpdateImpl(std::size_t mode, const Wnd* target);

        MeterType               m_meter_type;
        int                     m_source_object_id;

        GG::TextControl*        m_summary_title;

        GG::TextControl*        m_sum_label;
        GG::TextControl*        m_sum_value;

        std::vector<std::pair<GG::TextControl*, GG::TextControl*> >
                                m_effect_labels_and_values;

        GG::Y                   m_row_height;
        bool                    m_initialized;
    };


}

/////////////////////////////////////
//        PopulationPanel          //
/////////////////////////////////////
std::map<int, bool> PopulationPanel::s_expanded_map = std::map<int, bool>();

PopulationPanel::PopulationPanel(GG::X w, int object_id) :
    Wnd(GG::X0, GG::Y0, w, GG::Y(ClientUI::Pts()*2), GG::INTERACTIVE),
    m_popcenter_id(object_id),
    m_pop_stat(0),
    m_multi_icon_value_indicator(0), m_multi_meter_status_bar(0),
    m_expand_button(0)
{
    SetName("PopulationPanel");

    const UniverseObject* obj = GetUniverseObject(m_popcenter_id);
    if (!obj)
        throw std::invalid_argument("Attempted to construct a PopulationPanel with an invalid object id");
    const PopCenter* pop = dynamic_cast<const PopCenter*>(obj);
    if (!pop)
        throw std::invalid_argument("Attempted to construct a PopulationPanel with an object id is not a PopCenter");

    m_expand_button = new GG::Button(w - 16, GG::Y0, GG::X(16), GG::Y(16), "", ClientUI::GetFont(),
                                     GG::CLR_WHITE, GG::CLR_ZERO, GG::ONTOP | GG::INTERACTIVE);
    AttachChild(m_expand_button);
    m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrownormal.png"   ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowclicked.png"  ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowmouseover.png"), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    GG::Connect(m_expand_button->ClickedSignal, &PopulationPanel::ExpandCollapseButtonPressed, this);

    m_pop_stat = new StatisticIcon(GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y, ClientUI::SpeciesIcon(pop->SpeciesName()),
                                   0, 3, false);
    AttachChild(m_pop_stat);


    // meter and production indicators
    std::vector<std::pair<MeterType, MeterType> > meters;
    meters.push_back(std::make_pair(METER_POPULATION, METER_TARGET_POPULATION));

    // attach and show meter bars and large resource indicators
    m_multi_icon_value_indicator =  new MultiIconValueIndicator(Width() - 2*EDGE_PAD,   m_popcenter_id, meters);
    m_multi_meter_status_bar =      new MultiMeterStatusBar(Width() - 2*EDGE_PAD,       m_popcenter_id, meters);

    // determine if this panel has been created yet.
    std::map<int, bool>::iterator it = s_expanded_map.find(m_popcenter_id);
    if (it == s_expanded_map.end())
        s_expanded_map[m_popcenter_id] = false; // if not, default to collapsed state

    Refresh();
}

PopulationPanel::~PopulationPanel() {
    // manually delete all pointed-to controls that may or may not be attached as a child window at time of deletion
    delete m_pop_stat;
    delete m_multi_icon_value_indicator;
    delete m_multi_meter_status_bar;

    // don't need to manually delete m_expand_button, as it is attached as a child so will be deleted by ~Wnd
}

void PopulationPanel::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void PopulationPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    GG::Wnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        DoExpandCollapseLayout();
}

void PopulationPanel::ExpandCollapseButtonPressed()
{ ExpandCollapse(!s_expanded_map[m_popcenter_id]); }

void PopulationPanel::ExpandCollapse(bool expanded) {
    if (expanded == s_expanded_map[m_popcenter_id]) return; // nothing to do
    s_expanded_map[m_popcenter_id] = expanded;

    DoExpandCollapseLayout();
}

void PopulationPanel::DoExpandCollapseLayout() {
    // initially detach most things.  Some will be reattached later.
    DetachChild(m_pop_stat);

    // detach / hide meter bars and large resource indicators
    DetachChild(m_multi_meter_status_bar);
    DetachChild(m_multi_icon_value_indicator);


    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_popcenter_id]) {

        std::vector<StatisticIcon*> icons;
        icons.push_back(m_pop_stat);

        // position and reattach icons to be shown
        for (int n = 0; n < static_cast<int>(icons.size()); ++n) {
            GG::X x = MeterIconSize().x*n*7/2;

            if (x > Width() - m_expand_button->Width() - MeterIconSize().x*5/2) break;  // ensure icon doesn't extend past right edge of panel

            StatisticIcon* icon = icons[n];
            AttachChild(icon);
            icon->MoveTo(GG::Pt(x, GG::Y0));
            icon->Show();
        }

        Resize(GG::Pt(Width(), std::max(MeterIconSize().y, m_expand_button->Height())));

    } else {
        // attach and show meter bars and large resource indicators
        GG::Y top = UpperLeft().y;

        AttachChild(m_multi_icon_value_indicator);
        m_multi_icon_value_indicator->MoveTo(GG::Pt(GG::X(EDGE_PAD), GG::Y(EDGE_PAD)));
        m_multi_icon_value_indicator->Resize(GG::Pt(Width() - 2*EDGE_PAD, m_multi_icon_value_indicator->Height()));

        AttachChild(m_multi_meter_status_bar);
        m_multi_meter_status_bar->MoveTo(GG::Pt(GG::X(EDGE_PAD), m_multi_icon_value_indicator->LowerRight().y + EDGE_PAD - top));
        m_multi_meter_status_bar->Resize(GG::Pt(Width() - 2*EDGE_PAD, m_multi_meter_status_bar->Height()));

        MoveChildUp(m_expand_button);

        Resize(GG::Pt(Width(), m_multi_meter_status_bar->LowerRight().y + EDGE_PAD - top));
    }

    m_expand_button->MoveTo(GG::Pt(Width() - m_expand_button->Width(), GG::Y0));

    // update appearance of expand/collapse button
    if (s_expanded_map[m_popcenter_id]) {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrownormal.png"   ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrowclicked.png"  ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrowmouseover.png"), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    } else {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrownormal.png"   ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowclicked.png"  ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowmouseover.png"), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    }

    ExpandCollapseSignal();
}

void PopulationPanel::Render() {
    // Draw outline and background...

    // copied from CUIWnd
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // use GL to draw the lines
    glDisable(GL_TEXTURE_2D);
    GLint initial_modes[2];
    glGetIntegerv(GL_POLYGON_MODE, initial_modes);

    // draw background
    glPolygonMode(GL_BACK, GL_FILL);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndColor());
        glVertex(ul.x, ul.y);
        glVertex(lr.x, ul.y);
        glVertex(lr.x, lr.y);
        glVertex(ul.x, lr.y);
        glVertex(ul.x, ul.y);
    glEnd();

    // draw outer border on pixel inside of the outer edge of the window
    glPolygonMode(GL_BACK, GL_LINE);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndOuterBorderColor());
        glVertex(ul.x, ul.y);
        glVertex(lr.x, ul.y);
        glVertex(lr.x, lr.y);
        glVertex(ul.x, lr.y);
        glVertex(ul.x, ul.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    glEnable(GL_TEXTURE_2D);
}

void PopulationPanel::Update() {
    const PopCenter*        pop = GetPopCenter();
    const UniverseObject*   obj = GetUniverseObject(m_popcenter_id);

    if (!pop || !obj) {
        Logger().errorStream() << "PopulationPanel::Update couldn't get PopCenter or couldn't get UniverseObject";
        return;
    }

    // meter bar displays and stat icons
    m_multi_meter_status_bar->Update();
    m_multi_icon_value_indicator->Update();

    m_pop_stat->SetValue(pop->InitialMeterValue(METER_POPULATION));


    // tooltips
    boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd(new MeterBrowseWnd(m_popcenter_id, METER_POPULATION, METER_TARGET_POPULATION));
    m_pop_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_POPULATION, browse_wnd);
}

void PopulationPanel::Refresh() {
    Update();
    DoExpandCollapseLayout();
}

const PopCenter* PopulationPanel::GetPopCenter() const {
    const UniverseObject* obj = GetUniverseObject(m_popcenter_id);
    if (!obj) {
        Logger().errorStream() << "PopulationPanel tried to get an object with an invalid m_popcenter_id";
        return 0;
    }
    const PopCenter* pop = dynamic_cast<const PopCenter*>(obj);
    if (!pop) {
        Logger().errorStream() << "PopulationPanel failed casting an object pointer to a PopCenter pointer";
        return 0;
    }
    return pop;
}

void PopulationPanel::EnableOrderIssuing(bool enable/* = true*/)
{}


/////////////////////////////////////
//         ResourcePanel           //
/////////////////////////////////////
std::map<int, bool> ResourcePanel::s_expanded_map;

ResourcePanel::ResourcePanel(GG::X w, int object_id) :
    Wnd(GG::X0, GG::Y0, w, GG::Y(ClientUI::Pts()*9), GG::INTERACTIVE),
    m_rescenter_id(object_id),
    m_pop_mod_stat(0),
    m_growth_stat(0),
    m_mining_stat(0),
    m_industry_stat(0),
    m_research_stat(0),
    m_trade_stat(0),
    m_multi_icon_value_indicator(0),
    m_multi_meter_status_bar(0),
    m_focus_drop(0),
    m_expand_button(0)
{
    SetName("ResourcePanel");

    const UniverseObject* obj = GetUniverseObject(m_rescenter_id);
    if (!obj)
        throw std::invalid_argument("Attempted to construct a ResourcePanel with an object_id that is not an UniverseObject");
    const ResourceCenter* res = dynamic_cast<const ResourceCenter*>(obj);
    if (!res)
        throw std::invalid_argument("Attempted to construct a ResourcePanel with an UniverseObject that is not a ResourceCenter");

    SetChildClippingMode(ClipToClient);

    // expand / collapse button at top right
    m_expand_button = new GG::Button(w - 16, GG::Y0, GG::X(16), GG::Y(16), "", ClientUI::GetFont(), GG::CLR_WHITE, GG::CLR_ZERO, GG::ONTOP | GG::INTERACTIVE);
    AttachChild(m_expand_button);
    m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrownormal.png"   ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowclicked.png"  ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowmouseover.png"), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    GG::Connect(m_expand_button->ClickedSignal, &ResourcePanel::ExpandCollapseButtonPressed, this);


    // focus-selection droplist
    m_focus_drop = new CUIDropDownList(GG::X0, GG::Y0, MeterIconSize().x*4, MeterIconSize().y*3/2, MeterIconSize().y*7/2);
    AttachChild(m_focus_drop);
    GG::Connect(m_focus_drop->SelChangedSignal, &ResourcePanel::FocusDropListSelectionChanged,  this);
    m_focus_drop->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));


    // small resource indicators - for use when panel is collapsed
    m_growth_stat = new StatisticIcon(GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y,
                                        ClientUI::MeterIcon(METER_GROWTH), 0, 3, false);

    m_mining_stat = new StatisticIcon(GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y,
                                      ClientUI::MeterIcon(METER_MINING), 0, 3, false);
    AttachChild(m_mining_stat);

    m_industry_stat = new StatisticIcon(GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y,
                                        ClientUI::MeterIcon(METER_INDUSTRY), 0, 3, false);
    AttachChild(m_industry_stat);

    m_research_stat = new StatisticIcon(GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y,
                                        ClientUI::MeterIcon(METER_RESEARCH), 0, 3, false);
    AttachChild(m_research_stat);

    m_trade_stat = new StatisticIcon(GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y,
                                     ClientUI::MeterIcon(METER_TRADE), 0, 3, false);
    AttachChild(m_trade_stat);

    m_pop_mod_stat = new MeterModifiersIndicator(GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y,
                                                 m_rescenter_id, METER_TARGET_POPULATION);
    AttachChild(m_pop_mod_stat);


    // meter and production indicators
    std::vector<std::pair<MeterType, MeterType> > meters;
    meters.push_back(std::make_pair(METER_GROWTH,       INVALID_METER_TYPE));
    meters.push_back(std::make_pair(METER_MINING,       METER_TARGET_MINING));
    meters.push_back(std::make_pair(METER_INDUSTRY,     METER_TARGET_INDUSTRY));
    meters.push_back(std::make_pair(METER_RESEARCH,     METER_TARGET_RESEARCH));
    meters.push_back(std::make_pair(METER_TRADE,        METER_TARGET_TRADE));
    meters.push_back(std::make_pair(METER_CONSTRUCTION, METER_TARGET_CONSTRUCTION));

    m_multi_meter_status_bar =      new MultiMeterStatusBar(Width() - 2*EDGE_PAD,       m_rescenter_id, meters);
    m_multi_icon_value_indicator =  new MultiIconValueIndicator(Width() - 2*EDGE_PAD,   m_rescenter_id, meters);

    // determine if this panel has been created yet.
    std::map<int, bool>::iterator it = s_expanded_map.find(m_rescenter_id);
    if (it == s_expanded_map.end())
        s_expanded_map[m_rescenter_id] = false; // if not, default to collapsed state

    Refresh();
}

ResourcePanel::~ResourcePanel() {
    // manually delete all pointed-to controls that may or may not be attached as a child window at time of deletion
    delete m_multi_icon_value_indicator;
    delete m_multi_meter_status_bar;

    delete m_pop_mod_stat;
    delete m_growth_stat;
    delete m_mining_stat;
    delete m_industry_stat;
    delete m_research_stat;
    delete m_trade_stat;

    delete m_focus_drop;

    // don't need to manually delete m_expand_button, as it is attached as a child so will be deleted by ~Wnd
}

void ResourcePanel::ExpandCollapseButtonPressed() {
    ExpandCollapse(!s_expanded_map[m_rescenter_id]);
}

void ResourcePanel::ExpandCollapse(bool expanded) {
    if (expanded == s_expanded_map[m_rescenter_id]) return; // nothing to do
    s_expanded_map[m_rescenter_id] = expanded;

    DoExpandCollapseLayout();
}

void ResourcePanel::DoExpandCollapseLayout() {
    // initially detach everything (most things?).  Some will be reattached later.
    DetachChild(m_mining_stat);     DetachChild(m_industry_stat);
    DetachChild(m_research_stat);   DetachChild(m_trade_stat);
    DetachChild(m_pop_mod_stat);    DetachChild(m_growth_stat);

    DetachChild(m_focus_drop);

    DetachChild(m_multi_meter_status_bar);
    DetachChild(m_multi_icon_value_indicator);


    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_rescenter_id]) {
        const UniverseObject* obj = GetUniverseObject(m_rescenter_id);
        const ResourceCenter* res = dynamic_cast<const ResourceCenter*>(obj);

        if (res) {
            // determine which two resource icons to display while collapsed: the two with the highest production.
            // sort by insereting into multimap keyed by production amount, then taking the first two icons therein.
            std::multimap<double, StatisticIcon*> res_prod_icon_map;
            res_prod_icon_map.insert(std::pair<double, StatisticIcon*>(m_growth_stat->GetValue(),   m_growth_stat));
            res_prod_icon_map.insert(std::pair<double, StatisticIcon*>(m_mining_stat->GetValue(),   m_mining_stat));
            res_prod_icon_map.insert(std::pair<double, StatisticIcon*>(m_industry_stat->GetValue(), m_industry_stat));
            res_prod_icon_map.insert(std::pair<double, StatisticIcon*>(m_research_stat->GetValue(), m_research_stat));
            res_prod_icon_map.insert(std::pair<double, StatisticIcon*>(m_trade_stat->GetValue(),    m_trade_stat));
            res_prod_icon_map.insert(std::pair<double, StatisticIcon*>(m_pop_mod_stat->GetValue(),  m_pop_mod_stat));

            // position and reattach icons to be shown
            int n = 0;
            for (std::multimap<double, StatisticIcon*>::iterator it = res_prod_icon_map.end(); it != res_prod_icon_map.begin();) {
                GG::X x = MeterIconSize().x*n*7/2;

                if (x > Width() - m_expand_button->Width() - MeterIconSize().x*5/2) break;  // ensure icon doesn't extend past right edge of panel

                std::multimap<double, StatisticIcon*>::iterator it2 = --it;

                StatisticIcon* icon = it2->second;
                AttachChild(icon);
                icon->MoveTo(GG::Pt(x, GG::Y0));
                icon->Show();

                n++;
            }
        }

        Resize(GG::Pt(Width(), std::max(MeterIconSize().y, m_expand_button->Height())));
    } else {
        // attach / show focus selector drop
        m_focus_drop->Show();
        AttachChild(m_focus_drop);

        // attach and show meter bars and large resource indicators
        GG::Y top = UpperLeft().y;

        AttachChild(m_multi_icon_value_indicator);
        m_multi_icon_value_indicator->MoveTo(GG::Pt(GG::X(EDGE_PAD), m_focus_drop->LowerRight().y + EDGE_PAD - top));
        m_multi_icon_value_indicator->Resize(GG::Pt(Width() - 2*EDGE_PAD, m_multi_icon_value_indicator->Height()));

        AttachChild(m_multi_meter_status_bar);
        m_multi_meter_status_bar->MoveTo(GG::Pt(GG::X(EDGE_PAD), m_multi_icon_value_indicator->LowerRight().y + EDGE_PAD - top));
        m_multi_meter_status_bar->Resize(GG::Pt(Width() - 2*EDGE_PAD, m_multi_meter_status_bar->Height()));

        Resize(GG::Pt(Width(), m_multi_meter_status_bar->LowerRight().y + EDGE_PAD - top));
    }

    m_expand_button->MoveTo(GG::Pt(Width() - m_expand_button->Width(), GG::Y0));

    // update appearance of expand/collapse button
    if (s_expanded_map[m_rescenter_id]) {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrownormal.png"   ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrowclicked.png"  ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrowmouseover.png"), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    } else {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrownormal.png"   ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowclicked.png"  ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowmouseover.png"), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    }

    ExpandCollapseSignal();
}

void ResourcePanel::Render() {
    // Draw outline and background...

    // copied from CUIWnd
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // use GL to draw the lines
    glDisable(GL_TEXTURE_2D);
    GLint initial_modes[2];
    glGetIntegerv(GL_POLYGON_MODE, initial_modes);

    // draw background
    glPolygonMode(GL_BACK, GL_FILL);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndColor());
        glVertex(ul.x, ul.y);
        glVertex(lr.x, ul.y);
        glVertex(lr.x, lr.y);
        glVertex(ul.x, lr.y);
        glVertex(ul.x, ul.y);
    glEnd();

    // draw outer border on pixel inside of the outer edge of the window
    glPolygonMode(GL_BACK, GL_LINE);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndOuterBorderColor());
        glVertex(ul.x, ul.y);
        glVertex(lr.x, ul.y);
        glVertex(lr.x, lr.y);
        glVertex(ul.x, lr.y);
        glVertex(ul.x, ul.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    glEnable(GL_TEXTURE_2D);

    // draw details depending on state of ownership and expanded / collapsed status

    // determine ownership
    /*const UniverseObject* obj = GetUniverseObject(m_rescenter_id);
    if (obj->Unowned())
        // uninhabited
    else {
        if(!obj->OwnedBy(HumanClientApp::GetApp()->EmpireID()))
            // inhabited by other empire
        else
            // inhabited by this empire (and possibly other empires)
    }*/

}

void ResourcePanel::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void ResourcePanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    GG::Wnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        DoExpandCollapseLayout();
}

void ResourcePanel::Update() {
    // remove any old browse wnds
    m_growth_stat->ClearBrowseInfoWnd();
    m_multi_icon_value_indicator->ClearToolTip(METER_GROWTH);

    m_mining_stat->ClearBrowseInfoWnd();
    m_multi_icon_value_indicator->ClearToolTip(METER_MINING);

    m_industry_stat->ClearBrowseInfoWnd();
    m_multi_icon_value_indicator->ClearToolTip(METER_INDUSTRY);

    m_research_stat->ClearBrowseInfoWnd();
    m_multi_icon_value_indicator->ClearToolTip(METER_RESEARCH);

    m_trade_stat->ClearBrowseInfoWnd();
    m_multi_icon_value_indicator->ClearToolTip(METER_TRADE);

    m_pop_mod_stat->ClearBrowseInfoWnd();

    m_multi_icon_value_indicator->ClearToolTip(METER_CONSTRUCTION);


    const UniverseObject* obj = GetUniverseObject(m_rescenter_id);
    if (!obj)
        obj = GetEmpireKnownObject(m_rescenter_id, HumanClientApp::GetApp()->EmpireID());
    if (!obj) {
        Logger().errorStream() << "BuildingPanel::Update couldn't get object with id " << m_rescenter_id;
        return;
    }
    const ResourceCenter* res = dynamic_cast<const ResourceCenter*>(obj);
    if (!res) {
        Logger().errorStream() << "BuildingPanel::Update couldn't convert object with id " << m_rescenter_id << " to a ResourceCenter";
        return;
    }

    //std::cout << "ResourcePanel::Update() object: " << obj->Name() << std::endl;
    //Logger().debugStream() << "ResourcePanel::Update()";
    //Logger().debugStream() << obj->Dump();

    enum OWNERSHIP {OS_NONE, OS_FOREIGN, OS_SELF} owner = OS_NONE;

    // determine ownership
    if (obj->Unowned()) {
        owner = OS_NONE;  // uninhabited
    } else {
        if (!obj->OwnedBy(HumanClientApp::GetApp()->EmpireID()))
            owner = OS_FOREIGN; // inhabited by other empire
        else
            owner = OS_SELF; // inhabited by this empire (and possibly other empires)
    }


    // only allow focus changes in UI for planets this client's player's empire owns
    if (owner == OS_SELF) {
        m_focus_drop->Disable(false);
    } else {
        m_focus_drop->Disable(true);
    }


    // meter bar displays and production stats
    m_multi_meter_status_bar->Update();
    m_multi_icon_value_indicator->Update();

    m_growth_stat->SetValue(res->InitialMeterValue(METER_GROWTH));
    m_mining_stat->SetValue(res->InitialMeterValue(METER_MINING));
    m_industry_stat->SetValue(res->InitialMeterValue(METER_INDUSTRY));
    m_research_stat->SetValue(res->InitialMeterValue(METER_RESEARCH));
    m_trade_stat->SetValue(res->InitialMeterValue(METER_TRADE));
    m_pop_mod_stat->Update();


    // create an attach browse info wnds for each meter type on the icon + number stats used when collapsed and
    // for all meter types shown in the multi icon value indicator.  this replaces any previous-present
    // browse wnd on these indicators
    boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_rescenter_id, METER_GROWTH));
    m_growth_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_GROWTH, browse_wnd);

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_rescenter_id, METER_MINING, METER_TARGET_MINING));
    m_mining_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_MINING, browse_wnd);

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_rescenter_id, METER_INDUSTRY, METER_TARGET_INDUSTRY));
    m_industry_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_INDUSTRY, browse_wnd);

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_rescenter_id, METER_RESEARCH, METER_TARGET_RESEARCH));
    m_research_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_RESEARCH, browse_wnd);

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_rescenter_id, METER_TRADE, METER_TARGET_TRADE));
    m_trade_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_TRADE, browse_wnd);

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_rescenter_id, METER_CONSTRUCTION, METER_TARGET_CONSTRUCTION));
    m_multi_icon_value_indicator->SetToolTip(METER_CONSTRUCTION, browse_wnd);

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterModifiersIndicatorBrowseWnd(m_rescenter_id, METER_TARGET_POPULATION));
    m_pop_mod_stat->SetBrowseInfoWnd(browse_wnd);

    // focus droplist
    const std::vector<std::string>& available_foci = res->AvailableFoci();
    // refresh items in list
    m_focus_drop->Clear();
    for (std::vector<std::string>::const_iterator it = available_foci.begin(); it != available_foci.end(); ++it) {
        boost::shared_ptr<GG::Texture> texture = ClientUI::GetTexture(ClientUI::ArtDir() / res->FocusIcon(*it), true);
        GG::StaticGraphic* graphic = new GG::StaticGraphic(GG::X0, GG::Y0, MeterIconSize().x*3/2, MeterIconSize().y*3/2,
                                                           texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        GG::DropDownList::Row* row = new GG::DropDownList::Row(graphic->Width(), graphic->Height(), "FOCUS");
        row->push_back(dynamic_cast<GG::Control*>(graphic));
        m_focus_drop->Insert(row);
    }

    int drop_items = std::min(5, static_cast<int>(available_foci.size()));
    m_focus_drop->SetDropHeight(drop_items * MeterIconSize().y*3/2 + GG::Y(5));

    // set browse text and select appropriate focus in droplist
    std::string focus_text;
    if (!res->Focus().empty()) {
        for (unsigned int i = 0; i < available_foci.size(); ++i) {
            if (available_foci[i] == res->Focus()) {
                m_focus_drop->Select(i);
                focus_text = boost::io::str(FlexibleFormat(UserString("RP_FOCUS_TOOLTIP")) % UserString(res->Focus()));
            }
        }
    } else {
        m_focus_drop->Select(m_focus_drop->end());
    }
    m_focus_drop->SetBrowseText(focus_text);
}

void ResourcePanel::Refresh() {
    Update();
    DoExpandCollapseLayout();
}

void ResourcePanel::FocusDropListSelectionChanged(GG::DropDownList::iterator selected) {
    // all this funciton needs to do is emit FocusChangedSignal.  The code
    // preceeding that determines which focus was selected from the iterator 
    // parameter, does some safety checks, and disables UI sounds

    if (m_focus_drop->CurrentItem() == m_focus_drop->end()) {
        Logger().errorStream() << "ResourcePanel::FocusDropListSelectionChanged passed end / invalid interator";
        return;
    }

    const UniverseObject* obj = GetUniverseObject(m_rescenter_id);
    if (!obj)
        obj = GetEmpireKnownObject(m_rescenter_id, HumanClientApp::GetApp()->EmpireID());
    if (!obj) {
        Logger().errorStream() << "ResourcePanel::FocusDropListSelectionChanged couldn't get object with id " << m_rescenter_id;
        return;
    }
    const ResourceCenter* res = dynamic_cast<const ResourceCenter*>(obj);
    if (!res) {
        Logger().errorStream() << "ResourcePanel::FocusDropListSelectionChanged couldn't convert object with id " << m_rescenter_id << " to a ResourceCenter";
        return;
    }

    std::size_t i = m_focus_drop->IteratorToIndex(selected);
    if (i >= res->AvailableFoci().size()) {
        Logger().errorStream() << "ResourcePanel::FocusDropListSelectionChanged got invalid focus selected index: " << i;
        return;
    }

    Sound::TempUISoundDisabler sound_disabler;
    FocusChangedSignal(res->AvailableFoci().at(i));
}

void ResourcePanel::EnableOrderIssuing(bool enable/* = true*/) {
    const UniverseObject* obj = GetUniverseObject(m_rescenter_id);
    if (!enable || !obj || !obj->OwnedBy(HumanClientApp::GetApp()->EmpireID()))
        m_focus_drop->Disable();
    else
        m_focus_drop->Disable(false);
}


/////////////////////////////////////
//         MilitaryPanel           //
/////////////////////////////////////
std::map<int, bool> MilitaryPanel::s_expanded_map;

MilitaryPanel::MilitaryPanel(GG::X w, int planet_id) :
    Wnd(GG::X0, GG::Y0, w, GG::Y(ClientUI::Pts()*9), GG::INTERACTIVE),
    m_planet_id(planet_id),
    m_fleet_supply_stat(0),
    m_shield_stat(0),
    m_defense_stat(0),
    m_troops_stat(0),
    m_detection_stat(0),
    m_stealth_stat(0),
    m_multi_icon_value_indicator(0),
    m_multi_meter_status_bar(0),
    m_expand_button(0)
{
    SetName("MilitaryPanel");

    // expand / collapse button at top right    
    m_expand_button = new GG::Button(w - 16, GG::Y0, GG::X(16), GG::Y(16), "", ClientUI::GetFont(), GG::CLR_WHITE, GG::CLR_ZERO, GG::ONTOP | GG::INTERACTIVE);
    AttachChild(m_expand_button);
    m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrownormal.png"   ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowclicked.png"  ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowmouseover.png"), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    GG::Connect(m_expand_button->ClickedSignal, &MilitaryPanel::ExpandCollapseButtonPressed, this);

    // small meter indicators - for use when panel is collapsed
    m_fleet_supply_stat = new StatisticIcon(GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y, ClientUI::MeterIcon(METER_SUPPLY),
                                            0, 3, false);
    AttachChild(m_fleet_supply_stat);

    m_shield_stat = new StatisticIcon(GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y, ClientUI::MeterIcon(METER_SHIELD),
                                      0, 3, false);
    AttachChild(m_shield_stat);

    m_defense_stat = new StatisticIcon(GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y, ClientUI::MeterIcon(METER_DEFENSE),
                                       0, 3, false);
    AttachChild(m_defense_stat);

    m_troops_stat = new StatisticIcon(GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y, ClientUI::MeterIcon(METER_TROOPS),
                                       0, 3, false);
    AttachChild(m_troops_stat);

    m_detection_stat = new StatisticIcon(GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y, ClientUI::MeterIcon(METER_DETECTION),
                                         0, 3, false);
    AttachChild(m_detection_stat);

    m_stealth_stat = new StatisticIcon(GG::X0, GG::Y0, MeterIconSize().x, MeterIconSize().y, ClientUI::MeterIcon(METER_STEALTH),
                                       0, 3, false);
    AttachChild(m_stealth_stat);


    // meter and production indicators
    std::vector<std::pair<MeterType, MeterType> > meters;
    meters.push_back(std::make_pair(METER_SUPPLY, INVALID_METER_TYPE));
    meters.push_back(std::make_pair(METER_SHIELD, METER_MAX_SHIELD));
    meters.push_back(std::make_pair(METER_DEFENSE, METER_MAX_DEFENSE));
    meters.push_back(std::make_pair(METER_TROOPS, METER_MAX_TROOPS));
    meters.push_back(std::make_pair(METER_DETECTION, INVALID_METER_TYPE));
    meters.push_back(std::make_pair(METER_STEALTH, INVALID_METER_TYPE));


    m_multi_meter_status_bar =      new MultiMeterStatusBar(Width() - 2*EDGE_PAD,       m_planet_id, meters);
    m_multi_icon_value_indicator =  new MultiIconValueIndicator(Width() - 2*EDGE_PAD,   m_planet_id, meters);

    // determine if this panel has been created yet.
    std::map<int, bool>::iterator it = s_expanded_map.find(m_planet_id);
    if (it == s_expanded_map.end())
        s_expanded_map[m_planet_id] = false; // if not, default to collapsed state

    Refresh();
}

MilitaryPanel::~MilitaryPanel() {
    // manually delete all pointed-to controls that may or may not be attached as a child window at time of deletion
    delete m_fleet_supply_stat;
    delete m_shield_stat;
    delete m_defense_stat;
    delete m_troops_stat;
    delete m_detection_stat;
    delete m_stealth_stat;

    delete m_multi_icon_value_indicator;
    delete m_multi_meter_status_bar;

    // don't need to manually delete m_expand_button, as it is attached as a child so will be deleted by ~Wnd
}

void MilitaryPanel::ExpandCollapse(bool expanded) {
    if (expanded == s_expanded_map[m_planet_id]) return; // nothing to do
    s_expanded_map[m_planet_id] = expanded;

    DoExpandCollapseLayout();
}

void MilitaryPanel::Render() {
    if (Height() < 1) return;   // don't render if empty
    // Draw outline and background...

    // copied from CUIWnd
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // use GL to draw the lines
    glDisable(GL_TEXTURE_2D);
    GLint initial_modes[2];
    glGetIntegerv(GL_POLYGON_MODE, initial_modes);

    // draw background
    glPolygonMode(GL_BACK, GL_FILL);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndColor());
        glVertex(ul.x, ul.y);
        glVertex(lr.x, ul.y);
        glVertex(lr.x, lr.y);
        glVertex(ul.x, lr.y);
        glVertex(ul.x, ul.y);
    glEnd();

    // draw outer border on pixel inside of the outer edge of the window
    glPolygonMode(GL_BACK, GL_LINE);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndOuterBorderColor());
        glVertex(ul.x, ul.y);
        glVertex(lr.x, ul.y);
        glVertex(lr.x, lr.y);
        glVertex(ul.x, lr.y);
        glVertex(ul.x, ul.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    glEnable(GL_TEXTURE_2D);
}

void MilitaryPanel::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void MilitaryPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    GG::Wnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        DoExpandCollapseLayout();
}

void MilitaryPanel::Update() {
    const UniverseObject* obj = GetUniverseObject(m_planet_id);
    if (!obj)
        obj = GetEmpireKnownObject(m_planet_id, HumanClientApp::GetApp()->EmpireID());
    if (!obj) {
        Logger().errorStream() << "MilitaryPanel::Update coudln't get object with id  " << m_planet_id;
        return;
    }


    // meter bar displays and production stats
    m_multi_meter_status_bar->Update();
    m_multi_icon_value_indicator->Update();

    m_fleet_supply_stat->SetValue(obj->InitialMeterValue(METER_SUPPLY));
    m_shield_stat->SetValue(obj->InitialMeterValue(METER_SHIELD));
    m_defense_stat->SetValue(obj->InitialMeterValue(METER_DEFENSE));
    m_troops_stat->SetValue(obj->InitialMeterValue(METER_TROOPS));
    m_detection_stat->SetValue(obj->InitialMeterValue(METER_DETECTION));
    m_stealth_stat->SetValue(obj->InitialMeterValue(METER_STEALTH));

    // tooltips
    boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_planet_id, METER_SUPPLY));
    m_fleet_supply_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_SUPPLY, browse_wnd);

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_planet_id, METER_SHIELD, METER_MAX_SHIELD));
    m_shield_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_SHIELD, browse_wnd);

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_planet_id, METER_DEFENSE, METER_MAX_DEFENSE));
    m_defense_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_DEFENSE, browse_wnd);

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_planet_id, METER_TROOPS, METER_MAX_TROOPS));
    m_troops_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_TROOPS, browse_wnd);

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_planet_id, METER_DETECTION));
    m_detection_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_DETECTION, browse_wnd);

    browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(m_planet_id, METER_STEALTH));
    m_stealth_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_STEALTH, browse_wnd);
}

void MilitaryPanel::Refresh() {
    Update();
    DoExpandCollapseLayout();
}

void MilitaryPanel::ExpandCollapseButtonPressed()
{ ExpandCollapse(!s_expanded_map[m_planet_id]); }

void MilitaryPanel::DoExpandCollapseLayout() {
    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_planet_id]) {

        // detach / hide meter bars and large resource indicators
        DetachChild(m_multi_meter_status_bar);
        DetachChild(m_multi_icon_value_indicator);


        // determine which two resource icons to display while collapsed: the two with the highest production

        // sort by insereting into multimap keyed by production amount, then taking the first two icons therein
        std::vector<StatisticIcon*> meter_icons;
        meter_icons.push_back(m_fleet_supply_stat);
        meter_icons.push_back(m_shield_stat);
        meter_icons.push_back(m_defense_stat);
        meter_icons.push_back(m_troops_stat);
        meter_icons.push_back(m_detection_stat);
        meter_icons.push_back(m_stealth_stat);

        // initially detach all...
        for (std::vector<StatisticIcon*>::iterator it = meter_icons.begin(); it != meter_icons.end(); ++it)
            DetachChild(*it);

        // position and reattach icons to be shown
        int n = 0;
        for (std::vector<StatisticIcon*>::iterator it = meter_icons.begin(); it != meter_icons.end(); ++it) {
            GG::X x = MeterIconSize().x*n*7/2;

            if (x > Width() - m_expand_button->Width() - MeterIconSize().x*5/2) break;  // ensure icon doesn't extend past right edge of panel

            StatisticIcon* icon = *it;
            AttachChild(icon);
            icon->MoveTo(GG::Pt(x, GG::Y0));
            icon->Show();

            n++;
        }

        Resize(GG::Pt(Width(), std::max(MeterIconSize().y, m_expand_button->Height())));
    } else {
        // detach statistic icons
        DetachChild(m_fleet_supply_stat);   DetachChild(m_shield_stat);     DetachChild(m_defense_stat);
        DetachChild(m_troops_stat);         DetachChild(m_detection_stat);  DetachChild(m_stealth_stat);

        // attach and show meter bars and large resource indicators
        GG::Y top = UpperLeft().y;

        AttachChild(m_multi_icon_value_indicator);
        m_multi_icon_value_indicator->MoveTo(GG::Pt(GG::X(EDGE_PAD), GG::Y(EDGE_PAD)));
        m_multi_icon_value_indicator->Resize(GG::Pt(Width() - 2*EDGE_PAD, m_multi_icon_value_indicator->Height()));

        AttachChild(m_multi_meter_status_bar);
        m_multi_meter_status_bar->MoveTo(GG::Pt(GG::X(EDGE_PAD), m_multi_icon_value_indicator->LowerRight().y + EDGE_PAD - top));
        m_multi_meter_status_bar->Resize(GG::Pt(Width() - 2*EDGE_PAD, m_multi_meter_status_bar->Height()));

        MoveChildUp(m_expand_button);

        Resize(GG::Pt(Width(), m_multi_meter_status_bar->LowerRight().y + EDGE_PAD - top));
    }

    m_expand_button->MoveTo(GG::Pt(Width() - m_expand_button->Width(), GG::Y0));

    // update appearance of expand/collapse button
    if (s_expanded_map[m_planet_id]) {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrownormal.png"   ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrowclicked.png"  ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrowmouseover.png"), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    } else {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrownormal.png"   ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowclicked.png"  ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowmouseover.png"), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    }

    ExpandCollapseSignal();
}

void MilitaryPanel::EnableOrderIssuing(bool enable/* = true*/)
{}


/////////////////////////////////////
//    MultiIconValueIndicator      //
/////////////////////////////////////
MultiIconValueIndicator::MultiIconValueIndicator(GG::X w, int object_id,
                                                 const std::vector<std::pair<MeterType, MeterType> >& meter_types) :
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y1, GG::INTERACTIVE),
    m_icons(),
    m_meter_types(meter_types),
    m_object_ids()
{
    m_object_ids.push_back(object_id);
    Init();
}

MultiIconValueIndicator::MultiIconValueIndicator(GG::X w, const std::vector<int>& object_ids,
                                                 const std::vector<std::pair<MeterType, MeterType> >& meter_types) :
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y1, GG::INTERACTIVE),
    m_icons(),
    m_meter_types(meter_types),
    m_object_ids(object_ids)
{ Init(); }

MultiIconValueIndicator::MultiIconValueIndicator(GG::X w) :
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y1, GG::INTERACTIVE),
    m_icons(),
    m_meter_types(),
    m_object_ids()
{ Init(); }

MultiIconValueIndicator::~MultiIconValueIndicator()
{}  // nothing needs deleting, as all contained indicators are childs and auto deleted

void MultiIconValueIndicator::Init() {
    SetName("MultiIconValueIndicator");

    GG::X x(EDGE_PAD);
    for (std::vector<std::pair<MeterType, MeterType> >::const_iterator it = m_meter_types.begin(); it != m_meter_types.end(); ++it) {
        const MeterType PRIMARY_METER_TYPE = it->first;
        // get icon texture.
        boost::shared_ptr<GG::Texture> texture = ClientUI::MeterIcon(PRIMARY_METER_TYPE);

        // special case for population meter for an indicator showing only a
        // single popcenter: icon is species icon, rather than generic pop icon
        if (PRIMARY_METER_TYPE == METER_POPULATION && m_object_ids.size() == 1) {
            if (const UniverseObject* obj = GetUniverseObject(*m_object_ids.begin()))
                if (const PopCenter* pc = dynamic_cast<const PopCenter*>(obj))
                    texture = ClientUI::SpeciesIcon(pc->SpeciesName());
        }

        m_icons.push_back(new StatisticIcon(x, GG::Y(EDGE_PAD), MULTI_INDICATOR_ICON_WIDTH,
                                            MULTI_INDICATOR_ICON_HEIGHT + ClientUI::Pts()*3/2,
                                            texture, 0.0, 3, false));
        AttachChild(m_icons.back());
        x += MULTI_INDICATOR_ICON_WIDTH + MULTI_INDICATOR_ICON_SPACING;
    }
    if (!m_icons.empty())
        Resize(GG::Pt(Width(), EDGE_PAD + MULTI_INDICATOR_ICON_HEIGHT + ClientUI::Pts()*3/2));
    Update();
}

bool MultiIconValueIndicator::Empty()
{ return m_object_ids.empty(); }

void MultiIconValueIndicator::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // outline of whole control
    GG::FlatRectangle(ul, lr, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);
}

void MultiIconValueIndicator::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void MultiIconValueIndicator::Update() {
    if (m_icons.size() != m_meter_types.size()) {
        Logger().errorStream() << "MultiIconValueIndicator::Update has inconsitent numbers of icons and meter types";
        return;
    }

    for (std::size_t i = 0; i < m_icons.size(); ++i) {
        assert(m_icons[i]);
        double sum = 0.0;
        for (std::size_t j = 0; j < m_object_ids.size(); ++j) {
            int object_id = m_object_ids[j];
            const UniverseObject* obj = GetUniverseObject(object_id);
            if (!obj)
                obj = GetEmpireKnownObject(object_id, HumanClientApp::GetApp()->EmpireID());
            if (!obj) {
                Logger().errorStream() << "MultiIconValueIndicator::Update couldn't get object with id " << object_id;
                continue;
            }
            //Logger().debugStream() << "MultiIconValueIndicator::Update object:";
            //Logger().debugStream() << obj->Dump();
            sum += obj->InitialMeterValue(m_meter_types[i].first);
        }
        m_icons[i]->SetValue(sum);
    }
}

void MultiIconValueIndicator::SetToolTip(MeterType meter_type, const boost::shared_ptr<GG::BrowseInfoWnd>& browse_wnd) {
    for (unsigned int i = 0; i < m_icons.size(); ++i)
        if (m_meter_types.at(i).first == meter_type)
            m_icons.at(i)->SetBrowseInfoWnd(browse_wnd);
}

void MultiIconValueIndicator::ClearToolTip(MeterType meter_type) {
    for (unsigned int i = 0; i < m_icons.size(); ++i)
        if (m_meter_types.at(i).first == meter_type)
            m_icons.at(i)->ClearBrowseInfoWnd();
}

/////////////////////////////////////
//       MultiMeterStatusBar       //
/////////////////////////////////////
MultiMeterStatusBar::MultiMeterStatusBar(GG::X w, int object_id, const std::vector<std::pair<MeterType, MeterType> >& meter_types) :
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y1, GG::INTERACTIVE),
    m_bar_shading_texture(ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "meter_bar_shading.png")),
    m_meter_types(meter_types),
    m_initial_values(),
    m_projected_values(),
    m_target_max_values(),
    m_object_id(object_id),
    m_bar_colours()
{
    SetName("MultiMeterStatusBar");
    Update();
}

void MultiMeterStatusBar::Render() {
    GG::Clr DARY_GREY = GG::Clr(44, 44, 44, 255);
    GG::Clr HALF_GREY = GG::Clr(128, 128, 128, 128);

    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // outline of whole control
    GG::FlatRectangle(ul, lr, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);

    const GG::X BAR_LEFT = ClientUpperLeft().x + EDGE_PAD - 1;
    const GG::X BAR_RIGHT = ClientLowerRight().x - EDGE_PAD + 1;
    const GG::X BAR_MAX_LENGTH = BAR_RIGHT - BAR_LEFT;
    const GG::Y TOP = ClientUpperLeft().y + EDGE_PAD - 1;
    GG::Y y = TOP;

    for (unsigned int i = 0; i < m_initial_values.size(); ++i) {
        // bar grey backgrounds
        GG::FlatRectangle(GG::Pt(BAR_LEFT, y), GG::Pt(BAR_RIGHT, y + BAR_HEIGHT), DARY_GREY, DARY_GREY, 0);

        y += BAR_HEIGHT + BAR_PAD;
    }


    // lines for 20, 40, 60, 80
    glDisable(GL_TEXTURE_2D);
    glColor(HALF_GREY);
    glBegin(GL_LINES);
        glVertex(BAR_LEFT +   BAR_MAX_LENGTH/5, TOP);
        glVertex(BAR_LEFT +   BAR_MAX_LENGTH/5, y - BAR_PAD);
        glVertex(BAR_LEFT + 2*BAR_MAX_LENGTH/5, TOP);
        glVertex(BAR_LEFT + 2*BAR_MAX_LENGTH/5, y - BAR_PAD);
        glVertex(BAR_LEFT + 3*BAR_MAX_LENGTH/5, TOP);
        glVertex(BAR_LEFT + 3*BAR_MAX_LENGTH/5, y - BAR_PAD);
        glVertex(BAR_LEFT + 4*BAR_MAX_LENGTH/5, TOP);
        glVertex(BAR_LEFT + 4*BAR_MAX_LENGTH/5, y - BAR_PAD);
    glEnd();
    glEnable(GL_TEXTURE_2D);


    // current, initial, and target/max horizontal bars for each pair of MeterType
    y = TOP;
    for (unsigned int i = 0; i < m_initial_values.size(); ++i) {
        GG::Clr clr;

        const GG::Y BAR_TOP = y;
        const GG::Y BAR_BOTTOM = BAR_TOP + BAR_HEIGHT;

        const bool SHOW_INITIAL = (m_initial_values[i] != Meter::INVALID_VALUE);
        const bool SHOW_PROJECTED = (m_projected_values[i] != Meter::INVALID_VALUE);
        const bool SHOW_TARGET_MAX = (m_target_max_values[i] != Meter::INVALID_VALUE);


        const GG::X INITIAL_RIGHT(BAR_LEFT + BAR_MAX_LENGTH * m_initial_values[i] / MULTI_METER_STATUS_BAR_DISPLAYED_METER_RANGE);
        const GG::Y INITIAL_TOP(BAR_TOP);
        if (SHOW_INITIAL) {
            // initial value
            const GG::X INITIAL_RIGHT(BAR_LEFT + BAR_MAX_LENGTH * m_initial_values[i] / MULTI_METER_STATUS_BAR_DISPLAYED_METER_RANGE);
            const GG::Y INITIAL_TOP(BAR_TOP);
            glColor(m_bar_colours[i]);
            m_bar_shading_texture->OrthoBlit(GG::Pt(BAR_LEFT, INITIAL_TOP), GG::Pt(INITIAL_RIGHT, BAR_BOTTOM));
            // black border
            GG::FlatRectangle(GG::Pt(BAR_LEFT, INITIAL_TOP), GG::Pt(INITIAL_RIGHT, BAR_BOTTOM), GG::CLR_ZERO, GG::CLR_BLACK, 1);
        }

        const GG::X PROJECTED_RIGHT(BAR_LEFT + BAR_MAX_LENGTH * m_projected_values[i] / MULTI_METER_STATUS_BAR_DISPLAYED_METER_RANGE);
        const GG::Y PROJECTED_TOP(INITIAL_TOP);
        if (SHOW_PROJECTED) {
            // projected colour bar with black border
            if (PROJECTED_RIGHT > INITIAL_RIGHT) {
                GG::FlatRectangle(GG::Pt(INITIAL_RIGHT - 1, PROJECTED_TOP), GG::Pt(PROJECTED_RIGHT, BAR_BOTTOM), ClientUI::StatIncrColor(), GG::CLR_BLACK, 1);
            } else if (PROJECTED_RIGHT < INITIAL_RIGHT) {
                GG::FlatRectangle(GG::Pt(PROJECTED_RIGHT - 1, PROJECTED_TOP), GG::Pt(INITIAL_RIGHT, BAR_BOTTOM), ClientUI::StatDecrColor(), GG::CLR_BLACK, 1);
            }
        }

        const GG::X TARGET_MAX_RIGHT(BAR_LEFT + BAR_MAX_LENGTH * m_target_max_values[i] / MULTI_METER_STATUS_BAR_DISPLAYED_METER_RANGE);
        if (SHOW_TARGET_MAX && TARGET_MAX_RIGHT > BAR_LEFT) {
            // max / target value
            //glColor(DarkColor(m_bar_colours[i]));
            //m_bar_shading_texture->OrthoBlit(GG::Pt(BAR_LEFT, BAR_TOP), GG::Pt(TARGET_MAX_RIGHT, BAR_BOTTOM));
            // black border
            GG::FlatRectangle(GG::Pt(BAR_LEFT, BAR_TOP), GG::Pt(TARGET_MAX_RIGHT, BAR_BOTTOM), GG::CLR_ZERO, m_bar_colours[i], 1);
        }

        // move down position of next bar, if any
        y += BAR_HEIGHT + BAR_PAD;
    }
}

void MultiMeterStatusBar::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void MultiMeterStatusBar::Update() {
    m_initial_values.clear();   // initial value of .first MeterTypes at the start of this turn
    m_projected_values.clear(); // projected current value of .first MeterTypes for the start of next turn
    m_target_max_values.clear();// current values of the .second MeterTypes in m_meter_types

    const UniverseObject* obj = GetUniverseObject(m_object_id);
    if (!obj)
        obj = GetEmpireKnownObject(m_object_id, HumanClientApp::GetApp()->EmpireID());
    if (!obj) {
        Logger().errorStream() << "MultiMeterStatusBar couldn't get object with id " << m_object_id;
        return;
    }

    int num_bars = 0;   // count number of valid bars' data added

    for (std::vector<std::pair<MeterType, MeterType> >::const_iterator meter_pairs_it = m_meter_types.begin(); meter_pairs_it != m_meter_types.end(); ++meter_pairs_it) {
        const std::pair<MeterType, MeterType>& meter_types_pair = *meter_pairs_it;
        const Meter* actual_meter = obj->GetMeter(meter_types_pair.first);
        const Meter* target_max_meter = obj->GetMeter(meter_types_pair.second);

        if (actual_meter || target_max_meter) {
            ++num_bars;
            if (actual_meter) {
                m_initial_values.push_back(actual_meter->Initial());
                if (target_max_meter)
                    m_projected_values.push_back(obj->NextTurnCurrentMeterValue(meter_types_pair.first));
                else
                    m_projected_values.push_back(actual_meter->Initial());
            } else {
                m_initial_values.push_back(Meter::INVALID_VALUE);
                m_projected_values.push_back(Meter::INVALID_VALUE);
            }
            if (target_max_meter) {
                m_target_max_values.push_back(target_max_meter->Current());
            } else {
                m_target_max_values.push_back(Meter::INVALID_VALUE);
            }
            m_bar_colours.push_back(MeterColor(meter_types_pair.first));
        }
    }

    // calculate height from number of bars to be shown
    const GG::Y HEIGHT = num_bars*BAR_HEIGHT + (num_bars - 1)*BAR_PAD + 2*EDGE_PAD - 2;
    Resize(GG::Pt(Width(), HEIGHT));
}


/////////////////////////////////////
//     MeterModifiersIndicator     //
/////////////////////////////////////
MeterModifiersIndicator::MeterModifiersIndicator(GG::X x, GG::Y y, GG::X w, GG::Y h,
                                                 int source_object_id, MeterType meter_type) :
    StatisticIcon(x, y, w, h, ClientUI::MeterIcon(meter_type), 0.0, 3, false),
    m_source_object_id(source_object_id),
    m_meter_type(meter_type),
    m_empire_meter_type("")
{}

MeterModifiersIndicator::MeterModifiersIndicator(GG::X x, GG::Y y, GG::X w, GG::Y h,
                                                 int source_object_id, const std::string& empire_meter_type) :
    StatisticIcon(x, y, w, h, ClientUI::GetTexture(""), 0.0, 3, false),
    m_source_object_id(source_object_id),
    m_meter_type(INVALID_METER_TYPE),
    m_empire_meter_type(empire_meter_type)
{}

void MeterModifiersIndicator::Update() {
    double modifications_sum = 0.0;

    if (m_source_object_id == INVALID_OBJECT_ID) {
        this->SetValue(0.0);
        return;
    }

    const Universe& universe = GetUniverse();
    const ObjectMap& objects = universe.Objects();
    const Effect::AccountingMap& effect_accounting_map = universe.GetEffectAccountingMap();

    // for every object that has the appropriate meter type, get its affect accounting info
    for (ObjectMap::const_iterator obj_it = objects.const_begin();
         obj_it != objects.const_end(); ++obj_it)
    {
        int target_object_id = obj_it->first;
        const UniverseObject* obj = obj_it->second;
        // does object have relevant meter?
        const Meter* meter = obj->GetMeter(m_meter_type);
        if (!meter)
            continue;

        // is any effect accounting available for target object?
        Effect::AccountingMap::const_iterator map_it = effect_accounting_map.find(target_object_id);
        if (map_it == effect_accounting_map.end())
            continue;
        const std::map<MeterType, std::vector<Effect::AccountingInfo> >& meter_map = map_it->second;

        // is any effect accounting available for this indicator's meter type?
        std::map<MeterType, std::vector<Effect::AccountingInfo> >::const_iterator meter_it =
            meter_map.find(m_meter_type);
        if (meter_it == meter_map.end())
            continue;
        const std::vector<Effect::AccountingInfo>& accounts = meter_it->second;

        // does the target object's effect accounting have any modifications
        // by this indicator's source object?  (may be more than one)
        for (std::vector<Effect::AccountingInfo>::const_iterator account_it = accounts.begin();
             account_it != accounts.end(); ++account_it)
        {
            if (account_it->source_id != m_source_object_id)
                continue;
            modifications_sum += account_it->meter_change;
        }
    }
    this->SetValue(modifications_sum);
}


/////////////////////////////////////
//         BuildingsPanel          //
/////////////////////////////////////
std::map<int, bool> BuildingsPanel::s_expanded_map = std::map<int, bool>();

BuildingsPanel::BuildingsPanel(GG::X w, int columns, int planet_id) :
    GG::Wnd(GG::X0, GG::Y0, w, GG::Y(Value(w)), GG::INTERACTIVE),
    m_planet_id(planet_id),
    m_columns(columns),
    m_building_indicators(),
    m_expand_button(0)
{
    SetName("BuildingsPanel");

    if (m_columns < 1) throw std::invalid_argument("Attempted to create a BuidingsPanel with less than 1 column");

    // expand / collapse button at top right    
    m_expand_button = new GG::Button(w - 16, GG::Y0, GG::X(16), GG::Y(16), "", ClientUI::GetFont(), GG::CLR_WHITE);
    AttachChild(m_expand_button);
    m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrownormal.png"   ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowclicked.png"  ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowmouseover.png"), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    GG::Connect(m_expand_button->ClickedSignal, &BuildingsPanel::ExpandCollapseButtonPressed, this);

    // get owner, connect its production queue changed signal to update this panel
    const UniverseObject* planet = GetUniverseObject(m_planet_id);
    if (!planet)
        planet = GetEmpireKnownObject(m_planet_id, HumanClientApp::GetApp()->EmpireID());
    if (planet) {
        if (const Empire* empire = Empires().Lookup(planet->Owner())) {
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
    delete m_expand_button;
}

void BuildingsPanel::ExpandCollapse(bool expanded) {
    if (expanded == s_expanded_map[m_planet_id]) return; // nothing to do
    s_expanded_map[m_planet_id] = expanded;

    DoExpandCollapseLayout();
}

void BuildingsPanel::Render() {
    if (Height() < 1) return;   // don't render if empty
    // Draw outline and background...

    // copied from CUIWnd
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // use GL to draw the lines
    glDisable(GL_TEXTURE_2D);
    GLint initial_modes[2];
    glGetIntegerv(GL_POLYGON_MODE, initial_modes);

    // draw background
    glPolygonMode(GL_BACK, GL_FILL);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndColor());
        glVertex(ul.x, ul.y);
        glVertex(lr.x, ul.y);
        glVertex(lr.x, lr.y);
        glVertex(ul.x, lr.y);
        glVertex(ul.x, ul.y);
    glEnd();

    // draw outer border on pixel inside of the outer edge of the window
    glPolygonMode(GL_BACK, GL_LINE);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndOuterBorderColor());
        glVertex(ul.x, ul.y);
        glVertex(lr.x, ul.y);
        glVertex(lr.x, lr.y);
        glVertex(ul.x, lr.y);
        glVertex(ul.x, ul.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    glEnable(GL_TEXTURE_2D);
}

void BuildingsPanel::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void BuildingsPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    GG::Wnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        DoExpandCollapseLayout();
}

void BuildingsPanel::Update() {
    //std::cout << "BuildingsPanel::Update" << std::endl;

    // remove old indicators
    for (std::vector<BuildingIndicator*>::iterator it = m_building_indicators.begin(); it != m_building_indicators.end(); ++it) {
        DetachChild(*it);
        delete (*it);
    }
    m_building_indicators.clear();

    const Planet* plt = GetPlanet(m_planet_id);
    if (!plt)
        plt = GetEmpireKnownPlanet(m_planet_id, HumanClientApp::GetApp()->EmpireID());
    if (!plt) {
        Logger().errorStream() << "BuildingsPanel::Update couldn't get planet with id " << m_planet_id;
        return;
    }
    const std::set<int>& buildings = plt->Buildings();

    const int indicator_size = static_cast<int>(Value(Width() * 1.0 / m_columns));

    // get existing / finished buildings and use them to create building indicators
    for (std::set<int>::const_iterator it = buildings.begin(); it != buildings.end(); ++it) {
        int object_id = *it;
        const Building* building = GetBuilding(object_id);
        if (!building)
            building = GetEmpireKnownBuilding(object_id, HumanClientApp::GetApp()->EmpireID());
        if (!building) {
            Logger().errorStream() << "BuildingsPanel::Update couldn't get building with id: " << object_id << " on planet " << plt->Name();
            continue;
        }
        BuildingIndicator* ind = new BuildingIndicator(GG::X(indicator_size), object_id);
        m_building_indicators.push_back(ind);
    }

    // get in-progress buildings
    const Empire* empire = Empires().Lookup(plt->Owner());
    if (!empire)
        return;

    const ProductionQueue& queue = empire->GetProductionQueue();

    int queue_index = 0;
    for (ProductionQueue::const_iterator queue_it = queue.begin(); queue_it != queue.end(); ++queue_it, ++queue_index) {
        const ProductionQueue::Element elem = *queue_it;

        BuildType type = elem.item.build_type;
        if (type != BT_BUILDING) continue;  // don't show in-progress ships in BuildingsPanel...
        int location = elem.location;
        if (location != plt->ID()) continue;    // don't show buildings located elsewhere

        double total_cost;
        int total_turns;
        boost::tie(total_cost, total_turns) = empire->ProductionCostAndTime(type, elem.item.name);

        double progress = std::max(0.0, empire->ProductionStatus(queue_index));
        double turns_completed = std::min<double>(total_turns, progress / total_cost);

        BuildingIndicator* ind = new BuildingIndicator(GG::X(indicator_size), elem.item.name, turns_completed);
        m_building_indicators.push_back(ind);
    }
}

void BuildingsPanel::Refresh() {
    //std::cout << "BuildingsPanel::Refresh" << std::endl;
    Update();
    DoExpandCollapseLayout();
}

void BuildingsPanel::ExpandCollapseButtonPressed()
{ ExpandCollapse(!s_expanded_map[m_planet_id]); }

void BuildingsPanel::DoExpandCollapseLayout() {
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

    m_expand_button->MoveTo(GG::Pt(Width() - m_expand_button->Width(), GG::Y0));

    // update appearance of expand/collapse button
    if (s_expanded_map[m_planet_id]) {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrownormal.png"   ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrowclicked.png"  ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrowmouseover.png"), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    } else {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrownormal.png"   ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowclicked.png"  ), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowmouseover.png"), GG::X0, GG::Y0, GG::X(32), GG::Y(32)));
    }
    Wnd::MoveChildUp(m_expand_button);

    ExpandCollapseSignal();
}

void BuildingsPanel::EnableOrderIssuing(bool enable/* = true*/) {
    for (std::vector<BuildingIndicator*>::iterator it = m_building_indicators.begin();
         it != m_building_indicators.end();
         ++it)
    {
        (*it)->EnableOrderIssuing(enable);
    }
}


/////////////////////////////////////
//       BuildingIndicator         //
/////////////////////////////////////
BuildingIndicator::BuildingIndicator(GG::X w, int building_id) :
    Wnd(GG::X0, GG::Y0, w, GG::Y(Value(w)), GG::INTERACTIVE),
    m_graphic(0),
    m_scrap_indicator(0),
    m_progress_bar(0),
    m_building_id(building_id),
    m_order_issuing_enabled(true)
{
    SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));

    if (const Building* building = GetBuilding(m_building_id))
        GG::Connect(building->StateChangedSignal,   &BuildingIndicator::Refresh,     this);

    Refresh();
}

BuildingIndicator::BuildingIndicator(GG::X w, const std::string& building_type, double turns_completed) :
    Wnd(GG::X0, GG::Y0, w, GG::Y(Value(w)), GG::INTERACTIVE),
    m_graphic(0),
    m_scrap_indicator(0),
    m_progress_bar(0),
    m_building_id(INVALID_OBJECT_ID),
    m_order_issuing_enabled(true)
{
    boost::shared_ptr<GG::Texture> texture = ClientUI::BuildingIcon(building_type);

    const BuildingType* type = GetBuildingType(building_type);
    const std::string& desc = type ? type->Description() : "";

    SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
        new IconTextBrowseWnd(texture, UserString(building_type), UserString(desc))));

    m_graphic = new GG::StaticGraphic(GG::X0, GG::Y0, w, GG::Y(Value(w)), texture,
                                      GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    AttachChild(m_graphic);

    int total_turns = type ? type->ProductionTime() : 1;
    m_progress_bar = new MultiTurnProgressBar(w, GG::Y(Value(w/5)), total_turns, turns_completed,
                                              GG::CLR_GRAY, GG::CLR_BLACK, GG::CLR_WHITE);
    m_progress_bar->MoveTo(GG::Pt(GG::X0, Height() - m_progress_bar->Height()));
    AttachChild(m_progress_bar);
}

void BuildingIndicator::Render() {
    // Draw outline and background...

    // copied from CUIWnd
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // use GL to draw the lines
    glDisable(GL_TEXTURE_2D);
    GLint initial_modes[2];
    glGetIntegerv(GL_POLYGON_MODE, initial_modes);

    // draw background
    glPolygonMode(GL_BACK, GL_FILL);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndColor());
        glVertex(ul.x, ul.y);
        glVertex(lr.x, ul.y);
        glVertex(lr.x, lr.y);
        glVertex(ul.x, lr.y);
        glVertex(ul.x, ul.y);
    glEnd();

    // draw outer border on pixel inside of the outer edge of the window
    glPolygonMode(GL_BACK, GL_LINE);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndOuterBorderColor());
        glVertex(ul.x, ul.y);
        glVertex(lr.x, ul.y);
        glVertex(lr.x, lr.y);
        glVertex(ul.x, lr.y);
        glVertex(ul.x, ul.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    glEnable(GL_TEXTURE_2D);
}

void BuildingIndicator::Refresh() {
    ClearBrowseInfoWnd();

    const Building* building = GetBuilding(m_building_id);
    if (!building)
        return;

    if (m_graphic) {
        delete m_graphic;
        m_graphic = 0;
    }
    if (m_scrap_indicator) {
        delete m_scrap_indicator;
        m_scrap_indicator = 0;
    }

    const BuildingType* type = building->GetBuildingType();
    if (!type)
        return;

    boost::shared_ptr<GG::Texture> texture = ClientUI::BuildingIcon(type->Name());
    m_graphic = new GG::StaticGraphic(GG::X0, GG::Y0, Width(), GG::Y(Value(Width())), texture,
                                      GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    AttachChild(m_graphic);

    std::string desc = UserString(type->Description());
    if (GetOptionsDB().Get<bool>("UI.autogenerated-effects-descriptions") && !type->Effects().empty())
        desc += boost::io::str(FlexibleFormat(UserString("ENC_EFFECTS_STR")) % EffectsDescription(type->Effects()));

    SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(new IconTextBrowseWnd(texture, UserString(type->Name()), desc)));

    if (building->OrderedScrapped()) {
        boost::shared_ptr<GG::Texture> scrap_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "scrapped.png", true);
        m_scrap_indicator = new GG::StaticGraphic(GG::X0, GG::Y0, Width(), GG::Y(Value(Width())), scrap_texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        AttachChild(m_scrap_indicator);
    }
}

void BuildingIndicator::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    Wnd::SizeMove(ul, lr);

    GG::Pt child_lr = lr - ul - GG::Pt(GG::X1, GG::Y1);   // extra pixel prevents graphic from overflowing border box

    if (m_graphic)
        m_graphic->SizeMove(GG::Pt(GG::X0, GG::Y0), child_lr);

    if (m_scrap_indicator)
        m_scrap_indicator->SizeMove(GG::Pt(GG::X0, GG::Y0), child_lr);

    GG::Y bar_top = Height() * 4 / 5;
    if (m_progress_bar)
        m_progress_bar->SizeMove(GG::Pt(GG::X0, bar_top), child_lr);
}

void BuildingIndicator::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void BuildingIndicator::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    // verify that this indicator represents an existing building, and not a
    // queued production item, and that the owner of the building is this
    // client's player's empire
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    Building* building = GetBuilding(m_building_id);
    if (!building || !building->OwnedBy(empire_id) || !m_order_issuing_enabled) {
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

    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor());
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


/////////////////////////////////////
//         SpecialsPanel           //
/////////////////////////////////////
SpecialsPanel::SpecialsPanel(GG::X w, int object_id) : 
    Wnd(GG::X0, GG::Y0, w, GG::Y(32), GG::INTERACTIVE),
    m_object_id(object_id),
    m_icons()
{
    SetName("SpecialsPanel");
    Update();
}

bool SpecialsPanel::InWindow(const GG::Pt& pt) const {
    bool retval = false;
    for (std::vector<GG::StaticGraphic*>::const_iterator it = m_icons.begin(); it != m_icons.end(); ++it) {
        if ((*it)->InWindow(pt))
            retval = true;
    }
    return retval;
}

void SpecialsPanel::Render()
{}

void SpecialsPanel::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void SpecialsPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    GG::Wnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        Update();
}

void SpecialsPanel::Update() {
    //std::cout << "SpecialsPanel::Update" << std::endl;
    for (std::vector<GG::StaticGraphic*>::iterator it = m_icons.begin(); it != m_icons.end(); ++it)
        DeleteChild(*it);
    m_icons.clear();


    // get specials to display
    const UniverseObject* obj = GetUniverseObject(m_object_id);
    if (!obj)
        obj = GetEmpireKnownObject(m_object_id, HumanClientApp::GetApp()->EmpireID());
    if (!obj) {
        Logger().errorStream() << "SpecialsPanel::Update couldn't get object with id " << m_object_id;
        return;
    }
    const std::map<std::string, int>& specials = obj->Specials();


    // get specials and use them to create specials icons
    for (std::map<std::string, int>::const_iterator it = specials.begin(); it != specials.end(); ++it) {
        const Special* special = GetSpecial(it->first);
        GG::StaticGraphic* graphic = new GG::StaticGraphic(GG::X0, GG::Y0, SPECIAL_ICON_WIDTH, SPECIAL_ICON_HEIGHT, ClientUI::SpecialIcon(special->Name()),
                                                           GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE, GG::INTERACTIVE);
        graphic->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));

        std::string desc = UserString(special->Description());
        if (GetOptionsDB().Get<bool>("UI.autogenerated-effects-descriptions") && !special->Effects().empty()) {
            desc += boost::io::str(FlexibleFormat(UserString("ENC_EFFECTS_STR")) % EffectsDescription(special->Effects()));
        }
        graphic->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
            new IconTextBrowseWnd(ClientUI::SpecialIcon(special->Name()),
                                  UserString(special->Name()),
                                  desc)));
        m_icons.push_back(graphic);
    }

    const GG::X AVAILABLE_WIDTH = Width() - EDGE_PAD;
    GG::X x(EDGE_PAD);
    GG::Y y(EDGE_PAD);

    for (std::vector<GG::StaticGraphic*>::iterator it = m_icons.begin(); it != m_icons.end(); ++it) {
        GG::StaticGraphic* icon = *it;
        icon->MoveTo(GG::Pt(x, y));
        AttachChild(icon);

        x += SPECIAL_ICON_WIDTH + EDGE_PAD;

        if (x + SPECIAL_ICON_WIDTH + EDGE_PAD > AVAILABLE_WIDTH) {
            x = GG::X(EDGE_PAD);
            y += SPECIAL_ICON_HEIGHT + EDGE_PAD;
        }
    }

    if (m_icons.empty()) {
        Resize(GG::Pt(Width(), GG::Y0));
    } else {
        Resize(GG::Pt(Width(), y + SPECIAL_ICON_HEIGHT + EDGE_PAD*2));
    }
}

void SpecialsPanel::EnableOrderIssuing(bool enable/* = true*/)
{}


/////////////////////////////////////
//        ShipDesignPanel          //
/////////////////////////////////////
ShipDesignPanel::ShipDesignPanel(GG::X w, GG::Y h, int design_id) :
    GG::Control(GG::X0, GG::Y0, w, h, GG::Flags<GG::WndFlag>()),
    m_design_id(design_id),
    m_graphic(0),
    m_name(0)
{
    if (const ShipDesign* design = GetShipDesign(m_design_id)) {
        m_graphic = new GG::StaticGraphic(GG::X0, GG::Y0, w, h, ClientUI::ShipDesignIcon(design_id), GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC);
        AttachChild(m_graphic);
        m_name = new GG::TextControl(GG::X0, GG::Y0, design->Name(), ClientUI::GetFont(), GG::CLR_WHITE);
        AttachChild(m_name);
    }
}

void ShipDesignPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Control::SizeMove(ul, lr);
    if (m_graphic)
        m_graphic->Resize(Size());
    if (m_name)
        m_name->Resize(GG::Pt(Width(), m_name->Height()));
}

void ShipDesignPanel::Render()
{}

void ShipDesignPanel::Update()
{}


/////////////////////////////////////
//       IconTextBrowseWnd         //
/////////////////////////////////////
IconTextBrowseWnd::IconTextBrowseWnd(const boost::shared_ptr<GG::Texture> texture, const std::string& title_text, const std::string& main_text) :
    GG::BrowseInfoWnd(GG::X0, GG::Y0, ICON_BROWSE_TEXT_WIDTH + ICON_BROWSE_ICON_WIDTH, GG::Y1)
{
    m_icon = new GG::StaticGraphic(GG::X0, GG::Y0, ICON_BROWSE_ICON_WIDTH, ICON_BROWSE_ICON_HEIGHT, texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE, GG::INTERACTIVE);
    AttachChild(m_icon);

    const boost::shared_ptr<GG::Font>& font = ClientUI::GetFont();
    const boost::shared_ptr<GG::Font>& font_bold = ClientUI::GetBoldFont();
    const GG::Y ROW_HEIGHT(IconTextBrowseWndRowHeight());

    m_title_text = new GG::TextControl(m_icon->Width() + GG::X(EDGE_PAD), GG::Y0, ICON_BROWSE_TEXT_WIDTH, ROW_HEIGHT, title_text,
                                       font_bold, ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    AttachChild(m_title_text);

    m_main_text = new GG::TextControl(m_icon->Width() + GG::X(EDGE_PAD), ROW_HEIGHT, ICON_BROWSE_TEXT_WIDTH, ICON_BROWSE_ICON_HEIGHT, main_text,
                                      font, ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_TOP | GG::FORMAT_WORDBREAK);
    AttachChild(m_main_text);

    m_main_text->SetMinSize(true);
    m_main_text->Resize(m_main_text->MinSize());
    Resize(GG::Pt(ICON_BROWSE_TEXT_WIDTH + ICON_BROWSE_ICON_WIDTH, std::max(m_icon->Height(), ROW_HEIGHT + m_main_text->Height())));
}

bool IconTextBrowseWnd::WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const {
    assert(mode <= wnd->BrowseModes().size());
    return true;
}

void IconTextBrowseWnd::Render() {
    GG::Pt      ul = UpperLeft();
    GG::Pt      lr = LowerRight();
    const GG::Y ROW_HEIGHT(IconTextBrowseWndRowHeight());
    GG::FlatRectangle(ul, lr, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);    // main background
    GG::FlatRectangle(GG::Pt(ul.x + ICON_BROWSE_ICON_WIDTH, ul.y), GG::Pt(lr.x, ul.y + ROW_HEIGHT), ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);    // top title filled background
}


//////////////////////////////////////
//  SystemResourceSummaryBrowseWnd  //
//////////////////////////////////////
SystemResourceSummaryBrowseWnd::SystemResourceSummaryBrowseWnd(ResourceType resource_type, int system_id, int empire_id) :
    GG::BrowseInfoWnd(GG::X0, GG::Y0, LABEL_WIDTH + VALUE_WIDTH, GG::Y1),
    m_resource_type(resource_type),
    m_system_id(system_id),
    m_empire_id(empire_id),
    m_production_label(0), m_allocation_label(0), m_import_export_label(0),
    row_height(1), production_label_top(0), allocation_label_top(0), import_export_label_top(0)
{}

bool SystemResourceSummaryBrowseWnd::WndHasBrowseInfo(const GG::Wnd* wnd, std::size_t mode) const {
    assert(mode <= wnd->BrowseModes().size());
    return true;
}

void SystemResourceSummaryBrowseWnd::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::FlatRectangle(ul, lr, OpaqueColor(ClientUI::WndColor()), ClientUI::WndOuterBorderColor(), 1);       // main background
    GG::FlatRectangle(GG::Pt(ul.x, ul.y + production_label_top), GG::Pt(lr.x, ul.y + production_label_top + row_height),
                      ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);                 // production label background
    GG::FlatRectangle(GG::Pt(ul.x, ul.y + allocation_label_top), GG::Pt(lr.x, ul.y + allocation_label_top + row_height),
                      ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);                 // allocation label background
    GG::FlatRectangle(GG::Pt(ul.x, ul.y + import_export_label_top), GG::Pt(lr.x, ul.y + import_export_label_top + row_height),
                      ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);                 // import or export label background
}

void SystemResourceSummaryBrowseWnd::UpdateImpl(std::size_t mode, const GG::Wnd* target) {
    // fully recreate browse wnd for each viewing.  finding all the queues, resourcepools and (maybe?) individual
    // UniverseObject that would have ChangedSignals that would need to be connected to the object that creates
    // this BrowseWnd seems like more trouble than it's worth to avoid recreating the BrowseWnd every time it's shown
    // (the alternative is to only reinitialize when something changes that would affect what's displayed in the
    // BrowseWnd, which is how MeterBrowseWnd works)
    Clear();
    Initialize();
}

void SystemResourceSummaryBrowseWnd::Initialize() {
    row_height = GG::Y(ClientUI::Pts() * 3/2);
    const GG::X TOTAL_WIDTH = LABEL_WIDTH + VALUE_WIDTH;

    const boost::shared_ptr<GG::Font>& font_bold = ClientUI::GetBoldFont();

    GG::Y top = GG::Y0;


    production_label_top = top;
    m_production_label = new GG::TextControl(GG::X0, production_label_top, TOTAL_WIDTH - EDGE_PAD,
                                             row_height, "", font_bold, ClientUI::TextColor(),
                                             GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
    AttachChild(m_production_label);
    top += row_height;
    UpdateProduction(top);


    allocation_label_top = top;
    m_allocation_label = new GG::TextControl(GG::X0, allocation_label_top, TOTAL_WIDTH - EDGE_PAD,
                                             row_height, "", font_bold, ClientUI::TextColor(),
                                             GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
    AttachChild(m_allocation_label);
    top += row_height;
    UpdateAllocation(top);


    import_export_label_top = top;
    m_import_export_label = new GG::TextControl(GG::X0, import_export_label_top, TOTAL_WIDTH - EDGE_PAD,
                                                row_height, "", font_bold, ClientUI::TextColor(),
                                                GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
    AttachChild(m_import_export_label);
    top += row_height;
    UpdateImportExport(top);


    Resize(GG::Pt(LABEL_WIDTH + VALUE_WIDTH, top));
}

void SystemResourceSummaryBrowseWnd::UpdateProduction(GG::Y& top) {
    // adds pairs of TextControl for ResourceCenter name and production of resource starting at vertical position \a top
    // and updates \a top to the vertical position after the last entry
    for (unsigned int i = 0; i < m_production_labels_and_amounts.size(); ++i) {
        DeleteChild(m_production_labels_and_amounts[i].first);
        DeleteChild(m_production_labels_and_amounts[i].second);
    }
    m_production_labels_and_amounts.clear();

    const System* system = GetSystem(m_system_id);
    if (!system || m_resource_type == INVALID_RESOURCE_TYPE)
        return;


    m_production = 0.0;


    const boost::shared_ptr<GG::Font>& font = ClientUI::GetFont();

    // add label-value pair for each resource-producing object in system to indicate amount of resource produced
    std::vector<int> obj_vec = system->FindObjectIDs();
    for (std::vector<int>::const_iterator it = obj_vec.begin(); it != obj_vec.end(); ++it) {
        const UniverseObject* obj = GetUniverseObject(*it);

        // display information only for the requested player
        if (m_empire_id != ALL_EMPIRES && !obj->OwnedBy(m_empire_id))
            continue;   // if m_empire_id == -1, display resource production for all empires.  otherwise, skip this resource production if it's not owned by the requested player

        const ResourceCenter* rc = dynamic_cast<const ResourceCenter*>(obj);
        if (!rc) continue;

        std::string name = obj->Name();
        double production = rc->InitialMeterValue(ResourceToMeter(m_resource_type));
        m_production += production;

        std::string amount_text = DoubleToString(production, 3, false);


        GG::TextControl* label = new GG::TextControl(GG::X0, top, LABEL_WIDTH, row_height,
                                                     name, font, ClientUI::TextColor(),
                                                     GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
        label->Resize(GG::Pt(LABEL_WIDTH, row_height));
        AttachChild(label);

        GG::TextControl* value = new GG::TextControl(LABEL_WIDTH, top, VALUE_WIDTH, row_height,
                                                     amount_text, font, ClientUI::TextColor(),
                                                     GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
        AttachChild(value);

        m_production_labels_and_amounts.push_back(std::pair<GG::TextControl*, GG::TextControl*>(label, value));

        top += row_height;
    }


    if (m_production_labels_and_amounts.empty()) {
        // add "blank" line to indicate no production
        GG::TextControl* label = new GG::TextControl(GG::X0, top, LABEL_WIDTH, row_height,
                                                     UserString("NOT_APPLICABLE"), font, ClientUI::TextColor(),
                                                     GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
        AttachChild(label);

        GG::TextControl* value = new GG::TextControl(LABEL_WIDTH, top, VALUE_WIDTH, row_height,
                                                     "", font, ClientUI::TextColor(),
                                                     GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
        AttachChild(value);

        m_production_labels_and_amounts.push_back(std::pair<GG::TextControl*, GG::TextControl*>(label, value));

        top += row_height;
    }


    // set production label
    std::string resource_text = "";
    switch (m_resource_type) {
    case RE_MINERALS:
        resource_text = UserString("MINERALS_PRODUCTION");  break;
    case RE_INDUSTRY:
        resource_text = UserString("INDUSTRY_PRODUCTION");  break;
    case RE_RESEARCH:
        resource_text = UserString("RESEARCH_PRODUCTION");  break;
    case RE_TRADE:
        resource_text = UserString("TRADE_PRODUCTION");     break;
    default:
        resource_text = UserString("UNKNOWN_VALUE_SYMBOL"); break;
    }

    m_production_label->SetText(boost::io::str(FlexibleFormat(UserString("RESOURCE_PRODUCTION_TOOLTIP")) %
                                                              resource_text %
                                                              DoubleToString(m_production, 3, false)));

    // height of label already added to top outside this function
}

void SystemResourceSummaryBrowseWnd::UpdateAllocation(GG::Y& top) {
    // adds pairs of TextControl for allocation of resources in system, starting at vertical position \a top and
    // updates \a top to be the vertical position after the last entry
    for (unsigned int i = 0; i < m_allocation_labels_and_amounts.size(); ++i) {
        DeleteChild(m_allocation_labels_and_amounts[i].first);
        DeleteChild(m_allocation_labels_and_amounts[i].second);
    }
    m_allocation_labels_and_amounts.clear();

    const System* system = GetSystem(m_system_id);
    if (!system || m_resource_type == INVALID_RESOURCE_TYPE)
        return;


    const boost::shared_ptr<GG::Font>& font = ClientUI::GetFont();

    m_allocation = 0.0;


    // add label-value pair for each resource-consuming object in system to indicate amount of resource consumed
    std::vector<int> obj_vec = system->FindObjectIDs();
    //// DEBUG
    //Logger().debugStream() << "System::FindObjects for system " << m_system->Name();
    //for (std::vector<UniverseObject*>::const_iterator it = obj_vec.begin(); it != obj_vec.end(); ++it)
    //    Logger().debugStream() << ".... " << (*it)->Name();
    //// END DEBUG


    for (std::vector<int>::const_iterator it = obj_vec.begin(); it != obj_vec.end(); ++it) {
        const UniverseObject* obj = GetUniverseObject(*it);

        // display information only for the requested player
        if (m_empire_id != ALL_EMPIRES && !obj->OwnedBy(m_empire_id))
            continue;   // if m_empire_id == ALL_EMPIRES, display resource production for all empires.  otherwise, skip this resource production if it's not owned by the requested player


        std::string name = obj->Name();


        double allocation = ObjectResourceConsumption(obj, m_resource_type, m_empire_id);


        // don't add summary entries for objects that consume no resource.  (otherwise there would be a loooong pointless list of 0's
        if (allocation <= 0.0) {
            if (allocation < 0.0)
                Logger().errorStream() << "object " << obj->Name() << " is reported having negative " << boost::lexical_cast<std::string>(m_resource_type) << " consumption";
            continue;
        }


        m_allocation += allocation;

        std::string amount_text = DoubleToString(allocation, 3, false);

        // TODO: for food only, colour allocation text depending on need of PopCenter:
        // - if allocation < need to avoid starvation: colour stat decr colour (red)
        // - if allocation > need to avoid starvation  and  allocation < need for max growth: colour generic text colour (white)
        // - if allocation = need for max growth: colour stat incr colour (green)
        // if (m_resource_type == RE_FOOD) {
        //     // get various needs, determine appropriate colour for food text
        //     GG::Clr text_colour = // something?
        //     amount_text = ColourWrappedtext(amount_text, text_colour);
        // }

        // TODO: for minerals and industry, consider something similar as colouring text for food above.


        GG::TextControl* label = new GG::TextControl(GG::X0, top, LABEL_WIDTH, row_height,
                                                     name, font, ClientUI::TextColor(),
                                                     GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
        AttachChild(label);


        GG::TextControl* value = new GG::TextControl(LABEL_WIDTH, top, VALUE_WIDTH, row_height,
                                                     amount_text, font, ClientUI::TextColor(),
                                                     GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
        AttachChild(value);

        m_allocation_labels_and_amounts.push_back(std::pair<GG::TextControl*, GG::TextControl*>(label, value));

        top += row_height;
    }


    if (m_allocation_labels_and_amounts.empty()) {
        // add "blank" line to indicate no allocation
        GG::TextControl* label = new GG::TextControl(GG::X0, top, LABEL_WIDTH, row_height,
                                                     UserString("NOT_APPLICABLE"), font, ClientUI::TextColor(),
                                                     GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
        AttachChild(label);

        GG::TextControl* value = new GG::TextControl(LABEL_WIDTH, top, VALUE_WIDTH, row_height,
                                                     "", font, ClientUI::TextColor(),
                                                     GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
        AttachChild(value);

        m_allocation_labels_and_amounts.push_back(std::pair<GG::TextControl*, GG::TextControl*>(label, value));

        top += row_height;
    }


    // set consumption / allocation label
    std::string resource_text = "";
    switch (m_resource_type) {
    case RE_MINERALS:
        resource_text = UserString("MINERALS_CONSUMPTION"); break;
    case RE_INDUSTRY:
        resource_text = UserString("INDUSTRY_CONSUMPTION"); break;
    case RE_RESEARCH:
        resource_text = UserString("RESEARCH_CONSUMPTION"); break;
    case RE_TRADE:
        resource_text = UserString("TRADE_CONSUMPTION");    break;
    default:
        resource_text = UserString("UNKNOWN_VALUE_SYMBOL"); break;
    }

    std::string system_allocation_text = DoubleToString(m_allocation, 3, false);

    // for research only, local allocation makes no sense
    if (m_resource_type == RE_RESEARCH && m_allocation == 0.0)
        system_allocation_text = UserString("NOT_APPLICABLE");


    m_allocation_label->SetText(boost::io::str(FlexibleFormat(UserString("RESOURCE_ALLOCATION_TOOLTIP")) %
                                                              resource_text %
                                                              system_allocation_text));

    // height of label already added to top outside this function
}

void SystemResourceSummaryBrowseWnd::UpdateImportExport(GG::Y& top) {
    m_import_export_label->SetText(UserString("IMPORT_EXPORT_TOOLTIP"));

    const Empire* empire = 0;

    // check for early exit cases...
    bool abort = false;
    if (m_empire_id == ALL_EMPIRES ||m_resource_type == RE_RESEARCH) {
        // multiple empires have complicated stockpiling which don't make sense to try to display.
        // Research use is nonlocalized, so importing / exporting doesn't make sense to display
        abort = true;
    } else {
        empire = Empires().Lookup(m_empire_id);
        if (!empire)
            abort = true;
    }


    std::string label_text = "", amount_text = "";


    if (!abort) {
        double difference = m_production - m_allocation;

        switch (m_resource_type) {
        case RE_MINERALS:
        case RE_TRADE:
        case RE_INDUSTRY:
            if (difference > 0.0) {
                // show surplus
                label_text = UserString("RESOURCE_EXPORT");
                amount_text = DoubleToString(difference, 3, false);
            } else if (difference < 0.0) {
                // show amount being imported
                label_text = UserString("RESOURCE_IMPORT");
                amount_text = DoubleToString(std::abs(difference), 3, false);
            } else {
                // show self-sufficiency
                label_text = UserString("RESOURCE_SELF_SUFFICIENT");
                amount_text = "";
            }
            break;
        case RE_RESEARCH:
        default:
            // show nothing
            abort = true;
            break;
        }
    }


    if (abort) {
        label_text = UserString("NOT_APPLICABLE");
        amount_text = "";   // no change
    }


    const boost::shared_ptr<GG::Font>& font = ClientUI::GetFont();

    // add label and amount.  may be "NOT APPLIABLE" and nothing if aborted above
    GG::TextControl* label = new GG::TextControl(GG::X0, top, LABEL_WIDTH, row_height,
                                                 label_text, font, ClientUI::TextColor(),
                                                 GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
    AttachChild(label);

    GG::TextControl* value = new GG::TextControl(LABEL_WIDTH, top, VALUE_WIDTH, row_height,
                                                 amount_text, font, ClientUI::TextColor(),
                                                 GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
    AttachChild(value);

    m_import_export_labels_and_amounts.push_back(std::pair<GG::TextControl*, GG::TextControl*>(label, value));

    top += row_height;
}

void SystemResourceSummaryBrowseWnd::Clear() {
    DeleteChild(m_production_label);
    DeleteChild(m_allocation_label);
    DeleteChild(m_import_export_label);

    for (std::vector<std::pair<GG::TextControl*, GG::TextControl*> >::iterator it = m_production_labels_and_amounts.begin(); it != m_production_labels_and_amounts.end(); ++it) {
        DeleteChild(it->first);
        DeleteChild(it->second);
    }
    m_production_labels_and_amounts.clear();

    for (std::vector<std::pair<GG::TextControl*, GG::TextControl*> >::iterator it = m_allocation_labels_and_amounts.begin(); it != m_allocation_labels_and_amounts.end(); ++it) {
        DeleteChild(it->first);
        DeleteChild(it->second);
    }
    m_allocation_labels_and_amounts.clear();

    for (std::vector<std::pair<GG::TextControl*, GG::TextControl*> >::iterator it = m_import_export_labels_and_amounts.begin(); it != m_import_export_labels_and_amounts.end(); ++it) {
        DeleteChild(it->first);
        DeleteChild(it->second);
    }
    m_import_export_labels_and_amounts.clear();
}


//////////////////////////////////////
//         MeterBrowseWnd           //
//////////////////////////////////////
MeterBrowseWnd::MeterBrowseWnd(int object_id, MeterType primary_meter_type, MeterType secondary_meter_type/* = INVALID_METER_TYPE*/) :
    GG::BrowseInfoWnd(GG::X0, GG::Y0, METER_BROWSE_LABEL_WIDTH + METER_BROWSE_VALUE_WIDTH, GG::Y1),
    m_primary_meter_type(primary_meter_type),
    m_secondary_meter_type(secondary_meter_type),
    m_object_id(object_id),
    m_summary_title(0),
    m_current_label(0), m_current_value(0),
    m_next_turn_label(0), m_next_turn_value(0),
    m_change_label(0), m_change_value(0),
    m_meter_title(0),
    m_row_height(1),
    m_initialized(false)
{}

bool MeterBrowseWnd::WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const {
    assert(mode <= wnd->BrowseModes().size());
    return true;
}

void MeterBrowseWnd::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    // main background
    GG::FlatRectangle(ul, lr, OpaqueColor(ClientUI::WndColor()), ClientUI::WndOuterBorderColor(), 1);

    // top title filled background
    if (m_summary_title)
        GG::FlatRectangle(m_summary_title->UpperLeft(), m_summary_title->LowerRight() + GG::Pt(GG::X(EDGE_PAD), GG::Y0), ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);

    // middle title filled background
    if (m_meter_title)
        GG::FlatRectangle(m_meter_title->UpperLeft(), m_meter_title->LowerRight() + GG::Pt(GG::X(EDGE_PAD), GG::Y0), ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);
}

namespace {
    const std::string& MeterToUserString(MeterType meter_type) {
        return UserString(GG::GetEnumMap<MeterType>().FromEnum(meter_type));
    }
}

void MeterBrowseWnd::Initialize() {
    m_row_height = GG::Y(ClientUI::Pts()*3/2);
    const GG::X TOTAL_WIDTH = METER_BROWSE_LABEL_WIDTH + METER_BROWSE_VALUE_WIDTH;

    const boost::shared_ptr<GG::Font>& font = ClientUI::GetFont();
    const boost::shared_ptr<GG::Font>& font_bold = ClientUI::GetBoldFont();

    // get objects and meters to verify that they exist
    const UniverseObject* obj = GetUniverseObject(m_object_id);
    if (!obj) {
        Logger().errorStream() << "MeterBrowseWnd couldn't get object with id " << m_object_id;
        return;
    }
    const Meter* primary_meter = obj->GetMeter(m_primary_meter_type);
    const Meter* secondary_meter = obj->GetMeter(m_secondary_meter_type);

    GG::Y top = GG::Y0;

    // create controls and do layout
    if (primary_meter && secondary_meter) {
        // both primary and secondary meter exist.  display a current value
        // summary at top with current and next turn values

        // special case for meters: use species name
        std::string summary_title_text;
        if (m_primary_meter_type == METER_POPULATION) {
            std::string human_readable_species_name;
            if (const PopCenter* pop = dynamic_cast<const PopCenter*>(obj)) {
                const std::string& species_name = pop->SpeciesName();
                if (!species_name.empty())
                    human_readable_species_name = UserString(species_name);
            }
            summary_title_text = boost::io::str(FlexibleFormat(UserString("TT_SPECIES_POPULATION")) % human_readable_species_name);
        } else {
            summary_title_text = MeterToUserString(m_primary_meter_type);
        }

        m_summary_title = new GG::TextControl(GG::X0, top, TOTAL_WIDTH - EDGE_PAD, m_row_height,
                                              summary_title_text, font_bold, ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
        AttachChild(m_summary_title);
        top += m_row_height;

        m_current_label = new GG::TextControl(GG::X0, top, METER_BROWSE_LABEL_WIDTH, m_row_height,
                                              UserString("TT_THIS_TURN"), font, ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
        AttachChild(m_current_label);
        m_current_value = new GG::TextControl(METER_BROWSE_LABEL_WIDTH, top, METER_BROWSE_VALUE_WIDTH, m_row_height,
                                              "", font, ClientUI::TextColor(), GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
        AttachChild(m_current_value);
        top += m_row_height;

        m_next_turn_label = new GG::TextControl(GG::X0, top, METER_BROWSE_LABEL_WIDTH, m_row_height,
                                                UserString("TT_NEXT_TURN"), font, ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
        AttachChild(m_next_turn_label);
        m_next_turn_value = new GG::TextControl(METER_BROWSE_LABEL_WIDTH, top, METER_BROWSE_VALUE_WIDTH, m_row_height, "",
                                                font, ClientUI::TextColor(), GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
        AttachChild(m_next_turn_value);
        top += m_row_height;

        m_change_label = new GG::TextControl(GG::X0, top, METER_BROWSE_LABEL_WIDTH, m_row_height, UserString("TT_CHANGE"),
                                             font, ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
        AttachChild(m_change_label);
        m_change_value = new GG::TextControl(METER_BROWSE_LABEL_WIDTH, top, METER_BROWSE_VALUE_WIDTH, m_row_height, "",
                                             font, ClientUI::TextColor(), GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
        AttachChild(m_change_value);
        top += m_row_height;
    }

    if (primary_meter) {
        // effect accounting meter breakdown total / summary.  Shows "Meter Name: Value"
        // above a list of effects.  Actual text is set in UpdateSummary() but
        // the control is created here.
        m_meter_title = new GG::TextControl(GG::X0, top, TOTAL_WIDTH - EDGE_PAD, m_row_height, "",
                                            font_bold, ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
        AttachChild(m_meter_title);
        top += m_row_height;
    }

    UpdateSummary();

    UpdateEffectLabelsAndValues(top);

    Resize(GG::Pt(METER_BROWSE_LABEL_WIDTH + METER_BROWSE_VALUE_WIDTH, top));

    m_initialized = true;
}

void MeterBrowseWnd::UpdateImpl(std::size_t mode, const Wnd* target) {
    // because a MeterBrowseWnd's contents depends only on the meters of a single object, if that object doesn't
    // change between showings of the meter browse wnd, it's not necessary to fully recreate the MeterBrowseWnd,
    // and it can be just reshown.without being altered.  To refresh a MeterBrowseWnd, recreate it by assigning
    // a new one as the moused-over object's BrowseWnd in this Wnd's place
    if (!m_initialized)
        Initialize();
}

void MeterBrowseWnd::UpdateSummary() {
    const UniverseObject* obj = GetUniverseObject(m_object_id);
    if (!obj)
        return;

    const Meter* primary_meter = obj->GetMeter(m_primary_meter_type);
    const Meter* secondary_meter = obj->GetMeter(m_secondary_meter_type);

    double breakdown_total = 0.0;
    std::string breakdown_meter_name;

    if (primary_meter && secondary_meter) {
        if (!m_current_value || !m_next_turn_value || !m_change_value) {
            Logger().errorStream() << "MeterBrowseWnd::UpdateSummary has primary and secondary meters, but is missing one or more controls to display them";
            return;
        }

        // Primary meter holds value from turn to turn and changes slow each turn.
        // The current value of the primary meter doesn't change with focus changes
        // so its growth from turn to turn is important to show
        double primary_current = obj->InitialMeterValue(m_primary_meter_type);
        double primary_next = obj->NextTurnCurrentMeterValue(m_primary_meter_type);
        double primary_change = primary_next - primary_current;

        m_current_value->SetText(DoubleToString(primary_current, 3, false));
        m_next_turn_value->SetText(DoubleToString(primary_next, 3, false));
        m_change_value->SetText(ColouredNumber(primary_change));

        // target or max meter total for breakdown summary
        breakdown_total = obj->CurrentMeterValue(m_secondary_meter_type);
        breakdown_meter_name = MeterToUserString(m_secondary_meter_type);

    } else if (primary_meter) {
        // unpaired meter total for breakdown summary
        breakdown_total = obj->InitialMeterValue(m_primary_meter_type);
        breakdown_meter_name = MeterToUserString(m_primary_meter_type);
    } else {
        Logger().errorStream() << "MeterBrowseWnd::UpdateSummary can't get primary meter";
        return;
    }

    // set accounting breakdown total / summary
    if (m_meter_title)
        m_meter_title->SetText(boost::io::str(FlexibleFormat(UserString("TT_BREAKDOWN_SUMMARY")) %
                                              breakdown_meter_name %
                                              DoubleToString(breakdown_total, 3, false)));
}

void MeterBrowseWnd::UpdateEffectLabelsAndValues(GG::Y& top) {
    // clear existing labels
    for (unsigned int i = 0; i < m_effect_labels_and_values.size(); ++i) {
        DeleteChild(m_effect_labels_and_values[i].first);
        DeleteChild(m_effect_labels_and_values[i].second);
    }
    m_effect_labels_and_values.clear();


    // get object and meter, aborting if not valid
    const UniverseObject* obj = GetUniverseObject(m_object_id);
    if (!obj) {
        Logger().errorStream() << "MeterBrowseWnd::UpdateEffectLabelsAndValues couldn't get object with id " << m_object_id;
        return;
    }


    // get effect accounting info for this MeterBrowseWnd's object, aborting if non available
    const Universe& universe = GetUniverse();
    const Effect::AccountingMap& effect_accounting_map = universe.GetEffectAccountingMap();
    Effect::AccountingMap::const_iterator map_it = effect_accounting_map.find(m_object_id);
    if (map_it == effect_accounting_map.end())
        return;
    const std::map<MeterType, std::vector<Effect::AccountingInfo> >& meter_map = map_it->second;


    // select which meter type to display accounting for.  if there is a valid
    // secondary meter type, then this is probably the target or max meter and
    // should have accounting displayed.  if there is no valid secondary meter
    // (and there is a valid primary meter) then that is probably an unpaired
    // meter and should have accounting displayed
    MeterType accounting_displayed_for_meter = INVALID_METER_TYPE;
    if (m_secondary_meter_type != INVALID_METER_TYPE)
        accounting_displayed_for_meter = m_secondary_meter_type;
    else if (m_primary_meter_type != INVALID_METER_TYPE)
        accounting_displayed_for_meter = m_primary_meter_type;
    if (accounting_displayed_for_meter == INVALID_METER_TYPE)
        return; // nothing to display


    // get accounting info for this MeterBrowseWnd's meter type, aborting if none available
    std::map<MeterType, std::vector<Effect::AccountingInfo> >::const_iterator meter_it = meter_map.find(accounting_displayed_for_meter);
    if (meter_it == meter_map.end() || meter_it->second.empty())
        return; // couldn't find appropriate meter type, or there were no entries for that meter.
    const std::vector<Effect::AccountingInfo>& info_vec = meter_it->second;


    const boost::shared_ptr<GG::Font>& font = ClientUI::GetFont();


    // add label-value pairs for each alteration recorded for this meter
    for (std::vector<Effect::AccountingInfo>::const_iterator info_it = info_vec.begin(); info_it != info_vec.end(); ++info_it) {
        const UniverseObject* source = GetUniverseObject(info_it->source_id);
        if (!source)
            continue;

        const Empire*   empire = 0;
        const Building* building = 0;
        const Planet*   planet = 0;
        const Ship*     ship = 0;
        std::string     text;
        std::string     name = source->Name();

        switch (info_it->cause_type) {
        case ECT_TECH: {
            name.clear();
            if (empire = Empires().Lookup(source->Owner()))
                name = empire->Name();
            const std::string& label_template = (info_it->custom_label.empty() ? "TT_TECH" : info_it->custom_label);
            text += boost::io::str(FlexibleFormat(UserString(label_template)) % name % UserString(info_it->specific_cause));
            break;
        }
        case ECT_BUILDING: {
            name.clear();
            if (building = universe_object_cast<const Building*>(source))
                if (planet = GetPlanet(building->PlanetID()))
                    name = planet->Name();
            const std::string& label_template = (info_it->custom_label.empty() ? "TT_BUILDING" : info_it->custom_label);
            text += boost::io::str(FlexibleFormat(UserString(label_template)) % name % UserString(info_it->specific_cause));
            break;
        }
        case ECT_SPECIAL: {
            const std::string& label_template = (info_it->custom_label.empty() ? "TT_SPECIAL" : info_it->custom_label);
            text += boost::io::str(FlexibleFormat(UserString(label_template)) % name % UserString(info_it->specific_cause));
            break;
        }
        case ECT_SPECIES: {
            //Logger().debugStream() << "Effect Species Meter Browse Wnd effect cause " << info_it->specific_cause << " custom label: " << info_it->custom_label;
            const std::string& label_template = (info_it->custom_label.empty() ? "TT_SPECIES" : info_it->custom_label);
            text += boost::io::str(FlexibleFormat(UserString(label_template)) % name % UserString(info_it->specific_cause));
            break;
        }
        case ECT_SHIP_HULL: {
            const std::string& label_template = (info_it->custom_label.empty() ? "TT_SHIP_HULL" : info_it->custom_label);
            text += boost::io::str(FlexibleFormat(UserString(label_template)) % name % UserString(info_it->specific_cause));
            break;
        }
        case ECT_SHIP_PART: {
            const std::string& label_template = (info_it->custom_label.empty() ? "TT_SHIP_PART" : info_it->custom_label);
            text += boost::io::str(FlexibleFormat(UserString(label_template)) % name % UserString(info_it->specific_cause));
            break;
        }
        case ECT_INHERENT:
            text += UserString("TT_INHERENT");
            break;

        case ECT_UNKNOWN_CAUSE: {
        default:
            const std::string& label_template = (info_it->custom_label.empty() ? "TT_UNKNOWN" : info_it->custom_label);
            text += UserString(label_template);
        }
        }

        GG::TextControl* label = new GG::TextControl(GG::X0, top, METER_BROWSE_LABEL_WIDTH, m_row_height, text, font, ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
        AttachChild(label);

        GG::TextControl* value = new GG::TextControl(METER_BROWSE_LABEL_WIDTH, top, METER_BROWSE_VALUE_WIDTH, m_row_height,
                                                     ColouredNumber(info_it->meter_change),
                                                     font, ClientUI::TextColor(), GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
        AttachChild(value);
        m_effect_labels_and_values.push_back(std::pair<GG::TextControl*, GG::TextControl*>(label, value));

        top += m_row_height;
    }
}

namespace {
    //////////////////////////////////////
    // MeterModifiersIndicatorBrowseWnd //
    //////////////////////////////////////
    MeterModifiersIndicatorBrowseWnd::MeterModifiersIndicatorBrowseWnd(int object_id, MeterType meter_type) :
        GG::BrowseInfoWnd(GG::X0, GG::Y0, METER_BROWSE_LABEL_WIDTH + METER_BROWSE_VALUE_WIDTH, GG::Y1),
        m_meter_type(meter_type),
        m_source_object_id(object_id),
        m_summary_title(0),
        m_sum_label(0),
        m_sum_value(0),
        m_row_height(1),
        m_initialized(false)
    {}

    bool MeterModifiersIndicatorBrowseWnd::WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const {
        assert(mode <= wnd->BrowseModes().size());
        return true;
    }

    void MeterModifiersIndicatorBrowseWnd::Render() {
        GG::Pt ul = UpperLeft();
        GG::Pt lr = LowerRight();
        // main background
        GG::FlatRectangle(ul, lr, OpaqueColor(ClientUI::WndColor()), ClientUI::WndOuterBorderColor(), 1);

        // top title filled background
        if (m_summary_title)
            GG::FlatRectangle(m_summary_title->UpperLeft(), m_summary_title->LowerRight() + GG::Pt(GG::X(EDGE_PAD), GG::Y0), ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);
    }

    void MeterModifiersIndicatorBrowseWnd::Initialize() {
        m_row_height = GG::Y(ClientUI::Pts()*3/2);
        GG::Y top = GG::Y0;

        const GG::X TOTAL_WIDTH = METER_BROWSE_LABEL_WIDTH + METER_BROWSE_VALUE_WIDTH;

        const boost::shared_ptr<GG::Font>& font = ClientUI::GetFont();
        const boost::shared_ptr<GG::Font>& font_bold = ClientUI::GetBoldFont();

        m_summary_title = new GG::TextControl(GG::X0, top, TOTAL_WIDTH - EDGE_PAD, m_row_height, "",
                                                font_bold, ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
        AttachChild(m_summary_title);
        top += m_row_height;

        // clear existing labels
        for (unsigned int i = 0; i < m_effect_labels_and_values.size(); ++i) {
            DeleteChild(m_effect_labels_and_values[i].first);
            DeleteChild(m_effect_labels_and_values[i].second);
        }
        m_effect_labels_and_values.clear();

        // set summary label
        const UniverseObject* obj = GetUniverseObject(m_source_object_id);
        if (!obj) {
            Logger().errorStream() << "MeterModifiersIndicatorBrowseWnd couldn't get object with id " << m_source_object_id;
            m_summary_title->SetText(boost::io::str(FlexibleFormat(UserString("TT_TARGETS_BREAKDOWN_SUMMARY")) %
                                                    MeterToUserString(m_meter_type) %
                                                    DoubleToString(0.0, 3, false)));
            return;
        }

        const Universe& universe = GetUniverse();
        const ObjectMap& objects = universe.Objects();
        const Effect::AccountingMap& effect_accounting_map = universe.GetEffectAccountingMap();

        double modifications_sum = 0.0;

        // for every object that has the appropriate meter type, get its affect accounting info
        for (ObjectMap::const_iterator obj_it = objects.const_begin();
                obj_it != objects.const_end(); ++obj_it)
        {
            int target_object_id = obj_it->first;
            const UniverseObject* target_obj = obj_it->second;
            // does object have relevant meter?
            const Meter* meter = target_obj->GetMeter(m_meter_type);
            if (!meter)
                continue;

            // is any effect accounting available for target object?
            Effect::AccountingMap::const_iterator map_it = effect_accounting_map.find(target_object_id);
            if (map_it == effect_accounting_map.end())
                continue;
            const std::map<MeterType, std::vector<Effect::AccountingInfo> >& meter_map = map_it->second;

            // is any effect accounting available for this indicator's meter type?
            std::map<MeterType, std::vector<Effect::AccountingInfo> >::const_iterator meter_it =
                meter_map.find(m_meter_type);
            if (meter_it == meter_map.end())
                continue;
            const std::vector<Effect::AccountingInfo>& accounts = meter_it->second;

            // does the target object's effect accounting have any modifications
            // by this indicator's source object?  (may be more than one)
            for (std::vector<Effect::AccountingInfo>::const_iterator account_it = accounts.begin();
                    account_it != accounts.end(); ++account_it)
            {
                if (account_it->source_id != m_source_object_id)
                    continue;
                modifications_sum += account_it->meter_change;

                const std::string& text = target_obj->Name();
                GG::TextControl* label = new GG::TextControl(GG::X0, top, METER_BROWSE_LABEL_WIDTH, m_row_height,
                                                                text, font, ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
                AttachChild(label);
                GG::TextControl* value = new GG::TextControl(METER_BROWSE_LABEL_WIDTH, top, METER_BROWSE_VALUE_WIDTH, m_row_height,
                                                                ColouredNumber(account_it->meter_change),
                                                                font, ClientUI::TextColor(), GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
                AttachChild(value);
                m_effect_labels_and_values.push_back(std::pair<GG::TextControl*, GG::TextControl*>(label, value));

                top += m_row_height;
            }
        }

        Resize(GG::Pt(METER_BROWSE_LABEL_WIDTH + METER_BROWSE_VALUE_WIDTH, top));

        m_summary_title->SetText(boost::io::str(FlexibleFormat(UserString("TT_TARGETS_BREAKDOWN_SUMMARY")) %
                                                MeterToUserString(m_meter_type) %
                                                DoubleToString(modifications_sum, 3, false)));
        m_initialized = true;
    }

    void MeterModifiersIndicatorBrowseWnd::UpdateImpl(std::size_t mode, const Wnd* target) {
        if (!m_initialized)
            Initialize();
    }
}

