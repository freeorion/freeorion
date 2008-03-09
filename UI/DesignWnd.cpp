#include "DesignWnd.h"

#include "../util/AppInterface.h"
#include "ClientUI.h"
#include "CUIWnd.h"
#include "CUIControls.h"
#include "EncyclopediaDetailPanel.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/ShipDesign.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>

#include <boost/format.hpp>


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

    static const std::string PART_CONTROL_DROP_TYPE_STRING = "Part Control";
    static const std::string EMPTY_STRING = "";
}

//////////////////////////////////////////////////
// PartControl                                  //
//////////////////////////////////////////////////
/** UI representation of a ship part.  Displayed in the PartPalette, and can be dragged onto SlotControls to
  * add parts to the design.
  */
class PartControl : public GG::Control {
public:
    /** \name Structors */ //@{
    PartControl(const PartType* part);
    //@}

    /** \name Accessors */ //@{
    const PartType*     Part() { return m_part; }
    const std::string&  PartName() { m_part ? m_part->Name() : EMPTY_STRING; }
    //@}

    /** \name Mutators */ //@{
    virtual void        Render() {} // m_icon does the lookin' nice

    static const int SIZE = 64;
private:
    GG::StaticGraphic*  m_icon;
    const PartType*     m_part;
};

PartControl::PartControl(const PartType* part) :
    GG::Control(0, 0, SIZE, SIZE, GG::CLICKABLE),
    m_icon(0),
    m_part(part)
{
    m_icon = new GG::StaticGraphic(0, 0, SIZE, SIZE, ClientUI::PartTexture(m_part->Name()), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_icon->Show();
    AttachChild(m_icon);
    SetDragDropDataType(PART_CONTROL_DROP_TYPE_STRING);
}

//////////////////////////////////////////////////
// PartsListBox                                 //
//////////////////////////////////////////////////
/** Arrangement of PartControls that can be dragged onto SlotControls
  */
class PartsListBox : public CUIListBox {
public:
    /** \name Structors */ //@{
    PartsListBox(int w, int h);
    //@}

    /** \name Mutators */ //@{
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    void            PopulateList();
    //@}

private:
    std::set<ShipPartClass>         m_part_classes_shown;   // which part classes should be shown
    std::pair<bool, bool>           m_availabilities_shown; // first indicates whether available parts should be shown.  second indicates whether unavailable parts should be shown

    GG::ListBox                     m_list_boxes;
    boost::shared_ptr<GG::Scroll>   m_scroll;
};

PartsListBox::PartsListBox(int w, int h) :
    CUIListBox(0, 0, w, h)
{}

void PartsListBox::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::LowerRight() - GG::Wnd::UpperLeft();

    // maybe later do something interesting with docking
    GG::Wnd::SizeMove(ul, lr);

    if (Visible() && old_size != GG::Wnd::Size())
        PopulateList();
}

void PartsListBox::PopulateList() {
    const int PART_SIZE = PartControl::SIZE;

    const int TOTAL_WIDTH = Size().x;
    const int NUM_COLUMNS = std::max(1, TOTAL_WIDTH / PART_SIZE);


    //for (unsigned int i = 0; i < m_build_type_buttons.size() - 1; ++i) {
    //    m_build_type_buttons[BuildType(BT_BUILDING + i)]->SizeMove(GG::Pt(x, 0), GG::Pt(x + button_width, button_height));
    //    x += button_width;
    //}
    //m_build_type_buttons[NUM_BUILD_TYPES]->SizeMove(GG::Pt(x, 0), GG::Pt(x + button_width, button_height)); x += button_width;

    //m_availability_buttons[0]->SizeMove(GG::Pt(x, 0), GG::Pt(x + button_width, button_height)); x += button_width;
    //m_availability_buttons[1]->SizeMove(GG::Pt(x, 0), GG::Pt(x + button_width, button_height));

    //m_buildable_items->SizeMove(GG::Pt(0, button_height), ClientSize() - GG::Pt(TEXT_MARGIN_X, TEXT_MARGIN_Y));
}

//////////////////////////////////////////////////
// DesignWnd::PartPalette                       //
//////////////////////////////////////////////////
/** Contains graphical list of PartControl which can be dragged and dropped onto slots to assign parts to those slots
  */
class DesignWnd::PartPalette : public CUIWnd {
public:
    /** \name Structors */ //@{
    PartPalette(int w, int h);
    //@}

    /** \name Mutators */ //@{
    void ShowClass(BuildType type, bool refresh_list = true);
    void ShowAllClasses(bool refresh_list = true);
    void HideClass(BuildType type, bool refresh_list = true);
    void HideAllClasses(bool refresh_list = true);

    void ShowAvailability(bool available, bool refresh_list = true);
    void HideAvailability(bool available, bool refresh_list = true);
    //@}

private:
    PartsListBox*   m_parts_list;
};

//////////////////////////////////////////////////
// DesignWnd::BaseSelector                      //
//////////////////////////////////////////////////
class DesignWnd::BaseSelector : public CUIWnd {
public:
};



//////////////////////////////////////////////////
// SlotControl                                  //
//////////////////////////////////////////////////
/** UI representation and drop-target for slots of a design.  PartControl may be dropped into slots to add
  * the corresponding parts to the ShipDesign, or the part may be set programmatically with SetPart().
  */
class SlotControl : public GG::Control {
    public:
        //! \name Signal Types //!@{
        typedef boost::signal<void ()>  SlotContentsAlteredSignalType;  //!< emitted when a part / slot is altered
        //@}

        /** \name Structors */ //@{
        SlotControl();
        //@}

        /** \name Mutators */ //@{
        virtual void    AcceptDrops(std::list<GG::Wnd*>& wnds, const GG::Pt& pt);
        virtual void    ChildrenDraggedAway(const std::list<Wnd*>& wnds, const Wnd* destination);

        virtual void    Render();

        void            SetPart(const std::string& part_name);
        void            SetPart(const PartType* part = 0);
        //@}

        mutable SlotContentsAlteredSignalType SlotContentsAlteredSignal;
    private:
        boost::shared_ptr<PartControl>  m_part;
    };


//////////////////////////////////////////////////
// DesignWnd::MainPanel                         //
//////////////////////////////////////////////////
class DesignWnd::MainPanel : public CUIWnd {
public:
    //! \name Signal Types //!@{
    typedef boost::signal<void ()>  DesignChangedSignalType;    //!< emitted when the design is changed
    //@}

    /** \name Structors */ //@{
    MainPanel(int w, int h);
    //@}

    /** \name Mutators */ //@{
    void        SetPart(const std::string& part_name);
    void        SetPart(const PartType* part);
    void        SetHull(const std::string& hull_name);
    void        SetHull(const HullType* hull);
    //@}
private:
    std::vector<boost::shared_ptr<SlotControl> >    m_slots;

    GG::Edit*                   m_design_name;
    GG::Edit*                   m_design_description;
    GG::Button*                 m_confirm_button;
    GG::Button*                 m_clear_button;
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

    m_detail_panel = new EncyclopediaDetailPanel(500, 300);
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
