#include "InfoPanels.h"

#include "../universe/UniverseObject.h"
#include "../universe/PopCenter.h"
#include "../universe/ResourceCenter.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../universe/Building.h"
#include "../Empire/Empire.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "../client/human/HumanClientApp.h"
#include "../util/OptionsDB.h"

#include <GG/DrawUtil.h>
#include <GG/GUI.h>
#include <GG/StaticGraphic.h>

#include <boost/lexical_cast.hpp>

using boost::lexical_cast;

namespace {
    GG::Clr METER_STATUS_BAR_COLOR(193, 193, 193, 255);
    GG::Clr METER_STATUS_BAR_GROWTH_COLOR(151, 151, 151, 200);
    GG::Clr METER_STATUS_BAR_SHRINK_COLOR(255, 0, 0, 255);
}

/////////////////////////////////////
//        PopulationPanel          //
/////////////////////////////////////
std::map<int, bool> PopulationPanel::s_expanded_map = std::map<int, bool>();
PopulationPanel::PopulationPanel(int w, const UniverseObject &obj) :
    Wnd(0, 0, w, ClientUI::Pts()*4/3, GG::CLICKABLE),
    m_popcenter_id(obj.ID()),
    m_expand_button(new GG::Button(w - 16, 0, 16, 16, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), GG::CLR_WHITE)),
    m_pop_stat(0),
    m_health_stat(0),
    m_pop_meter_bar(0),
    m_health_meter_bar(0)
{
    const PopCenter* pop = dynamic_cast<const PopCenter*>(&obj);
    if (!pop)
        throw std::invalid_argument("Attempted to construct a PopulationPanel with an UniverseObject that is not a PopCenter");

    AttachChild(m_expand_button);
    m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrownormal.png"   ), 0, 0, 32, 32));
    m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowclicked.png"  ), 0, 0, 32, 32));
    m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowmouseover.png"), 0, 0, 32, 32));
    GG::Connect(m_expand_button->ClickedSignal, &PopulationPanel::ExpandCollapseButtonPressed, this);

    int icon_size = ClientUI::Pts()*4/3;

    m_pop_stat = new StatisticIcon(0, 0, icon_size, icon_size, (ClientUI::ArtDir() / "icons" / "pop.png").native_file_string(), GG::CLR_WHITE,
                                   0, 3, false, false);
    AttachChild(m_pop_stat);
    m_pop_stat->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_pop_stat->SetBrowseText("Population!");

    m_health_stat = new StatisticIcon(w/2, 0, icon_size, icon_size, (ClientUI::ArtDir() / "icons" / "health.png").native_file_string() , GG::CLR_WHITE,
                                      0, 3, false, false);
    AttachChild(m_health_stat);
    m_health_stat->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_health_stat->SetBrowseText("Health!");

    m_pop_meter_bar = new MeterStatusBar2(w - m_pop_stat->Width() - icon_size*5/2 - m_expand_button->Width(), m_pop_stat->Height(), pop->PopulationMeter());
    m_pop_meter_bar->MoveTo(GG::Pt(m_pop_stat->Width() + icon_size*5/2, 0));
    m_pop_meter_bar->Hide();

    m_health_meter_bar = new MeterStatusBar2(w - m_health_stat->Width() - icon_size*5/2 - m_expand_button->Width(), m_health_stat->Height(), pop->HealthMeter());
    m_health_meter_bar->MoveTo(GG::Pt(m_health_stat->Width() + icon_size*5/2, m_pop_stat->LowerRight().y));
    m_health_meter_bar->Hide();

    // determine if this panel has been created yet.
    std::map<int, bool>::iterator it = s_expanded_map.find(m_popcenter_id);
    if (it == s_expanded_map.end())
        s_expanded_map[m_popcenter_id] = false; // if not, default to collapsed state
    
    DoExpandCollapseLayout();
    
    // these may be redundant...
    if (System* system = obj.GetSystem())
        m_connection_system_changed = GG::Connect(system->StateChangedSignal, &PopulationPanel::Update, this);
    m_connection_popcenter_changed = GG::Connect(obj.StateChangedSignal, &PopulationPanel::Update, this);

    Update();
}

PopulationPanel::~PopulationPanel()
{
    // HACK! These disconnects should not be necessary, since PopulationPanel is derived from boost::signals::trackable, but
    // a segfault occurrs if they are not done (as of Boost 1.33.1).
    m_connection_system_changed.disconnect();
    m_connection_popcenter_changed.disconnect();

    // manually delete all pointed-to controls that may or may not be attached as a child window at time of deletion
    DetachChild(m_pop_stat);
    delete m_pop_stat;
    DetachChild(m_health_stat);
    delete m_health_stat;
    DetachChild(m_pop_meter_bar);
    delete m_pop_meter_bar;
    DetachChild(m_health_meter_bar);
    delete m_health_meter_bar;

    // don't need to manually delete m_expand_button, as it is attached as a child so will be deleted by ~Wnd
}

void PopulationPanel::ExpandCollapseButtonPressed()
{
    ExpandCollapse(!s_expanded_map[m_popcenter_id]);
}

void PopulationPanel::ExpandCollapse(bool expanded)
{
    if (expanded == s_expanded_map[m_popcenter_id]) return; // nothing to do
    s_expanded_map[m_popcenter_id] = expanded;

    DoExpandCollapseLayout();

    ExpandCollapseSignal();

    Render();                       // redraw in new state
}

void PopulationPanel::DoExpandCollapseLayout()
{
    int icon_size = ClientUI::Pts()*4/3;

    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_popcenter_id]) {
        m_pop_stat->MoveTo(GG::Pt(0, 0));
        m_health_stat->MoveTo(GG::Pt(Width()/2, 0));

        m_pop_meter_bar->Hide();
        m_health_meter_bar->Hide();
        DetachChild(m_pop_meter_bar);
        DetachChild(m_health_meter_bar);

        Resize(GG::Pt(Width(), icon_size));
    } else {
        m_pop_stat->MoveTo(GG::Pt(0, 0));
        m_health_stat->MoveTo(GG::Pt(0, icon_size));

        AttachChild(m_pop_meter_bar);
        AttachChild(m_health_meter_bar);
        m_pop_meter_bar->Show();
        m_health_meter_bar->Show();

        Resize(GG::Pt(Width(), icon_size*2));
    }

    // update appearance of expand/collapse button
    if (s_expanded_map[m_popcenter_id])
    {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrownormal.png"   ), 0, 0, 32, 32));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrowclicked.png"  ), 0, 0, 32, 32));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrowmouseover.png"), 0, 0, 32, 32));
    }
    else
    {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrownormal.png"   ), 0, 0, 32, 32));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowclicked.png"  ), 0, 0, 32, 32));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowmouseover.png"), 0, 0, 32, 32));
    }
}

void PopulationPanel::MouseWheel(const GG::Pt& pt, int move, Uint32 keys)
{
    GG::Wnd *parent;
    if((parent = Parent()))
        parent->MouseWheel(pt, move, keys);
}

void PopulationPanel::Render() 
{
    // Draw outline and background...

    // copied from CUIWnd
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::Pt cl_ul = ClientUpperLeft();
    GG::Pt cl_lr = ClientLowerRight();

    // use GL to draw the lines
    glDisable(GL_TEXTURE_2D);
    GLint initial_modes[2];
    glGetIntegerv(GL_POLYGON_MODE, initial_modes);

    // draw background
    glPolygonMode(GL_BACK, GL_FILL);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndColor());
        glVertex2i(ul.x, ul.y);
        glVertex2i(lr.x, ul.y);
        glVertex2i(lr.x, lr.y);
        glVertex2i(ul.x, lr.y);
        glVertex2i(ul.x, ul.y);
    glEnd();

    // draw outer border on pixel inside of the outer edge of the window
    glPolygonMode(GL_BACK, GL_LINE);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndOuterBorderColor());
        glVertex2i(ul.x, ul.y);
        glVertex2i(lr.x, ul.y);
        glVertex2i(lr.x, lr.y);
        glVertex2i(ul.x, lr.y);
        glVertex2i(ul.x, ul.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    glEnable(GL_TEXTURE_2D);

    // draw details depending on state of ownership and expanded / collapsed status
    
    // determine ownership
    const UniverseObject* obj = GetUniverse().Object(m_popcenter_id);
    /*if(obj->Owners().empty()) 
        // uninhabited
    else
    {
        if(!obj->OwnedBy(HumanClientApp::GetApp()->EmpireID()))
            // inhabited by other empire
        else
            // inhabited by this empire (and possibly other empires)
    }*/

}

void PopulationPanel::Update()
{
    const PopCenter* pop = GetPopCenter();
    const UniverseObject* obj = GetUniverse().Object(m_popcenter_id);

    enum OWNERSHIP {OS_NONE, OS_FOREIGN, OS_SELF} owner = OS_NONE;

    // determine ownership    
    if(obj->Owners().empty()) 
        owner = OS_NONE;  // uninhabited
    else {
        if(!obj->OwnedBy(HumanClientApp::GetApp()->EmpireID()))
            owner = OS_FOREIGN; // inhabited by other empire
        else
            owner = OS_SELF; // inhabited by this empire (and possibly other empires)
    }

    if (owner = OS_SELF) {
        // mousover text
        m_pop_stat->SetValue(pop->PopPoints());
        std::string text = StatisticIcon::DoubleToString(pop->PopPoints(), 3, false, false) +
                           "/" +
                           StatisticIcon::DoubleToString(pop->MaxPop(), 3, false, false) +
                           " (" +
                           StatisticIcon::DoubleToString(pop->FuturePopGrowth(), 3, false, true) +
                           ")";
        m_pop_stat->SetBrowseText(text);

        m_health_stat->SetValue(pop->Health());
        text =             StatisticIcon::DoubleToString(pop->Health(), 3, false, false) +
                           "/" +
                           StatisticIcon::DoubleToString(pop->MaxHealth(), 3, false, false) +
                           " (" +
                           StatisticIcon::DoubleToString(pop->FutureHealthGrowth(), 3, false, true) +
                           ")";
        m_health_stat->SetBrowseText(text);

        m_pop_meter_bar->SetProjectedCurrent(pop->PopPoints() + pop->FuturePopGrowth());
        m_pop_meter_bar->SetProjectedMax(pop->MaxPop());
        m_health_meter_bar->SetProjectedCurrent(pop->Health() + pop->FutureHealthGrowth());
        m_health_meter_bar->SetProjectedMax(pop->MaxHealth());
    }
}

const PopCenter* PopulationPanel::GetPopCenter() const
{
    const UniverseObject* obj = GetUniverse().Object(m_popcenter_id);
    if (!obj) throw std::runtime_error("PopulationPanel tried to get an object with an invalid m_popcenter_id");
    const PopCenter* pop = dynamic_cast<const PopCenter*>(obj);
    if (!pop) throw std::runtime_error("PopulationPanel failed casting an object pointer to a PopCenter pointer");
    return pop;
}

PopCenter* PopulationPanel::GetPopCenter()
{
    UniverseObject* obj = GetUniverse().Object(m_popcenter_id);
    if (!obj) throw std::runtime_error("PopulationPanel tried to get an object with an invalid m_popcenter_id");
    PopCenter* pop = dynamic_cast<PopCenter*>(obj);
    if (!pop) throw std::runtime_error("PopulationPanel failed casting an object pointer to a PopCenter pointer");
    return pop;
}


/////////////////////////////////////
//         ResourcePanel           //
/////////////////////////////////////
std::map<int, bool> ResourcePanel::s_expanded_map = std::map<int, bool>();
ResourcePanel::ResourcePanel(int w, const UniverseObject &obj) :
    Wnd(0, 0, w, ClientUI::Pts()*9, GG::CLICKABLE),
    m_rescenter_id(obj.ID()),
    m_expand_button(new GG::Button(w - 16, 0, 16, 16, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), GG::CLR_WHITE)),
    m_farming_stat(0), m_mining_stat(0), m_industry_stat(0),
    m_research_stat(0), m_trade_stat(0), m_construction_stat(0),
    m_farming_meter_bar(0), m_mining_meter_bar(0), m_industry_meter_bar(0),
    m_research_meter_bar(0), m_trade_meter_bar(0), m_construction_meter_bar(0),
    m_primary_focus_drop(0), m_secondary_focus_drop(0)
{
    const ResourceCenter* res = dynamic_cast<const ResourceCenter*>(&obj);
    if (!res)
        throw std::invalid_argument("Attempted to construct a ResourcePanel with an UniverseObject that is not a ResourceCenter");

    
    // expand / collapse button at top right    
    AttachChild(m_expand_button);
    m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrownormal.png"   ), 0, 0, 32, 32));
    m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowclicked.png"  ), 0, 0, 32, 32));
    m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowmouseover.png"), 0, 0, 32, 32));
    GG::Connect(m_expand_button->ClickedSignal, &ResourcePanel::ExpandCollapseButtonPressed, this);

    
    int icon_size = ClientUI::Pts()*4/3;
    GG::DropDownList::Row* row;
    boost::shared_ptr<GG::Texture> texture;
    GG::StaticGraphic* graphic;


    // focus-selection droplists
    m_primary_focus_drop = new CUIDropDownList(0, 0, icon_size*4, icon_size, icon_size*6);

    texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "balanced.png");
    graphic = new GG::StaticGraphic(0, 0, icon_size*2, icon_size*2, texture, GG::GR_FITGRAPHIC);
    row = new GG::DropDownList::Row();
    row->push_back(dynamic_cast<GG::Control*>(graphic));
    m_primary_focus_drop->Insert(row);

    texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "farming.png");
    graphic = new GG::StaticGraphic(0, 0, icon_size*2, icon_size*2, texture, GG::GR_FITGRAPHIC);
    row = new GG::DropDownList::Row();
    row->push_back(dynamic_cast<GG::Control*>(graphic));
    m_primary_focus_drop->Insert(row);

    texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "mining.png");
    graphic = new GG::StaticGraphic(0, 0, icon_size*3/2, icon_size*3/2, texture, GG::GR_FITGRAPHIC);
    row = new GG::DropDownList::Row();
    row->push_back(dynamic_cast<GG::Control*>(graphic));
    m_primary_focus_drop->Insert(row);

    texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "industry.png");
    graphic = new GG::StaticGraphic(0, 0, icon_size*3/2, icon_size*3/2, texture, GG::GR_FITGRAPHIC);
    row = new GG::DropDownList::Row();
    row->push_back(dynamic_cast<GG::Control*>(graphic));
    m_primary_focus_drop->Insert(row);

    texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "research.png");
    graphic = new GG::StaticGraphic(0, 0, icon_size*3/2, icon_size*3/2, texture, GG::GR_FITGRAPHIC);
    row = new GG::DropDownList::Row();
    row->push_back(dynamic_cast<GG::Control*>(graphic));
    m_primary_focus_drop->Insert(row);

    texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "trade.png");
    graphic = new GG::StaticGraphic(0, 0, icon_size*3/2, icon_size*3/2, texture, GG::GR_FITGRAPHIC);
    row = new GG::DropDownList::Row();
    row->push_back(dynamic_cast<GG::Control*>(graphic));
    m_primary_focus_drop->Insert(row);

    AttachChild(m_primary_focus_drop);

    m_secondary_focus_drop = new CUIDropDownList(m_primary_focus_drop->LowerRight().x + icon_size/2, 0,
                                                 icon_size*6, icon_size*3/2, icon_size*12);

    texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "balanced.png");
    graphic = new GG::StaticGraphic(0, 0, icon_size, icon_size, texture, GG::GR_FITGRAPHIC);
    row = new GG::DropDownList::Row();
    row->push_back(graphic);
    m_secondary_focus_drop->Insert(row);

    texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "farming.png");
    graphic = new GG::StaticGraphic(0, 0, icon_size, icon_size, texture, GG::GR_FITGRAPHIC);
    row = new GG::DropDownList::Row();
    row->push_back(graphic);
    m_secondary_focus_drop->Insert(row);

    texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "mining.png");
    graphic = new GG::StaticGraphic(0, 0, icon_size*3/2, icon_size*3/2, texture, GG::GR_FITGRAPHIC);
    row = new GG::DropDownList::Row();
    row->push_back(dynamic_cast<GG::Control*>(graphic));
    m_secondary_focus_drop->Insert(row);

    texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "industry.png");
    graphic = new GG::StaticGraphic(0, 0, icon_size*3/2, icon_size*3/2, texture, GG::GR_FITGRAPHIC);
    row = new GG::DropDownList::Row();
    row->push_back(dynamic_cast<GG::Control*>(graphic));
    m_secondary_focus_drop->Insert(row);

    texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "research.png");
    graphic = new GG::StaticGraphic(0, 0, icon_size*3/2, icon_size*3/2, texture, GG::GR_FITGRAPHIC);
    row = new GG::DropDownList::Row();
    row->push_back(dynamic_cast<GG::Control*>(graphic));
    m_secondary_focus_drop->Insert(row);

    texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "trade.png");
    graphic = new GG::StaticGraphic(0, 0, icon_size*3/2, icon_size*3/2, texture, GG::GR_FITGRAPHIC);
    row = new GG::DropDownList::Row();
    row->push_back(dynamic_cast<GG::Control*>(graphic));
    m_secondary_focus_drop->Insert(row);

    AttachChild(m_secondary_focus_drop);

    GG::Connect(m_primary_focus_drop->SelChangedSignal, &ResourcePanel::PrimaryFocusDropListSelectionChanged, this);
    GG::Connect(m_secondary_focus_drop->SelChangedSignal, &ResourcePanel::SecondaryFocusDropListSelectionChanged, this);
    
    // resource indicators
    m_farming_stat = new StatisticIcon(0, 0, icon_size, icon_size, (ClientUI::ArtDir() / "icons" / "farming.png").native_file_string(), GG::CLR_WHITE,
                                       0, 3, false, false);
    AttachChild(m_farming_stat);
    m_farming_stat->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_farming_stat->SetBrowseText("Food Production");

    m_mining_stat = new StatisticIcon(0, 0, icon_size, icon_size, (ClientUI::ArtDir() / "icons" / "mining.png").native_file_string(), GG::CLR_WHITE,
                                      0, 3, false, false);
    AttachChild(m_mining_stat);
    m_mining_stat->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_mining_stat->SetBrowseText("Minerals Production");

    m_industry_stat = new StatisticIcon(0, 0, icon_size, icon_size, (ClientUI::ArtDir() / "icons" / "industry.png").native_file_string(), GG::CLR_WHITE,
                                        0, 3, false, false);
    AttachChild(m_industry_stat);
    m_industry_stat->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_industry_stat->SetBrowseText("Industrial Output");

    m_research_stat = new StatisticIcon(0, 0, icon_size, icon_size, (ClientUI::ArtDir() / "icons" / "research.png").native_file_string(), GG::CLR_WHITE,
                                        0, 3, false, false);
    AttachChild(m_research_stat);
    m_research_stat->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_research_stat->SetBrowseText("Research Output");

    m_trade_stat = new StatisticIcon(0, 0, icon_size, icon_size, (ClientUI::ArtDir() / "icons" / "trade.png").native_file_string(), GG::CLR_WHITE,
                                     0, 3, false, false);
    AttachChild(m_trade_stat);
    m_trade_stat->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_trade_stat->SetBrowseText("Trade Level");

    m_construction_stat = new StatisticIcon(0, 0, icon_size, icon_size, (ClientUI::ArtDir() / "icons" / "construction.png").native_file_string(), GG::CLR_WHITE,
                                            0, 3, false, false);
    AttachChild(m_construction_stat);
    m_construction_stat->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_construction_stat->SetBrowseText("Infrastructure Construction Level");

    // meter bars
    int top = UpperLeft().y;
    int bar_width = w - m_farming_stat->Width() - icon_size*5/2 - m_expand_button->Width();
    int bar_left = m_farming_stat->Width() + icon_size*5/2;
    int bar_height = m_farming_stat->Height();

    m_farming_meter_bar = new MeterStatusBar2(bar_width, bar_height, res->FarmingMeter());
    m_farming_meter_bar->MoveTo(GG::Pt(bar_left, m_primary_focus_drop->LowerRight().y - top + icon_size/2));
    m_farming_meter_bar->Hide();

    m_mining_meter_bar = new MeterStatusBar2(bar_width, bar_height, res->MiningMeter());
    m_mining_meter_bar->MoveTo(GG::Pt(bar_left, m_farming_meter_bar->LowerRight().y - top));
    m_mining_meter_bar->Hide();

    m_industry_meter_bar = new MeterStatusBar2(bar_width, bar_height, res->IndustryMeter());
    m_industry_meter_bar->MoveTo(GG::Pt(bar_left, m_mining_meter_bar->LowerRight().y - top));
    m_industry_meter_bar->Hide();

    m_research_meter_bar = new MeterStatusBar2(bar_width, bar_height, res->ResearchMeter());
    m_research_meter_bar->MoveTo(GG::Pt(bar_left, m_industry_meter_bar->LowerRight().y - top));
    m_research_meter_bar->Hide();

    m_trade_meter_bar = new MeterStatusBar2(bar_width, bar_height, res->TradeMeter());
    m_trade_meter_bar->MoveTo(GG::Pt(bar_left, m_research_meter_bar->LowerRight().y - top));
    m_trade_meter_bar->Hide();

    m_construction_meter_bar = new MeterStatusBar2(bar_width, bar_height, res->ConstructionMeter());
    m_construction_meter_bar->MoveTo(GG::Pt(bar_left, m_trade_meter_bar->LowerRight().y - top + icon_size/2));
    m_construction_meter_bar->Hide();

    // determine if this panel has been created yet.
    std::map<int, bool>::iterator it = s_expanded_map.find(m_rescenter_id);
    if (it == s_expanded_map.end())
        s_expanded_map[m_rescenter_id] = false; // if not, default to collapsed state
    
    DoExpandCollapseLayout();
    
    // these may be redundant...
    if (System* system = obj.GetSystem())
        m_connection_system_changed = GG::Connect(system->StateChangedSignal, &ResourcePanel::Update, this);
    m_connection_rescenter_changed = GG::Connect(obj.StateChangedSignal, &ResourcePanel::Update, this);

    Update();
}

ResourcePanel::~ResourcePanel()
{
    // HACK! These disconnects should not be necessary, since PopulationPanel is derived from boost::signals::trackable, but
    // a segfault occurrs if they are not done (as of Boost 1.33.1).
    m_connection_system_changed.disconnect();
    m_connection_rescenter_changed.disconnect();

    // manually delete all pointed-to controls that may or may not be attached as a child window at time of deletion
    DetachChild(m_farming_stat);
    delete m_farming_stat;
    DetachChild(m_mining_stat);
    delete m_mining_stat;
    DetachChild(m_industry_stat);
    delete m_industry_stat;
    DetachChild(m_research_stat);
    delete m_research_stat;
    DetachChild(m_trade_stat);
    delete m_trade_stat;
    DetachChild(m_construction_stat);
    delete m_construction_stat;

    DetachChild(m_farming_meter_bar);
    delete m_farming_meter_bar;
    DetachChild(m_mining_meter_bar);
    delete m_mining_meter_bar;
    DetachChild(m_industry_meter_bar);
    delete m_industry_meter_bar;
    DetachChild(m_research_meter_bar);
    delete m_research_meter_bar;
    DetachChild(m_trade_meter_bar);
    delete m_trade_meter_bar;
    DetachChild(m_construction_meter_bar);
    delete m_construction_meter_bar;

    DetachChild(m_primary_focus_drop);
    delete m_primary_focus_drop;
    DetachChild(m_secondary_focus_drop);
    delete m_secondary_focus_drop;

    // don't need to manually delete m_expand_button, as it is attached as a child so will be deleted by ~Wnd
}

void ResourcePanel::ExpandCollapseButtonPressed()
{
    ExpandCollapse(!s_expanded_map[m_rescenter_id]);
}

void ResourcePanel::ExpandCollapse(bool expanded)
{
    if (expanded == s_expanded_map[m_rescenter_id]) return; // nothing to do
    s_expanded_map[m_rescenter_id] = expanded;

    DoExpandCollapseLayout();

    ExpandCollapseSignal();

    Render();                       // redraw in new state
}

void ResourcePanel::DoExpandCollapseLayout()
{
    int icon_size = ClientUI::Pts()*4/3;

    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_rescenter_id]) {
        m_secondary_focus_drop->Hide();
        DetachChild(m_secondary_focus_drop);
        
        m_primary_focus_drop->Hide();
        DetachChild(m_primary_focus_drop);

        /* Ordering of indicators in collapsed mode:
           - if primary focus is specialized:
             - primary focus resource goes first.
             - if secondary focus is specialized to something else
               - secondary focus goes second
        */
        
        m_farming_stat->MoveTo(GG::Pt(0, 0));
        //m_farming_stat->Hide();
        //DetachChild(m_farming_stat);

        m_mining_stat->MoveTo(GG::Pt(m_farming_stat->LowerRight().x + icon_size*2, 0));
        //m_mining_stat->Hide();
        //DetachChild(m_mining_stat);

        m_industry_stat->MoveTo(GG::Pt(m_mining_stat->LowerRight().x + icon_size*2, 0));
        //m_industry_stat->Hide();
        //DetachChild(m_industry_stat);

        m_research_stat->MoveTo(GG::Pt(m_industry_stat->LowerRight().x + icon_size*2, 0));
        //m_research_stat->Hide();
        //DetachChild(m_research_stat);

        m_trade_stat->MoveTo(GG::Pt(m_research_stat->LowerRight().x + icon_size*2, 0));
        //m_trade_stat->Hide();
        //DetachChild(m_trade_stat);

        m_construction_stat->MoveTo(GG::Pt(m_trade_stat->LowerRight().x + icon_size*2, 0));
        m_construction_stat->Hide();
        DetachChild(m_construction_stat);

        DetachChild(m_farming_meter_bar);
        m_farming_meter_bar->Hide();
        
        DetachChild(m_mining_meter_bar);
        m_mining_meter_bar->Hide();
        
        DetachChild(m_industry_meter_bar);
        m_industry_meter_bar->Hide();
        
        DetachChild(m_research_meter_bar);
        m_research_meter_bar->Hide();
        
        DetachChild(m_trade_meter_bar);
        m_trade_meter_bar->Hide();
        
        DetachChild(m_construction_meter_bar);
        m_construction_meter_bar->Hide();

        Resize(GG::Pt(Width(), icon_size));
    } else {
        m_secondary_focus_drop->Show();
        AttachChild(m_secondary_focus_drop);
        
        m_primary_focus_drop->Show();
        AttachChild(m_primary_focus_drop);

        int top = UpperLeft().y;

        m_farming_stat->MoveTo(GG::Pt(0, m_primary_focus_drop->LowerRight().y - top + icon_size/2));
        m_farming_stat->Show();
        AttachChild(m_farming_stat);

        m_mining_stat->MoveTo(GG::Pt(0, m_farming_stat->LowerRight().y - top));
        m_mining_stat->Show();
        AttachChild(m_mining_stat);

        m_industry_stat->MoveTo(GG::Pt(0, m_mining_stat->LowerRight().y - top));
        m_industry_stat->Show();
        AttachChild(m_industry_stat);

        m_research_stat->MoveTo(GG::Pt(0, m_industry_stat->LowerRight().y - top));
        m_research_stat->Show();
        AttachChild(m_research_stat);

        m_trade_stat->MoveTo(GG::Pt(0, m_research_stat->LowerRight().y - top));
        m_trade_stat->Show();
        AttachChild(m_trade_stat);

        m_construction_stat->MoveTo(GG::Pt(0, m_trade_stat->LowerRight().y - top + icon_size/2));
        m_construction_stat->Show();
        AttachChild(m_construction_stat);

        AttachChild(m_farming_meter_bar);
        m_farming_meter_bar->Show();
        
        AttachChild(m_mining_meter_bar);
        m_mining_meter_bar->Show();
        
        AttachChild(m_industry_meter_bar);
        m_industry_meter_bar->Show();
        
        AttachChild(m_research_meter_bar);
        m_research_meter_bar->Show();
        
        AttachChild(m_trade_meter_bar);
        m_trade_meter_bar->Show();
        
        AttachChild(m_construction_meter_bar);
        m_construction_meter_bar->Show();

        Resize(GG::Pt(Width(), m_construction_stat->LowerRight().y - top));
    }

    // update appearance of expand/collapse button
    if (s_expanded_map[m_rescenter_id])
    {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrownormal.png"   ), 0, 0, 32, 32));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrowclicked.png"  ), 0, 0, 32, 32));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrowmouseover.png"), 0, 0, 32, 32));
    }
    else
    {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrownormal.png"   ), 0, 0, 32, 32));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowclicked.png"  ), 0, 0, 32, 32));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowmouseover.png"), 0, 0, 32, 32));
    }
}

void ResourcePanel::MouseWheel(const GG::Pt& pt, int move, Uint32 keys)
{
    GG::Wnd *parent;
    if((parent = Parent()))
        parent->MouseWheel(pt, move, keys);
}

void ResourcePanel::Render() 
{
    // Draw outline and background...

    // copied from CUIWnd
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::Pt cl_ul = ClientUpperLeft();
    GG::Pt cl_lr = ClientLowerRight();

    // use GL to draw the lines
    glDisable(GL_TEXTURE_2D);
    GLint initial_modes[2];
    glGetIntegerv(GL_POLYGON_MODE, initial_modes);

    // draw background
    glPolygonMode(GL_BACK, GL_FILL);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndColor());
        glVertex2i(ul.x, ul.y);
        glVertex2i(lr.x, ul.y);
        glVertex2i(lr.x, lr.y);
        glVertex2i(ul.x, lr.y);
        glVertex2i(ul.x, ul.y);
    glEnd();

    // draw outer border on pixel inside of the outer edge of the window
    glPolygonMode(GL_BACK, GL_LINE);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndOuterBorderColor());
        glVertex2i(ul.x, ul.y);
        glVertex2i(lr.x, ul.y);
        glVertex2i(lr.x, lr.y);
        glVertex2i(ul.x, lr.y);
        glVertex2i(ul.x, ul.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    glEnable(GL_TEXTURE_2D);

    // draw details depending on state of ownership and expanded / collapsed status
    
    // determine ownership
    const UniverseObject* obj = GetUniverse().Object(m_rescenter_id);
    /*if(obj->Owners().empty()) 
        // uninhabited
    else
    {
        if(!obj->OwnedBy(HumanClientApp::GetApp()->EmpireID()))
            // inhabited by other empire
        else
            // inhabited by this empire (and possibly other empires)
    }*/

}

void ResourcePanel::Update()
{
    const ResourceCenter* res = GetResourceCenter();
    const UniverseObject* obj = GetUniverse().Object(m_rescenter_id);

    enum OWNERSHIP {OS_NONE, OS_FOREIGN, OS_SELF} owner = OS_NONE;

    // determine ownership
    const std::set<int> owners = obj->Owners();

    if(owners.empty()) {
        owner = OS_NONE;  // uninhabited
    } else {
        if(!obj->OwnedBy(HumanClientApp::GetApp()->EmpireID()))
            owner = OS_FOREIGN; // inhabited by other empire
        else
            owner = OS_SELF; // inhabited by this empire (and possibly other empires)
    }

    if (owner == OS_SELF) {
        m_primary_focus_drop->Disable(false);
        m_secondary_focus_drop->Disable(false);
    } else {
        m_primary_focus_drop->Disable(true);
        m_secondary_focus_drop->Disable(true);
    }

    m_farming_stat->SetValue(res->FarmingPoints());
    m_farming_meter_bar->SetProjectedCurrent(res->ProjectedCurrent(METER_FARMING));
    m_farming_meter_bar->SetProjectedMax(res->FarmingMeter().Max());
    
    m_mining_stat->SetValue(res->MiningPoints());
    m_mining_meter_bar->SetProjectedCurrent(res->ProjectedCurrent(METER_MINING));
    m_mining_meter_bar->SetProjectedMax(res->MiningMeter().Max());

    m_industry_stat->SetValue(res->IndustryPoints());
    m_industry_meter_bar->SetProjectedCurrent(res->ProjectedCurrent(METER_INDUSTRY));
    m_industry_meter_bar->SetProjectedMax(res->IndustryMeter().Max());

    m_research_stat->SetValue(res->ResearchPoints());
    m_research_meter_bar->SetProjectedCurrent(res->ProjectedCurrent(METER_RESEARCH));
    m_research_meter_bar->SetProjectedMax(res->ResearchMeter().Max());
    
    m_trade_stat->SetValue(res->TradePoints());
    m_trade_meter_bar->SetProjectedCurrent(res->ProjectedCurrent(METER_TRADE));
    m_trade_meter_bar->SetProjectedMax(res->TradeMeter().Max());

    switch (res->PrimaryFocus())
    {
    case FOCUS_BALANCED:
        m_primary_focus_drop->Select(0);  break;
    case FOCUS_FARMING:
        m_primary_focus_drop->Select(1);  break;
    case FOCUS_MINING:
        m_primary_focus_drop->Select(2);  break;
    case FOCUS_INDUSTRY:
        m_primary_focus_drop->Select(3);  break;
    case FOCUS_RESEARCH:
        m_primary_focus_drop->Select(4);  break;
    case FOCUS_TRADE:
        m_primary_focus_drop->Select(5);  break;
    }
    
    switch (res->SecondaryFocus())
    {
    case FOCUS_BALANCED:
        m_secondary_focus_drop->Select(0);  break;
    case FOCUS_FARMING:
        m_secondary_focus_drop->Select(1);  break;
    case FOCUS_MINING:
        m_secondary_focus_drop->Select(2);  break;
    case FOCUS_INDUSTRY:
        m_secondary_focus_drop->Select(3);  break;
    case FOCUS_RESEARCH:
        m_secondary_focus_drop->Select(4);  break;
    case FOCUS_TRADE:
        m_secondary_focus_drop->Select(5);  break;
    }
}

const ResourceCenter* ResourcePanel::GetResourceCenter() const
{
    const UniverseObject* obj = GetUniverse().Object(m_rescenter_id);
    if (!obj) throw std::runtime_error("ResourcePanel tried to get an object with an invalid m_rescenter_id");
    const ResourceCenter* res = dynamic_cast<const ResourceCenter*>(obj);
    if (!res) throw std::runtime_error("ResourcePanel failed casting an object pointer to a ResourceCenter pointer");
    return res;
}

ResourceCenter* ResourcePanel::GetResourceCenter()
{
    UniverseObject* obj = GetUniverse().Object(m_rescenter_id);
    if (!obj) throw std::runtime_error("ResourcePanel tried to get an object with an invalid m_rescenter_id");
    ResourceCenter* res = dynamic_cast<ResourceCenter*>(obj);
    if (!res) throw std::runtime_error("ResourcePanel failed casting an object pointer to a ResourceCenter pointer");
    return res;
}

void ResourcePanel::PrimaryFocusDropListSelectionChanged(int selected)
{
    FocusType focus;
    switch (selected) {
    case 0:
        focus = FOCUS_BALANCED;
        break;
    case 1:
        focus = FOCUS_FARMING;
        break;
    case 2:
        focus = FOCUS_MINING;
        break;
    case 3:
        focus = FOCUS_INDUSTRY;
        break;
    case 4:
        focus = FOCUS_RESEARCH;
        break;
    case 5:
        focus = FOCUS_TRADE;
        break;
    default:
        throw std::invalid_argument("PrimaryFocusDropListSelectionChanged called with invalid cell/focus selection.");
        break;
    }
    PrimaryFocusChangedSignal(focus);
}

void ResourcePanel::SecondaryFocusDropListSelectionChanged(int selected)
{
    FocusType focus;
    switch (selected) {
    case 0:
        focus = FOCUS_BALANCED;
        break;
    case 1:
        focus = FOCUS_FARMING;
        break;
    case 2:
        focus = FOCUS_MINING;
        break;
    case 3:
        focus = FOCUS_INDUSTRY;
        break;
    case 4:
        focus = FOCUS_RESEARCH;
        break;
    case 5:
        focus = FOCUS_TRADE;
        break;
    default:
        throw std::invalid_argument("SecondaryFocusDropListSelectionChanged called with invalid cell/focus selection.");
        break;
    }
    SecondaryFocusChangedSignal(focus);
}

/////////////////////////////////////
//         MeterStatusBar          //
/////////////////////////////////////
MeterStatusBar2::MeterStatusBar2(int w, int h, const Meter& meter) :
    GG::Wnd(0, 0, w, h, 0),
    m_meter(meter),
    m_initial_current(m_meter.Current()),
    m_initial_max(m_meter.Max()),
    m_projected_current(m_meter.Current()),
    m_projected_max(m_meter.Max())
{}

void MeterStatusBar2::SetProjectedCurrent(double current)
{
    assert(Meter::METER_MIN <= current && current <= Meter::METER_MAX);
    m_projected_current = current;
}
void MeterStatusBar2::SetProjectedMax(double max)
{
    assert(Meter::METER_MIN <= max && max <= Meter::METER_MAX);
    m_projected_max = max;
}

void MeterStatusBar2::Render()
{
    GG::Clr CLR_LIGHT_GRAY(192, 192, 192, 255);

    const GG::Pt MARGIN = GG::Pt(1, 2);
    GG::Pt working_space = Size() - GG::Pt(2 * MARGIN.x, 2 * MARGIN.y);
    GG::Pt main_ul = UpperLeft() + MARGIN;      // upper-left of full thickness max / current meter bars
    GG::Pt main_lr = main_ul + working_space;   // bottom-right
    GG::Pt delta_ul = GG::Pt(main_ul.x, main_ul.y + working_space.y / 4);
    GG::Pt delta_lr = GG::Pt(delta_ul.x, delta_ul.y + working_space.y / 2);
    
    int w = working_space.x;

    // outline of whole length of meter bar
    GG::FlatRectangle(main_ul.x, main_ul.y, main_lr.x, main_lr.y, GG::CLR_BLACK, GG::CLR_GRAY, 1); 

    // max value
    GG::FlatRectangle(main_ul.x, main_ul.y, main_ul.x + static_cast<int>(w * m_projected_max / (Meter::METER_MAX - Meter::METER_MIN) + 0.5), main_lr.y, GG::CLR_GRAY, GG::CLR_CYAN, 0);

    // current (initial) value
    GG::FlatRectangle(main_ul.x, main_ul.y, main_ul.x + static_cast<int>(w * m_initial_current / (Meter::METER_MAX - Meter::METER_MIN) + 0.5), main_lr.y, CLR_LIGHT_GRAY, GG::CLR_CYAN, 0);

    // projected value
    GG::Clr clr = GG::CLR_GREEN;
    if (m_projected_current < m_initial_current) clr = GG::CLR_RED;
    GG::FlatRectangle(delta_ul.x + static_cast<int>(w * m_initial_current / (Meter::METER_MAX - Meter::METER_MIN) + 0.5), delta_ul.y, 
                      delta_ul.x + static_cast<int>(w * m_projected_current / (Meter::METER_MAX - Meter::METER_MIN) + 0.5), delta_lr.y,
                      clr, GG::CLR_CYAN, 0);
}

void MeterStatusBar2::MouseWheel(const GG::Pt& pt, int move, Uint32 keys)
{
    GG::Wnd *parent;
    if((parent = Parent()))
        parent->MouseWheel(pt, move, keys);
}

/////////////////////////////////////
//         BuildingsPanel          //
/////////////////////////////////////
std::map<int, bool> BuildingsPanel::s_expanded_map = std::map<int, bool>();

BuildingsPanel::BuildingsPanel(int w, int columns, const Planet &plt) :
    GG::Wnd(0, 0, w, w, GG::CLICKABLE),
    m_planet_id(plt.ID()),
    m_columns(columns),
    m_building_indicators(),
    m_expand_button(new GG::Button(w - 16, 0, 16, 16, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), GG::CLR_WHITE))
{
    if (m_columns < 1) throw std::invalid_argument("Attempted to create a BuidingsPanel with less than 1 column");

    // expand / collapse button at top right    
    AttachChild(m_expand_button);
    m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrownormal.png"   ), 0, 0, 32, 32));
    m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowclicked.png"  ), 0, 0, 32, 32));
    m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowmouseover.png"), 0, 0, 32, 32));
    GG::Connect(m_expand_button->ClickedSignal, &BuildingsPanel::ExpandCollapseButtonPressed, this);

    // get owners, connect their production queue changed signals to update this panel
    const std::set<int>& owners = plt.Owners();
    for (std::set<int>::const_iterator it = owners.begin(); it != owners.end(); ++it) {
        const Empire* empire = Empires().Lookup(*it);
        if (!empire) continue;  // shouldn't be a problem... maybe put check for it later
        const ProductionQueue& queue = empire->GetProductionQueue();
        GG::Connect(queue.ProductionQueueChangedSignal, &BuildingsPanel::Refresh, this);
    }

    Update();
}

BuildingsPanel::~BuildingsPanel()
{
    for (std::vector<BuildingIndicator*>::iterator it = m_building_indicators.begin(); it != m_building_indicators.end(); ++it)
        delete *it;
    m_building_indicators.clear();
}

void BuildingsPanel::ExpandCollapse(bool expanded)
{
    if (expanded == s_expanded_map[m_planet_id]) return; // nothing to do
    s_expanded_map[m_planet_id] = expanded;

    DoExpandCollapseLayout();

    ExpandCollapseSignal();

    Render();                       // redraw in new state
}

void BuildingsPanel::Refresh()
{
    Update();
    ExpandCollapseSignal();
    Render();
}

void BuildingsPanel::Render()
{
    // Draw outline and background...

    // copied from CUIWnd
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::Pt cl_ul = ClientUpperLeft();
    GG::Pt cl_lr = ClientLowerRight();

    // use GL to draw the lines
    glDisable(GL_TEXTURE_2D);
    GLint initial_modes[2];
    glGetIntegerv(GL_POLYGON_MODE, initial_modes);

    // draw background
    glPolygonMode(GL_BACK, GL_FILL);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndColor());
        glVertex2i(ul.x, ul.y);
        glVertex2i(lr.x, ul.y);
        glVertex2i(lr.x, lr.y);
        glVertex2i(ul.x, lr.y);
        glVertex2i(ul.x, ul.y);
    glEnd();

    // draw outer border on pixel inside of the outer edge of the window
    glPolygonMode(GL_BACK, GL_LINE);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndOuterBorderColor());
        glVertex2i(ul.x, ul.y);
        glVertex2i(lr.x, ul.y);
        glVertex2i(lr.x, lr.y);
        glVertex2i(ul.x, lr.y);
        glVertex2i(ul.x, ul.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    glEnable(GL_TEXTURE_2D);
}

void BuildingsPanel::MouseWheel(const GG::Pt& pt, int move, Uint32 keys)
{
    GG::Wnd *parent;
    if((parent = Parent()))
        parent->MouseWheel(pt, move, keys);
}

void BuildingsPanel::Update()
{
    for (std::vector<BuildingIndicator*>::iterator it = m_building_indicators.begin(); it != m_building_indicators.end(); ++it) {
        DetachChild(*it);
        delete (*it);
    }
    m_building_indicators.clear();

    const Universe& universe = GetUniverse();
    const Planet* plt = universe.Object<Planet>(m_planet_id);
    const std::set<int>& buildings = plt->Buildings();

    const int indicator_size = static_cast<int>(static_cast<double>(Width()) / m_columns);

    // get existing / finished buildings and use them to create building indicators
    for (std::set<int>::const_iterator it = buildings.begin(); it != buildings.end(); ++it) {
        const BuildingType* building_type = universe.Object<Building>(*it)->GetBuildingType();        
        BuildingIndicator* ind = new BuildingIndicator(indicator_size, *building_type);
        m_building_indicators.push_back(ind);
    }

    // get in-progress buildings
    // may in future need to do this for all empires, but for now, just doing the empires that own the planet
    const std::set<int>& owners = plt->Owners();
    for (std::set<int>::const_iterator own_it = owners.begin(); own_it != owners.end(); ++own_it) {
        const Empire* empire = Empires().Lookup(*own_it);
        if (!empire) continue;  // shouldn't be a problem... maybe put check for it later
        const ProductionQueue& queue = empire->GetProductionQueue();
        
        int queue_index = 0;
        for (ProductionQueue::const_iterator queue_it = queue.begin(); queue_it != queue.end(); ++queue_it, ++queue_index) {
            const ProductionQueue::Element elem = *queue_it;

            BuildType type = elem.item.build_type;
            if (type != BT_BUILDING) continue;  // don't show in-progress ships in BuildingsPanel...

            const BuildingType* building_type = GetBuildingType(elem.item.name);
            
            double turn_cost;
            int turns;
            boost::tie(turn_cost, turns) = empire->ProductionCostAndTime(type, elem.item.name);
            
            double progress = empire->ProductionStatus(queue_index);
            if (progress == -1.0) progress = 0.0;

            double partial_turn = std::fmod(progress, turn_cost) / turn_cost;
            int turns_completed = static_cast<int>(progress / turn_cost);
            
            BuildingIndicator* ind = new BuildingIndicator(indicator_size, *building_type, turns, turns_completed, partial_turn);
            m_building_indicators.push_back(ind);
        }
    }
    DoExpandCollapseLayout();
}

void BuildingsPanel::ExpandCollapseButtonPressed()
{
    ExpandCollapse(!s_expanded_map[m_planet_id]);
}

void BuildingsPanel::DoExpandCollapseLayout()
{

    int row = 0;
    int column = 0;
    const int w = Width();      // horizontal space in which to place indicators
    const int padding = 5;      // space around and between adjacent indicators
    const int effective_width = w - padding * (m_columns + 1);  // padding on either side and between
    const int indicator_size = static_cast<int>(static_cast<double>(effective_width) / m_columns);
    const int icon_size = ClientUI::Pts()*4/3;
    int height;

    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_planet_id]) {
        int n = 0;
        for (std::vector<BuildingIndicator*>::iterator it = m_building_indicators.begin(); it != m_building_indicators.end(); ++it) {
            BuildingIndicator* ind = *it;
            
            int x = icon_size * n;

            if (x < (w - m_expand_button->Width() - icon_size)) {
                ind->MoveTo(GG::Pt(n*icon_size, 0));
                ind->Resize(GG::Pt(icon_size, icon_size));
                AttachChild(ind);
            } else {
                DetachChild(ind);
                ind->Hide();
            }
            ++n;
        }
        height = m_expand_button->Height();

    } else {
        for (std::vector<BuildingIndicator*>::iterator it = m_building_indicators.begin(); it != m_building_indicators.end(); ++it) {
            BuildingIndicator* ind = *it;

            int x = padding * (column + 1) + indicator_size * column;
            int y = padding * (row + 1) + indicator_size * row;
            ind->MoveTo(GG::Pt(x, y));
            
            ind->Resize(GG::Pt(indicator_size, indicator_size));
            
            AttachChild(ind);
            ind->Show();

            ++column;
            if (column >= m_columns) {
                column = 0;
                ++row;
            }
        }

        if (column == 0)
            height = padding * (row + 1) + row * indicator_size;        // if column is 0, then there are no buildings in the next row
        else
            height = padding * (row + 2) + (row + 1) * indicator_size;  // if column != 0, there are buildings in the next row, so need to make space
    }

    if (height < icon_size) height = icon_size;

    Resize(GG::Pt(Width(), height));

    // update appearance of expand/collapse button
    if (s_expanded_map[m_planet_id])
    {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrownormal.png"   ), 0, 0, 32, 32));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrowclicked.png"  ), 0, 0, 32, 32));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrowmouseover.png"), 0, 0, 32, 32));
    }
    else
    {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrownormal.png"   ), 0, 0, 32, 32));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowclicked.png"  ), 0, 0, 32, 32));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowmouseover.png"), 0, 0, 32, 32));
    }
    Wnd::MoveChildUp(m_expand_button);
}

Planet* BuildingsPanel::GetPlanet()
{
    Planet* plt = GetUniverse().Object<Planet>(m_planet_id);
    if (!plt) throw std::runtime_error("BuildingsPanel tried to get a planet with an invalid m_planet_id");
    return plt;
}

const Planet* BuildingsPanel::GetPlanet() const
{
    const Planet* plt = GetUniverse().Object<Planet>(m_planet_id);
    if (!plt) throw std::runtime_error("BuildingsPanel tried to get a planet with an invalid m_planet_id");
    return plt;
}

/////////////////////////////////////
//       BuildingIndicator         //
/////////////////////////////////////
BuildingIndicator::BuildingIndicator(int w, const BuildingType &type) :
    Wnd(0, 0, w, w, GG::CLICKABLE),
    m_type(type),
    m_graphic(0),
    m_progress_bar(0)
{
    boost::shared_ptr<GG::Texture> texture = ClientUI::GetTexture(ClientUI::ArtDir() / type.Graphic());
    m_graphic = new GG::StaticGraphic(0, 0, w, w, texture, GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
    AttachChild(m_graphic);
}

BuildingIndicator::BuildingIndicator(int w, const BuildingType &type, int turns,
                                     int turns_completed, double partial_turn) :
    Wnd(0, 0, w, w, GG::CLICKABLE),
    m_type(type),
    m_graphic(0),
    m_progress_bar(0)
{
    boost::shared_ptr<GG::Texture> texture = ClientUI::GetTexture(ClientUI::ArtDir() / type.Graphic());
    m_graphic = new GG::StaticGraphic(0, 0, w, w, texture, GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
    AttachChild(m_graphic);

    m_progress_bar = new MultiTurnProgressBar(w, w/5, turns, turns_completed, partial_turn, GG::CLR_GRAY, GG::CLR_BLACK, GG::CLR_WHITE);
    m_progress_bar->MoveTo(GG::Pt(0, Height() - m_progress_bar->Height()));
    AttachChild(m_progress_bar);
}

void BuildingIndicator::Render()
{
    // Draw outline and background...

    // copied from CUIWnd
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::Pt cl_ul = ClientUpperLeft();
    GG::Pt cl_lr = ClientLowerRight();

    // use GL to draw the lines
    glDisable(GL_TEXTURE_2D);
    GLint initial_modes[2];
    glGetIntegerv(GL_POLYGON_MODE, initial_modes);

    // draw background
    glPolygonMode(GL_BACK, GL_FILL);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndColor());
        glVertex2i(ul.x, ul.y);
        glVertex2i(lr.x, ul.y);
        glVertex2i(lr.x, lr.y);
        glVertex2i(ul.x, lr.y);
        glVertex2i(ul.x, ul.y);
    glEnd();

    // draw outer border on pixel inside of the outer edge of the window
    glPolygonMode(GL_BACK, GL_LINE);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndOuterBorderColor());
        glVertex2i(ul.x, ul.y);
        glVertex2i(lr.x, ul.y);
        glVertex2i(lr.x, lr.y);
        glVertex2i(ul.x, lr.y);
        glVertex2i(ul.x, ul.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    glEnable(GL_TEXTURE_2D);
}

void BuildingIndicator::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    Wnd::SizeMove(ul, lr);

    GG::Pt child_lr = lr - ul - GG::Pt(1, 1);   // extra pixel prevents graphic from overflowing border box
    
    if (m_graphic)
        m_graphic->SizeMove(GG::Pt(0, 0), child_lr);
    
    int bar_top = Height() * 4 / 5;
    if (m_progress_bar)
        m_progress_bar->SizeMove(GG::Pt(0, bar_top), child_lr);
}
void BuildingIndicator::MouseWheel(const GG::Pt& pt, int move, Uint32 keys)
{
    GG::Wnd *parent;
    if((parent = Parent()))
        parent->MouseWheel(pt, move, keys);
}

