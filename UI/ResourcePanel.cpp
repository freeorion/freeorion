#include "ResourcePanel.h"

#include <GG/Button.h>

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../universe/Effect.h"
#include "../universe/ResourceCenter.h"
#include "../universe/TemporaryPtr.h"
#include "../universe/UniverseObject.h"
#include "../client/human/HumanClientApp.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "MeterBrowseWnd.h"
#include "MultiIconValueIndicator.h"
#include "MultiMeterStatusBar.h"

namespace {
    /** Returns text wrapped in GG RGBA tags for specified colour */
    std::string ColourWrappedtext(const std::string& text, const GG::Clr colour)
    { return GG::RgbaTag(colour) + text + "</rgba>"; }

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

    const int       EDGE_PAD(3);

    const GG::X     METER_BROWSE_LABEL_WIDTH(300);
    const GG::X     METER_BROWSE_VALUE_WIDTH(50);

    /** How big we want meter icons with respect to the current UI font size.
      * Meters should scale along font size, but not below the size for the
      * default 12 points font. */
    GG::Pt MeterIconSize() {
        const int icon_size = std::max(ClientUI::Pts(), 12) * 4/3;
        return GG::Pt(GG::X(icon_size), GG::Y(icon_size));
    }

    const std::string& MeterToUserString(MeterType meter_type) {
        return UserString(EnumToString(meter_type));
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

        CUILabel*               m_summary_title;

        std::vector<std::pair<CUILabel*, CUILabel*> >
                                m_effect_labels_and_values;

        GG::Y                   m_row_height;
        bool                    m_initialized;
    };

    //////////////////////////////////////
    // MeterModifiersIndicatorBrowseWnd //
    //////////////////////////////////////
    MeterModifiersIndicatorBrowseWnd::MeterModifiersIndicatorBrowseWnd(int object_id, MeterType meter_type) :
        GG::BrowseInfoWnd(GG::X0, GG::Y0, METER_BROWSE_LABEL_WIDTH + METER_BROWSE_VALUE_WIDTH, GG::Y1),
        m_meter_type(meter_type),
        m_source_object_id(object_id),
        m_summary_title(0),
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

        m_summary_title = new CUILabel("", GG::FORMAT_RIGHT);
        m_summary_title->MoveTo(GG::Pt(GG::X0, top));
        m_summary_title->Resize(GG::Pt(TOTAL_WIDTH - EDGE_PAD, m_row_height));
        m_summary_title->SetFont(ClientUI::GetBoldFont());
        AttachChild(m_summary_title);
        top += m_row_height;

        // clear existing labels
        for (unsigned int i = 0; i < m_effect_labels_and_values.size(); ++i) {
            DeleteChild(m_effect_labels_and_values[i].first);
            DeleteChild(m_effect_labels_and_values[i].second);
        }
        m_effect_labels_and_values.clear();

        // set summary label
        TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_source_object_id);
        if (!obj) {
            ErrorLogger() << "MeterModifiersIndicatorBrowseWnd couldn't get object with id " << m_source_object_id;
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
        for (ObjectMap::const_iterator<> obj_it = objects.const_begin();
                obj_it != objects.const_end(); ++obj_it)
        {
            int target_object_id = obj_it->ID();
            TemporaryPtr<const UniverseObject> target_obj = *obj_it;
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

                CUILabel* label = new CUILabel(target_obj->Name(), GG::FORMAT_RIGHT);
                label->MoveTo(GG::Pt(GG::X0, top));
                label->Resize(GG::Pt(METER_BROWSE_LABEL_WIDTH, m_row_height));
                AttachChild(label);

                CUILabel* value = new CUILabel(ColouredNumber(account_it->meter_change));
                value->MoveTo(GG::Pt(METER_BROWSE_LABEL_WIDTH, top));
                value->Resize(GG::Pt(METER_BROWSE_VALUE_WIDTH, m_row_height));
                AttachChild(value);
                m_effect_labels_and_values.push_back(std::pair<CUILabel*, CUILabel*>(label, value));

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

std::map<int, bool> ResourcePanel::s_expanded_map;

ResourcePanel::ResourcePanel(GG::X w, int object_id) :
    AccordionPanel(w),
    m_rescenter_id(object_id),
    //m_pop_mod_stat(0),
    m_industry_stat(0),
    m_research_stat(0),
    m_trade_stat(0),
    m_construction_stat(0),
    m_multi_icon_value_indicator(0),
    m_multi_meter_status_bar(0)
{
    SetName("ResourcePanel");

    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_rescenter_id);
    if (!obj)
        throw std::invalid_argument("Attempted to construct a ResourcePanel with an object_id that is not an UniverseObject");
    TemporaryPtr<const ResourceCenter> res = boost::dynamic_pointer_cast<const ResourceCenter>(obj);
    if (!res)
        throw std::invalid_argument("Attempted to construct a ResourcePanel with an UniverseObject that is not a ResourceCenter");

    SetChildClippingMode(ClipToClient);

    GG::Connect(m_expand_button->LeftClickedSignal, &ResourcePanel::ExpandCollapseButtonPressed, this);

    // small resource indicators - for use when panel is collapsed
    m_industry_stat = new StatisticIcon(ClientUI::MeterIcon(METER_INDUSTRY), 0, 3, false);
    AttachChild(m_industry_stat);

    m_research_stat = new StatisticIcon(ClientUI::MeterIcon(METER_RESEARCH), 0, 3, false);
    AttachChild(m_research_stat);

    m_construction_stat = new StatisticIcon(ClientUI::MeterIcon(METER_CONSTRUCTION), 0, 3, false);
    AttachChild(m_construction_stat);

    m_trade_stat = new StatisticIcon(ClientUI::MeterIcon(METER_TRADE), 0, 3, false);
    AttachChild(m_trade_stat);


    // meter and production indicators
    std::vector<std::pair<MeterType, MeterType> > meters;
    meters.push_back(std::make_pair(METER_INDUSTRY,     METER_TARGET_INDUSTRY));
    meters.push_back(std::make_pair(METER_RESEARCH,     METER_TARGET_RESEARCH));
    meters.push_back(std::make_pair(METER_CONSTRUCTION, METER_TARGET_CONSTRUCTION));
    meters.push_back(std::make_pair(METER_TRADE,        METER_TARGET_TRADE));

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

    delete m_industry_stat;
    delete m_research_stat;
    delete m_trade_stat;
    delete m_construction_stat;
}

void ResourcePanel::ExpandCollapseButtonPressed() {
    ExpandCollapse(!s_expanded_map[m_rescenter_id]);
}

void ResourcePanel::ExpandCollapse(bool expanded) {
    if (expanded == s_expanded_map[m_rescenter_id]) return; // nothing to do
    s_expanded_map[m_rescenter_id] = expanded;

    DoLayout();
}

void ResourcePanel::DoLayout() {
    // initially detach everything (most things?).  Some will be reattached later.
    DetachChild(m_industry_stat);   DetachChild(m_research_stat);
    DetachChild(m_trade_stat);      DetachChild(m_construction_stat);

    DetachChild(m_multi_meter_status_bar);
    DetachChild(m_multi_icon_value_indicator);

    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_rescenter_id);

    // update size of panel and position and visibility of widgets
    if (!s_expanded_map[m_rescenter_id]) {
        TemporaryPtr<const ResourceCenter> res = boost::dynamic_pointer_cast<const ResourceCenter>(obj);

        if (res) {
            // determine which two resource icons to display while collapsed: the two with the highest production.
            // sort by insereting into multimap keyed by production amount, then taking the first two icons therein.
            // add a slight offest to the sorting key to control order in case of ties
            std::multimap<double, StatisticIcon*> res_prod_icon_map;
            res_prod_icon_map.insert(std::pair<double, StatisticIcon*>(m_industry_stat->GetValue()+0.0003,     m_industry_stat));
            res_prod_icon_map.insert(std::pair<double, StatisticIcon*>(m_research_stat->GetValue()+0.0002,     m_research_stat));
            res_prod_icon_map.insert(std::pair<double, StatisticIcon*>(m_trade_stat->GetValue(),        m_trade_stat));
            res_prod_icon_map.insert(std::pair<double, StatisticIcon*>(m_construction_stat->GetValue()+0.0001, m_construction_stat));

            // position and reattach icons to be shown
            int n = 0;
            for (std::multimap<double, StatisticIcon*>::iterator it = res_prod_icon_map.end(); it != res_prod_icon_map.begin();) {
                GG::X x = MeterIconSize().x*n*7/2;

                if (x > Width() - m_expand_button->Width() - MeterIconSize().x*5/2) break;  // ensure icon doesn't extend past right edge of panel

                std::multimap<double, StatisticIcon*>::iterator it2 = --it;

                StatisticIcon* icon = it2->second;
                AttachChild(icon);
                GG::Pt icon_ul(x, GG::Y0);
                GG::Pt icon_lr = icon_ul + MeterIconSize();
                icon->SizeMove(icon_ul, icon_lr);
                icon->Show();

                n++;
            }
        }

        Resize(GG::Pt(Width(), std::max(MeterIconSize().y, m_expand_button->Height())));
    } else {
        GG::Y top = GG::Y0;

        // attach and show meter bars and large resource indicators
        AttachChild(m_multi_icon_value_indicator);
        m_multi_icon_value_indicator->MoveTo(GG::Pt(GG::X(EDGE_PAD), top));
        m_multi_icon_value_indicator->Resize(GG::Pt(Width() - 2*EDGE_PAD, m_multi_icon_value_indicator->Height()));
        top += m_multi_icon_value_indicator->Height() + EDGE_PAD;

        AttachChild(m_multi_meter_status_bar);
        m_multi_meter_status_bar->MoveTo(GG::Pt(GG::X(EDGE_PAD), top));
        m_multi_meter_status_bar->Resize(GG::Pt(Width() - 2*EDGE_PAD, m_multi_meter_status_bar->Height()));
        top += m_multi_icon_value_indicator->Height() + EDGE_PAD;

        MoveChildUp(m_expand_button);

        Resize(GG::Pt(Width(), top));
    }

    m_expand_button->MoveTo(GG::Pt(Width() - m_expand_button->Width(), GG::Y0));

    SetCollapsed(!s_expanded_map[m_rescenter_id]);
}

void ResourcePanel::Render() {
    // Draw outline and background...
    GG::FlatRectangle(UpperLeft(), LowerRight(), ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);

    // draw details depending on state of ownership and expanded / collapsed status

    // determine ownership
    /*TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_rescenter_id);
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

    AccordionPanel::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        DoLayout();
}

void ResourcePanel::Update() {
    // remove any old browse wnds
    m_industry_stat->ClearBrowseInfoWnd();
    m_multi_icon_value_indicator->ClearToolTip(METER_INDUSTRY);

    m_research_stat->ClearBrowseInfoWnd();
    m_multi_icon_value_indicator->ClearToolTip(METER_RESEARCH);

    m_trade_stat->ClearBrowseInfoWnd();
    m_multi_icon_value_indicator->ClearToolTip(METER_TRADE);

    m_construction_stat->ClearBrowseInfoWnd();
    m_multi_icon_value_indicator->ClearToolTip(METER_CONSTRUCTION);


    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(m_rescenter_id);
    if (!obj) {
        ErrorLogger() << "BuildingPanel::Update couldn't get object with id " << m_rescenter_id;
        return;
    }


    //std::cout << "ResourcePanel::Update() object: " << obj->Name() << std::endl;
    //DebugLogger() << "ResourcePanel::Update()";
    //DebugLogger() << obj->Dump();

    // meter bar displays and production stats
    m_multi_meter_status_bar->Update();
    m_multi_icon_value_indicator->Update();

    m_industry_stat->SetValue(obj->InitialMeterValue(METER_INDUSTRY));
    m_research_stat->SetValue(obj->InitialMeterValue(METER_RESEARCH));
    m_trade_stat->SetValue(obj->InitialMeterValue(METER_TRADE));
    m_construction_stat->SetValue(obj->InitialMeterValue(METER_CONSTRUCTION));


    // create an attach browse info wnds for each meter type on the icon + number stats used when collapsed and
    // for all meter types shown in the multi icon value indicator.  this replaces any previous-present
    // browse wnd on these indicators
    boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd;

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
    m_construction_stat->SetBrowseInfoWnd(browse_wnd);
    m_multi_icon_value_indicator->SetToolTip(METER_CONSTRUCTION, browse_wnd);

    //browse_wnd = boost::shared_ptr<GG::BrowseInfoWnd>(new MeterModifiersIndicatorBrowseWnd(m_rescenter_id, METER_TARGET_POPULATION));
    //m_pop_mod_stat->SetBrowseInfoWnd(browse_wnd);
}

void ResourcePanel::Refresh() {
    Update();
    DoLayout();
}

void ResourcePanel::EnableOrderIssuing(bool enable/* = true*/)
{}
