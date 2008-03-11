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
    PartsListBox(int x, int y, int w, int h);
    //@}

    /** \name Mutators */ //@{
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    void            Populate();

    void            ShowClass(ShipPartClass part_class, bool refresh_list = true);
    void            ShowAllClasses(bool refresh_list = true);
    void            HideClass(ShipPartClass part_class, bool refresh_list = true);
    void            HideAllClasses(bool refresh_list = true);

    void            ShowAvailability(bool available, bool refresh_list = true);
    void            HideAvailability(bool available, bool refresh_list = true);
    //@}

private:
    std::set<ShipPartClass> m_part_classes_shown;   // which part classes should be shown
    std::pair<bool, bool>   m_availabilities_shown; // first indicates whether available parts should be shown.  second indicates whether unavailable parts should be shown

    std::map<const PartType*, boost::shared_ptr<PartControl> >  m_part_controls;

    int                     m_previous_num_columns;
};

PartsListBox::PartsListBox(int x, int y, int w, int h) :
    CUIListBox(x, y, w, h),
    m_part_classes_shown(),
    m_availabilities_shown(std::make_pair(false, false)),
    m_part_controls(),  // TODO: Store PartControls sorted by PartType::Class
    m_previous_num_columns(-1)
{
    // create PartControl objects for all parts
    const PartTypeManager& manager = GetPartTypeManager();
    for (PartTypeManager::iterator part_it = manager.begin(); part_it != manager.end(); ++part_it) {
        const PartType* part = part_it->second;
        m_part_controls[part] = boost::shared_ptr<PartControl>(new PartControl(part));
    }
}

void PartsListBox::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::LowerRight() - GG::Wnd::UpperLeft();

    // maybe later do something interesting with docking
    GG::Wnd::SizeMove(ul, lr);

    if (Visible() && old_size != GG::Wnd::Size()) {
        // determine how many columns can fit in the box now...
        const int PART_SIZE = PartControl::SIZE;
        const int TOTAL_WIDTH = Size().x - ClientUI::ScrollWidth();
        const int NUM_COLUMNS = std::max(1, TOTAL_WIDTH / PART_SIZE);

        if (NUM_COLUMNS != m_previous_num_columns)
            Populate();
    }
}

void PartsListBox::Populate() {
    const int PART_SIZE = PartControl::SIZE;
    const int PART_PAD = 6;
    const int TOTAL_WIDTH = Width() - ClientUI::ScrollWidth();
    const int NUM_COLUMNS = std::max(1, (TOTAL_WIDTH - PART_PAD) / (PART_SIZE + PART_PAD));

    const int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire* empire = Empires().Lookup(empire_id);
    const PartTypeManager& manager = GetPartTypeManager();

    int cur_col = NUM_COLUMNS;
    GG::ListBox::Row* cur_row = 0;  // ...?

    for (std::map<const PartType*, boost::shared_ptr<PartControl> >::iterator control_it = m_part_controls.begin();
         control_it != m_part_controls.end();
         ++control_it)
    {
        const PartType* part = control_it->first;
        // check whether this part should be shown in list
        ShipPartClass part_class = part->Class();
        if (m_part_classes_shown.find(part_class) == m_part_classes_shown.end())
            continue;   // part of this class is not requested to be shown
        bool part_available = empire->ShipPartAvailable(part->Name());
        if (!(part_available && m_availabilities_shown.first) && !(!part_available && m_availabilities_shown.first))
            continue;   // part is available but available parts shouldn't be shown, or part isn't available and not available parts shouldn't be shown

        // part should be shown in list.  Add the control to the end of the current row, or start of a new row if the current row is full
        if (cur_col >= NUM_COLUMNS) {
            cur_col = 0;
            cur_row = new GG::ListBox::Row(TOTAL_WIDTH, PART_SIZE + PART_PAD, "");
            Insert(cur_row);
        }


    }

    // keep track of how many columns are present now
    m_previous_num_columns = NUM_COLUMNS;
}

void PartsListBox::ShowClass(ShipPartClass part_class, bool refresh_list) {
    if (m_part_classes_shown.find(part_class) == m_part_classes_shown.end()) {
        m_part_classes_shown.insert(part_class);
        if (refresh_list)
            Populate();
    }
}

void PartsListBox::ShowAllClasses(bool refresh_list) {
    for (ShipPartClass part_class = ShipPartClass(0); part_class != NUM_SHIP_PART_CLASSES; part_class = ShipPartClass(part_class + 1))
        m_part_classes_shown.insert(part_class);
    if (refresh_list)
        Populate();
}

void PartsListBox::HideClass(ShipPartClass part_class, bool refresh_list) {
    std::set<ShipPartClass>::iterator it = m_part_classes_shown.find(part_class);
    if (it != m_part_classes_shown.end()) {
        m_part_classes_shown.erase(it);
        if (refresh_list)
            Populate();
    }
}

void PartsListBox::HideAllClasses(bool refresh_list) {
    m_part_classes_shown.clear();
    if (refresh_list)
        Populate();
}

void PartsListBox::ShowAvailability(bool available, bool refresh_list) {
    if (available) {
        if (!m_availabilities_shown.first) {
            m_availabilities_shown.first = true;
            if (refresh_list)
                Populate();
        }
    } else {
        if (!m_availabilities_shown.second) {
            m_availabilities_shown.second = true;
            if (refresh_list)
                Populate();
        }
    }
}

void PartsListBox::HideAvailability(bool available, bool refresh_list) {
    if (available) {
        if (m_availabilities_shown.first) {
            m_availabilities_shown.first = false;
            if (refresh_list)
                Populate();
        }
    } else {
        if (m_availabilities_shown.second) {
            m_availabilities_shown.second = false;
            if (refresh_list)
                Populate();
        }
    }
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
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);

    void            ShowClass(ShipPartClass part_class, bool refresh_list = true);
    void            ShowAllClasses(bool refresh_list = true);
    void            HideClass(ShipPartClass part_class, bool refresh_list = true);
    void            HideAllClasses(bool refresh_list = true);

    void            ShowAvailability(bool available, bool refresh_list = true);
    void            HideAvailability(bool available, bool refresh_list = true);
    //@}

private:
    void            DoLayout();

    PartsListBox*   m_parts_list;

    std::map<ShipPartClass, boost::shared_ptr<GG::Button> > m_class_buttons;
    std::vector<boost::shared_ptr<GG::Button> >             m_availability_buttons;
};

DesignWnd::PartPalette::PartPalette(int w, int h) :
    CUIWnd(UserString("DESIGN_WND_PART_PALETTE_TITLE"), 0, 0, w, h, GG::ONTOP | GG::CLICKABLE | GG::DRAGABLE | GG::RESIZABLE),
    m_parts_list(0)
{
    //TempUISoundDisabler sound_disabler;     // should be redundant with disabler in DesignWnd::DesignWnd.  uncomment if this is not the case
    EnableChildClipping(true);

    m_parts_list = new PartsListBox(0, 0, 10, 10);
    AttachChild(m_parts_list);

    // class buttons
    for (ShipPartClass part_class = ShipPartClass(0); part_class != NUM_SHIP_PART_CLASSES; part_class = ShipPartClass(part_class + 1)) {
        m_class_buttons[part_class] = boost::shared_ptr<GG::Button>(new CUIButton(10, 10, 10, UserString(boost::lexical_cast<std::string>(part_class))));
        AttachChild(m_class_buttons[part_class].get());
    }

    m_availability_buttons.push_back(boost::shared_ptr<GG::Button>(new CUIButton(10, 10, 10, UserString("PRODUCTION_WND_AVAILABILITY_AVAILABLE"))));
    AttachChild(m_availability_buttons[0].get());
    m_availability_buttons.push_back(boost::shared_ptr<GG::Button>(new CUIButton(10, 10, 10, UserString("PRODUCTION_WND_AVAILABILITY_UNAVAILABLE"))));
    AttachChild(m_availability_buttons[1].get());

    DoLayout();
}

void DesignWnd::PartPalette::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    CUIWnd::SizeMove(ul, lr);
    DoLayout();
}

void DesignWnd::PartPalette::DoLayout() {
    const int PTS = ClientUI::Pts();
    const int PTS_WIDE = PTS/2;         // guess at how wide per character the font needs
    const int BUTTON_HEIGHT = PTS*3/2;
    const int BUTTON_SEPARATION = 3;    // vertical or horizontal sepration between adjacent buttons
    const int BUTTON_EDGE_PAD = 2;      // distance from edges of control to buttons
    const int RIGHT_EDGE_PAD = 8;       // to account for border of CUIWnd

    const int USABLE_WIDTH = std::max(ClientWidth() - RIGHT_EDGE_PAD, 1);   // space in which to fit buttons
    const int GUESSTIMATE_NUM_CHARS_IN_BUTTON_LABEL = 16;                   // rough guesstimate... avoid overly long part class names
    const int MIN_BUTTON_WIDTH = PTS_WIDE*GUESSTIMATE_NUM_CHARS_IN_BUTTON_LABEL;
    const int MAX_BUTTONS_PER_ROW = std::max(USABLE_WIDTH / (MIN_BUTTON_WIDTH + BUTTON_SEPARATION), 1);

    const int NUM_CLASS_BUTTONS = std::max(1, static_cast<int>(m_class_buttons.size()));
    const int NUM_AVAILABILITY_BUTTONS_PER_ROW = (NUM_CLASS_BUTTONS + 2 > MAX_BUTTONS_PER_ROW) ? 1 : 2;

    const int MAX_CLASS_BUTTONS_PER_ROW = std::max(1, MAX_BUTTONS_PER_ROW - NUM_AVAILABILITY_BUTTONS_PER_ROW);

    const int NUM_CLASS_BUTTON_ROWS = NUM_CLASS_BUTTONS / MAX_CLASS_BUTTONS_PER_ROW;
    const int NUM_CLASS_BUTTONS_PER_ROW = NUM_CLASS_BUTTONS / NUM_CLASS_BUTTON_ROWS;

    const int TOTAL_BUTTONS_PER_ROW = NUM_CLASS_BUTTONS_PER_ROW + NUM_AVAILABILITY_BUTTONS_PER_ROW;

    const int BUTTON_WIDTH = (USABLE_WIDTH - (TOTAL_BUTTONS_PER_ROW - 1)*BUTTON_SEPARATION) / TOTAL_BUTTONS_PER_ROW;

    const int COL_OFFSET = BUTTON_WIDTH + BUTTON_SEPARATION;    // horizontal distance between each column of buttons
    const int ROW_OFFSET = BUTTON_HEIGHT + BUTTON_SEPARATION;   // vertical distance between each row of buttons

    // place class buttons
    int col = MAX_CLASS_BUTTONS_PER_ROW, row = -1;
    for (std::map<ShipPartClass, boost::shared_ptr<GG::Button> >::iterator it = m_class_buttons.begin(); it != m_class_buttons.end(); ++it) {
        if (col >= MAX_CLASS_BUTTONS_PER_ROW) {
            col = 0;
            ++row;
        }
        GG::Pt ul(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
        GG::Pt lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
        it->second->SizeMove(ul, lr);
        ++col;
    }

    // place parts list.  note: assuming at least as many rows of class buttons as availability buttons, as this
    //                          is quite likely, as long as there are two or more classes of parts, and it's
    //                          slightly less work than being more careful to ensure this is the case
    m_parts_list->SizeMove(GG::Pt(0, BUTTON_EDGE_PAD + ROW_OFFSET*(row + 1)), ClientSize() - GG::Pt(BUTTON_SEPARATION, BUTTON_SEPARATION));


    // place availability buttons
    col = MAX_CLASS_BUTTONS_PER_ROW;
    row = 0;
    GG::Pt ul(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
    GG::Pt lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_availability_buttons[0]->SizeMove(ul, lr);
    if (NUM_AVAILABILITY_BUTTONS_PER_ROW <= 1)
        ++row;
    else
        ++col;
    ul = GG::Pt(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
    lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_availability_buttons[1]->SizeMove(ul, lr);
}

void DesignWnd::PartPalette::ShowClass(ShipPartClass part_class, bool refresh_list) {
    m_parts_list->ShowClass(part_class, refresh_list);
}

void DesignWnd::PartPalette::ShowAllClasses(bool refresh_list) {
    m_parts_list->ShowAllClasses(refresh_list);
}

void DesignWnd::PartPalette::HideClass(ShipPartClass part_class, bool refresh_list) {
    m_parts_list->HideClass(part_class, refresh_list);
}

void DesignWnd::PartPalette::HideAllClasses(bool refresh_list) {
    m_parts_list->HideAllClasses(refresh_list);
}

void DesignWnd::PartPalette::ShowAvailability(bool available, bool refresh_list) {
    m_parts_list->ShowAvailability(available, refresh_list);
}

void DesignWnd::PartPalette::HideAvailability(bool available, bool refresh_list) {
    m_parts_list->HideAvailability(available, refresh_list);
}

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

    m_part_palette = new PartPalette(500, 300);
    AttachChild(m_part_palette);
    m_part_palette->MoveTo(GG::Pt(0, 400));
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
