#include "DesignWnd.h"

#include "../util/AppInterface.h"
#include "ClientUI.h"
#include "CUIWnd.h"
#include "CUIControls.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/ShipDesign.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>

#include <boost/format.hpp>
#include <cmath>

namespace {
    // functor connecting slot droplists from which parts are selected to adding parts to the design.
    // external is true if the list is for an external slot, or false if the list is for an internal slot
    // slot is the index of the slot in the external or internal array of parts (and droplists)
    // droplist_index is the index of the selected item in the droplist
    struct SelectPartFunctor {
        SelectPartFunctor(DesignWnd* design_wnd, const std::map<int, std::string>& part_names_by_row_index,
                          unsigned int slot) :
            m_design_wnd(design_wnd),
            m_part_names_by_row_index(part_names_by_row_index),
            m_slot(slot)
        {}

        void operator()(int droplist_index) {
            std::string part_name = "";
            std::map<int, std::string>::const_iterator it = m_part_names_by_row_index.find(droplist_index);
            if (it != m_part_names_by_row_index.end())
                part_name = it->second;
            m_design_wnd->PartSelected(part_name, m_slot);
        }

        DesignWnd* const m_design_wnd;
        const std::map<int, std::string> m_part_names_by_row_index;
        unsigned int m_slot;
    };

    // functor connecting hull droplist from hull is selected to setting design hull
    // droplist_index is the index of the selected item in the droplist
    struct SelectHullFunctor {
        SelectHullFunctor(DesignWnd* design_wnd, const std::map<int, std::string>& hull_names_by_row_index) :
            m_design_wnd(design_wnd),
            m_hull_names_by_row_index(hull_names_by_row_index)
        {}

        void operator()(int droplist_index) {
            std::string hull_name = "";
            std::map<int, std::string>::const_iterator it = m_hull_names_by_row_index.find(droplist_index);
            if (it != m_hull_names_by_row_index.end())
                hull_name = it->second;
            m_design_wnd->HullSelected(hull_name);
        }

        DesignWnd* const m_design_wnd;
        const std::map<int, std::string> m_hull_names_by_row_index;
    };
}

//////////////////////////////////////////////////
// DesignWnd::DetailPanel                       //
//////////////////////////////////////////////////
class DesignWnd::DetailPanel : public CUIWnd {
public:
    DetailPanel(int w, int h);

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    void Render();
    void LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys);

    /* need to redefine this so that icons and name can be put at the top of the Wnd, rather
       than being restricted to the client area of a CUIWnd */
    GG::Pt ClientUpperLeft() const;

    void SetItem(const PartType* part);
    void SetItem(const HullType* hull);

private:
    static const int TEXT_MARGIN_X = 3;
    static const int TEXT_MARGIN_Y = 3;

    void Reset();
    void DoLayout();

    const PartType*     m_part;
    const HullType*     m_hull;
    GG::TextControl*    m_name_text;        // name
    GG::TextControl*    m_cost_text;        // cost and build time
    GG::TextControl*    m_summary_text;     // general purpose of part or hull (eg. stealth, colonization, short range)
    CUIMultiEdit*       m_description_box;  // detailed and lengthy description
    GG::StaticGraphic*  m_graphic;
};

DesignWnd::DetailPanel::DetailPanel(int w, int h) :
    CUIWnd("", 1, 1, w - 1, h - 1, GG::ONTOP | GG::CLICKABLE | GG::DRAGABLE | GG::RESIZABLE),
    m_part(0),
    m_hull(0)
{
    const int PTS = ClientUI::Pts();
    const int NAME_PTS = PTS*3/2;
    const int COST_PTS = PTS;
    const int SUMMARY_PTS = PTS*4/3;

    m_name_text =       new GG::TextControl(0, 0, 10, 10, "", GG::GUI::GetGUI()->GetFont(ClientUI::FontBold(),  NAME_PTS),      ClientUI::TextColor());
    m_cost_text =       new GG::TextControl(0, 0, 10, 10, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(),      COST_PTS),      ClientUI::TextColor());
    m_summary_text =    new GG::TextControl(0, 0, 10, 10, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(),      SUMMARY_PTS),   ClientUI::TextColor());
    m_description_box =    new CUIMultiEdit(0, 0, 10, 10, "", GG::MULTI_WORDBREAK | GG::MULTI_READ_ONLY);
    m_description_box->SetColor(GG::CLR_ZERO);
    m_description_box->SetInteriorColor(GG::CLR_ZERO);

    m_graphic = 0;

    AttachChild(m_name_text);
    AttachChild(m_cost_text);
    AttachChild(m_summary_text);
    AttachChild(m_description_box);

    DoLayout();
}

void DesignWnd::DetailPanel::DoLayout() {
    const int PTS = ClientUI::Pts();
    const int NAME_PTS = PTS*3/2;
    const int COST_PTS = PTS;
    const int SUMMARY_PTS = PTS*4/3;

    const int ICON_SIZE = 12 + NAME_PTS + COST_PTS + SUMMARY_PTS;

    // name
    GG::Pt ul = GG::Pt(0, 0);
    GG::Pt lr = ul + GG::Pt(Width(), NAME_PTS + 4);
    m_name_text->SizeMove(ul, lr);

    // cost / turns
    ul += GG::Pt(0, m_name_text->Height());
    lr = ul + GG::Pt(Width(), COST_PTS + 4);
    m_cost_text->SizeMove(ul, lr);

    // one line summary
    ul += GG::Pt(0, m_cost_text->Height());
    lr = ul + GG::Pt(Width(), SUMMARY_PTS + 4);
    m_summary_text->SizeMove(ul, lr);

    // main verbose description (fluff, effects, unlocks, ...)
    ul = GG::Pt(1, ICON_SIZE + TEXT_MARGIN_Y + 1);
    lr = ul + GG::Pt(Width() - TEXT_MARGIN_X - BORDER_RIGHT, Height() - BORDER_BOTTOM - ul.y - TEXT_MARGIN_Y);
    m_description_box->SizeMove(ul, lr);

    // icons
    if (m_graphic) {
        ul = GG::Pt(1, 1);
        lr = ul + GG::Pt(ICON_SIZE, ICON_SIZE);
        m_graphic->SizeMove(ul, lr);
    }
}

void DesignWnd::DetailPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::LowerRight() - GG::Wnd::UpperLeft();

    // maybe later do something interesting with docking
    GG::Wnd::SizeMove(ul, lr);

    if (Visible() && old_size != GG::Wnd::Size())
        DoLayout();
}

GG::Pt DesignWnd::DetailPanel::ClientUpperLeft() const {
    return GG::Wnd::UpperLeft();
}

void DesignWnd::DetailPanel::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    const int ICON_SIZE = m_summary_text->LowerRight().y - m_name_text->UpperLeft().y;
    GG::Pt cl_ul = ul + GG::Pt(BORDER_LEFT, ICON_SIZE + BORDER_BOTTOM);
    GG::Pt cl_lr = lr - GG::Pt(BORDER_RIGHT, BORDER_BOTTOM);

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
        glVertex2i(lr.x, lr.y - OUTER_EDGE_ANGLE_OFFSET);
        glVertex2i(lr.x - OUTER_EDGE_ANGLE_OFFSET, lr.y);
        glVertex2i(ul.x, lr.y);
        glVertex2i(ul.x, ul.y);
    glEnd();

    // draw outer border on pixel inside of the outer edge of the window
    glPolygonMode(GL_BACK, GL_LINE);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndOuterBorderColor());
        glVertex2i(ul.x, ul.y);
        glVertex2i(lr.x, ul.y);
        glVertex2i(lr.x, lr.y - OUTER_EDGE_ANGLE_OFFSET);
        glVertex2i(lr.x - OUTER_EDGE_ANGLE_OFFSET, lr.y);
        glVertex2i(ul.x, lr.y);
        glVertex2i(ul.x, ul.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    // draw inner border, including extra resize-tab lines
    glBegin(GL_LINE_STRIP);
        glColor(ClientUI::WndInnerBorderColor());
        glVertex2i(cl_ul.x, cl_ul.y);
        glVertex2i(cl_lr.x, cl_ul.y);
        glVertex2i(cl_lr.x, cl_lr.y - INNER_BORDER_ANGLE_OFFSET);
        glVertex2i(cl_lr.x - INNER_BORDER_ANGLE_OFFSET, cl_lr.y);
        glVertex2i(cl_ul.x, cl_lr.y);
        glVertex2i(cl_ul.x, cl_ul.y);
    glEnd();
    glBegin(GL_LINES);
        // draw the extra lines of the resize tab
        glColor(ClientUI::WndInnerBorderColor());
        glVertex2i(cl_lr.x, cl_lr.y - RESIZE_HASHMARK1_OFFSET);
        glVertex2i(cl_lr.x - RESIZE_HASHMARK1_OFFSET, cl_lr.y);
        
        glVertex2i(cl_lr.x, cl_lr.y - RESIZE_HASHMARK2_OFFSET);
        glVertex2i(cl_lr.x - RESIZE_HASHMARK2_OFFSET, cl_lr.y);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

void DesignWnd::DetailPanel::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys)
{
    if (m_drag_offset != GG::Pt(-1, -1)) {  // resize-dragging
        GG::Pt new_lr = pt - m_drag_offset;

        // constrain to within parent
        if (GG::Wnd* parent = Parent()) {
            GG::Pt max_lr = parent->ClientLowerRight();
            new_lr.x = std::min(new_lr.x, max_lr.x);
            new_lr.y = std::min(new_lr.y, max_lr.y);
        }

        Resize(new_lr - UpperLeft());
    } else {    // normal-dragging
        GG::Pt final_move = move;

        if (GG::Wnd* parent = Parent()) {
            GG::Pt ul = UpperLeft(), lr = LowerRight();
            GG::Pt new_ul = ul + move, new_lr = lr + move;

            GG::Pt min_ul = parent->ClientUpperLeft() + GG::Pt(1, 1);
            GG::Pt max_lr = parent->ClientLowerRight();
            GG::Pt max_ul = max_lr - this->Size();

            new_ul.x = std::max(min_ul.x, std::min(max_ul.x, new_ul.x));
            new_ul.y = std::max(min_ul.y, std::min(max_ul.y, new_ul.y));

            final_move = new_ul - ul;
        }

        GG::Wnd::LDrag(pt, final_move, mod_keys);
    }
}

void DesignWnd::DetailPanel::Reset()
{
    if (m_graphic) {
        DeleteChild(m_graphic);
        m_graphic = 0;
    }
    if (!m_part && !m_hull) {
        m_name_text->SetText("");
        m_summary_text->SetText("");
        m_cost_text->SetText("");
        m_description_box->SetText("");
        return;
    }

    std::string name;
    boost::shared_ptr<GG::Texture> texture;
    if (m_part) {
        name = m_part->Name();
        texture = ClientUI::PartTexture(name);
    } else /*if (m_hull)*/ {
        name = m_hull->Name();
        texture = ClientUI::HullTexture(name);
    }

    m_graphic = new GG::StaticGraphic(0, 0, 10, 10, texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);

    m_graphic->Show();
    AttachChild(m_graphic);

    DoLayout();

    m_name_text->SetText(UserString(name));
    using boost::io::str;
    using boost::format;

    // get details of part or hull.  both have similar values, so need only to store results and then use
    // common code after to actually display it
    int turns = 99;
    double cost = 9999.99;
    std::string class_text = "";
    std::string part_or_hull_string = "";
    std::string detailed_description = "";
    if (m_part) {
        turns = m_part->BuildTime();
        cost = m_part->Cost();
        class_text = boost::lexical_cast<std::string>(m_part->Class());
        part_or_hull_string = "DESIGN_SHIP_PART";
        detailed_description = m_part->Description();
    } else /*if (m_hull)*/ {
        turns = m_hull->BuildTime();
        cost = m_hull->Cost();
        // hulls have no class, so class_test remains = ""
        part_or_hull_string = "DESIGN_SHIP_HULL";
        detailed_description = m_hull->Description();
    }
    const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects =
        m_part ? m_part->Effects() : m_hull->Effects();


    m_summary_text->SetText("<i>" + str(format(UserString("DESIGN_DETAIL_TYPE_STR"))
        % UserString(class_text)
        % UserString(part_or_hull_string)) + "</i>");

    m_cost_text->SetText(str(format(UserString("DESIGN_SHIP_COST_STR"))
        % static_cast<int>(cost + 0.5)
        % turns));

    std::string description_str = str(format(UserString("DESIGN_DETAIL_DESCRIPTION_STR"))
                                      % UserString(detailed_description));


    if (!effects.empty()) {
        description_str += str(format(UserString("DESIGN_DETAIL_EFFECTS_STR"))
                               % EffectsDescription(effects));
    }

    m_description_box->SetText(description_str);
}

void DesignWnd::DetailPanel::SetItem(const PartType* part) {
    m_hull = 0;
    m_part = part;
    Reset();
}

void DesignWnd::DetailPanel::SetItem(const HullType* hull) {
    m_part = 0;
    m_hull = hull;
    Reset();
}


//////////////////////////////////////////////////
// DesignWnd::BaseSelector                      //
//////////////////////////////////////////////////
class DesignWnd::BaseSelector : public CUIWnd {
public:
};


//////////////////////////////////////////////////
// DesignWnd::PartPalette                       //
//////////////////////////////////////////////////
class DesignWnd::PartPalette : public CUIWnd {
public:
};


//////////////////////////////////////////////////
// DesignWnd::MainPanel                         //
//////////////////////////////////////////////////
class DesignWnd::MainPanel : public CUIWnd {
public:
};


//////////////////////////////////////////////////
// DesignWnd                                    //
//////////////////////////////////////////////////
DesignWnd::DesignWnd(int w, int h) :
    GG::Wnd(0, 0, w, h, GG::ONTOP | GG::CLICKABLE),
    m_selected_hull(""),
    m_selected_parts(),
    m_hulls_list(0),
    m_parts_lists(),
    m_parts_list_labels(),
    m_add_design_button(0),
    m_design_name_edit(0),
    m_design_description_edit(0),
    m_detail_panel(0),
    m_base_selector(0),
    m_part_palette(0),
    m_main_panel(0)
{
    TempUISoundDisabler sound_disabler;

    EnableChildClipping(true);

    m_add_design_button = new CUIButton(100, 100, 120, UserString("DESIGN_ADD_TEST"));
    AttachChild(m_add_design_button);
    GG::Connect(m_add_design_button->ClickedSignal, &DesignWnd::AddDesign, this);

    m_design_name_edit = new CUIEdit(100, 125, 150, UserString("DESIGN_NAME_DEFAULT"));
    AttachChild(m_design_name_edit);

    m_design_description_edit = new CUIEdit(100, 150, 200, UserString("DESIGN_DESCRIPTION_DEFAULT"));
    AttachChild(m_design_description_edit);

    GG::Connect(this->DesignChangedSignal, &DesignWnd::DesignChanged, this);

    const int DROPLIST_HEIGHT = ClientUI::Pts() + 4;
    m_hulls_list = new CUIDropDownList(325, 150, 150, DROPLIST_HEIGHT, 100);
    m_hulls_list->SetStyle(GG::LIST_NOSORT);
    AttachChild(m_hulls_list);

    std::map<int, std::string> hull_names_by_row_index;
    int cur_hull_index = 0;

    const HullTypeManager& hull_manager = GetHullTypeManager();
    for (HullTypeManager::iterator it = hull_manager.begin(); it != hull_manager.end(); ++it) {
        const std::string& hull_name = it->first;

        m_hulls_list->Insert(new CUISimpleDropDownListRow(UserString(hull_name)), cur_hull_index);
        hull_names_by_row_index[cur_hull_index] = hull_name;
        ++cur_hull_index;
    }

    // connect, then select, so that signal from selection causes parts lists to be set up
    GG::Connect(m_hulls_list->SelChangedSignal, SelectHullFunctor(this, hull_names_by_row_index));
    m_hulls_list->Select(0);

    m_detail_panel = new DetailPanel(500, 300);
    AttachChild(m_detail_panel);
}

void DesignWnd::Reset() {
}

void DesignWnd::Sanitize() {
}

void DesignWnd::Render() {
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

bool DesignWnd::ValidateCurrentDesign() {
    return ShipDesign::ValidDesign(m_selected_hull, m_selected_parts);
}

void DesignWnd::AddDesign() {
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire* empire = Empires().Lookup(empire_id);
    if (!empire) return;

    if (!ValidateCurrentDesign()) {
        Logger().errorStream() << "DesignWnd::AddDesign tried to add an invalid ShipDesign";
        return;
    }

    // make sure name isn't blank.  TODO: prevent duplicate names?
    std::string name = m_design_name_edit->WindowText();
    if (name == "")
        name = UserString("DESIGN_NAME_DEFAULT");

    // create design from stuff chosen in UI
    ShipDesign* design = new ShipDesign(name, m_design_description_edit->WindowText(), empire_id, CurrentTurn(),
                                        m_selected_hull, m_selected_parts, "misc/base1.png", "some model");

    if (!design) {
        Logger().errorStream() << "DesignWnd::AddDesign failed to create a new ShipDesign object";
        return;
    }

    int new_design_id = HumanClientApp::GetApp()->GetNewDesignID();
    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new ShipDesignOrder(empire_id, new_design_id, *design)));

    Logger().debugStream() << "Added new design: " << design->Name();

    const Universe& universe = GetUniverse();
    for (Universe::ship_design_iterator it = universe.beginShipDesigns(); it != universe.endShipDesigns(); ++it)
        Logger().debugStream() << "Shipdesign: " << it->second->Name();
}

void DesignWnd::HullSelected(const std::string& hull_name) {
    if (hull_name.empty()) {
        SetDesignHull("");
        return;
    }

    const HullType* hull = GetHullType(hull_name);
    // make sure hull was found.  if not, use default blank hull_name = ""
    if (!hull) {
        Logger().errorStream() << "DesignWnd::HullSelected: couldn't find hull " << hull_name;
        return;
    }
    if (m_detail_panel)
        m_detail_panel->SetItem(hull);
    SetDesignHull(hull_name);
}

void DesignWnd::SetDesignHull(const std::string& hull) {
    if (hull == m_selected_hull)
        return; // nothing to do...

    TempUISoundDisabler sound_disabler;

    // set new hull
    m_selected_hull = hull;

    // clear old parts and lists
    m_parts_lists.clear();
    m_parts_list_labels.clear();
    m_selected_parts.clear();

    // get number of slots needed for this hull.  (defaults to zero if hull is invalid)
    unsigned int num_slots = 0;
    const HullTypeManager& hull_manager = GetHullTypeManager();
    const HullType* hull_type = hull_manager.GetHullType(hull);
    if (hull_type) {
        num_slots = hull_type->NumSlots();
    } else {
        m_selected_hull = "";
        Logger().errorStream() << "DesignWnd::SetDesignHull : passed invalid hull name: " << hull;
        return;
    }

    const std::vector<HullType::Slot>& slots = hull_type->Slots();


    // add slot drop lists to provide enough slot drops for the number of slots in the hull
    const PartTypeManager& part_manager = GetPartTypeManager();
    const int DROPLIST_LABEL_LEFT = 500;
    const int DROPLIST_LABEL_WIDTH = 50;
    const int GAP = 25;
    const int DROPLIST_HEIGHT = ClientUI::Pts() + 4;
    const int DROPLIST_DROP_HEIGHT = DROPLIST_HEIGHT * 8;
    const int DROPLIST_LEFT = DROPLIST_LABEL_LEFT + DROPLIST_LABEL_WIDTH + GAP;
    const int DROPLIST_WIDTH = 150;

    boost::shared_ptr<GG::Font> font = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts());

    unsigned int cur_slot = 0;
    while (static_cast<unsigned int>(m_parts_lists.size()) < num_slots) {
        // create label for slot drop list
        ShipSlotType slot_type = slots[cur_slot].type;
        const std::string DROPLIST_LABEL_TEXT = UserString(boost::lexical_cast<std::string>(slot_type));
        const int DROPLIST_TOP = 150 + cur_slot*DROPLIST_HEIGHT*3/2;
        GG::TextControl* label = new GG::TextControl(DROPLIST_LABEL_LEFT, DROPLIST_TOP, DROPLIST_LABEL_WIDTH, DROPLIST_HEIGHT, DROPLIST_LABEL_TEXT, font, ClientUI::TextColor(), GG::FORMAT_RIGHT, GG::CLICKABLE);
        m_parts_list_labels.push_back(boost::shared_ptr<GG::TextControl>(label));
        AttachChild(label);

        // create slot drop list
        CUIDropDownList* list = new CUIDropDownList(DROPLIST_LEFT, DROPLIST_TOP, DROPLIST_WIDTH, DROPLIST_HEIGHT, DROPLIST_DROP_HEIGHT);
        list->SetStyle(GG::LIST_NOSORT);
        m_parts_lists.push_back(boost::shared_ptr<CUIDropDownList>(list));
        AttachChild(list);

        // map keep tracks of which part is in which row of the current droplist
        std::map<int, std::string> part_names_by_row_index;

        // make a "no part" list entry
        const int no_part_index = 0;
        list->Insert(new CUISimpleDropDownListRow(UserString("DESIGN_NO_PART")), no_part_index);
        part_names_by_row_index[no_part_index] = "";

        int cur_part_index = no_part_index + 1;
        // populate list
        for (PartTypeManager::iterator it = part_manager.begin(); it != part_manager.end(); ++it) {
            const std::string& part_name = it->first;
            const PartType* part = it->second;
            // only add parts that can be mouted in the current slot's type
            if (part->CanMountInSlotType(slot_type)) {
                list->Insert(new CUISimpleDropDownListRow(UserString(part_name)), cur_part_index);
                part_names_by_row_index[cur_part_index] = part_name;
                ++cur_part_index;
            }
        }

        // select no part by default
        m_selected_parts.push_back("");
        list->Select(no_part_index);

        GG::Connect(list->SelChangedSignal, SelectPartFunctor(this, part_names_by_row_index, cur_slot));

        ++cur_slot;
    }

    DesignChangedSignal();
}

void DesignWnd::DoLayout() {
}

void DesignWnd::PartSelected(const std::string& part_name, unsigned int slot) {
    if (slot >= static_cast<unsigned int>(m_parts_lists.size()))
        throw std::invalid_argument("DesignWnd::PartSelected called with invalid slot");

    if (part_name.empty()) {
        SetDesignPart("", slot);
        return;
    }

    const PartType* part = GetPartType(part_name);
    // make sure part was found.  if not, use default blank part_name = ""
    if (!part) {
        Logger().errorStream() << "DesignWnd::PartSelected: couldn't find part " << part_name;
        return;
    }

    const HullTypeManager& hull_manager = GetHullTypeManager();
    const HullType* hull_type = hull_manager.GetHullType(m_selected_hull);
    if (!hull_type) {
        Logger().errorStream() << "DesignWnd::PartSelected: couldn't get own hull...?";
        return;
    }

    // verify that part can be put in indicated slot
    std::vector<HullType::Slot> slots = hull_type->Slots();
    ShipSlotType slot_type = slots[slot].type;

    if (m_detail_panel)
        m_detail_panel->SetItem(part);
    if (part->CanMountInSlotType(slot_type))
        SetDesignPart(part_name, slot);
}

void DesignWnd::SetDesignPart(const std::string& part, unsigned int slot) {
    m_selected_parts.at(slot) = part;
    DesignChangedSignal();
}

void DesignWnd::DesignChanged() {
    if (ValidateCurrentDesign())
        m_add_design_button->Disable(false);
    else
        m_add_design_button->Disable();
}
