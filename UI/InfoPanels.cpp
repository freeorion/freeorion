#include "InfoPanels.h"

#include "../universe/UniverseObject.h"
#include "../universe/PopCenter.h"
#include "../universe/ResourceCenter.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../universe/Building.h"
#include "../universe/Special.h"
#include "../Empire/Empire.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "../client/human/HumanClientApp.h"
#include "../util/OptionsDB.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"

#include <GG/DrawUtil.h>
#include <GG/GUI.h>
#include <GG/StaticGraphic.h>
#include <GG/BrowseInfoWnd.h>
#include <GG/StyleFactory.h>
#include <GG/WndEvent.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

using boost::lexical_cast;

namespace {
    class MeterBrowseWnd : public GG::BrowseInfoWnd {
    public:
        MeterBrowseWnd(MeterType meter_type, const UniverseObject* obj, const std::map<MeterType, std::vector<Universe::EffectAccountingInfo> >& meter_map) :
            GG::BrowseInfoWnd(0, 0, LABEL_WIDTH + VALUE_WIDTH, 1),
            m_meter_type(meter_type),
            m_obj(obj),
            m_meter_map(meter_map),
            initialized(false)
        {}

        virtual bool WndHasBrowseInfo(const Wnd* wnd, int mode) const {
            const std::vector<Wnd::BrowseInfoMode>& browse_modes = wnd->BrowseModes();
            assert(0 <= mode && mode <= static_cast<int>(browse_modes.size()));
            return true;
        }

        virtual void Render() {
            GG::Pt ul = UpperLeft();
            GG::Pt lr = LowerRight();
            GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, OpaqueColor(ClientUI::WndColor()), ClientUI::WndOuterBorderColor(), 1);    // main background
            GG::FlatRectangle(ul.x, ul.y, lr.x, ul.y + row_height, ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);    // top title filled background
            GG::FlatRectangle(ul.x, ul.y + 4*row_height, lr.x, ul.y + 5*row_height, ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);    // middle title filled background
        }

    private:
        void Initialize() {
            row_height = ClientUI::Pts()*3/2;
            const int TOTAL_WIDTH = LABEL_WIDTH + VALUE_WIDTH;
            
            const boost::shared_ptr<GG::Font>& font = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts());
            const boost::shared_ptr<GG::Font>& font_bold = GG::GUI::GetGUI()->GetFont(ClientUI::FontBold(), ClientUI::Pts());
            
            m_summary_title = new GG::TextControl(0, 0, TOTAL_WIDTH - EDGE_PAD, row_height, "", font_bold, ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
            AttachChild(m_summary_title);

            m_current_label = new GG::TextControl(0, row_height, LABEL_WIDTH, row_height, UserString("TT_CURRENT"), font, ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
            AttachChild(m_current_label);
            m_current_value = new GG::TextControl(LABEL_WIDTH, row_height, VALUE_WIDTH, row_height, "", font, ClientUI::TextColor(), GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
            AttachChild(m_current_value);

            m_next_turn_label = new GG::TextControl(0, row_height*2, LABEL_WIDTH, row_height, UserString("TT_NEXT"), font, ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
            AttachChild(m_next_turn_label);
            m_next_turn_value = new GG::TextControl(LABEL_WIDTH, row_height*2, VALUE_WIDTH, row_height, "", font, ClientUI::TextColor(), GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
            AttachChild(m_next_turn_value);

            m_change_label = new GG::TextControl(0, row_height*3, LABEL_WIDTH, row_height, UserString("TT_CHANGE"), font, ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
            AttachChild(m_change_label);
            m_change_value = new GG::TextControl(LABEL_WIDTH, row_height*3, VALUE_WIDTH, row_height, "", font, ClientUI::TextColor(), GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
            AttachChild(m_change_value);

            m_meter_title = new GG::TextControl(0, row_height*4, TOTAL_WIDTH - EDGE_PAD, row_height, "", font_bold, ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
            AttachChild(m_meter_title);

            UpdateSummary();
            UpdateEffectLabelsAndValues();
            int y = m_meter_title->LowerRight().y;
            for (unsigned int i = 0; i < m_effect_labels_and_values.size(); ++i) {
                m_effect_labels_and_values[i].first->MoveTo(GG::Pt(0, y));
                m_effect_labels_and_values[i].second->MoveTo(GG::Pt(LABEL_WIDTH, y));
                y += row_height;
            }

            Resize(GG::Pt(LABEL_WIDTH + VALUE_WIDTH, y));

            initialized = true;
        }

        virtual void UpdateImpl(int mode, const Wnd* target) {
            if (!initialized)
                Initialize();
        }

        // total resource production or amounts summary text
        void UpdateSummary() {
            const Meter* meter = m_obj->GetMeter(m_meter_type);
            if (!meter) return;
            
            double current = m_obj->MeterPoints(m_meter_type);
            double next = m_obj->ProjectedMeterPoints(m_meter_type);
            double change = next - current;
            double meter_max = meter->Max();

            m_current_value->SetText(DoubleToString(current, 2, false, false));
            m_next_turn_value->SetText(DoubleToString(next, 2, false, false));
            GG::Clr clr = ClientUI::TextColor();
            if (change > 0.0)
                clr = ClientUI::StatIncrColor();
            else if (change < 0.0)
                clr = ClientUI::StatDecrColor();
            m_change_value->SetText(GG::RgbaTag(clr) + DoubleToString(change, 2, false, true) + "</rgba>");
            m_meter_title->SetText(boost::io::str(FlexibleFormat(UserString("TT_MAX_METER")) % DoubleToString(meter_max, 2, false, false)));

            switch (m_meter_type) {
            case METER_POPULATION:
                m_summary_title->SetText(UserString("PP_POPULATION"));  break;
            case METER_FARMING:
                m_summary_title->SetText(UserString("RP_FOOD"));        break;
            case METER_INDUSTRY:
                m_summary_title->SetText(UserString("RP_INDUSTRY"));    break;
            case METER_RESEARCH:
                m_summary_title->SetText(UserString("RP_RESEARCH"));    break;
            case METER_TRADE:
               m_summary_title->SetText(UserString("RP_TRADE"));        break;
            case METER_MINING:
                m_summary_title->SetText(UserString("RP_MINERALS"));    break;
            case METER_CONSTRUCTION:
                m_summary_title->SetText(UserString("RP_CONSTRUCTION"));break;
            case METER_HEALTH:
                m_summary_title->SetText(UserString("PP_HEALTH"));      break;
            default:
                m_summary_title->SetText("");                           break;
            }
        }

        // meter effect entries
        void UpdateEffectLabelsAndValues() {
            for (unsigned int i = 0; i < m_effect_labels_and_values.size(); ++i) {
                DeleteChild(m_effect_labels_and_values[i].first);
                DeleteChild(m_effect_labels_and_values[i].second);
            }
            m_effect_labels_and_values.clear();

            const Meter* meter = m_obj->GetMeter(m_meter_type);
            if (!meter) return;
            
            // determine if meter_map contains info about the meter that this MeterBrowseWnd is describing
            std::map<MeterType, std::vector<Universe::EffectAccountingInfo> >::const_iterator meter_it = m_meter_map.find(m_meter_type);
            if (meter_it == m_meter_map.end() || meter_it->second.empty())
                return; // couldn't find appropriate meter type, or there were no entries for that meter.

            const boost::shared_ptr<GG::Font>& font = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts());

            // add label-value pairs for each alteration recorded for this meter
            const std::vector<Universe::EffectAccountingInfo>& info_vec = meter_it->second;
            for (std::vector<Universe::EffectAccountingInfo>::const_iterator info_it = info_vec.begin(); info_it != info_vec.end(); ++info_it) {
                const UniverseObject* source = GetUniverse().Object(info_it->source_id);

                int empire_id = info_it->caused_by_empire_id;
                const Empire* empire = 0;
                const Building* building = 0;
                const Planet* planet = 0;
                std::string text = "", name = "";

                switch (info_it->cause_type) {
                case ECT_UNIVERSE_TABLE_ADJUSTMENT:
                    text += UserString("TT_BASIC_FOCUS_AND_UNIVERSE");
                    break;

                case ECT_TECH:
                    if (empire_id >= 0) {
                        empire = EmpireManager().Lookup(empire_id);
                        if (empire)
                            name = empire->Name();
                    }
                    text += boost::io::str(FlexibleFormat(UserString("TT_TECH")) % name % UserString(info_it->specific_cause));
                    break;

                case ECT_BUILDING:
                    building = dynamic_cast<const Building*>(source);
                    if (building) {
                        planet = building->GetPlanet();
                        if (planet) {
                            name = planet->Name();
                        }
                    }
                    text += boost::io::str(FlexibleFormat(UserString("TT_BUILDING")) % name % UserString(info_it->specific_cause));
                    break;

                case ECT_SPECIAL:
                    text += boost::io::str(FlexibleFormat(UserString("TT_SPECIAL")) % UserString(info_it->specific_cause));
                    break;

                case ECT_UNKNOWN_CAUSE:
                default:
                    text += UserString("TT_UNKNOWN");
                }
                GG::TextControl* label = new GG::TextControl(0, 0, LABEL_WIDTH, row_height, text, font, ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
                label->Resize(GG::Pt(LABEL_WIDTH, row_height));
                AttachChild(label);
                GG::Clr clr = ClientUI::TextColor();
                if (info_it->meter_change > 0.0)
                    clr = ClientUI::StatIncrColor();
                else if (info_it->meter_change < 0.0)
                    clr = ClientUI::StatDecrColor();
                GG::TextControl* value = new GG::TextControl(VALUE_WIDTH, 0, VALUE_WIDTH, row_height, 
                                                             GG::RgbaTag(clr) + DoubleToString(info_it->meter_change, 2, false, true) + "</rgba>",
                                                             font, ClientUI::TextColor(), GG::FORMAT_CENTER | GG::FORMAT_VCENTER);
                AttachChild(value);
                m_effect_labels_and_values.push_back(std::pair<GG::TextControl*, GG::TextControl*>(label, value));
            }
        }
                
        MeterType m_meter_type;
        const UniverseObject* m_obj;
        const std::map<MeterType, std::vector<Universe::EffectAccountingInfo> >& m_meter_map;

        GG::TextControl* m_summary_title;

        GG::TextControl* m_current_label;
        GG::TextControl* m_current_value;
        GG::TextControl* m_next_turn_label;
        GG::TextControl* m_next_turn_value;
        GG::TextControl* m_change_label;
        GG::TextControl* m_change_value;

        GG::TextControl* m_meter_title;

        std::vector<std::pair<GG::TextControl*, GG::TextControl*> > m_effect_labels_and_values;

        int row_height;

        static const int LABEL_WIDTH = 300;
        static const int VALUE_WIDTH = 50;
        static const int EDGE_PAD = 3;

        bool initialized;
    };

    class IconTextBrowseWnd : public GG::BrowseInfoWnd {
    public:
        IconTextBrowseWnd(const boost::shared_ptr<GG::Texture> texture, const std::string& title_text, const std::string& main_text) :
            GG::BrowseInfoWnd(0, 0, TEXT_WIDTH + ICON_WIDTH, 1),
            ROW_HEIGHT(ClientUI::Pts()*3/2)
        {
            m_icon = new GG::StaticGraphic(0, 0, ICON_WIDTH, ICON_WIDTH, texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE, GG::CLICKABLE);
            AttachChild(m_icon);

            const boost::shared_ptr<GG::Font>& font = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts());
            const boost::shared_ptr<GG::Font>& font_bold = GG::GUI::GetGUI()->GetFont(ClientUI::FontBold(), ClientUI::Pts());

            m_title_text = new GG::TextControl(m_icon->Width() + TEXT_PAD, 0, TEXT_WIDTH, ROW_HEIGHT, UserString(title_text),
                                               font_bold, ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
            AttachChild(m_title_text);

            m_main_text = new GG::TextControl(m_icon->Width() + TEXT_PAD, ROW_HEIGHT, TEXT_WIDTH, ICON_WIDTH, UserString(main_text),
                                              font, ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_TOP | GG::FORMAT_WORDBREAK);
            AttachChild(m_main_text);

            m_main_text->SetMinSize(true);
            m_main_text->Resize(m_main_text->MinSize());
            Resize(GG::Pt(TEXT_WIDTH + ICON_WIDTH, std::max(m_icon->Height(), ROW_HEIGHT + m_main_text->Height())));
        }
        virtual bool WndHasBrowseInfo(const Wnd* wnd, int mode) const {
            const std::vector<Wnd::BrowseInfoMode>& browse_modes = wnd->BrowseModes();
            assert(0 <= mode && mode <= static_cast<int>(browse_modes.size()));
            return true;
        }

        virtual void Render() {
            GG::Pt ul = UpperLeft();
            GG::Pt lr = LowerRight();
            GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);    // main background
            GG::FlatRectangle(ul.x + ICON_WIDTH, ul.y, lr.x, ul.y + ROW_HEIGHT, ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0);    // top title filled background
        }

    private:
        GG::StaticGraphic* m_icon;
        GG::TextControl* m_title_text;
        GG::TextControl* m_main_text;

        static const int TEXT_WIDTH = 400;
        static const int TEXT_PAD = 3;
        static const int ICON_WIDTH = 64;
        const int ROW_HEIGHT;
    };

    GG::Clr MeterColor(MeterType meter_type)
    {
        switch (meter_type) {
        case METER_FARMING:
            return GG::CLR_YELLOW;
            break;
        case METER_MINING:
        case METER_HEALTH:
            return GG::CLR_RED;
            break;
        case METER_INDUSTRY:
            return GG::CLR_BLUE;
            break;
        case METER_RESEARCH:
            return GG::CLR_GREEN;
            break;
        case METER_TRADE:
            return GG::Clr(255, 148, 0, 255);   // orange
            break;
        case METER_CONSTRUCTION:
        case METER_POPULATION:
        default:
            return GG::CLR_WHITE;
        }
    }

}

/////////////////////////////////////
//        PopulationPanel          //
/////////////////////////////////////
std::map<int, bool> PopulationPanel::s_expanded_map = std::map<int, bool>();
PopulationPanel::PopulationPanel(int w, const UniverseObject &obj) :
    Wnd(0, 0, w, ClientUI::Pts()*4/3, GG::CLICKABLE),
    m_popcenter_id(obj.ID()),
    m_pop_stat(0), m_health_stat(0),
    m_multi_icon_value_indicator(0), m_multi_meter_status_bar(0),
    m_expand_button(new GG::Button(w - 16, 0, 16, 16, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), GG::CLR_WHITE, GG::CLR_ZERO, GG::ONTOP | GG::CLICKABLE))
{
    SetText("PopulationPanel");

    const PopCenter* pop = dynamic_cast<const PopCenter*>(&obj);
    if (!pop)
        throw std::invalid_argument("Attempted to construct a PopulationPanel with an UniverseObject that is not a PopCenter");

    AttachChild(m_expand_button);
    m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrownormal.png"   ), 0, 0, 32, 32));
    m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowclicked.png"  ), 0, 0, 32, 32));
    m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowmouseover.png"), 0, 0, 32, 32));
    GG::Connect(m_expand_button->ClickedSignal, &PopulationPanel::ExpandCollapseButtonPressed, this);

    int icon_size = ClientUI::Pts()*4/3;

    m_pop_stat = new StatisticIcon(0, 0, icon_size, icon_size, ClientUI::MeterIcon(METER_POPULATION),
                                   0, 3, false, false);
    AttachChild(m_pop_stat);

    m_health_stat = new StatisticIcon(w/2, 0, icon_size, icon_size, ClientUI::MeterIcon(METER_HEALTH),
                                      0, 3, false, false);
    AttachChild(m_health_stat);


    int tooltip_delay = GetOptionsDB().Get<int>("UI.tooltip-delay");
    m_pop_stat->SetBrowseModeTime(tooltip_delay);
    m_health_stat->SetBrowseModeTime(tooltip_delay);


    // meter and production indicators
    std::vector<MeterType> meters;
    meters.push_back(METER_POPULATION); meters.push_back(METER_HEALTH);

    // attach and show meter bars and large resource indicators
    int top = UpperLeft().y;

    m_multi_icon_value_indicator = new MultiIconValueIndicator(Width() - 2*EDGE_PAD, obj, meters);
    m_multi_icon_value_indicator->MoveTo(GG::Pt(EDGE_PAD, EDGE_PAD - top));
    m_multi_icon_value_indicator->Resize(GG::Pt(Width() - 2*EDGE_PAD, m_multi_icon_value_indicator->Height()));

    m_multi_meter_status_bar = new MultiMeterStatusBar(Width() - 2*EDGE_PAD, obj, meters);
    m_multi_meter_status_bar->MoveTo(GG::Pt(EDGE_PAD, m_multi_icon_value_indicator->LowerRight().y + EDGE_PAD - top));
    m_multi_meter_status_bar->Resize(GG::Pt(Width() - 2*EDGE_PAD, m_multi_meter_status_bar->Height()));


    // determine if this panel has been created yet.
    std::map<int, bool>::iterator it = s_expanded_map.find(m_popcenter_id);
    if (it == s_expanded_map.end())
        s_expanded_map[m_popcenter_id] = false; // if not, default to collapsed state
    
    Refresh();
}

PopulationPanel::~PopulationPanel()
{
    // manually delete all pointed-to controls that may or may not be attached as a child window at time of deletion
    delete m_pop_stat;
    delete m_health_stat;
    delete m_multi_icon_value_indicator;
    delete m_multi_meter_status_bar;

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
}

void PopulationPanel::DoExpandCollapseLayout()
{
    int icon_size = ClientUI::Pts()*4/3;

    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_popcenter_id]) {
        // detach / hide meter bars and large resource indicators
        DetachChild(m_multi_meter_status_bar);
        DetachChild(m_multi_icon_value_indicator);
      
        AttachChild(m_pop_stat);
        AttachChild(m_health_stat);

        Resize(GG::Pt(Width(), icon_size));
    } else {
        // detach statistic icons
        DetachChild(m_health_stat); DetachChild(m_pop_stat);

        AttachChild(m_multi_icon_value_indicator);
        AttachChild(m_multi_meter_status_bar);
        MoveChildUp(m_expand_button);

        int top = UpperLeft().y;
        Resize(GG::Pt(Width(), m_multi_meter_status_bar->LowerRight().y + EDGE_PAD - top));
    }

    // update appearance of expand/collapse button
    if (s_expanded_map[m_popcenter_id]) {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrownormal.png"   ), 0, 0, 32, 32));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrowclicked.png"  ), 0, 0, 32, 32));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "uparrowmouseover.png"), 0, 0, 32, 32));
    } else {
        m_expand_button->SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrownormal.png"   ), 0, 0, 32, 32));
        m_expand_button->SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowclicked.png"  ), 0, 0, 32, 32));
        m_expand_button->SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "downarrowmouseover.png"), 0, 0, 32, 32));
    }

    ExpandCollapseSignal();
}

void PopulationPanel::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{
    GG::Wnd *parent;
    if((parent = Parent()))
        parent->MouseWheel(pt, move, mod_keys);
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
    /*const UniverseObject* obj = GetUniverse().Object(m_popcenter_id);
    if(obj->Owners().empty()) 
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
    const Universe& universe = GetUniverse();
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
 
    m_pop_stat->SetValue(pop->MeterPoints(METER_POPULATION));
    m_health_stat->SetValue(pop->MeterPoints(METER_HEALTH));

    const Universe::EffectAccountingMap& effect_accounting_map = universe.GetEffectAccountingMap();
    const std::map<MeterType, std::vector<Universe::EffectAccountingInfo> >* meter_map = 0;
    Universe::EffectAccountingMap::const_iterator map_it = effect_accounting_map.find(m_popcenter_id);
    if (map_it != effect_accounting_map.end())
        meter_map = &(map_it->second);

    // meter bar displays and production stats
    m_multi_meter_status_bar->Update();
    m_multi_icon_value_indicator->Update();

    m_pop_stat->SetValue(pop->ProjectedMeterPoints(METER_POPULATION));
    m_health_stat->SetValue(pop->ProjectedMeterPoints(METER_HEALTH));


    // tooltips
    if (meter_map) {
        boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(METER_POPULATION, obj, *meter_map));
        m_pop_stat->SetBrowseInfoWnd(browse_wnd);
        m_multi_icon_value_indicator->SetToolTip(METER_POPULATION, browse_wnd);

        browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(METER_HEALTH, obj, *meter_map));
        m_health_stat->SetBrowseInfoWnd(browse_wnd);
        m_multi_icon_value_indicator->SetToolTip(METER_HEALTH, browse_wnd);
    }
}

void PopulationPanel::Refresh()
{
    Update();
    DoExpandCollapseLayout();
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
std::map<int, bool> ResourcePanel::s_expanded_map;

ResourcePanel::ResourcePanel(int w, const UniverseObject &obj) :
    Wnd(0, 0, w, ClientUI::Pts()*9, GG::CLICKABLE),
    m_rescenter_id(obj.ID()),
    m_farming_stat(0),
    m_mining_stat(0),
    m_industry_stat(0),
    m_research_stat(0),
    m_trade_stat(0),
    m_multi_icon_value_indicator(0),
    m_multi_meter_status_bar(0),
    m_primary_focus_drop(0),
    m_secondary_focus_drop(0),
    m_expand_button(new GG::Button(w - 16, 0, 16, 16, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), GG::CLR_WHITE))
{
    SetText("ResourcePanel");

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
    std::vector<boost::shared_ptr<GG::Texture> > textures;
    textures.push_back(ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / "balanced.png"));
    textures.push_back(ClientUI::MeterIcon(METER_FARMING));
    textures.push_back(ClientUI::MeterIcon(METER_MINING));
    textures.push_back(ClientUI::MeterIcon(METER_INDUSTRY));
    textures.push_back(ClientUI::MeterIcon(METER_RESEARCH));
    textures.push_back(ClientUI::MeterIcon(METER_TRADE));

    m_primary_focus_drop = new CUIDropDownList(0, 0, icon_size*4, icon_size*3/2, icon_size*19/2);
    for (std::vector<boost::shared_ptr<GG::Texture> >::const_iterator it = textures.begin(); it != textures.end(); ++it) {
        graphic = new GG::StaticGraphic(0, 0, icon_size*3/2, icon_size*3/2, *it, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        row = new GG::DropDownList::Row(graphic->Width(), graphic->Height(), "focus_drop");
        row->push_back(dynamic_cast<GG::Control*>(graphic));
        m_primary_focus_drop->Insert(row);
    }
    AttachChild(m_primary_focus_drop);

    m_secondary_focus_drop = new CUIDropDownList(m_primary_focus_drop->LowerRight().x + icon_size/2, 0,
                                                 icon_size*4, icon_size*3/2, icon_size*19/2);
    for (std::vector<boost::shared_ptr<GG::Texture> >::const_iterator it = textures.begin(); it != textures.end(); ++it) {
        graphic = new GG::StaticGraphic(0, 0, icon_size*3/2, icon_size*3/2, *it, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        row = new GG::DropDownList::Row(graphic->Width(), graphic->Height(), "focus_drop");
        row->push_back(dynamic_cast<GG::Control*>(graphic));
        m_secondary_focus_drop->Insert(row);
    }
    AttachChild(m_secondary_focus_drop);

    int tooltip_delay = GetOptionsDB().Get<int>("UI.tooltip-delay");
    m_primary_focus_drop->SetBrowseModeTime(tooltip_delay);
    m_secondary_focus_drop->SetBrowseModeTime(tooltip_delay);

    GG::Connect(m_primary_focus_drop->SelChangedSignal, &ResourcePanel::PrimaryFocusDropListSelectionChanged, this);
    GG::Connect(m_secondary_focus_drop->SelChangedSignal, &ResourcePanel::SecondaryFocusDropListSelectionChanged, this);
    
    // small resource indicators - for use when panel is collapsed
    m_farming_stat = new StatisticIcon(0, 0, icon_size, icon_size, ClientUI::MeterIcon(METER_FARMING),
                                       0, 3, false, false);
    AttachChild(m_farming_stat);
    m_mining_stat = new StatisticIcon(0, 0, icon_size, icon_size, ClientUI::MeterIcon(METER_MINING),
                                      0, 3, false, false);
    AttachChild(m_mining_stat);

    m_industry_stat = new StatisticIcon(0, 0, icon_size, icon_size, ClientUI::MeterIcon(METER_INDUSTRY),
                                        0, 3, false, false);
    AttachChild(m_industry_stat);

    m_research_stat = new StatisticIcon(0, 0, icon_size, icon_size, ClientUI::MeterIcon(METER_RESEARCH),
                                        0, 3, false, false);
    AttachChild(m_research_stat);

    m_trade_stat = new StatisticIcon(0, 0, icon_size, icon_size, ClientUI::MeterIcon(METER_TRADE),
                                     0, 3, false, false);
    AttachChild(m_trade_stat);


    m_farming_stat->SetBrowseModeTime(tooltip_delay);
    m_mining_stat->SetBrowseModeTime(tooltip_delay);
    m_industry_stat->SetBrowseModeTime(tooltip_delay);
    m_research_stat->SetBrowseModeTime(tooltip_delay);
    m_trade_stat->SetBrowseModeTime(tooltip_delay);


    // meter and production indicators
    std::vector<MeterType> meters;
    meters.push_back(METER_FARMING);    meters.push_back(METER_MINING); meters.push_back(METER_INDUSTRY);
    meters.push_back(METER_RESEARCH);   meters.push_back(METER_TRADE);  meters.push_back(METER_CONSTRUCTION);

    m_multi_meter_status_bar = new MultiMeterStatusBar(Width() - 2*EDGE_PAD, obj, meters);
    m_multi_icon_value_indicator = new MultiIconValueIndicator(Width() - 2*EDGE_PAD, obj, meters);

    // determine if this panel has been created yet.
    std::map<int, bool>::iterator it = s_expanded_map.find(m_rescenter_id);
    if (it == s_expanded_map.end())
        s_expanded_map[m_rescenter_id] = false; // if not, default to collapsed state
    
    Refresh();
}

ResourcePanel::~ResourcePanel()
{
    // manually delete all pointed-to controls that may or may not be attached as a child window at time of deletion
    delete m_multi_icon_value_indicator;
    delete m_multi_meter_status_bar;

    delete m_farming_stat;
    delete m_mining_stat;
    delete m_industry_stat;
    delete m_research_stat;
    delete m_trade_stat;

    delete m_primary_focus_drop;
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
}

void ResourcePanel::DoExpandCollapseLayout()
{
    int icon_size = ClientUI::Pts()*4/3;

    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_rescenter_id]) {
        DetachChild(m_secondary_focus_drop);
        DetachChild(m_primary_focus_drop);

        // detach / hide meter bars and large resource indicators
        DetachChild(m_multi_meter_status_bar);
        DetachChild(m_multi_icon_value_indicator);


        // determine which two resource icons to display while collapsed: the two with the highest production
        const ResourceCenter* res = GetResourceCenter();

        // sort by insereting into multimap keyed by production amount, then taking the first two icons therein
        std::multimap<double, StatisticIcon*> res_prod_icon_map;
        res_prod_icon_map.insert(std::pair<double, StatisticIcon*>(res->ProjectedMeterPoints(METER_FARMING),    m_farming_stat));
        res_prod_icon_map.insert(std::pair<double, StatisticIcon*>(res->ProjectedMeterPoints(METER_MINING),     m_mining_stat));
        res_prod_icon_map.insert(std::pair<double, StatisticIcon*>(res->ProjectedMeterPoints(METER_INDUSTRY),   m_industry_stat));
        res_prod_icon_map.insert(std::pair<double, StatisticIcon*>(res->ProjectedMeterPoints(METER_RESEARCH),   m_research_stat));
        res_prod_icon_map.insert(std::pair<double, StatisticIcon*>(res->ProjectedMeterPoints(METER_TRADE),      m_trade_stat));

        // initially detach all...
        for (std::multimap<double, StatisticIcon*>::iterator it = res_prod_icon_map.begin(); it != res_prod_icon_map.end(); ++it)
            DetachChild(it->second);
                
        // position and reattach icons to be shown
        int n = 0;
        for (std::multimap<double, StatisticIcon*>::iterator it = res_prod_icon_map.end(); it != res_prod_icon_map.begin();) {
            int x = icon_size*n*7/2;

            if (x > Width() - m_expand_button->Width() - icon_size*5/2) break;  // ensure icon doesn't extend past right edge of panel
            
            std::multimap<double, StatisticIcon*>::iterator it2 = --it;
            
            StatisticIcon* icon = it2->second;
            AttachChild(icon);
            icon->MoveTo(GG::Pt(n * icon_size*7/2, 0));
            icon->Show();

            n++;
        }

        Resize(GG::Pt(Width(), icon_size));
    } else {
        // detach statistic icons
        DetachChild(m_farming_stat);    DetachChild(m_mining_stat); DetachChild(m_industry_stat);
        DetachChild(m_research_stat);   DetachChild(m_trade_stat);

        // attach / show focus selector drops
        m_secondary_focus_drop->Show();
        AttachChild(m_secondary_focus_drop);
        
        m_primary_focus_drop->Show();
        AttachChild(m_primary_focus_drop);

        // attach and show meter bars and large resource indicators
        int top = UpperLeft().y;

        AttachChild(m_multi_icon_value_indicator);
        m_multi_icon_value_indicator->MoveTo(GG::Pt(EDGE_PAD, m_primary_focus_drop->LowerRight().y + EDGE_PAD - top));
        m_multi_icon_value_indicator->Resize(GG::Pt(Width() - 2*EDGE_PAD, m_multi_icon_value_indicator->Height()));

        AttachChild(m_multi_meter_status_bar);
        m_multi_meter_status_bar->MoveTo(GG::Pt(EDGE_PAD, m_multi_icon_value_indicator->LowerRight().y + EDGE_PAD - top));
        m_multi_meter_status_bar->Resize(GG::Pt(Width() - 2*EDGE_PAD, m_multi_meter_status_bar->Height()));

        Resize(GG::Pt(Width(), m_multi_meter_status_bar->LowerRight().y + EDGE_PAD - top));
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

    ExpandCollapseSignal();
}

void ResourcePanel::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{
    GG::Wnd *parent;
    if((parent = Parent()))
        parent->MouseWheel(pt, move, mod_keys);
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
    /*const UniverseObject* obj = GetUniverse().Object(m_rescenter_id);
    if(obj->Owners().empty()) 
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
    const Universe& universe = GetUniverse();
    const UniverseObject* obj = universe.Object(m_rescenter_id);

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

    const Universe::EffectAccountingMap& effect_accounting_map = universe.GetEffectAccountingMap();
    const std::map<MeterType, std::vector<Universe::EffectAccountingInfo> >* meter_map = 0;
    Universe::EffectAccountingMap::const_iterator map_it = effect_accounting_map.find(m_rescenter_id);
    if (map_it != effect_accounting_map.end())
        meter_map = &(map_it->second);

    // meter bar displays and production stats
    m_multi_meter_status_bar->Update();
    m_multi_icon_value_indicator->Update();

    m_farming_stat->SetValue(res->ProjectedMeterPoints(METER_FARMING));
    m_mining_stat->SetValue(res->ProjectedMeterPoints(METER_MINING));
    m_industry_stat->SetValue(res->ProjectedMeterPoints(METER_INDUSTRY));
    m_research_stat->SetValue(res->ProjectedMeterPoints(METER_RESEARCH));
    m_trade_stat->SetValue(res->ProjectedMeterPoints(METER_TRADE));

    // tooltips
    if (meter_map) {
        boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(METER_FARMING, obj, *meter_map));
        m_farming_stat->SetBrowseInfoWnd(browse_wnd);
        m_multi_icon_value_indicator->SetToolTip(METER_FARMING, browse_wnd);

        browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(METER_MINING, obj, *meter_map));
        m_mining_stat->SetBrowseInfoWnd(browse_wnd);
        m_multi_icon_value_indicator->SetToolTip(METER_MINING, browse_wnd);
    
        browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(METER_INDUSTRY, obj, *meter_map));
        m_industry_stat->SetBrowseInfoWnd(browse_wnd);
        m_multi_icon_value_indicator->SetToolTip(METER_INDUSTRY, browse_wnd);

        browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(METER_RESEARCH, obj, *meter_map));
        m_research_stat->SetBrowseInfoWnd(browse_wnd);
        m_multi_icon_value_indicator->SetToolTip(METER_RESEARCH, browse_wnd);

        browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(METER_TRADE, obj, *meter_map));
        m_trade_stat->SetBrowseInfoWnd(browse_wnd);
        m_multi_icon_value_indicator->SetToolTip(METER_TRADE, browse_wnd);

        browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterBrowseWnd(METER_CONSTRUCTION, obj, *meter_map));
        m_multi_icon_value_indicator->SetToolTip(METER_CONSTRUCTION, browse_wnd);
    }

    // focus droplists
    std::string text;
    switch (res->PrimaryFocus())
    {
    case FOCUS_BALANCED:
        m_primary_focus_drop->Select(0);
        text = boost::io::str(FlexibleFormat(UserString("RP_PRIMARY_FOCUS_TOOLTIP")) % UserString("FOCUS_BALANCED"));
        break;
    case FOCUS_FARMING:
        m_primary_focus_drop->Select(1);
        text = boost::io::str(FlexibleFormat(UserString("RP_PRIMARY_FOCUS_TOOLTIP")) % UserString("FOCUS_FARMING"));
        break;
    case FOCUS_MINING:
        m_primary_focus_drop->Select(2);
        text = boost::io::str(FlexibleFormat(UserString("RP_PRIMARY_FOCUS_TOOLTIP")) % UserString("FOCUS_MINING"));
        break;
    case FOCUS_INDUSTRY:
        m_primary_focus_drop->Select(3);
        text = boost::io::str(FlexibleFormat(UserString("RP_PRIMARY_FOCUS_TOOLTIP")) % UserString("FOCUS_INDUSTRY"));
        break;
    case FOCUS_RESEARCH:
        m_primary_focus_drop->Select(4);
        text = boost::io::str(FlexibleFormat(UserString("RP_PRIMARY_FOCUS_TOOLTIP")) % UserString("FOCUS_RESEARCH"));
        break;
    case FOCUS_TRADE:
        m_primary_focus_drop->Select(5);
        text = boost::io::str(FlexibleFormat(UserString("RP_PRIMARY_FOCUS_TOOLTIP")) % UserString("FOCUS_TRADE"));
        break;
    default:
        m_primary_focus_drop->Select(-1);
        text = boost::io::str(FlexibleFormat(UserString("RP_PRIMARY_FOCUS_TOOLTIP")) % UserString("FOCUS_UNKNOWN"));
        break;
    }
    m_primary_focus_drop->SetBrowseText(text);

    switch (res->SecondaryFocus())
    {
    case FOCUS_BALANCED:
        m_secondary_focus_drop->Select(0);
        text = boost::io::str(FlexibleFormat(UserString("RP_SECONDARY_FOCUS_TOOLTIP")) % UserString("FOCUS_BALANCED"));
        break;
    case FOCUS_FARMING:
        m_secondary_focus_drop->Select(1);
        text = boost::io::str(FlexibleFormat(UserString("RP_SECONDARY_FOCUS_TOOLTIP")) % UserString("FOCUS_FARMING"));
        break;
    case FOCUS_MINING:
        m_secondary_focus_drop->Select(2);
        text = boost::io::str(FlexibleFormat(UserString("RP_SECONDARY_FOCUS_TOOLTIP")) % UserString("FOCUS_MINING"));
        break;
    case FOCUS_INDUSTRY:
        m_secondary_focus_drop->Select(3);
        text = boost::io::str(FlexibleFormat(UserString("RP_SECONDARY_FOCUS_TOOLTIP")) % UserString("FOCUS_INDUSTRY"));
        break;
    case FOCUS_RESEARCH:
        m_secondary_focus_drop->Select(4);
        text = boost::io::str(FlexibleFormat(UserString("RP_SECONDARY_FOCUS_TOOLTIP")) % UserString("FOCUS_RESEARCH"));
        break;
    case FOCUS_TRADE:
        m_secondary_focus_drop->Select(5);
        text = boost::io::str(FlexibleFormat(UserString("RP_SECONDARY_FOCUS_TOOLTIP")) % UserString("FOCUS_TRADE"));
        break;
    default:
        m_secondary_focus_drop->Select(-1);
        text = boost::io::str(FlexibleFormat(UserString("RP_SECONDARY_FOCUS_TOOLTIP")) % UserString("FOCUS_UNKNOWN"));
        break;
    }
    m_secondary_focus_drop->SetBrowseText(text);
}

void ResourcePanel::Refresh()
{
    Update();
    DoExpandCollapseLayout();
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
//    MultiIconValueIndicator      //
/////////////////////////////////////
MultiIconValueIndicator::MultiIconValueIndicator(int w, const UniverseObject& obj, const std::vector<MeterType>& meter_types) :
    GG::Wnd(0, 0, w, 1, GG::CLICKABLE),
    m_icons(),
    m_meter_types(meter_types),
    m_obj_vec()
{
    m_obj_vec.push_back(&obj);

    SetText("MultiIconValueIndicator");

    int x = EDGE_PAD;
    for (std::vector<MeterType>::const_iterator it = m_meter_types.begin(); it != m_meter_types.end(); ++it) {
        boost::shared_ptr<GG::Texture> texture = ClientUI::MeterIcon(*it);
        m_icons.push_back(new StatisticIcon(x, EDGE_PAD, ICON_WIDTH, ICON_WIDTH + ClientUI::Pts()*3/2, texture,
                                            0.0, 3, false, false));
        AttachChild(m_icons.back());
        m_icons.back()->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
        x += ICON_WIDTH + ICON_SPACING;
    }
    if (!m_icons.empty())
        Resize(GG::Pt(w, EDGE_PAD + ICON_WIDTH + ClientUI::Pts()*3/2));
    Update();
}

MultiIconValueIndicator::MultiIconValueIndicator(int w, const std::vector<const UniverseObject*>& obj_vec, const std::vector<MeterType>& meter_types) :
    GG::Wnd(0, 0, w, 1, GG::CLICKABLE),
    m_icons(),
    m_meter_types(meter_types),
    m_obj_vec(obj_vec)
{
    SetText("MultiIconValueIndicator");

    int x = EDGE_PAD;
    for (std::vector<MeterType>::const_iterator it = m_meter_types.begin(); it != m_meter_types.end(); ++it) {
        boost::shared_ptr<GG::Texture> texture = ClientUI::MeterIcon(*it);
        m_icons.push_back(new StatisticIcon(x, EDGE_PAD, ICON_WIDTH, ICON_WIDTH + ClientUI::Pts()*3/2, texture,
                                            0.0, 3, false, false));
        AttachChild(m_icons.back());
        m_icons.back()->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
        x += ICON_WIDTH + ICON_SPACING;
    }
    if (!m_icons.empty())
        Resize(GG::Pt(w, EDGE_PAD + ICON_WIDTH + ClientUI::Pts()*3/2));
    Update();
}

MultiIconValueIndicator::MultiIconValueIndicator(int w) :
    GG::Wnd(0, 0, w, 1, GG::CLICKABLE),
    m_icons(),
    m_meter_types(),
    m_obj_vec()
{
    SetText("MultiIconValueIndicator");
}

void MultiIconValueIndicator::Render()
{
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // outline of whole control
    GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);
}

void MultiIconValueIndicator::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{
    GG::Wnd *parent;
    if((parent = Parent()))
        parent->MouseWheel(pt, move, mod_keys);
}

void MultiIconValueIndicator::Update()
{
    for (unsigned int i = 0; i < m_icons.size(); ++i) {
        double sum = 0.0;
        for (unsigned int j = 0; j < m_obj_vec.size(); ++j) {
            const UniverseObject* obj = m_obj_vec.at(j);
            sum += obj->ProjectedMeterPoints(m_meter_types.at(i));
        }
        m_icons.at(i)->SetValue(sum);
    }
}

void MultiIconValueIndicator::SetToolTip(MeterType meter_type, const boost::shared_ptr<GG::BrowseInfoWnd>& browse_wnd)
{
    for (unsigned int i = 0; i < m_icons.size(); ++i)
        if (m_meter_types.at(i) == meter_type)
            m_icons.at(i)->SetBrowseInfoWnd(browse_wnd);
}


/////////////////////////////////////
//       MultiMeterStatusBar       //
/////////////////////////////////////
MultiMeterStatusBar::MultiMeterStatusBar(int w, const UniverseObject& obj, const std::vector<MeterType>& meter_types) :
    GG::Wnd(0, 0, w, 1, GG::CLICKABLE),
    m_bar_shading_texture(ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "meter_bar_shading.png")),
    m_meter_types(meter_types),
    m_initial_maxes(),
    m_initial_currents(),
    m_projected_maxes(),
    m_projected_currents(),
    m_obj(obj),
    m_bar_colours()
{
    SetText("MultiMeterStatusBar");
    Update();
}

void MultiMeterStatusBar::Render()
{
    GG::Clr DARY_GREY = GG::Clr(44, 44, 44, 255);
    GG::Clr HALF_GREY = GG::Clr(128, 128, 128, 128);

    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // outline of whole control
    GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);

    const int BAR_LEFT = ClientUpperLeft().x + EDGE_PAD;
    const int BAR_RIGHT = ClientLowerRight().x - EDGE_PAD;
    const int BAR_MAX_LENGTH = BAR_RIGHT - BAR_LEFT;
    const int TOP = ClientUpperLeft().y + EDGE_PAD;
    int y = TOP;

    for (unsigned int i = 0; i < m_initial_maxes.size(); ++i) {
        // bar grey backgrounds
        GG::FlatRectangle(BAR_LEFT, y, BAR_RIGHT, y + BAR_HEIGHT, DARY_GREY, DARY_GREY, 0);

        y += BAR_HEIGHT + BAR_PAD;
    }


    // lines for 20, 40, 60, 80 %
    glDisable(GL_TEXTURE_2D);
    glColor(HALF_GREY);
    glBegin(GL_LINES);
    glVertex2i(BAR_LEFT +   BAR_MAX_LENGTH/5, TOP);
    glVertex2i(BAR_LEFT +   BAR_MAX_LENGTH/5, y - BAR_PAD);
    glVertex2i(BAR_LEFT + 2*BAR_MAX_LENGTH/5, TOP);
    glVertex2i(BAR_LEFT + 2*BAR_MAX_LENGTH/5, y - BAR_PAD);
    glVertex2i(BAR_LEFT + 3*BAR_MAX_LENGTH/5, TOP);
    glVertex2i(BAR_LEFT + 3*BAR_MAX_LENGTH/5, y - BAR_PAD);
    glVertex2i(BAR_LEFT + 4*BAR_MAX_LENGTH/5, TOP);
    glVertex2i(BAR_LEFT + 4*BAR_MAX_LENGTH/5, y - BAR_PAD);
    glEnd();
    glEnable(GL_TEXTURE_2D);


    y = TOP;
    for (unsigned int i = 0; i < m_initial_maxes.size(); ++i) {
        GG::Clr clr;

        const int MAX_RIGHT = BAR_LEFT + static_cast<int>(BAR_MAX_LENGTH * m_projected_maxes[i] / (Meter::METER_MAX - Meter::METER_MIN));
        const int BORDER = 1;
        const int BAR_BOTTOM = y + BAR_HEIGHT;

        // max value
        if (MAX_RIGHT > BAR_LEFT) {
            glColor(DarkColor(m_bar_colours[i]));
            m_bar_shading_texture->OrthoBlit(GG::Pt(BAR_LEFT, y), GG::Pt(MAX_RIGHT, BAR_BOTTOM));
        }

        const int CUR_RIGHT = BAR_LEFT + static_cast<int>(BAR_MAX_LENGTH * m_initial_currents[i] / (Meter::METER_MAX - Meter::METER_MIN));
        const int PROJECTED_RIGHT = BAR_LEFT + static_cast<int>(BAR_MAX_LENGTH * m_projected_currents[i] / (Meter::METER_MAX - Meter::METER_MIN));
        const int PROJECTED_TOP = y + 3*EDGE_PAD/2;

        GG::Clr projected_clr = ClientUI::StatIncrColor();
        if (m_projected_currents[i] < m_initial_currents[i]) projected_clr = ClientUI::StatDecrColor();

        if (PROJECTED_RIGHT > CUR_RIGHT) {
            // projected border
            glColor(GG::CLR_BLACK);
            GG::FlatRectangle(CUR_RIGHT, PROJECTED_TOP,     PROJECTED_RIGHT + 1, BAR_BOTTOM, GG::CLR_BLACK, GG::CLR_BLACK, 0);
            // projected colour bar
            GG::FlatRectangle(CUR_RIGHT, PROJECTED_TOP + 1, PROJECTED_RIGHT,     BAR_BOTTOM, projected_clr, projected_clr, 0);
            // current value
            glColor(m_bar_colours[i]);
            m_bar_shading_texture->OrthoBlit(GG::Pt(BAR_LEFT, y), GG::Pt(CUR_RIGHT, BAR_BOTTOM));
            // black border
            GG::FlatRectangle(BAR_LEFT - BORDER, y - BORDER, MAX_RIGHT + BORDER, BAR_BOTTOM + BORDER, GG::CLR_ZERO, GG::CLR_BLACK, 1);
        } else {
            // current value
            glColor(m_bar_colours[i]);
            m_bar_shading_texture->OrthoBlit(GG::Pt(BAR_LEFT, y), GG::Pt(CUR_RIGHT, BAR_BOTTOM));
            if (PROJECTED_RIGHT < CUR_RIGHT) {
                // projected border
                glColor(GG::CLR_BLACK);
                GG::FlatRectangle(PROJECTED_RIGHT - 1, PROJECTED_TOP,     CUR_RIGHT, BAR_BOTTOM, GG::CLR_BLACK, GG::CLR_BLACK, 0);
                // projected colour bar
                glColor(m_bar_colours[i]);
                GG::FlatRectangle(PROJECTED_RIGHT,     PROJECTED_TOP + 1, CUR_RIGHT, BAR_BOTTOM, projected_clr, projected_clr, 0);
            }
            // black border
            GG::FlatRectangle(BAR_LEFT - BORDER, y - BORDER, CUR_RIGHT + BORDER, BAR_BOTTOM + BORDER, GG::CLR_ZERO, GG::CLR_BLACK, 1);
        }

        y += BAR_HEIGHT + BAR_PAD;
    }
}

void MultiMeterStatusBar::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{
    GG::Wnd *parent;
    if((parent = Parent()))
        parent->MouseWheel(pt, move, mod_keys);
}

void MultiMeterStatusBar::Update()
{
    std::vector<const Meter*> meters;
    for (std::vector<MeterType>::const_iterator it = m_meter_types.begin(); it != m_meter_types.end(); ++it) {
        const Meter* meter = m_obj.GetMeter(*it);
        if (!meter) 
            throw std::runtime_error("MultiMeterStatusBar::Update() tried to get a meter from and object that didn't have a meter of the specified type");
        meters.push_back(meter);
    }
    const int NUM_BARS = meters.size();

    const int HEIGHT = NUM_BARS*BAR_HEIGHT + (NUM_BARS - 1)*BAR_PAD + 2*EDGE_PAD;

    m_initial_maxes.clear();
    m_initial_currents.clear();
    m_projected_maxes.clear();
    m_projected_currents.clear();
    for (int i = 0; i < NUM_BARS; ++i) {
        const Meter* meter = meters[i];
        m_initial_maxes.push_back(meter->InitialMax());
        m_initial_currents.push_back(meter->InitialCurrent());
        m_projected_maxes.push_back(meter->Max());
        m_projected_currents.push_back(m_obj.ProjectedCurrentMeter(m_meter_types[i]));
        m_bar_colours.push_back(MeterColor(m_meter_types[i]));
    }

    Resize(GG::Pt(Width(), HEIGHT));
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
    SetText("BuildingsPanel");

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

    Refresh();
}

BuildingsPanel::~BuildingsPanel()
{
    // delete building indicators
    for (std::vector<BuildingIndicator*>::iterator it = m_building_indicators.begin(); it != m_building_indicators.end(); ++it)
        delete *it;
    m_building_indicators.clear();
}

void BuildingsPanel::ExpandCollapse(bool expanded)
{
    if (expanded == s_expanded_map[m_planet_id]) return; // nothing to do
    s_expanded_map[m_planet_id] = expanded;

    DoExpandCollapseLayout();
}

void BuildingsPanel::Render()
{
    if (Height() < 1) return;   // don't render if empty
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

void BuildingsPanel::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{
    GG::Wnd *parent;
    if((parent = Parent()))
        parent->MouseWheel(pt, move, mod_keys);
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
            int location = elem.location;
            if (location != plt->ID()) continue;    // don't show buildings located elsewhere

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
}

void BuildingsPanel::Refresh()
{
    Update();
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

    if (m_building_indicators.empty()) {
        height = 0;  // hide if empty
        DetachChild(m_expand_button);
    } else {
        AttachChild(m_expand_button);
        m_expand_button->Show();
        if (height < icon_size) height = icon_size;
    }

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

    ExpandCollapseSignal();
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
    boost::shared_ptr<GG::Texture> texture = ClientUI::BuildingTexture(type.Name());

    SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(new IconTextBrowseWnd(texture, type.Name(), type.Description())));
        
    m_graphic = new GG::StaticGraphic(0, 0, w, w, texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    AttachChild(m_graphic);
}

BuildingIndicator::BuildingIndicator(int w, const BuildingType &type, int turns,
                                     int turns_completed, double partial_turn) :
    Wnd(0, 0, w, w, GG::CLICKABLE),
    m_type(type),
    m_graphic(0),
    m_progress_bar(0)
{
    boost::shared_ptr<GG::Texture> texture = ClientUI::BuildingTexture(type.Name());

    SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(new IconTextBrowseWnd(texture, type.Name(), type.Description())));

    m_graphic = new GG::StaticGraphic(0, 0, w, w, texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
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
void BuildingIndicator::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{
    GG::Wnd *parent;
    if((parent = Parent()))
        parent->MouseWheel(pt, move, mod_keys);
}

/////////////////////////////////////
//         SpecialsPanel           //
/////////////////////////////////////
SpecialsPanel::SpecialsPanel(int w, const UniverseObject &obj) : 
    Wnd(0, 0, w, 32, GG::CLICKABLE),
    m_object_id(obj.ID()),
    m_icons()
{
    SetText("SpecialsPanel");

    Update();
}

bool SpecialsPanel::InWindow(const GG::Pt& pt) const
{
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
{
    GG::Wnd *parent;
    if((parent = Parent()))
        parent->MouseWheel(pt, move, mod_keys);
}

void SpecialsPanel::Update()
{
    for (std::vector<GG::StaticGraphic*>::iterator it = m_icons.begin(); it != m_icons.end(); ++it)
        DeleteChild(*it);
    m_icons.clear();

    const UniverseObject* obj = GetObject();
    const std::set<std::string>& specials = obj->Specials();

    const int icon_size = 24;

    int tooltip_time = GetOptionsDB().Get<int>("UI.tooltip-delay");

    // get specials and use them to create specials icons
    for (std::set<std::string>::const_iterator it = specials.begin(); it != specials.end(); ++it) {
        const Special* special = GetSpecial(*it);
        GG::StaticGraphic* graphic = new GG::StaticGraphic(0, 0, icon_size, icon_size, ClientUI::SpecialTexture(special->Name()), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE, GG::CLICKABLE);
        graphic->SetBrowseModeTime(tooltip_time);
        graphic->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(new IconTextBrowseWnd(ClientUI::SpecialTexture(special->Name()), special->Name(), special->Description())));
        m_icons.push_back(graphic);
    }

    const int AVAILABLE_WIDTH = Width() - EDGE_PAD;
    int x = EDGE_PAD;
    int y = EDGE_PAD;

    for (std::vector<GG::StaticGraphic*>::iterator it = m_icons.begin(); it != m_icons.end(); ++it) {
        GG::StaticGraphic* icon = *it;
        icon->MoveTo(GG::Pt(x, y));
        AttachChild(icon);

        x += icon_size + EDGE_PAD;

        if (x + icon_size + EDGE_PAD > AVAILABLE_WIDTH) {
            x = EDGE_PAD;
            y += icon_size + EDGE_PAD;
        }
    }

    if (m_icons.empty()) {
        Resize(GG::Pt(Width(), 0));
    } else {
        Resize(GG::Pt(Width(), y + icon_size + EDGE_PAD*2));
    }
}

UniverseObject* SpecialsPanel::GetObject()
{
    UniverseObject* obj = GetUniverse().Object(m_object_id);
    if (!obj) throw std::runtime_error("SpecialsPanel tried to get a planet with an invalid m_object_id");
    return obj;
}

const UniverseObject* SpecialsPanel::GetObject() const
{
    const UniverseObject* obj = GetUniverse().Object(m_object_id);
    if (!obj) throw std::runtime_error("SpecialsPanel tried to get a planet with an invalid m_object_id");
    return obj;
}
