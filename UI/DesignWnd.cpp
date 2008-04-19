#include "DesignWnd.h"

#include "../util/AppInterface.h"
#include "ClientUI.h"
#include "CUIWnd.h"
#include "CUIControls.h"
#include "InfoPanels.h"
#include "EncyclopediaDetailPanel.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/ShipDesign.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/TabWnd.h>

#include <boost/cast.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>

#include <algorithm>


namespace {
    static const std::string PART_CONTROL_DROP_TYPE_STRING = "Part Control";
    static const std::string EMPTY_STRING = "";
    static const int BASES_LIST_BOX_ROW_HEIGHT = 100;
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
    const PartType*     Part() const { return m_part; }
    const std::string&  PartName() const { return m_part ? m_part->Name() : EMPTY_STRING; }
    //@}

    /** \name Mutators */ //@{
    virtual void        Render();
    virtual void        LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void        LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    //@}

    mutable boost::signal<void (const PartType*)> ClickedSignal;
    mutable boost::signal<void (const PartType*)> DoubleClickedSignal;

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
    //Logger().debugStream() << "PartControl::PartControl this: " << this << " part: " << part << " named: " << (part ? part->Name() : "no part");
    m_icon = new GG::StaticGraphic(0, 0, SIZE, SIZE, ClientUI::PartTexture(m_part->Name()), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_icon->Show();
    AttachChild(m_icon);
    SetDragDropDataType(PART_CONTROL_DROP_TYPE_STRING);
}

void PartControl::Render() {}

void PartControl::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    ClickedSignal(m_part);
}

void PartControl::LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    DoubleClickedSignal(m_part);
}

//////////////////////////////////////////////////
// PartsListBox                                 //
//////////////////////////////////////////////////
/** Arrangement of PartControls that can be dragged onto SlotControls
  */
class PartsListBox : public CUIListBox {
public:
    class PartsListBoxRow : public CUIListBox::Row {
    public:
        PartsListBoxRow(int w, int h);
        virtual void    ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination);
    };

    /** \name Structors */ //@{
    PartsListBox(int x, int y, int w, int h);
    //@}

    /** \name Accessors */ //@{
    const std::set<ShipPartClass>&  GetClassesShown() const;
    const std::pair<bool, bool>&    GetAvailabilitiesShown() const; // .first -> available items; .second -> unavailable items
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

    mutable boost::signal<void (const PartType*)> PartTypeClickedSignal;
    mutable boost::signal<void (const PartType*)> PartTypeDoubleClickedSignal;

private:
    std::set<ShipPartClass> m_part_classes_shown;   // which part classes should be shown
    std::pair<bool, bool>   m_availabilities_shown; // first indicates whether available parts should be shown.  second indicates whether unavailable parts should be shown

    int                     m_previous_num_columns;
};

PartsListBox::PartsListBoxRow::PartsListBoxRow(int w, int h) :
    CUIListBox::Row(w, h, "")    // drag_drop_data_type = "" implies not draggable row
{}

void PartsListBox::PartsListBoxRow::ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination) {
    if (wnds.empty())
        return;
    const GG::Wnd* wnd = wnds.front();  // should only be one wnd in list due because PartControls can only be dragged one-at-a-time
    const GG::Control* control = dynamic_cast<const GG::Control*>(wnd);
    if (!control)
        return;

    GG::Control* dragged_control = 0;

    // find control in row
    unsigned int i = -1;
    for (i = 0; i < size(); ++i) {
        dragged_control = (*this)[i];
        if (dragged_control == control)
            break;
        else
            dragged_control = 0;
    }

    if (!dragged_control)
        return;

    PartControl* part_control = dynamic_cast<PartControl*>(dragged_control);
    const PartType* part_type = 0;
    if (part_control)
        part_type = part_control->Part();

    RemoveCell(i);  // Wnd that accepts drop takes ownership of dragged-away control

    if (part_type) {
        part_control = new PartControl(part_type);
        const PartsListBox* parent = dynamic_cast<const PartsListBox*>(Parent());
        if (parent) {
            GG::Connect(part_control->ClickedSignal,        parent->PartTypeClickedSignal);
            GG::Connect(part_control->DoubleClickedSignal,  parent->PartTypeDoubleClickedSignal);
        }
        SetCell(i, part_control);
    }
}

PartsListBox::PartsListBox(int x, int y, int w, int h) :
    CUIListBox(x, y, w, h),
    m_part_classes_shown(),
    m_availabilities_shown(std::make_pair(false, false)),
    m_previous_num_columns(-1)
{
    SetStyle(GG::LIST_NOSEL);
}

const std::set<ShipPartClass>& PartsListBox::GetClassesShown() const {
    return m_part_classes_shown;
}

const std::pair<bool, bool>& PartsListBox::GetAvailabilitiesShown() const {
    return m_availabilities_shown;
}

void PartsListBox::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::LowerRight() - GG::Wnd::UpperLeft();

    // maybe later do something interesting with docking
    CUIListBox::SizeMove(ul, lr);

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
    const int TOTAL_WIDTH = ClientWidth() - ClientUI::ScrollWidth();
    const int NUM_COLUMNS = std::max(1, TOTAL_WIDTH / PART_SIZE);

    const int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire* empire = Empires().Lookup(empire_id);
    if (!empire)
        return;
    const PartTypeManager& manager = GetPartTypeManager();

    int cur_col = NUM_COLUMNS;
    PartsListBoxRow* cur_row = 0;

    // remove parts currently in rows of listbox
    Clear();

    // loop through all possible parts
    for (PartTypeManager::iterator part_it = manager.begin(); part_it != manager.end(); ++part_it) {
        const PartType* part = part_it->second;

        // check whether this part should be shown in list
        ShipPartClass part_class = part->Class();
        if (m_part_classes_shown.find(part_class) == m_part_classes_shown.end())
            continue;   // part of this class is not requested to be shown
        bool part_available = empire->ShipPartAvailable(part->Name());
        if (!(part_available && m_availabilities_shown.first) && !(!part_available && m_availabilities_shown.second))
            continue;   // part is available but available parts shouldn't be shown, or part isn't available and not available parts shouldn't be shown

        // part should be shown in list.

        // check if current row is full, and make a new row if necessary
        if (cur_col >= NUM_COLUMNS) {
            if (cur_row)
                Insert(cur_row);
            cur_col = 0;
            cur_row = new PartsListBoxRow(TOTAL_WIDTH, PART_SIZE);
        }
        ++cur_col;

        // make new part control and add to row
        PartControl* control = new PartControl(part);
        GG::Connect(control->ClickedSignal,         PartsListBox::PartTypeClickedSignal);
        GG::Connect(control->DoubleClickedSignal,   PartsListBox::PartTypeDoubleClickedSignal);
        cur_row->push_back(control);
    }
    // add any incomplete rows
    if (cur_row)
        Insert(cur_row);

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
    void            ToggleClass(ShipPartClass part_class, bool refresh_list = true);
    void            ToggleAllClasses(bool refresh_list = true);

    void            ShowAvailability(bool available, bool refresh_list = true);
    void            HideAvailability(bool available, bool refresh_list = true);
    void            ToggleAvailability(bool available, bool refresh_list = true);

    void            Reset();
    //@}

    mutable boost::signal<void (const PartType*)> PartTypeClickedSignal;
    mutable boost::signal<void (const PartType*)> PartTypeDoubleClickedSignal;

private:
    void            DoLayout();

    PartsListBox*   m_parts_list;

    std::map<ShipPartClass, CUIButton*> m_class_buttons;
    std::pair<CUIButton*, CUIButton*>   m_availability_buttons;
};

DesignWnd::PartPalette::PartPalette(int w, int h) :
    CUIWnd(UserString("DESIGN_WND_PART_PALETTE_TITLE"), 0, 0, w, h, GG::ONTOP | GG::CLICKABLE | GG::DRAGABLE | GG::RESIZABLE),
    m_parts_list(0)
{
    //TempUISoundDisabler sound_disabler;     // should be redundant with disabler in DesignWnd::DesignWnd.  uncomment if this is not the case
    EnableChildClipping(true);

    m_parts_list = new PartsListBox(0, 0, 10, 10);
    AttachChild(m_parts_list);
    GG::Connect(m_parts_list->PartTypeClickedSignal,        PartTypeClickedSignal);
    GG::Connect(m_parts_list->PartTypeDoubleClickedSignal,  PartTypeDoubleClickedSignal);

    // class buttons
    for (ShipPartClass part_class = ShipPartClass(0); part_class != NUM_SHIP_PART_CLASSES; part_class = ShipPartClass(part_class + 1)) {
        m_class_buttons[part_class] = (new CUIButton(10, 10, 10, UserString(boost::lexical_cast<std::string>(part_class))));
        AttachChild(m_class_buttons[part_class]);
        GG::Connect(m_class_buttons[part_class]->ClickedSignal,
                    boost::bind(&DesignWnd::PartPalette::ToggleClass, this, part_class, true));
    }

    // availability buttons
    CUIButton* button = new CUIButton(10, 10, 10, UserString("PRODUCTION_WND_AVAILABILITY_AVAILABLE"));
    m_availability_buttons.first = button;
    AttachChild(button);
    GG::Connect(button->ClickedSignal, 
                boost::bind(&DesignWnd::PartPalette::ToggleAvailability, this, true, true));
    button = new CUIButton(10, 10, 10, UserString("PRODUCTION_WND_AVAILABILITY_UNAVAILABLE"));
    m_availability_buttons.second = button;
    AttachChild(button);
    GG::Connect(button->ClickedSignal, 
                boost::bind(&DesignWnd::PartPalette::ToggleAvailability, this, false, true));

    ShowAllClasses(false);
    ShowAvailability(true, false);

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
    const int GUESSTIMATE_NUM_CHARS_IN_BUTTON_LABEL = 14;                   // rough guesstimate... avoid overly long part class names
    const int MIN_BUTTON_WIDTH = PTS_WIDE*GUESSTIMATE_NUM_CHARS_IN_BUTTON_LABEL;
    const int MAX_BUTTONS_PER_ROW = std::max(USABLE_WIDTH / (MIN_BUTTON_WIDTH + BUTTON_SEPARATION), 1);

    const int NUM_CLASS_BUTTONS = std::max(1, static_cast<int>(m_class_buttons.size()));
    const int NUM_AVAILABILITY_BUTTONS_PER_ROW = (NUM_CLASS_BUTTONS + 2 > MAX_BUTTONS_PER_ROW) ? 1 : 2;

    const int MAX_CLASS_BUTTONS_PER_ROW = std::max(1, MAX_BUTTONS_PER_ROW - NUM_AVAILABILITY_BUTTONS_PER_ROW);

    const int NUM_CLASS_BUTTON_ROWS = static_cast<int>(std::ceil(static_cast<float>(NUM_CLASS_BUTTONS) / MAX_CLASS_BUTTONS_PER_ROW));
    const int NUM_CLASS_BUTTONS_PER_ROW = static_cast<int>(std::ceil(static_cast<float>(NUM_CLASS_BUTTONS) / NUM_CLASS_BUTTON_ROWS));

    const int TOTAL_BUTTONS_PER_ROW = NUM_CLASS_BUTTONS_PER_ROW + NUM_AVAILABILITY_BUTTONS_PER_ROW;

    const int BUTTON_WIDTH = (USABLE_WIDTH - (TOTAL_BUTTONS_PER_ROW - 1)*BUTTON_SEPARATION) / TOTAL_BUTTONS_PER_ROW;

    const int COL_OFFSET = BUTTON_WIDTH + BUTTON_SEPARATION;    // horizontal distance between each column of buttons
    const int ROW_OFFSET = BUTTON_HEIGHT + BUTTON_SEPARATION;   // vertical distance between each row of buttons

    // place class buttons
    int col = NUM_CLASS_BUTTONS_PER_ROW, row = -1;
    for (std::map<ShipPartClass, CUIButton*>::iterator it = m_class_buttons.begin(); it != m_class_buttons.end(); ++it) {
        if (col >= NUM_CLASS_BUTTONS_PER_ROW) {
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
    col = NUM_CLASS_BUTTONS_PER_ROW;
    row = 0;
    GG::Pt ul(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
    GG::Pt lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_availability_buttons.first->SizeMove(ul, lr);
    if (NUM_AVAILABILITY_BUTTONS_PER_ROW <= 1)
        ++row;
    else
        ++col;
    ul = GG::Pt(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
    lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_availability_buttons.second->SizeMove(ul, lr);
}

void DesignWnd::PartPalette::ShowClass(ShipPartClass part_class, bool refresh_list) {
    if (part_class >= ShipPartClass(0) && part_class < NUM_SHIP_PART_CLASSES) {
        m_parts_list->ShowClass(part_class, refresh_list);
        m_class_buttons[part_class]->MarkSelectedGray();
    } else {
        throw std::invalid_argument("PartPalette::ShowClass was passed an invalid ShipPartClass");
    }
}

void DesignWnd::PartPalette::ShowAllClasses(bool refresh_list) {
    m_parts_list->ShowAllClasses(refresh_list);
    for (std::map<ShipPartClass, CUIButton*>::iterator it = m_class_buttons.begin(); it != m_class_buttons.end(); ++it)
        it->second->MarkSelectedGray();
}

void DesignWnd::PartPalette::HideClass(ShipPartClass part_class, bool refresh_list) {
    if (part_class >= ShipPartClass(0) && part_class < NUM_SHIP_PART_CLASSES) {
        m_parts_list->HideClass(part_class, refresh_list);
        m_class_buttons[part_class]->MarkNotSelected();
    } else {
        throw std::invalid_argument("PartPalette::HideClass was passed an invalid ShipPartClass");
    }
}

void DesignWnd::PartPalette::HideAllClasses(bool refresh_list) {
    m_parts_list->HideAllClasses(refresh_list);
    for (std::map<ShipPartClass, CUIButton*>::iterator it = m_class_buttons.begin(); it != m_class_buttons.end(); ++it)
        it->second->MarkNotSelected();
}

void DesignWnd::PartPalette::ToggleClass(ShipPartClass part_class, bool refresh_list)
{
    if (part_class >= ShipPartClass(0) && part_class < NUM_SHIP_PART_CLASSES) {
        const std::set<ShipPartClass>& classes_shown = m_parts_list->GetClassesShown();
        if (classes_shown.find(part_class) == classes_shown.end())
            ShowClass(part_class, refresh_list);
        else
            HideClass(part_class, refresh_list);
    } else {
        throw std::invalid_argument("PartPalette::ToggleClass was passed an invalid ShipPartClass");
    } 
}

void DesignWnd::PartPalette::ToggleAllClasses(bool refresh_list)
{
    const std::set<ShipPartClass>& classes_shown = m_parts_list->GetClassesShown();
    if (classes_shown.size() == NUM_SHIP_PART_CLASSES)
        HideAllClasses(refresh_list);
    else
        ShowAllClasses(refresh_list);
}

void DesignWnd::PartPalette::ShowAvailability(bool available, bool refresh_list)
{
    m_parts_list->ShowAvailability(available, refresh_list);
    if (available)
        m_availability_buttons.first->MarkSelectedGray();
    else
        m_availability_buttons.second->MarkSelectedGray();
}

void DesignWnd::PartPalette::HideAvailability(bool available, bool refresh_list) {
    m_parts_list->HideAvailability(available, refresh_list);
    if (available)
        m_availability_buttons.first->MarkNotSelected();
    else
        m_availability_buttons.second->MarkNotSelected();
}

void DesignWnd::PartPalette::ToggleAvailability(bool available, bool refresh_list) {
    const std::pair<bool, bool>& avail_shown = m_parts_list->GetAvailabilitiesShown();
    if (available) {
        if (avail_shown.first)
            HideAvailability(true, refresh_list);
        else
            ShowAvailability(true, refresh_list);
    } else {
        if (avail_shown.second)
            HideAvailability(false, refresh_list);
        else
            ShowAvailability(false, refresh_list);
    }
}

void DesignWnd::PartPalette::Reset() {
    m_parts_list->Populate();
}

//////////////////////////////////////////////////
// HullControl                                  //
//////////////////////////////////////////////////
/** UI representation of a ship hull, displayed a BasesListBox.
  */
class HullControl : public GG::Control {
public:
    /** \name Structors */ //@{
    HullControl(int w, int h, const HullType* hull_type);
    //@}

    /** \name Accessors */ //@{
    const HullType*     Hull() { return m_hull; }
    //@}

    /** \name Mutators */ //@{
    virtual void        Render();
    virtual void        LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    //@}

    mutable boost::signal<void (const HullType*)> ClickedSignal;

    static const int SIZE = 64;
private:
    GG::StaticGraphic*  m_graphic;
    GG::TextControl*    m_name;
    const HullType*     m_hull;
};

HullControl::HullControl(int w, int h, const HullType* hull) :
    GG::Control(0, 0, w, h, GG::CLICKABLE),
    m_graphic(0),
    m_name(0),
    m_hull(hull)
{
    //m_icon = new GG::StaticGraphic(0, 0, SIZE, SIZE, ClientUI::PartTexture(m_part->Name()), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    //m_icon->Show();
    //AttachChild(m_icon);
    SetDragDropDataType(PART_CONTROL_DROP_TYPE_STRING);
}

void HullControl::Render() {

}

void HullControl::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    ClickedSignal(m_hull);
}

//////////////////////////////////////////////////
// BasesListBox                                  //
//////////////////////////////////////////////////
/** List of starting points for designs, such as empty hulls, existing designs kept by this empire or seen 
  * elsewhere in the universe, design template scripts or saved (on disk) designs from previous games.
  */
class BasesListBox : public CUIListBox {
public:
    class BasesListBoxRow : public CUIListBox::Row {
    public:
        BasesListBoxRow(int w, int h);
        virtual void                    Render();
        virtual void                    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    };

    class HullAndPartsListBoxRow : public BasesListBoxRow {
    public:
        class HullPanel : public GG::Control {
        public:
            HullPanel(int w, int h, const std::string& hull);
            virtual void                    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
            virtual void                    Render() {}
        private:
            GG::StaticGraphic*              m_graphic;
            GG::TextControl*                m_name;
            GG::TextControl*                m_cost_and_build_time;
        };
        HullAndPartsListBoxRow(int w, int h, const std::string& hull, const std::vector<std::string>& parts);
        const std::string&              Hull() const { return m_hull; }
        const std::vector<std::string>& Parts() const { return m_parts; }
    private:
        std::string                     m_hull;
        std::vector<std::string>        m_parts;
    };

    class CompletedDesignListBoxRow : public BasesListBoxRow {
    public:
        CompletedDesignListBoxRow(int w, int h, int design_id);
        int                             DesignID() const { return m_design_id; }
    private:
        int                             m_design_id;
    };

    /** \name Structors */ //@{
    BasesListBox(int x, int y, int w, int h);
    //@}

    /** \name Accessors */ //@{
    const std::pair<bool, bool>&    GetAvailabilitiesShown() const;
    //@}

    /** \name Mutators */ //@{
    virtual void                    SizeMove(const GG::Pt& ul, const GG::Pt& lr);

    void                            SetEmpireShown(int empire_id, bool refresh_list = true);

    virtual void                    Populate();

    void                            ShowEmptyHulls(bool refresh_list = true);
    void                            ShowCompletedDesigns(bool refresh_list = true);

    void                            ShowAvailability(bool available, bool refresh_list = true);
    void                            HideAvailability(bool available, bool refresh_list = true);
    //@}

    mutable boost::signal<void (int)>
                                    DesignSelectedSignal;                   //!< an existing complete design that is known to this empire was selected (double-clicked)
    mutable boost::signal<void (const std::string&, const std::vector<std::string>&)>
                                    DesignComponentsSelectedSignal;         //!< a hull and a set of parts (which may be empty) was selected (double-clicked)
    mutable boost::signal<void (const HullType*)>
                                    HullBrowsedSignal;                      //!< a hull was browsed (clicked once)

private:
    void                            PropegateDoubleClickSignal(int index, GG::ListBox::Row* row);
    void                            PropegateLeftClickSignal(int index, GG::ListBox::Row* row, const GG::Pt& pt);

    GG::Pt                          ListRowSize();

    void                            PopulateWithEmptyHulls();
    void                            PopulateWithCompletedDesigns();

    int                             m_empire_id_shown;
    std::pair<bool, bool>           m_availabilities_shown;             // first indicates whether available parts should be shown.  second indicates whether unavailable parts should be shown
    bool                            m_showing_empty_hulls, m_showing_completed_designs;

    std::set<int>                   m_designs_in_list;
    std::set<std::string>           m_hulls_in_list;
};

BasesListBox::BasesListBoxRow::BasesListBoxRow(int w, int h) :
    CUIListBox::Row(w, h, "BasesListBoxRow")
{}

void BasesListBox::BasesListBoxRow::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, ClientUI::WndColor(), GG::CLR_WHITE, 1);
}

void BasesListBox::BasesListBoxRow::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    CUIListBox::Row::SizeMove(ul, lr);
    if (!empty() && old_size != Size())
        at(0)->Resize(Size());
}

BasesListBox::HullAndPartsListBoxRow::HullPanel::HullPanel(int w, int h, const std::string& hull) :
    GG::Control(0, 0, w, h, GG::Flags<GG::WndFlag>()),
    m_graphic(0),
    m_name(0),
    m_cost_and_build_time(0)
{
    const HullType* hull_type = GetHullType(hull);
    if (hull_type) {
        m_graphic = new GG::StaticGraphic(0, 0, w, h, ClientUI::HullTexture(hull), GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC);
        AttachChild(m_graphic);
        m_name = new GG::TextControl(0, 0, UserString(hull_type->Name()), GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor(), GG::FORMAT_NONE);
        AttachChild(m_name);
    }
}

void BasesListBox::HullAndPartsListBoxRow::HullPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Control::SizeMove(ul, lr);
    if (m_graphic)
        m_graphic->Resize(Size());
    if (m_name)
        m_name->Resize(GG::Pt(Width(), m_name->Height()));
    if (m_cost_and_build_time) {
        //m_cost_and_build_time->???
    }
}

BasesListBox::HullAndPartsListBoxRow::HullAndPartsListBoxRow(int w, int h, const std::string& hull, const std::vector<std::string>& parts) :
    BasesListBoxRow(w, h),
    m_hull(hull),
    m_parts(parts)
{
    const HullType* hull_type = GetHullType(m_hull);
    if (hull_type && m_parts.empty()) {
        // contents are just a hull
        push_back(new HullPanel(w, h, m_hull));
    } else {
        // contents are a hull and parts  TODO: make a HullAndPartsPanel
        push_back(new GG::StaticGraphic(0, 0, w, h, ClientUI::HullTexture(hull), GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC));
    }
}

BasesListBox::CompletedDesignListBoxRow::CompletedDesignListBoxRow(int w, int h, int design_id) :
    BasesListBoxRow(w, h),
    m_design_id(design_id)
{
    const ShipDesign* ship_design = GetShipDesign(design_id);
    std::string hull = "";
    if (ship_design)
        hull = ship_design->Hull();
    push_back(new ShipDesignPanel(w, h, design_id));
}

BasesListBox::BasesListBox(int x, int y, int w, int h) :
    CUIListBox(x, y, w, h),
    m_empire_id_shown(-1),  // all empires
    m_availabilities_shown(std::make_pair(false, false)),
    m_showing_empty_hulls(false),
    m_showing_completed_designs(false)
{
    GG::Connect(DoubleClickedSignal,    &BasesListBox::PropegateDoubleClickSignal,  this);
    GG::Connect(LeftClickedSignal,      &BasesListBox::PropegateLeftClickSignal,  this);
}

const std::pair<bool, bool>& BasesListBox::GetAvailabilitiesShown() const {
    return m_availabilities_shown;
}

void BasesListBox::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    CUIListBox::SizeMove(ul, lr);
    if (old_size != Size()) {
        const GG::Pt row_size = ListRowSize();
        for (int i = 0; i < NumRows(); ++i)
            GetRow(i).Resize(row_size);
    }
}

void BasesListBox::SetEmpireShown(int empire_id, bool refresh_list) {
    m_empire_id_shown = empire_id;
    if (refresh_list)
        Populate();
}

void BasesListBox::Populate() {
    // abort of not visible to see results
    if (!Visible())
        return;

    // populate list as appropriate for types of bases shown
    if (m_showing_empty_hulls)
        PopulateWithEmptyHulls();
    if (m_showing_completed_designs)
        PopulateWithCompletedDesigns();
}

GG::Pt BasesListBox::ListRowSize() {
    return GG::Pt(Width() - ClientUI::ScrollWidth() - 5, BASES_LIST_BOX_ROW_HEIGHT);
}

void BasesListBox::PopulateWithEmptyHulls() {
    const bool showing_available = m_availabilities_shown.first;
    const bool showing_unavailable = m_availabilities_shown.second;
    //Logger().debugStream() << "BasesListBox::PopulateWithEmptyHulls showing available (t, f):  " << showing_available << ", " << showing_unavailable;
    const Empire* empire = Empires().Lookup(m_empire_id_shown); // may return 0
    Logger().debugStream() << "BasesListBox::PopulateWithEmptyHulls m_empire_id_shown: " << m_empire_id_shown;

    //Logger().debugStream() << "... hulls in list: ";
    //for (std::set<std::string>::const_iterator it = m_hulls_in_list.begin(); it != m_hulls_in_list.end(); ++it)
    //    Logger().debugStream() << "... ... hull: " << *it;

    // loop through all hulls, determining if they need to be added to list
    std::set<std::string> hulls_to_add;
    std::set<std::string> hulls_to_remove;

    const HullTypeManager& manager = GetHullTypeManager();
    for (HullTypeManager::iterator it = manager.begin(); it != manager.end(); ++it) {
        const std::string& hull_name = it->first;

        // add or retain in list 1) all hulls if no empire is specified, or 
        //                       2) hulls of appropriate availablility for set empire
        if (!empire ||
            (showing_available && empire->ShipHullAvailable(hull_name)) ||
            (showing_unavailable && !empire->ShipHullAvailable(hull_name)))
        {
            // add or retain hull in list
            if (m_hulls_in_list.find(hull_name) == m_hulls_in_list.end())
                hulls_to_add.insert(hull_name);
        } else {
            // remove or don't add hull to list
            if (m_hulls_in_list.find(hull_name) != m_hulls_in_list.end())
                hulls_to_remove.insert(hull_name);
        }
    }

    //Logger().debugStream() << "... hulls to remove from list: ";
    //for (std::set<std::string>::const_iterator it = hulls_to_remove.begin(); it != hulls_to_remove.end(); ++it)
    //    Logger().debugStream() << "... ... hull: " << *it;
    //Logger().debugStream() << "... hulls to add to list: ";
    //for (std::set<std::string>::const_iterator it = hulls_to_add.begin(); it != hulls_to_add.end(); ++it)
    //    Logger().debugStream() << "... ... hull: " << *it;


    // loop through list, removing rows as appropriate
    for (int i = 0; i != this->NumRows();) {
        //Logger().debugStream() << " row index: " << i;
        const HullAndPartsListBoxRow* row = dynamic_cast<const HullAndPartsListBoxRow*>(&GetRow(i));
        if (!row) {
            ++i;
            continue;
        }
        const std::string& current_row_hull = row->Hull();
        //Logger().debugStream() << " current row hull: " << current_row_hull;
        if (hulls_to_remove.find(current_row_hull) != hulls_to_remove.end()) {
            //Logger().debugStream() << " ... removing";
            m_hulls_in_list.erase(current_row_hull);    // erase from set before deleting row, so as to not invalidate current_row_hull reference to deleted row's member string
            Row* erased_row = Erase(i);
            delete erased_row;
        } else {
            ++i;
            continue;
        }
    }

    // loop through hulls to add, adding to list
    std::vector<std::string> empty_parts_vec;
    const GG::Pt row_size = ListRowSize();
    for (std::set<std::string>::const_iterator it = hulls_to_add.begin(); it != hulls_to_add.end(); ++it) {
        const std::string& hull_name = *it;
        HullAndPartsListBoxRow* row = new HullAndPartsListBoxRow(row_size.x, row_size.y, hull_name, empty_parts_vec);
        Insert(row);
        row->Resize(row_size);

        m_hulls_in_list.insert(hull_name);
    }
}

void BasesListBox::PopulateWithCompletedDesigns() {
    const bool showing_available = m_availabilities_shown.first;
    const bool showing_unavailable = m_availabilities_shown.second;
    const Empire* empire = Empires().Lookup(m_empire_id_shown); // may return 0
    const Universe& universe = GetUniverse();

    // loop through all hulls, determining if they need to be added or removed from list
    std::set<int> designs_to_add;
    std::set<int> designs_to_remove;

    if (empire) {
        for (Empire::ShipDesignItr it = empire->ShipDesignBegin(); it != empire->ShipDesignEnd(); ++it) {
            int design_id = *it;

            // add or retain in list designs of appropriate availablility retained by current empire
            if ((showing_available && empire->ShipDesignAvailable(design_id)) ||
                (showing_unavailable && !empire->ShipDesignAvailable(design_id)))
            {
                // add or retain design in list
                if (m_designs_in_list.find(design_id) == m_designs_in_list.end())
                    designs_to_add.insert(design_id);
            } else {
                // remove or don't add design to list
                if (m_designs_in_list.find(design_id) != m_designs_in_list.end())
                    designs_to_remove.insert(design_id);
            }
        }
    } else {
        // all empires / all known designs
        for (Universe::ship_design_iterator it = universe.beginShipDesigns(); it != universe.endShipDesigns(); ++it) {
            int design_id = it->first;
            // add or retain design in list
            if (m_designs_in_list.find(design_id) == m_designs_in_list.end())
                designs_to_add.insert(design_id);
        }
    }

    // loop through list, removing rows as appropriate
    for (int i = 0; i != this->NumRows();) {
        //Logger().debugStream() << " row index: " << i;
        const CompletedDesignListBoxRow* row = dynamic_cast<const CompletedDesignListBoxRow*>(&GetRow(i));
        if (!row) {
            ++i;
            continue;
        }
        int current_row_design_id = row->DesignID();
        //Logger().debugStream() << " current row hull: " << current_row_design_id;
        if (designs_to_remove.find(current_row_design_id) != designs_to_remove.end()) {
            //Logger().debugStream() << " ... removing";
            m_designs_in_list.erase(current_row_design_id);    // erase from set before deleting row, so as to not invalidate current_row_hull reference to deleted row's member string
            Row* erased_row = Erase(i);
            delete erased_row;
        } else {
            ++i;
            continue;
        }
    }

    // loop through designs to add, adding to list
    const GG::Pt row_size = ListRowSize();
    for (std::set<int>::const_iterator it = designs_to_add.begin(); it != designs_to_add.end(); ++it) {
        int design_id = *it;
        CompletedDesignListBoxRow* row = new CompletedDesignListBoxRow(row_size.x, row_size.y, design_id);
        Insert(row);
        row->Resize(row_size);

        m_designs_in_list.insert(design_id);
    }
}

void BasesListBox::PropegateLeftClickSignal(int index, GG::ListBox::Row* row, const GG::Pt& pt) {
    // determine type of row that was clicked, and emit appropriate signal

    HullAndPartsListBoxRow* box_row = dynamic_cast<HullAndPartsListBoxRow*>(row);
    if (box_row) {
        const std::string& hull_name = box_row->Hull();
        const HullType* hull_type = GetHullType(hull_name);
        const std::vector<std::string>& parts = box_row->Parts();
        if (hull_type && parts.empty()) {
            HullBrowsedSignal(hull_type);
            return;
        }
    }
}

void BasesListBox::PropegateDoubleClickSignal(int index, GG::ListBox::Row* row) {
    // determine type of row that was clicked, and emit appropriate signal

    HullAndPartsListBoxRow* hp_row = dynamic_cast<HullAndPartsListBoxRow*>(row);
    if (hp_row) {
        DesignComponentsSelectedSignal(hp_row->Hull(), hp_row->Parts());
        return;
    }

    CompletedDesignListBoxRow* cd_row = dynamic_cast<CompletedDesignListBoxRow*>(row);
    if (cd_row) {
        DesignSelectedSignal(cd_row->DesignID());
        return;
    }
}

void BasesListBox::ShowEmptyHulls(bool refresh_list) {
    m_showing_empty_hulls = true;
    m_showing_completed_designs = false;
    if (refresh_list)
        Populate();
}

void BasesListBox::ShowCompletedDesigns(bool refresh_list) {
    m_showing_empty_hulls = false;
    m_showing_completed_designs = true;
    if (refresh_list)
        Populate();
}

void BasesListBox::ShowAvailability(bool available, bool refresh_list) {
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

void BasesListBox::HideAvailability(bool available, bool refresh_list) {
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
// DesignWnd::BaseSelector                      //
//////////////////////////////////////////////////
class DesignWnd::BaseSelector : public CUIWnd {
public:
    /** \name Structors */ //@{
    BaseSelector(int w, int h);
    //@}

    /** \name Mutators */ //@{
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    void            Reset();
    void            ToggleAvailability(bool available);
    void            SetEmpireShown(int empire_id);
    void            ShowAvailability(bool available);
    void            HideAvailability(bool available);
    //@}

    mutable boost::signal<void (int)>
                    DesignSelectedSignal;                   //!< an existing complete design that is known to this empire was selected (double-clicked)
    mutable boost::signal<void (const std::string&, const std::vector<std::string>&)>
                    DesignComponentsSelectedSignal;         //!< a hull and a set of parts (which may be empty) was selected (double-clicked)
    mutable boost::signal<void (const HullType*)>
                    HullBrowsedSignal;                      //!< a hull was browsed (clicked once)
private:
    void            DoLayout();
    void            WndSelected(int index);

    int                                 m_empire_id;

    GG::TabWnd*                         m_tabs;
    BasesListBox*                       m_hulls_list;           // empty hulls on which a new design can be based
    BasesListBox*                       m_designs_list;         // designs this empire has created or learned how to make
    std::pair<CUIButton*, CUIButton*>   m_availability_buttons;
};

DesignWnd::BaseSelector::BaseSelector(int w, int h) :
    CUIWnd(UserString("DESIGN_WND_STARTS"), 0, 0, w, h, GG::CLICKABLE | GG::RESIZABLE | GG::ONTOP | GG::DRAGABLE),
    m_empire_id(-1),
    m_tabs(0),
    m_hulls_list(0),
    m_designs_list(0)
{
    CUIButton* button = new CUIButton(10, 10, 10, UserString("PRODUCTION_WND_AVAILABILITY_AVAILABLE"));
    m_availability_buttons.first = button;
    AttachChild(button);
    GG::Connect(button->ClickedSignal, 
                boost::bind(&DesignWnd::BaseSelector::ToggleAvailability, this, true));
    button = new CUIButton(10, 10, 10, UserString("PRODUCTION_WND_AVAILABILITY_UNAVAILABLE"));
    m_availability_buttons.second = button;
    AttachChild(button);
    GG::Connect(button->ClickedSignal, 
                boost::bind(&DesignWnd::BaseSelector::ToggleAvailability, this, false));

    m_tabs = new GG::TabWnd(5, 2, 10, 10, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::WndColor(), ClientUI::TextColor(), GG::TAB_BAR_DETACHED, GG::CLICKABLE);
    GG::Connect(m_tabs->WndChangedSignal,                       &DesignWnd::BaseSelector::WndSelected,      this);
    AttachChild(m_tabs);

    m_hulls_list = new BasesListBox(0, 0, 10, 10);
    m_tabs->AddWnd(m_hulls_list, UserString("DESIGN_WND_HULLS"));
    m_hulls_list->ShowEmptyHulls(false);
    GG::Connect(m_hulls_list->DesignComponentsSelectedSignal,   DesignWnd::BaseSelector::DesignComponentsSelectedSignal);
    GG::Connect(m_hulls_list->HullBrowsedSignal,                DesignWnd::BaseSelector::HullBrowsedSignal);

    m_designs_list = new BasesListBox(0, 0, 10, 10);
    m_tabs->AddWnd(m_designs_list, UserString("DESIGN_WND_FINISHED_DESIGNS"));
    m_designs_list->ShowCompletedDesigns(false);
    GG::Connect(m_designs_list->DesignSelectedSignal,           DesignWnd::BaseSelector::DesignSelectedSignal);

    //m_saved_designs_list = new CUIListBox(0, 0, 10, 10);
    m_tabs->AddWnd(new GG::TextControl(0, 0, 30, 20, UserString("DESIGN_NO_PART"),
                                       GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()),
                                       ClientUI::TextColor()),
                   UserString("DESIGN_WND_SAVED_DESIGNS"));

    //m_templates_list = new CUIListBox(0, 0, 10, 10);
    m_tabs->AddWnd(new GG::TextControl(0, 0, 30, 20, UserString("DESIGN_NO_PART"),
                                       GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()),
                                       ClientUI::TextColor()),
                   UserString("DESIGN_WND_TEMPLATES"));

    ShowAvailability(true);
    DoLayout();
}

void DesignWnd::BaseSelector::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    CUIWnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void DesignWnd::BaseSelector::Reset() {
    const int empire_id = HumanClientApp::GetApp()->EmpireID();
    SetEmpireShown(empire_id);

    if (!m_tabs)
        return;
    GG::Wnd* wnd = m_tabs->CurrentWnd();
    if (wnd) {
        BasesListBox* base_box = dynamic_cast<BasesListBox*>(wnd);
        if (base_box)
            base_box->Populate();
    }
}

void DesignWnd::BaseSelector::SetEmpireShown(int empire_id) {
    if (m_hulls_list)
        m_hulls_list->SetEmpireShown(empire_id, true);
    if (m_designs_list)
        m_designs_list->SetEmpireShown(empire_id, true);
}

void DesignWnd::BaseSelector::ShowAvailability(bool available) {
    if (m_hulls_list)
        m_hulls_list->ShowAvailability(available, true);
    if (m_designs_list)
        m_designs_list->ShowAvailability(available, true);
    if (available)
        m_availability_buttons.first->MarkSelectedGray();
    else
        m_availability_buttons.second->MarkSelectedGray();
}

void DesignWnd::BaseSelector::HideAvailability(bool available) {
    if (m_hulls_list)
        m_hulls_list->HideAvailability(available, true);
    if (m_designs_list)
        m_designs_list->HideAvailability(available, true);
    if (available)
        m_availability_buttons.first->MarkNotSelected();
    else
        m_availability_buttons.second->MarkNotSelected();
}

void DesignWnd::BaseSelector::ToggleAvailability(bool available) {
    const std::pair<bool, bool>& avail_shown = m_hulls_list->GetAvailabilitiesShown();
    if (available) {
        if (avail_shown.first)
            HideAvailability(true);
        else
            ShowAvailability(true);
    } else {
        if (avail_shown.second)
            HideAvailability(false);
        else
            ShowAvailability(false);
    }
}


void DesignWnd::BaseSelector::DoLayout() {
    const int LEFT_PAD = 5;
    const int TOP_PAD = 2;
    const int AVAILABLE_WIDTH = ClientWidth() - 2*LEFT_PAD;
    const int BUTTON_SEPARATION = 3;
    const int BUTTON_WIDTH = (AVAILABLE_WIDTH - BUTTON_SEPARATION) / 2;
    const int PTS = ClientUI::Pts();
    const int BUTTON_HEIGHT = PTS * 2;

    int top = TOP_PAD;
    int left = LEFT_PAD;

    m_availability_buttons.first->SizeMove(GG::Pt(left, top), GG::Pt(left + BUTTON_WIDTH, top + BUTTON_HEIGHT));
    left = left + BUTTON_WIDTH + BUTTON_SEPARATION;
    m_availability_buttons.second->SizeMove(GG::Pt(left, top), GG::Pt(left + BUTTON_WIDTH, top + BUTTON_HEIGHT));
    left = LEFT_PAD;
    top = top + BUTTON_HEIGHT + BUTTON_SEPARATION;

    if (!m_tabs)
        return;
    m_tabs->SizeMove(GG::Pt(left, top), ClientSize() - GG::Pt(LEFT_PAD, TOP_PAD));
}

void DesignWnd::BaseSelector::WndSelected(int index) {
    Reset();
}

//////////////////////////////////////////////////
// SlotControl                                  //
//////////////////////////////////////////////////
/** UI representation and drop-target for slots of a design.  PartControl may be dropped into slots to add
  * the corresponding parts to the ShipDesign, or the part may be set programmatically with SetPart().
  */
class SlotControl : public GG::Control {
public:
    /** \name Structors */ //@{
    SlotControl();
    SlotControl(double x, double y, ShipSlotType slot_type);
    //@}

    /** \name Accessors */ //@{
    virtual void DropsAcceptable(DropsAcceptableIter first,
                                 DropsAcceptableIter last,
                                 const GG::Pt& pt) const;

    ShipSlotType    SlotType() const;
    double          XPositionFraction() const;
    double          YPositionFraction() const;
    const PartType* GetPart() const;
    //@}

    /** \name Mutators */ //@{
    virtual void    StartingChildDragDrop(const GG::Wnd* wnd, const GG::Pt& offset);
    virtual void    CancellingChildDragDrop(const std::vector<const GG::Wnd*>& wnds);
    virtual void    AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt);
    virtual void    ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination);
    virtual void    DragDropEnter(const GG::Pt& pt, const std::map<GG::Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys);
    virtual void    DragDropLeave();

    virtual void    Render();
    void            Highlight(bool actually = true);

    void            SetPart(const std::string& part_name);  //!< used to programmatically set the PartControl in this slot.  Does not emit signal
    void            SetPart(const PartType* part_type = 0); //!< used to programmatically set the PartControl in this slot.  Does not emit signal
    //@}

    mutable boost::signal<void (const PartType*)> SlotContentsAlteredSignal;    //!< emitted when the contents of a slot are altered by the dragging a PartControl in or out of the slot.  signal should be caught and the slot contents set using SetPart accordingly
    mutable boost::signal<void (const PartType*)> PartTypeClickedSignal;

private:
    void            EmitNullSlotContentsAlteredSignal();            // emits SlotContentsAlteredSignal with PartType* = 0.  needed because boost::signal is noncopyable, so boost::bind can't be used to bind the parameter 0 to SlotContentsAlteredSignal::operator()

    bool            m_highlighted;
    ShipSlotType    m_slot_type;
    double          m_x_position_fraction, m_y_position_fraction;   // position on hull image where slot should be shown, as a fraction of that image's size
    PartControl*    m_part_control;
};

SlotControl::SlotControl() :
    GG::Control(0, 0, PartControl::SIZE, PartControl::SIZE, GG::CLICKABLE),
    m_highlighted(false),
    m_slot_type(INVALID_SHIP_SLOT_TYPE),
    m_x_position_fraction(0.4),
    m_y_position_fraction(0.4),
    m_part_control(0)
{
    SetDragDropDataType("");
}

SlotControl::SlotControl(double x, double y, ShipSlotType slot_type) :
    GG::Control(0, 0, PartControl::SIZE, PartControl::SIZE, GG::CLICKABLE),
    m_highlighted(false),
    m_slot_type(slot_type),
    m_x_position_fraction(x),
    m_y_position_fraction(y),
    m_part_control(0)
{
    SetDragDropDataType("");
}

void SlotControl::DropsAcceptable(DropsAcceptableIter first,
                                  DropsAcceptableIter last,
                                  const GG::Pt& pt) const
{
    bool acceptable_part_found = false;
    for (DropsAcceptableIter it = first; it != last; ++it) {
        if (!acceptable_part_found && it->first->DragDropDataType() == PART_CONTROL_DROP_TYPE_STRING) {
            const PartControl* part_control = boost::polymorphic_downcast<const PartControl*>(it->first);
            const PartType* part_type = part_control->Part();
            if (part_type &&
                part_type->CanMountInSlotType(m_slot_type) &&
                part_control != m_part_control) {
                it->second = true;
                acceptable_part_found = true;
            } else {
                it->second = false;
            }
        } else {
            it->second = false;
        }
    }
}

ShipSlotType SlotControl::SlotType() const {
    return m_slot_type;
}

double SlotControl::XPositionFraction() const {
    return m_x_position_fraction;
}

double SlotControl::YPositionFraction() const {
    return m_y_position_fraction;
}

const PartType* SlotControl::GetPart() const {
    if (m_part_control)
        return m_part_control->Part();
    else
        return 0;
}

void SlotControl::StartingChildDragDrop(const GG::Wnd* wnd, const GG::Pt& offset) {
    if (!m_part_control)
        return;

    const PartControl* control = dynamic_cast<const PartControl*>(wnd);
    if (!control)
        return;

    if (control == m_part_control)
        m_part_control->Hide();
}

void SlotControl::CancellingChildDragDrop(const std::vector<const GG::Wnd*>& wnds) {
    if (!m_part_control)
        return;

    for (std::vector<const GG::Wnd*>::const_iterator it = wnds.begin(); it != wnds.end(); ++it) {
        const PartControl* control = dynamic_cast<const PartControl*>(*it);
        if (!control)
            continue;

        if (control == m_part_control)
            m_part_control->Show();
    }
}

void SlotControl::AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt) {
    assert(wnds.size() == 1);

    const GG::Wnd* wnd = *(wnds.begin());
    const PartControl* control = boost::polymorphic_downcast<const PartControl*>(wnd);
    const PartType* part_type = control->Part();

    delete control;

    //Logger().debugStream() << "SlotControl::AcceptDrops part_type: " << (part_type ? part_type->Name() : "no part");
    SlotContentsAlteredSignal(part_type);
}

void SlotControl::ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination) {
    if (wnds.empty())
        return;
    const GG::Wnd* wnd = wnds.front();
    const PartControl* part_control = dynamic_cast<const PartControl*>(wnd);
    if (part_control != m_part_control)
        return;
    m_part_control = 0; // SlotContentsAlteredSignal is connected to this->SetPart, which will delete m_part_control if it is not null.  The drop-accepting Wnd is responsible for deleting the accepted Wnd, so setting m_part_control = 0 here prevents this->SetPart from deleting it prematurely
    SlotContentsAlteredSignal(0);
}

void SlotControl::DragDropEnter(const GG::Pt& pt, const std::map<GG::Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys) {
}

void SlotControl::DragDropLeave() {
    if (m_part_control)
        m_part_control->Hide();
}

void SlotControl::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    // TODO: Render differently depending on ShipSlotType

    if (m_highlighted)
        GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, ClientUI::WndColor(), GG::CLR_WHITE, 2);
    else
        GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);
}

void SlotControl::Highlight(bool actually) {
    m_highlighted = actually;
}

void SlotControl::SetPart(const std::string& part_name) {
    SetPart(GetPartType(part_name));
}

void SlotControl::SetPart(const PartType* part_type) {
    if (m_part_control) {
        delete m_part_control;
        m_part_control = 0;
    }
    if (part_type) {
        m_part_control = new PartControl(part_type);
        AttachChild(m_part_control);
        GG::Connect(m_part_control->ClickedSignal, PartTypeClickedSignal);                  // single click shows encyclopedia data

        GG::Connect(m_part_control->DoubleClickedSignal,
                    boost::bind(&SlotControl::EmitNullSlotContentsAlteredSignal, this));    // double click clears slot
    }
}

void SlotControl::EmitNullSlotContentsAlteredSignal() {
    SlotContentsAlteredSignal(0);
}


//////////////////////////////////////////////////
// DesignWnd::MainPanel                         //
//////////////////////////////////////////////////
class DesignWnd::MainPanel : public CUIWnd {
public:
    /** \name Structors */ //@{
    MainPanel(int w, int h);
    //@}

    /** \name Accessors */ //@{
    const std::vector<std::string>  Parts() const;              //!< returns vector of names of parts in slots of current shown design.  empty slots are represented with empty stri
    const std::string&              Hull() const;               //!< returns name of hull of current shown design
    const std::string&              DesignName() const;         //!< returns name currently entered for design
    const std::string&              DesignDescription() const;  //!< returns description currently entered for design
    //@}

    /** \name Mutators */ //@{
    virtual void                    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    void                            Sanitize();

    void                            SetPart(const std::string& part_name, unsigned int slot);   //!< puts specified part in specified slot.  does nothing if slot is out of range of available slots for current hull
    void                            SetPart(const PartType* part, unsigned int slot);
    void                            SetParts(const std::vector<std::string>& parts);            //!< puts specified parts in slots.  attempts to put each part into the slot corresponding to its place in the passed vector.  if a part cannot be placed, it is ignored.  more parts than there are slots available are ignored, and slots for which there are insufficient parts in the passed vector are unmodified
    /** attempts to add the specified part to the design, if possible.  will first attempt to add part to
      * an empty slot of the appropriate type, and if no appropriate slots are available, may or may not move
      * other parts around within the design to open up a compatible slot in which to add this part (and then
      * add it).  may also do nothing. */
    void                            AddPart(const PartType* part);
    void                            ClearParts();                                               //!< removes all parts from design.  hull is not altered
    void                            SetHull(const std::string& hull_name);                      //!< sets the design hull to the specified hull, displaying appropriate background image and creating appropriate SlotControls
    void                            SetHull(const HullType* hull);
    void                            SetDesign(const ShipDesign* ship_design);                   //!< sets the displayed design by setting the appropriate hull and parts
    void                            SetDesign(int design_id);                                   //!< sets the displayed design by setting the appropriate hull and parts
    void                            SetDesignComponents(const std::string& hull,
                                                        const std::vector<std::string>& parts); //!< sets design hull and parts to those specified

    void                            HighlightSlotType(std::vector<ShipSlotType>& slot_types);   //!< renders slots of the indicated types differently, perhaps to indicate that that those slots can be drop targets for a particular part?
    //@}

    mutable boost::signal<void ()>                  DesignChangedSignal;        //!< emitted when the design is changed (by adding or removing parts, not name or description changes)
    mutable boost::signal<void (const PartType*)>   PartTypeClickedSignal;      //!< propegates signals from contained SlotControls that signal that a part has been clicked
    mutable boost::signal<void ()>                  DesignConfirmedSignal;      //!< emitted when the user clicks the m_confirm_button to add the new design to the player's empire

private:
    // disambiguate overloaded SetPart function, because otherwise boost::bind wouldn't be able to tell them apart
    typedef void (DesignWnd::MainPanel::*SetPartFuncPtrType)(const PartType* part, unsigned int slot);
    static SetPartFuncPtrType const s_set_part_func_ptr;

    void                            Populate();                                 //!< creates and places SlotControls for current hull
    void                            DoLayout();                                 //!< positions buttons, text entry boxes and SlotControls
    void                            DesignChanged();                            //!< responds to the design being changed

    const HullType*             m_hull;

    GG::StaticGraphic*          m_background_image;

    std::vector<SlotControl*>   m_slots;

    GG::TextControl*            m_design_name_label;
    GG::Edit*                   m_design_name;
    GG::TextControl*            m_design_description_label;
    GG::Edit*                   m_design_description;
    GG::Button*                 m_confirm_button;
    GG::Button*                 m_clear_button;
};

// static
DesignWnd::MainPanel::SetPartFuncPtrType const DesignWnd::MainPanel::s_set_part_func_ptr = &DesignWnd::MainPanel::SetPart;

DesignWnd::MainPanel::MainPanel(int w, int h) :
    CUIWnd(UserString("DESIGN_WND_MAIN_PANEL_TITLE"), 0, 0, w, h, GG::CLICKABLE | GG::DRAGABLE | GG::RESIZABLE),
    m_hull(0),
    m_background_image(0),
    m_slots(),
    m_design_name_label(0),
    m_design_name(0),
    m_design_description_label(0),
    m_design_description(0),
    m_confirm_button(0),
    m_clear_button(0)
{
    EnableChildClipping();

    boost::shared_ptr<GG::Font> font = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts());

    m_design_name_label = new GG::TextControl(0, 0, 10, 10, UserString("DESIGN_WND_DESIGN_NAME"), font, 
                                              ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER,
                                              GG::CLICKABLE | GG::ONTOP);
    AttachChild(m_design_name_label);

    m_design_name = new CUIEdit(0, 0, 10, UserString("DESIGN_NAME_DEFAULT"), font, ClientUI::CtrlBorderColor(),
                                ClientUI::TextColor(), ClientUI::EditIntColor(), GG::CLICKABLE | GG::ONTOP);
    AttachChild(m_design_name);

    m_design_description_label = new GG::TextControl(0, 0, 10, 10, UserString("DESIGN_WND_DESIGN_DESCRIPTION"), font, 
                                                     ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER,
                                                     GG::CLICKABLE | GG::ONTOP);
    AttachChild(m_design_description_label);

    m_design_description = new CUIEdit(0, 0, 10, UserString("DESIGN_DESCRIPTION_DEFAULT"), font, ClientUI::CtrlBorderColor(),
                                ClientUI::TextColor(), ClientUI::EditIntColor(), GG::CLICKABLE | GG::ONTOP);
    AttachChild(m_design_description);

    m_confirm_button = new CUIButton(0, 0, 10, UserString("DESIGN_WND_CONFIRM"), font, ClientUI::ButtonColor(),
                                     ClientUI::CtrlBorderColor(), 1, ClientUI::TextColor(), GG::CLICKABLE | GG::ONTOP);
    AttachChild(m_confirm_button);
    GG::Connect(m_confirm_button->ClickedSignal, DesignConfirmedSignal);
    m_confirm_button->Disable(true);

    m_clear_button = new CUIButton(0, 0, 10, UserString("DESIGN_WND_CLEAR"), font, ClientUI::ButtonColor(),
                                   ClientUI::CtrlBorderColor(), 1, ClientUI::TextColor(), GG::CLICKABLE | GG::ONTOP);
    AttachChild(m_clear_button);
    GG::Connect(m_clear_button->ClickedSignal, &DesignWnd::MainPanel::ClearParts, this);


    GG::Connect(this->DesignChangedSignal, &DesignWnd::MainPanel::DesignChanged, this);
}

const std::vector<std::string> DesignWnd::MainPanel::Parts() const {
    std::vector<std::string> retval;
    for (std::vector<SlotControl*>::const_iterator it = m_slots.begin(); it != m_slots.end(); ++it) {
        const PartType* part_type = (*it)->GetPart();
        if (part_type)
            retval.push_back(part_type->Name());
        else
            retval.push_back("");
    }
    return retval;
}

const std::string& DesignWnd::MainPanel::Hull() const {
    if (m_hull)
        return m_hull->Name();
    else
        return EMPTY_STRING;
}

const std::string& DesignWnd::MainPanel::DesignName() const {
    if (m_design_name)
        return m_design_name->WindowText();
    else
        return EMPTY_STRING;
}

const std::string& DesignWnd::MainPanel::DesignDescription() const {
    if (m_design_description)
        return m_design_description->WindowText();
    else
        return EMPTY_STRING;
}


void DesignWnd::MainPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    CUIWnd::SizeMove(ul, lr);
    DoLayout();
}

void DesignWnd::MainPanel::Sanitize() {
    SetHull(0);
    m_design_name->SetText(UserString("DESIGN_NAME_DEFAULT"));
    m_design_description->SetText(UserString("DESIGN_DESCRIPTION_DEFAULT"));
}

void DesignWnd::MainPanel::SetPart(const std::string& part_name, unsigned int slot) {
    SetPart(GetPartType(part_name), slot);
}

void DesignWnd::MainPanel::SetPart(const PartType* part, unsigned int slot) {
    //Logger().debugStream() << "DesignWnd::MainPanel::SetPart(" << (part ? part->Name() : "no part") << ", slot " << slot << ")";
    if (slot < 0 || slot > m_slots.size()) {
        Logger().errorStream() << "DesignWnd::MainPanel::SetPart specified nonexistant slot";
        return;
    }
    m_slots[slot]->SetPart(part);
    DesignChangedSignal();
}

void  DesignWnd::MainPanel::SetParts(const std::vector<std::string>& parts) {
    unsigned int num_parts = std::min(parts.size(), m_slots.size());
    for (unsigned int i = 0; i < num_parts; ++i)
        m_slots[i]->SetPart(parts[i]);
    DesignChangedSignal();
}

void DesignWnd::MainPanel::AddPart(const PartType* part) {
    // TODO: IMPLEMENT THIS
}

void DesignWnd::MainPanel::ClearParts() {
    for (unsigned int i = 0; i < m_slots.size(); ++i)
        m_slots[i]->SetPart(0);
    DesignChangedSignal();
}

void DesignWnd::MainPanel::SetHull(const std::string& hull_name) {
    SetHull(GetHullType(hull_name));
}

void DesignWnd::MainPanel::SetHull(const HullType* hull) {
    m_hull = hull;
    DeleteChild(m_background_image);
    m_background_image = 0;
    if (m_hull) {
        boost::shared_ptr<GG::Texture> texture = ClientUI::HullTexture(hull->Name());
        m_background_image = new GG::StaticGraphic(0, 0, 1, 1, texture, GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC);
        AttachChild(m_background_image);
        MoveChildDown(m_background_image);
    }
    Populate();
    DoLayout();
    DesignChangedSignal();
}

void DesignWnd::MainPanel::SetDesign(const ShipDesign* ship_design) {
    if (!ship_design) {
        SetHull(0);
        return;
    }

    m_design_name->SetText(ship_design->Name());
    m_design_description->SetText(ship_design->Description());

    const HullType* hull_type = ship_design->GetHull();
    SetHull(hull_type);

    const std::vector<std::string>& parts_vec = ship_design->Parts();
    for (unsigned int i = 0; i < parts_vec.size() && i < m_slots.size(); ++i)
        m_slots[i]->SetPart(GetPartType(parts_vec[i]));
    DesignChangedSignal();
}

void DesignWnd::MainPanel::SetDesign(int design_id) {
    SetDesign(GetShipDesign(design_id));
}

void DesignWnd::MainPanel::SetDesignComponents(const std::string& hull, const std::vector<std::string>& parts) {
    SetHull(hull);
    SetParts(parts);
}

void DesignWnd::MainPanel::HighlightSlotType(std::vector<ShipSlotType>& slot_types) {
    for (std::vector<SlotControl*>::iterator it = m_slots.begin(); it != m_slots.end(); ++it) {
        SlotControl* control = *it;
        ShipSlotType slot_type = control->SlotType();
        if (std::find(slot_types.begin(), slot_types.end(), slot_type) != slot_types.end())
            control->Highlight(true);
        else
            control->Highlight(false);
    }
}

void DesignWnd::MainPanel::Populate(){
    for (std::vector<SlotControl*>::iterator it = m_slots.begin(); it != m_slots.end(); ++it)
        delete *it;
    m_slots.clear();

    if (!m_hull)
        return;

    const std::vector<HullType::Slot>& hull_slots = m_hull->Slots();

    for (std::vector<HullType::Slot>::size_type i = 0; i != hull_slots.size(); ++i) {
        const HullType::Slot& slot = hull_slots[i];
        SlotControl* slot_control = new SlotControl(slot.x, slot.y, slot.type);
        m_slots.push_back(slot_control);
        AttachChild(slot_control);
        boost::function<void (const PartType*)> set_part_func =
            boost::bind(DesignWnd::MainPanel::s_set_part_func_ptr, this, _1, i);
        GG::Connect(slot_control->SlotContentsAlteredSignal, set_part_func);
        GG::Connect(slot_control->PartTypeClickedSignal, PartTypeClickedSignal);
    }
}

void DesignWnd::MainPanel::DoLayout() {
    // position labels and text edit boxes for name and description and buttons to clear and confirm design

    const int PTS = ClientUI::Pts();
    const int PTS_WIDE = PTS / 2;           // guess at how wide per character the font needs
    const int BUTTON_HEIGHT = PTS * 2;
    const int LABEL_WIDTH = PTS_WIDE * 15;
    const int PAD = 6;
    const int MIN_EDIT_WIDTH = PTS_WIDE * 50;
    const int GUESSTIMATE_NUM_CHARS_IN_BUTTON_TEXT = 25;    // rough guesstimate... avoid overly long part class names
    const int BUTTON_WIDTH = PTS_WIDE*GUESSTIMATE_NUM_CHARS_IN_BUTTON_TEXT;

    int edit_right = ClientWidth();
    int edit_height = BUTTON_HEIGHT;

    if (m_confirm_button) {
        GG::Pt lr = GG::Pt(ClientWidth(), BUTTON_HEIGHT) + GG::Pt(-PAD, PAD);
        GG::Pt ul = lr - GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
        m_confirm_button->SizeMove(ul, lr);
        edit_right = ul.x - PAD;
    }
    if (m_clear_button) {
        GG::Pt lr = ClientSize() + GG::Pt(-PAD, -PAD);
        GG::Pt ul = lr - GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
        m_clear_button->SizeMove(ul, lr);
    }

    if (m_design_name)
        edit_height = m_design_name->Height();

    int x = PAD, y = PAD;
    if (m_design_name_label) {
        GG::Pt ul = GG::Pt(x, y);
        GG::Pt lr = ul + GG::Pt(LABEL_WIDTH, edit_height);
        m_design_name_label->SizeMove(ul, lr);
        x = lr.x + PAD;
    }
    if (m_design_name) {
        GG::Pt ul = GG::Pt(x, y);
        GG::Pt lr = GG::Pt(edit_right, y + edit_height);
        m_design_name->SizeMove(ul, lr);
        x = lr.x + PAD;
    }

    x = PAD;
    y += (edit_height + PAD);

    if (m_design_description_label) {
        GG::Pt ul = GG::Pt(x, y);
        GG::Pt lr = ul + GG::Pt(LABEL_WIDTH, edit_height);
        m_design_description_label->SizeMove(ul, lr);
        x = lr.x + PAD;
    }
    if (m_design_description) {
        GG::Pt ul = GG::Pt(x, y);
        GG::Pt lr = GG::Pt(edit_right, y + edit_height);
        m_design_description->SizeMove(ul, lr);
        x = lr.x + PAD;
    }

    y += (edit_height);

    // place background image of hull
    GG::Rect background_rect = GG::Rect(GG::Pt(0, y), ClientLowerRight());

    if (m_background_image) {
        GG::Pt ul = GG::Pt(0, y);
        GG::Pt lr = ClientSize();
        m_background_image->SizeMove(ul, lr);
        background_rect = m_background_image->RenderedArea();
    }

    // place slot controls over image of hull
    for (std::vector<SlotControl*>::iterator it = m_slots.begin(); it != m_slots.end(); ++it) {
        SlotControl* slot = *it;
        int x = background_rect.Left() - slot->Width()/2 - ClientUpperLeft().x + static_cast<int>(slot->XPositionFraction() * background_rect.Width());
        int y = background_rect.Top() - slot->Height()/2 - ClientUpperLeft().y + static_cast<int>(slot->YPositionFraction() * background_rect.Height());
        slot->MoveTo(GG::Pt(x, y));
    }
}

void DesignWnd::MainPanel::DesignChanged() {
    if (m_hull && !(m_hull->Name()).empty() && ShipDesign::ValidDesign(m_hull->Name(), Parts()))
        m_confirm_button->Disable(false);
    else
        m_confirm_button->Disable(true);
}


//////////////////////////////////////////////////
// DesignWnd                                    //
//////////////////////////////////////////////////
DesignWnd::DesignWnd(int w, int h) :
    GG::Wnd(0, 0, w, h, GG::ONTOP | GG::CLICKABLE),
    m_detail_panel(0),
    m_base_selector(0),
    m_part_palette(0),
    m_main_panel(0)
{
    TempUISoundDisabler sound_disabler;
    EnableChildClipping(true);

    m_detail_panel = new EncyclopediaDetailPanel(500, 150);
    AttachChild(m_detail_panel);
    m_detail_panel->MoveTo(GG::Pt(250, 0));

    m_main_panel = new MainPanel(500, 250);
    AttachChild(m_main_panel);
    GG::Connect(m_main_panel->PartTypeClickedSignal,            &EncyclopediaDetailPanel::SetItem,  m_detail_panel);
    GG::Connect(m_main_panel->DesignConfirmedSignal,            &DesignWnd::AddDesign,              this);
    m_main_panel->MoveTo(GG::Pt(250, 150));

    m_part_palette = new PartPalette(500, 200);
    AttachChild(m_part_palette);
    GG::Connect(m_part_palette->PartTypeClickedSignal,          &EncyclopediaDetailPanel::SetItem,  m_detail_panel);
    GG::Connect(m_part_palette->PartTypeDoubleClickedSignal,    &DesignWnd::MainPanel::AddPart,     m_main_panel);
    m_part_palette->MoveTo(GG::Pt(250, 400));

    m_base_selector = new BaseSelector(250, 500);
    AttachChild(m_base_selector);
    GG::Connect(m_base_selector->DesignSelectedSignal,          &MainPanel::SetDesign,              m_main_panel);
    GG::Connect(m_base_selector->DesignComponentsSelectedSignal,&MainPanel::SetDesignComponents,    m_main_panel);
    GG::Connect(m_base_selector->HullBrowsedSignal,             &EncyclopediaDetailPanel::SetItem,  m_detail_panel);
    m_base_selector->MoveTo(GG::Pt(0, 0));

    GG::Connect(this->EmpireDesignsChangedSignal,               &BaseSelector::Reset,               m_base_selector);
}

void DesignWnd::Reset() {
    m_part_palette->Reset();
    m_base_selector->Reset();
}

void DesignWnd::Sanitize() {
    m_main_panel->Sanitize();
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

void DesignWnd::AddDesign() {
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire* empire = Empires().Lookup(empire_id);
    if (!empire) return;

    std::vector<std::string> parts = m_main_panel->Parts();
    const std::string& hull_name = m_main_panel->Hull();

    if (!ShipDesign::ValidDesign(hull_name, parts)) {
        Logger().errorStream() << "DesignWnd::AddDesign tried to add an invalid ShipDesign";
        return;
    }

    // make sure name isn't blank.  TODO: prevent duplicate names?
    std::string name = m_main_panel->DesignName();
    if (name == "")
        name = UserString("DESIGN_NAME_DEFAULT");

    const std::string& description = m_main_panel->DesignDescription();

    const HullType* hull = GetHullType(hull_name);
    std::string graphic = "hulls_design/generic_hull.png";
    if (hull)
        graphic = hull->Graphic();

    // create design from stuff chosen in UI
    ShipDesign* design = new ShipDesign(name, description, empire_id, CurrentTurn(),
                                        hull_name, parts, graphic, "some model");

    if (!design) {
        Logger().errorStream() << "DesignWnd::AddDesign failed to create a new ShipDesign object";
        return;
    }

    int new_design_id = HumanClientApp::GetApp()->GetNewDesignID();
    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new ShipDesignOrder(empire_id, new_design_id, *design)));

    EmpireDesignsChangedSignal();

    Logger().debugStream() << "Added new design: " << design->Name();

    //const Universe& universe = GetUniverse();
    //for (Universe::ship_design_iterator it = universe.beginShipDesigns(); it != universe.endShipDesigns(); ++it)
    //    Logger().debugStream() << "Shipdesign: " << it->second->Name();
}

void DesignWnd::DoLayout() {
}
