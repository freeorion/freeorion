#include "DesignWnd.h"

#include "ClientUI.h"
#include "CUIWnd.h"
#include "CUIControls.h"
#include "EncyclopediaDetailPanel.h"
#include "IconTextBrowseWnd.h"
#include "ShipDesignPanel.h"
#include "Sound.h"
#include "TextBrowseWnd.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Order.h"
#include "../util/OptionsDB.h"
#include "../util/ScopedTimer.h"
#include "../util/Directories.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "../universe/UniverseObject.h"
#include "../universe/ShipDesign.h"
#include "../parse/Parse.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/TabWnd.h>

#include <boost/cast.hpp>
#include <boost/function.hpp>
#include <boost/timer.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <algorithm>

namespace {
    const std::string   PART_CONTROL_DROP_TYPE_STRING = "Part Control";
    const std::string   HULL_PARTS_ROW_DROP_TYPE_STRING = "Hull and Parts Row";
    const std::string   COMPLETE_DESIGN_ROW_DROP_STRING = "Complete Design Row";
    const std::string   SAVED_DESIGN_ROW_DROP_STRING = "Saved Design Row";
    const std::string   EMPTY_STRING = "";
    const GG::Y         BASES_LIST_BOX_ROW_HEIGHT(100);
    const GG::X         PART_CONTROL_WIDTH(54);
    const GG::Y         PART_CONTROL_HEIGHT(54);
    const GG::X         SLOT_CONTROL_WIDTH(60);
    const GG::Y         SLOT_CONTROL_HEIGHT(60);
    const int           PAD(3);

    /** Returns texture with which to render a SlotControl, depending on \a slot_type. */
    boost::shared_ptr<GG::Texture>  SlotBackgroundTexture(ShipSlotType slot_type) {
        if (slot_type == SL_EXTERNAL)
            return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "ship_parts" / "external_slot.png", true);
        else if (slot_type == SL_INTERNAL)
            return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "ship_parts" / "internal_slot.png", true);
        else if (slot_type == SL_CORE)
            return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "ship_parts" / "core_slot.png", true);
        else
            return ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "missing.png", true);
    }

    /** Returns background texture with which to render a PartControl, depending on the
      * types of slot that the indicated \a part can be put into. */
    boost::shared_ptr<GG::Texture>  PartBackgroundTexture(const PartType* part) {
        if (part) {
            bool ex = part->CanMountInSlotType(SL_EXTERNAL);
            bool in = part->CanMountInSlotType(SL_INTERNAL);
            bool co = part->CanMountInSlotType(SL_CORE);

            if (ex && in)
                return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "ship_parts" / "independent_part.png", true);
            else if (ex)
                return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "ship_parts" / "external_part.png", true);
            else if (in)
                return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "ship_parts" / "internal_part.png", true);
            else if (co)
                return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "ship_parts" / "core_part.png", true);
        }
        return ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "missing.png", true);
    }

    float GetMainStat(const PartType* part_type)  {
        if (!part_type)
            return 0.0f;
        switch (part_type->Class()) {
            case PC_SHORT_RANGE:
            case PC_POINT_DEFENSE:
            case PC_MISSILES:
            case PC_FIGHTERS:
            case PC_SHIELD:
            case PC_DETECTION:
            case PC_STEALTH:
            case PC_FUEL:
            case PC_COLONY:
            case PC_ARMOUR:
            case PC_SPEED:
            case PC_RESEARCH:
            case PC_INDUSTRY:
            case PC_TRADE:
                return part_type->Capacity();
                break;
            case PC_GENERAL:
            case PC_BOMBARD:
            case PC_PRODICTION_LOCATION:
            default:
                return 0.0f;
        }
    }
    typedef std::map<std::pair<ShipPartClass, ShipSlotType>,
                     std::vector<const PartType* > >            PartGroupsType;

    const std::string DESIGN_FILENAME_EXTENSION = ".txt";
    const std::string UNABLE_TO_OPEN_FILE = "Unable to open file";
    boost::filesystem::path SavedDesignsDir() { return GetUserDir() / "shipdesigns"; }

    class SavedDesignsManager {
    public:
        const std::map<std::string, ShipDesign*>& GetDesigns() const
        { return m_saved_designs; }

        static SavedDesignsManager& GetSavedDesignsManager() {
            static SavedDesignsManager manager;
            return manager;
        }

        void ClearDesigns() {
            for (std::map<std::string, ShipDesign*>::iterator it = m_saved_designs.begin();
                 it != m_saved_designs.end(); ++it)
            { delete it->second; }
        }

        void RefreshDesigns() {
            using namespace boost::filesystem;
            ClearDesigns();

            path saved_designs_dir = SavedDesignsDir();
            if (!exists(saved_designs_dir))
                return;

            // scan files, parse for designs, add to this manager's designs
            std::vector<path> design_files;
            for (directory_iterator dir_it(saved_designs_dir);
                 dir_it != directory_iterator(); ++dir_it)
            {
                const path& file_path = dir_it->path();
                if (!is_regular_file(file_path))
                    continue;
                if (file_path.extension() != DESIGN_FILENAME_EXTENSION)
                { continue; }
                design_files.push_back(file_path);
            }

            for (std::vector<path>::iterator it = design_files.begin();
                 it != design_files.end(); ++it)
            {
                try {
                    std::map<std::string, ShipDesign*> file_designs;
                    parse::ship_designs(*it, file_designs);

                    for (std::map<std::string, ShipDesign*>::iterator d_it = file_designs.begin();
                         d_it != file_designs.end(); ++d_it)
                    {
                        if (m_saved_designs.find(d_it->first) == m_saved_designs.end()) {
                            m_saved_designs[d_it->first] = d_it->second;
                        } else {
                            // duplicate design name!
                            delete d_it->second;
                        }
                    }
                } catch (...) {
                    ErrorLogger() << "Failed to parse designs in " << PathString(*it);
                    continue;
                }
            }
        }

        const ShipDesign* GetDesign(const std::string& design_name) {
            std::map<std::string, ShipDesign*>::const_iterator it = m_saved_designs.find(design_name);
            if (it == m_saved_designs.end())
                return 0;
            return it->second;
        }

        std::map<std::string, ShipDesign*>::iterator begin() { return m_saved_designs.begin(); }
        std::map<std::string, ShipDesign*>::iterator end()   { return m_saved_designs.end(); }

        /* Causes the human client Empire to add all saved designs. */
        void LoadAllSavedDesigns() {
            int empire_id = HumanClientApp::GetApp()->EmpireID();
            if (const Empire* empire = GetEmpire(empire_id)) {
                DebugLogger() << "SavedDesignsManager::LoadAllSavedDesigns";
                for (std::map<std::string, ShipDesign*>::iterator savedit = begin();
                    savedit != end(); ++savedit)
                {
                    bool already_got = false;
                    for (Empire::ShipDesignItr it = empire->ShipDesignBegin();
                        it != empire->ShipDesignEnd(); ++it)
                    {
                        const ShipDesign& ship_design = *GetShipDesign(*it);
                        if (ship_design == *(savedit->second)) {
                            already_got = true;
                            break;
                        }
                    }
                    if (!already_got) {
                        DebugLogger() << "SavedDesignsManager::LoadAllSavedDesigns adding saved design: " << savedit->second->Name();
                        int new_design_id = HumanClientApp::GetApp()->GetNewDesignID();
                        HumanClientApp::GetApp()->Orders().IssueOrder(
                            OrderPtr(new ShipDesignOrder(empire_id, new_design_id, *(savedit->second))));
                    } else {
                        DebugLogger() << "SavedDesignsManager::LoadAllSavedDesigns saved design already present: " << savedit->second->Name();
                    }
                }
            } else {
                DebugLogger() << "SavedDesignsManager::LoadAllSavedDesigns HumanClient Does Not Control an Empire";
            }
        }

    private:
        SavedDesignsManager() {
            if (s_instance)
                throw std::runtime_error("Attempted to create more than one SavedDesignsManager.");
            s_instance = this;
            RefreshDesigns();
        }

        ~SavedDesignsManager()
        { ClearDesigns(); }

        std::map<std::string, ShipDesign*>  m_saved_designs;
        static SavedDesignsManager*         s_instance;
    };
    SavedDesignsManager* SavedDesignsManager::s_instance = 0;

    SavedDesignsManager& GetSavedDesignsManager()
    { return SavedDesignsManager::GetSavedDesignsManager(); }

    void WriteDesignToFile(int design_id, boost::filesystem::path& file) {
        const ShipDesign* design = GetShipDesign(design_id);
        if (!design)
            return;

        try {
            boost::filesystem::ofstream ofs(file);
            if (!ofs)
                throw std::runtime_error(UNABLE_TO_OPEN_FILE);
            ofs << design->Dump();

        } catch (const std::exception& e) {
            ErrorLogger() << "Error writing design file.  Exception: " << ": " << e.what();
            ClientUI::MessageBox(e.what(), true);
        }
    }

    void ShowSaveDesignDialog(int design_id) {
        const ShipDesign* design = GetShipDesign(design_id);
        if (!design)
            return;

        // ensure directory present
        boost::filesystem::path designs_dir_path(SavedDesignsDir());
        if (!exists(designs_dir_path))
            boost::filesystem::create_directories(designs_dir_path);

        // default file name: take design name, process a bit to make nicer / safe
        std::string default_file_name = design->Name();
        boost::trim(default_file_name);
        boost::replace_all(default_file_name, " ", "_");
        std::string bad_chars = "/?<>\\:*|\".";
        for (unsigned int i = 0; i < bad_chars.length(); ++i)
            boost::replace_all(default_file_name, bad_chars.substr(i,1), "");

        if (default_file_name.length() > 200)
            default_file_name = default_file_name.substr(0, 200);

        default_file_name += DESIGN_FILENAME_EXTENSION;

        std::vector<std::pair<std::string, std::string> > filters;
        filters.push_back(std::make_pair(UserString("SHIP_DESIGN_FILES"),
                                         "*" + DESIGN_FILENAME_EXTENSION));

        try {
            FileDlg dlg(PathString(designs_dir_path),
                        PathString(designs_dir_path / default_file_name),
                        true, false, filters);
            dlg.Run();
            if (!dlg.Result().empty()) {
                boost::filesystem::path save_path =
                boost::filesystem::absolute(*dlg.Result().begin());
                WriteDesignToFile(design_id, save_path);
            }
        } catch (const std::exception& e) {
            ClientUI::MessageBox(e.what(), true);
            return;
        }
    }
}

//////////////////////////////////////////////////
// PartControl                                  //
//////////////////////////////////////////////////
/** UI representation of a ship part.  Displayed in the PartPalette, and can be
  * dragged onto SlotControls to add parts to the design. */
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

    mutable boost::signals2::signal<void (const PartType*)> ClickedSignal;
    mutable boost::signals2::signal<void (const PartType*)> DoubleClickedSignal;
private:
    GG::StaticGraphic*  m_icon;
    GG::StaticGraphic*  m_background;
    const PartType*     m_part;
};

PartControl::PartControl(const PartType* part) :
    GG::Control(GG::X0, GG::Y0, SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT, GG::INTERACTIVE),
    m_icon(0),
    m_background(0),
    m_part(part)
{
    if (!m_part)
        return;

        m_background = new GG::StaticGraphic(PartBackgroundTexture(m_part), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        m_background->Resize(GG::Pt(SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT));
        m_background->Show();
        AttachChild(m_background);


        // position of part image centred within part control.  control is size of a slot, but the
        // part image is smaller
        GG::X part_left = (Width() - PART_CONTROL_WIDTH) / 2;
        GG::Y part_top = (Height() - PART_CONTROL_HEIGHT) / 2;

        //DebugLogger() << "PartControl::PartControl this: " << this << " part: " << part << " named: " << (part ? part->Name() : "no part");
        m_icon = new GG::StaticGraphic(ClientUI::PartIcon(m_part->Name()), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        m_icon->MoveTo(GG::Pt(part_left, part_top));
        m_icon->Resize(GG::Pt(PART_CONTROL_WIDTH, PART_CONTROL_HEIGHT));
        m_icon->Show();
        AttachChild(m_icon);


        SetDragDropDataType(PART_CONTROL_DROP_TYPE_STRING);


        //DebugLogger() << "PartControl::PartControl part name: " << m_part->Name();
        SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
        SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
            new IconTextBrowseWnd(ClientUI::PartIcon(m_part->Name()),
                                                     UserString(m_part->Name()),
                                                     UserString(m_part->Description()))
                                                    )
                                 );
}

void PartControl::Render() {}

void PartControl::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ ClickedSignal(m_part); }

void PartControl::LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ DoubleClickedSignal(m_part); }


//////////////////////////////////////////////////
// PartsListBox                                 //
//////////////////////////////////////////////////
/** Arrangement of PartControls that can be dragged onto SlotControls */
class PartsListBox : public CUIListBox {
public:
    class PartsListBoxRow : public CUIListBox::Row {
    public:
        PartsListBoxRow(GG::X w, GG::Y h);
        virtual void    ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination);
    };

    /** \name Structors */ //@{
    PartsListBox(void);
    //@}

    /** \name Accessors */ //@{
    const std::set<ShipPartClass>&  GetClassesShown() const;
    const std::set<ShipSlotType>&   GetSlotTypesShown() const;
    const std::pair<bool, bool>&    GetAvailabilitiesShown() const; // .first -> available items; .second -> unavailable items
    //@}
    bool                            GetShowingSuperfluous() const { return m_show_superfluous_parts; }

    /** \name Mutators */ //@{
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);

    PartGroupsType  GroupAvailableDisplayableParts(const Empire* empire);
    void            CullSuperfluousParts(std::vector<const PartType* >& this_group,
                                         ShipPartClass pclass, int empire_id, int loc_id);
    void            Populate();

    void            ShowClass(ShipPartClass part_class, bool refresh_list = true);
    void            ShowAllClasses(bool refresh_list = true);
    void            HideClass(ShipPartClass part_class, bool refresh_list = true);
    void            HideAllClasses(bool refresh_list = true);

    void            ShowAvailability(bool available, bool refresh_list = true);
    void            HideAvailability(bool available, bool refresh_list = true);

    void            ShowSuperfluousParts(bool refresh_list = true);
    void            HideSuperfluousParts(bool refresh_list = true);
    //@}

    mutable boost::signals2::signal<void (const PartType*)> PartTypeClickedSignal;
    mutable boost::signals2::signal<void (const PartType*)> PartTypeDoubleClickedSignal;

private:
    std::set<ShipPartClass> m_part_classes_shown;   // which part classes should be shown
    std::pair<bool, bool>   m_availabilities_shown; // first indicates whether available parts should be shown.  second indicates whether unavailable parts should be shown
    bool                    m_show_superfluous_parts;

    int                     m_previous_num_columns;
};

PartsListBox::PartsListBoxRow::PartsListBoxRow(GG::X w, GG::Y h) :
    CUIListBox::Row(w, h, "")    // drag_drop_data_type = "" implies not draggable row
{}

void PartsListBox::PartsListBoxRow::ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination) {
    if (wnds.empty())
        return;
    const GG::Wnd* wnd = wnds.front();  // should only be one wnd in list because PartControls doesn't allow selection, so dragging is only one-at-a-time
    const GG::Control* control = dynamic_cast<const GG::Control*>(wnd);
    if (!control)
        delete wnd;

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

PartsListBox::PartsListBox(void) :
    CUIListBox(),
    m_part_classes_shown(),
    m_availabilities_shown(std::make_pair(false, false)),
    m_show_superfluous_parts(true),
    m_previous_num_columns(-1)
{ SetStyle(GG::LIST_NOSEL); }

const std::set<ShipPartClass>& PartsListBox::GetClassesShown() const
{ return m_part_classes_shown; }

const std::pair<bool, bool>& PartsListBox::GetAvailabilitiesShown() const
{ return m_availabilities_shown; }

void PartsListBox::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    // maybe later do something interesting with docking
    CUIListBox::SizeMove(ul, lr);

    if (Visible() && old_size != GG::Wnd::Size()) {
        // determine how many columns can fit in the box now...
        const GG::X TOTAL_WIDTH = Size().x - ClientUI::ScrollWidth();
        const int NUM_COLUMNS = std::max(1, Value(TOTAL_WIDTH / (SLOT_CONTROL_WIDTH + GG::X(PAD))));

        if (NUM_COLUMNS != m_previous_num_columns)
            Populate();
    }
}

PartGroupsType PartsListBox::GroupAvailableDisplayableParts(const Empire* empire) {
    const PartTypeManager& manager = GetPartTypeManager();
    PartGroupsType part_groups;
    // loop through all possible parts
    for (PartTypeManager::iterator part_it = manager.begin(); part_it != manager.end(); ++part_it) {
        const PartType* part = part_it->second;
        if (!part->Producible())
            continue;

        // check whether this part should be shown in list
        ShipPartClass part_class = part->Class();
        if (m_part_classes_shown.find(part_class) == m_part_classes_shown.end())
            continue;   // part of this class is not requested to be shown

        bool part_available = empire ? empire->ShipPartAvailable(part->Name()) : true;
        if (!(part_available && m_availabilities_shown.first) &&
            !(!part_available && m_availabilities_shown.second))
        {
            // part is available but available parts shouldn't be shown, or
            // part isn't available and not available parts shouldn't be shown
            continue;
        }

        const std::vector<ShipSlotType>& slot_types = part->MountableSlotTypes();
        for (std::vector<ShipSlotType>::const_iterator it = slot_types.begin();
             it != slot_types.end(); ++it)
        { part_groups[std::make_pair(part_class, *it)].push_back(part); }
    }
    return part_groups;
}

void PartsListBox::CullSuperfluousParts(std::vector<const PartType* >& this_group,
                                        ShipPartClass pclass, int empire_id, int loc_id)
{
    /// This is not merely a check for obsolescence; see PartsListBox::Populate for more info
    for (std::vector<const PartType* >::iterator part_it = this_group.begin();
         part_it != this_group.end(); ++part_it)
    {
        const PartType* checkPart = *part_it;
        for (std::vector<const PartType* >::iterator check_it = this_group.begin();
             check_it != this_group.end(); ++check_it )
        {
            const PartType* ref_part = *check_it;
            if ((GetMainStat(checkPart) < GetMainStat(ref_part)) &&
                (checkPart->ProductionCost(empire_id, loc_id) >= ref_part->ProductionCost(empire_id, loc_id)) &&
                (checkPart->ProductionTime(empire_id, loc_id) >= ref_part->ProductionTime(empire_id, loc_id)))
            {
                this_group.erase(part_it--);
                break;
            }
        }
    }
}

void PartsListBox::Populate() {
    ScopedTimer scoped_timer("PartsListBox::Populate");

    const GG::X TOTAL_WIDTH = ClientWidth() - ClientUI::ScrollWidth();
    const int NUM_COLUMNS = std::max(1, Value(TOTAL_WIDTH / (SLOT_CONTROL_WIDTH + GG::X(PAD))));

    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire* empire = GetEmpire(empire_id);  // may be 0

    int cur_col = NUM_COLUMNS;
    PartsListBoxRow* cur_row = 0;

    // remove parts currently in rows of listbox
    Clear();

    /** 
     * The Parts are first filtered for availability to this empire and according to the current 
     * selections of which part classes are to be displayed.  Then, in order to eliminate presentation
     * of clearly suboptimal parts, such as Mass Driver I when Mass Driver II is available at the same
     * cost & build time, some orgnization, paring and sorting of parts is done. The previously 
     * filtered parts are grouped according to (class, slot).  Within each group, parts are compared 
     * and pared for display; only parts within the same group may suppress display of each other.
     * The paring is (currently) done on the basis of main stat, construction cost, and construction 
     * time. If two parts have the same class and slot, and one has a lower main stat but also a lower
     * cost, they will both be presented; if one has a higher main stat and is at least as good on cost
     * and time, it will suppress the other.  
     * 
     * An example of one of the more subtle possible results is that if a part class had multiple parts
     * with different but overlapping MountableSlotType patterns, then a part with two possible slot
     * types might be rendered superfluous for the first slot type by a first other part, be rendered
     * superfluous for the second slot type by a second other part, even if neither of the latter two
     * parts would be considered to individually render the former part obsolete.
     */    

    /// filter parts by availability and current designation of classes for display; group according to (class, slot)
    PartGroupsType part_groups = GroupAvailableDisplayableParts(empire);

    // get empire id and location to use for cost and time comparisons
    int loc_id = INVALID_OBJECT_ID;
    if (empire) {
        TemporaryPtr<const UniverseObject> location = GetUniverseObject(empire->CapitalID());
        loc_id = location ? location->ID() : INVALID_OBJECT_ID;
    }

    // if showing parts for a particular empire, cull redundant parts (if enabled)
    if (empire) {
        for (PartGroupsType::iterator group_it=part_groups.begin();
             group_it != part_groups.end(); group_it++)
        {
            ShipPartClass pclass = group_it->first.first;
            // currently, only cull ShortRange Weapons, though the culling code
            // is more broadly applicable.
            if (pclass == PC_SHORT_RANGE && !m_show_superfluous_parts)
                CullSuperfluousParts(group_it->second, pclass, empire_id, loc_id);
        }
    }

    // now sort the parts within each group according to main stat, via weak
    // sorting in a multimap also, if a part was in multiple groups due to being
    // compatible with multiple slot types, ensure it is only displayed once
    std::set<const PartType* > already_added;
    for (PartGroupsType::iterator group_it=part_groups.begin();
         group_it != part_groups.end(); group_it++)
    {
        std::vector<const PartType* > this_group = group_it->second;
        std::multimap<double, const PartType*> sorted_group;
        for (std::vector<const PartType* >::iterator part_it = this_group.begin();
             part_it != this_group.end(); ++part_it)
        {
            const PartType* part = *part_it;
            if (already_added.find(part) != already_added.end())
                continue;
            already_added.insert(part);
            sorted_group.insert(std::make_pair(GetMainStat(part), part));
        }

        // take the sorted parts and make UI elements (technically rows) for the PartsListBox
        for (std::multimap<double, const PartType*>::iterator sorted_it = sorted_group.begin();
             sorted_it != sorted_group.end(); ++sorted_it)
        {
            const PartType* part = sorted_it->second;
            // check if current row is full, and make a new row if necessary
            if (cur_col >= NUM_COLUMNS) {
                if (cur_row)
                    Insert(cur_row);
                cur_col = 0;
                cur_row = new PartsListBoxRow(TOTAL_WIDTH, SLOT_CONTROL_HEIGHT + GG::Y(PAD));
            }
            ++cur_col;

            // make new part control and add to row
            PartControl* control = new PartControl(part);
            GG::Connect(control->ClickedSignal,         PartsListBox::PartTypeClickedSignal);
            GG::Connect(control->DoubleClickedSignal,   PartsListBox::PartTypeDoubleClickedSignal);
            cur_row->push_back(control);
        }
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

void PartsListBox::ShowSuperfluousParts(bool refresh_list) {
    if (m_show_superfluous_parts)
        return;
    m_show_superfluous_parts = true;
    if (refresh_list)
        Populate();
}

void PartsListBox::HideSuperfluousParts(bool refresh_list) {
    if (!m_show_superfluous_parts)
        return;
    m_show_superfluous_parts = false;
    if (refresh_list)
        Populate();
}


//////////////////////////////////////////////////
// DesignWnd::PartPalette                       //
//////////////////////////////////////////////////
/** Contains graphical list of PartControl which can be dragged and dropped
  * onto slots to assign parts to those slots */
class DesignWnd::PartPalette : public CUIWnd {
public:
    /** \name Structors */ //@{
    PartPalette(GG::X w, GG::Y h);
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

    void            ShowSuperfluous(bool refresh_list = true);
    void            HideSuperfluous(bool refresh_list = true);
    void            ToggleSuperfluous(bool refresh_list = true);

    void            Reset();
    //@}

    mutable boost::signals2::signal<void (const PartType*)> PartTypeClickedSignal;
    mutable boost::signals2::signal<void (const PartType*)> PartTypeDoubleClickedSignal;

private:
    void            DoLayout();

    PartsListBox*   m_parts_list;

    std::map<ShipPartClass, CUIButton*> m_class_buttons;
    std::pair<CUIButton*, CUIButton*>   m_availability_buttons;
    CUIButton*                          m_superfluous_parts_button;
};

DesignWnd::PartPalette::PartPalette(GG::X w, GG::Y h) :
    CUIWnd(UserString("DESIGN_WND_PART_PALETTE_TITLE"), GG::X0, GG::Y0, w, h,
           GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE),
    m_parts_list(0),
    m_superfluous_parts_button(0)
{
    //TempUISoundDisabler sound_disabler;     // should be redundant with disabler in DesignWnd::DesignWnd.  uncomment if this is not the case
    SetChildClippingMode(ClipToClient);

    m_parts_list = new PartsListBox();
    AttachChild(m_parts_list);
    GG::Connect(m_parts_list->PartTypeClickedSignal,        PartTypeClickedSignal);
    GG::Connect(m_parts_list->PartTypeDoubleClickedSignal,  PartTypeDoubleClickedSignal);

    const PartTypeManager& part_manager = GetPartTypeManager();

    // class buttons
    for (ShipPartClass part_class = ShipPartClass(0); part_class != NUM_SHIP_PART_CLASSES; part_class = ShipPartClass(part_class + 1)) {
        // are there any parts of this class?
        bool part_of_this_class_exists = false;
        for (PartTypeManager::iterator part_it = part_manager.begin(); part_it != part_manager.end(); ++part_it) {
            if (const PartType* part = part_it->second) {
                if (part->Class() == part_class) {
                    part_of_this_class_exists = true;
                    break;
                }
            }
        }
        if (!part_of_this_class_exists)
            continue;

        m_class_buttons[part_class] = new CUIButton(UserString(boost::lexical_cast<std::string>(part_class)), ClientUI::ButtonHiliteColor(), ClientUI::ButtonHiliteBorderColor());
        AttachChild(m_class_buttons[part_class]);
        GG::Connect(m_class_buttons[part_class]->LeftClickedSignal,
                    boost::bind(&DesignWnd::PartPalette::ToggleClass, this, part_class, true));
    }

    // availability buttons
    m_availability_buttons.first = new CUIButton(UserString("PRODUCTION_WND_AVAILABILITY_AVAILABLE"), ClientUI::ButtonHiliteColor(), ClientUI::ButtonHiliteBorderColor());
    AttachChild(m_availability_buttons.first);
    GG::Connect(m_availability_buttons.first->LeftClickedSignal,
                boost::bind(&DesignWnd::PartPalette::ToggleAvailability, this, true, true));
    m_availability_buttons.second = new CUIButton(UserString("PRODUCTION_WND_AVAILABILITY_UNAVAILABLE"), ClientUI::ButtonHiliteColor(), ClientUI::ButtonHiliteBorderColor());
    AttachChild(m_availability_buttons.second);
    GG::Connect(m_availability_buttons.second->LeftClickedSignal,
                boost::bind(&DesignWnd::PartPalette::ToggleAvailability, this, false, true));

    // superfluous parts button
    m_superfluous_parts_button = new CUIButton(UserString("PRODUCTION_WND_REDUNDANT"), ClientUI::ButtonHiliteColor(), ClientUI::ButtonHiliteBorderColor());
    AttachChild(m_superfluous_parts_button);
    GG::Connect(m_superfluous_parts_button->LeftClickedSignal,
                boost::bind(&DesignWnd::PartPalette::ToggleSuperfluous, this, true));

    // default to showing nothing
    ShowAllClasses(false);
    ShowAvailability(true, false);
    ShowSuperfluous(false);

    DoLayout();
}

void DesignWnd::PartPalette::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    CUIWnd::SizeMove(ul, lr);
    DoLayout();
}

void DesignWnd::PartPalette::DoLayout() {
    const int PTS = ClientUI::Pts();
    const GG::X PTS_WIDE(PTS/2);         // guess at how wide per character the font needs
    const GG::Y  BUTTON_HEIGHT(PTS*3/2);
    const int BUTTON_SEPARATION = 3;    // vertical or horizontal sepration between adjacent buttons
    const int BUTTON_EDGE_PAD = 2;      // distance from edges of control to buttons
    const GG::X RIGHT_EDGE_PAD(8);       // to account for border of CUIWnd

    const GG::X USABLE_WIDTH = std::max(ClientWidth() - RIGHT_EDGE_PAD, GG::X1);   // space in which to fit buttons
    const int GUESSTIMATE_NUM_CHARS_IN_BUTTON_LABEL = 14;                   // rough guesstimate... avoid overly long part class names
    const GG::X MIN_BUTTON_WIDTH = PTS_WIDE*GUESSTIMATE_NUM_CHARS_IN_BUTTON_LABEL;
    const int MAX_BUTTONS_PER_ROW = std::max(Value(USABLE_WIDTH / (MIN_BUTTON_WIDTH + BUTTON_SEPARATION)), 1);

    const int NUM_CLASS_BUTTONS = std::max(1, static_cast<int>(m_class_buttons.size()));
    const int NUM_SUPERFLUOUS_CULL_BUTTONS = 1;
    const int NUM_AVAILABILITY_BUTTONS = 2;
    const int NUM_NON_CLASS_BUTTONS = NUM_SUPERFLUOUS_CULL_BUTTONS + NUM_AVAILABILITY_BUTTONS;

    // determine whether to put non-class buttons (availability and redundancy)
    // in one column or two.
    // -> if class buttons fill up fewer rows than (the non-class buttons in one
    // column), split the non-class buttons into two columns
    int num_non_class_buttons_per_row = 1;
    if (NUM_CLASS_BUTTONS < NUM_NON_CLASS_BUTTONS*(MAX_BUTTONS_PER_ROW - num_non_class_buttons_per_row))
        num_non_class_buttons_per_row = 2;

    const int MAX_CLASS_BUTTONS_PER_ROW = std::max(1, MAX_BUTTONS_PER_ROW - num_non_class_buttons_per_row);

    const int NUM_CLASS_BUTTON_ROWS = static_cast<int>(std::ceil(static_cast<float>(NUM_CLASS_BUTTONS) / MAX_CLASS_BUTTONS_PER_ROW));
    const int NUM_CLASS_BUTTONS_PER_ROW = static_cast<int>(std::ceil(static_cast<float>(NUM_CLASS_BUTTONS) / NUM_CLASS_BUTTON_ROWS));

    const int TOTAL_BUTTONS_PER_ROW = NUM_CLASS_BUTTONS_PER_ROW + num_non_class_buttons_per_row;

    const GG::X BUTTON_WIDTH = (USABLE_WIDTH - (TOTAL_BUTTONS_PER_ROW - 1)*BUTTON_SEPARATION) / TOTAL_BUTTONS_PER_ROW;

    const GG::X COL_OFFSET = BUTTON_WIDTH + BUTTON_SEPARATION;    // horizontal distance between each column of buttons
    const GG::Y ROW_OFFSET = BUTTON_HEIGHT + BUTTON_SEPARATION;   // vertical distance between each row of buttons

    // place class buttons
    int col = NUM_CLASS_BUTTONS_PER_ROW;
    int row = -1;
    for (std::map<ShipPartClass, CUIButton*>::iterator it = m_class_buttons.begin();
         it != m_class_buttons.end(); ++it)
    {
        if (col >= NUM_CLASS_BUTTONS_PER_ROW) {
            col = 0;
            ++row;
        }
        GG::Pt ul(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
        GG::Pt lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
        it->second->SizeMove(ul, lr);
        ++col;
    }

    // place parts list.  note: assuming at least as many rows of class buttons
    //                          as availability buttons, as should be the case
    //                          given how num_non_class_buttons_per_row is determined
    m_parts_list->SizeMove(GG::Pt(GG::X0, BUTTON_EDGE_PAD + ROW_OFFSET*(row + 1)),
                           ClientSize() - GG::Pt(GG::X(2*BUTTON_SEPARATION), GG::Y(2*BUTTON_SEPARATION)));


    // place parts list.  note: assuming at least as many rows of class buttons as availability buttons, as should
    //                          be the case given how num_non_class_buttons_per_row is determined
    m_parts_list->SizeMove(GG::Pt(GG::X0, BUTTON_EDGE_PAD + ROW_OFFSET*(row + 1)), ClientSize() - GG::Pt(GG::X(BUTTON_SEPARATION), GG::Y(BUTTON_SEPARATION)));

    GG::Pt ul, lr;

    // place slot type buttons
    col = NUM_CLASS_BUTTONS_PER_ROW;
    row = 0;
    ul = GG::Pt(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
    lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_superfluous_parts_button->SizeMove(ul, lr);

    // place availability buttons
    if (num_non_class_buttons_per_row > 1) {
        ++col;
        row = 0;
    } else {
        ++row;
    }
    ul = GG::Pt(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
    lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_availability_buttons.first->SizeMove(ul, lr);

    if (row != 0 && num_non_class_buttons_per_row > 2) {
        ++col;
        row = 0;
    } else {
        ++row;
    }
    ul = GG::Pt(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
    lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_availability_buttons.second->SizeMove(ul, lr);


    //GG::Pt ul, lr;

    //// place availability buttons
    //if (num_non_class_buttons_per_row > 1) {
    //    ++col;
    //    row = 0;
    //}
    //ul = GG::Pt(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
    //lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    //m_availability_buttons.first->SizeMove(ul, lr);

    //++row;
    //ul = GG::Pt(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
    //lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    //m_availability_buttons.second->SizeMove(ul, lr);

    //// place superfluous culling button
    //++row;
    //ul = GG::Pt(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
    //lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    //m_superfluous_parts_button->SizeMove(ul, lr);

}

void DesignWnd::PartPalette::ShowClass(ShipPartClass part_class, bool refresh_list) {
    if (part_class >= ShipPartClass(0) && part_class < NUM_SHIP_PART_CLASSES) {
        m_parts_list->ShowClass(part_class, refresh_list);
        m_class_buttons[part_class]->SetCheck();
    } else {
        throw std::invalid_argument("PartPalette::ShowClass was passed an invalid ShipPartClass");
    }
}

void DesignWnd::PartPalette::ShowAllClasses(bool refresh_list) {
    m_parts_list->ShowAllClasses(refresh_list);
    for (std::map<ShipPartClass, CUIButton*>::iterator it = m_class_buttons.begin(); it != m_class_buttons.end(); ++it)
        it->second->SetCheck();
}

void DesignWnd::PartPalette::HideClass(ShipPartClass part_class, bool refresh_list) {
    if (part_class >= ShipPartClass(0) && part_class < NUM_SHIP_PART_CLASSES) {
        m_parts_list->HideClass(part_class, refresh_list);
        m_class_buttons[part_class]->SetCheck(false);
    } else {
        throw std::invalid_argument("PartPalette::HideClass was passed an invalid ShipPartClass");
    }
}

void DesignWnd::PartPalette::HideAllClasses(bool refresh_list) {
    m_parts_list->HideAllClasses(refresh_list);
    for (std::map<ShipPartClass, CUIButton*>::iterator it = m_class_buttons.begin(); it != m_class_buttons.end(); ++it)
        it->second->SetCheck(false);
}

void DesignWnd::PartPalette::ToggleClass(ShipPartClass part_class, bool refresh_list) {
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

void DesignWnd::PartPalette::ShowAvailability(bool available, bool refresh_list) {
    m_parts_list->ShowAvailability(available, refresh_list);
    if (available)
        m_availability_buttons.first->SetCheck();
    else
        m_availability_buttons.second->SetCheck();
}

void DesignWnd::PartPalette::HideAvailability(bool available, bool refresh_list) {
    m_parts_list->HideAvailability(available, refresh_list);
    if (available)
        m_availability_buttons.first->SetCheck(false);
    else
        m_availability_buttons.second->SetCheck(false);
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

void DesignWnd::PartPalette::ShowSuperfluous(bool refresh_list) {
    m_parts_list->ShowSuperfluousParts(refresh_list);
    m_superfluous_parts_button->SetCheck();
}

void DesignWnd::PartPalette::HideSuperfluous(bool refresh_list) {
    m_parts_list->HideSuperfluousParts(refresh_list);
    m_superfluous_parts_button->SetCheck(false);
}

void DesignWnd::PartPalette::ToggleSuperfluous(bool refresh_list) {
    bool showing_superfluous = m_parts_list->GetShowingSuperfluous();
    if (showing_superfluous)
        HideSuperfluous(refresh_list);
    else
        ShowSuperfluous(refresh_list);
}

void DesignWnd::PartPalette::Reset()
{ m_parts_list->Populate(); }


//////////////////////////////////////////////////
// BasesListBox                                  //
//////////////////////////////////////////////////
/** List of starting points for designs, such as empty hulls, existing designs
  * kept by this empire or seen elsewhere in the universe, design template
  * scripts or saved (on disk) designs from previous games. */
class BasesListBox : public CUIListBox {
public:
    /** \name Structors */ //@{
    BasesListBox(void);
    //@}

    /** \name Accessors */ //@{
    const std::pair<bool, bool>&    GetAvailabilitiesShown() const;
    //@}

    /** \name Mutators */ //@{
    virtual void                    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    virtual void                    ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination);

    void                            SetEmpireShown(int empire_id, bool refresh_list = true);

    virtual void                    Populate();

    void                            ShowEmptyHulls(bool refresh_list = true);
    void                            ShowCompletedDesigns(bool refresh_list = true);
    void                            ShowSavedDesigns(bool refresh_list = true);
    void                            ShowMonsters(bool refresh_list = true);

    void                            ShowAvailability(bool available, bool refresh_list = true);
    void                            HideAvailability(bool available, bool refresh_list = true);
    //@}

    mutable boost::signals2::signal<void (int)>                 DesignSelectedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::vector<std::string>&)>
                                                                DesignComponentsSelectedSignal;
    mutable boost::signals2::signal<void (const std::string&)>  SavedDesignSelectedSignal;

    mutable boost::signals2::signal<void (const ShipDesign*)>   DesignClickedSignal;
    mutable boost::signals2::signal<void (const HullType*)>     HullClickedSignal;
    mutable boost::signals2::signal<void (const ShipDesign*)>   DesignRightClickedSignal;

    class HullPanel : public GG::Control {
    public:
        HullPanel(GG::X w, GG::Y h, const std::string& hull);
        virtual void                    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
        virtual void                    Render() {}
    private:
        GG::StaticGraphic*              m_graphic;
        CUILabel*                       m_name;
    };

    class SavedDesignPanel : public GG::Control {
    public:
        SavedDesignPanel(GG::X w, GG::Y h, const std::string& saved_design_name);
        virtual void                    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
        virtual void                    Render() {}
    private:
        GG::StaticGraphic*              m_graphic;
        CUILabel*                       m_name;
    };

    class BasesListBoxRow : public CUIListBox::Row {
    public:
        BasesListBoxRow(GG::X w, GG::Y h);
        virtual void                    Render();
        virtual void                    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    };

    class HullAndPartsListBoxRow : public BasesListBoxRow {
    public:
        HullAndPartsListBoxRow(GG::X w, GG::Y h);
        HullAndPartsListBoxRow(GG::X w, GG::Y h, const std::string& hull,
                               const std::vector<std::string>& parts);
        const std::string&              Hull() const    { return m_hull; }
        const std::vector<std::string>& Parts() const   { return m_parts; }
    protected:
        std::string                     m_hull;
        std::vector<std::string>        m_parts;
    };

    class CompletedDesignListBoxRow : public BasesListBoxRow {
    public:
        CompletedDesignListBoxRow(GG::X w, GG::Y h, int design_id);
        int                             DesignID() const { return m_design_id; }
    private:
        int                             m_design_id;
    };

    class SavedDesignListBoxRow : public HullAndPartsListBoxRow {
    public:
        SavedDesignListBoxRow(GG::X w, GG::Y h, const std::string& design_name);
        const std::string&              DesignName() const;
        const std::string&              Description() const;
        bool                            LookupInStringtable() const;

    private:
        std::string                     m_design_name;
    };

private:
    void    BaseDoubleClicked(GG::ListBox::iterator it);
    void    BaseLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt);
    void    BaseRightClicked(GG::ListBox::iterator it, const GG::Pt& pt);

    GG::Pt  ListRowSize();
    void    InitRowSizes();

    void    PopulateWithEmptyHulls();
    void    PopulateWithCompletedDesigns();
    void    PopulateWithSavedDesigns();
    void    PopulateWithMonsters();

    int                         m_empire_id_shown;
    std::pair<bool, bool>       m_availabilities_shown; // .first indicates whether available parts should be shown.  .second indicates whether unavailable parts should be shown
    bool                        m_showing_empty_hulls;
    bool                        m_showing_completed_designs;
    bool                        m_showing_saved_designs;
    bool                        m_showing_monsters;

    std::set<int>               m_designs_in_list;
    std::set<std::string>       m_hulls_in_list;
    std::set<std::string>       m_saved_desgins_in_list;

    boost::signals2::connection m_empire_designs_changed_signal;
};

BasesListBox::BasesListBoxRow::BasesListBoxRow(GG::X w, GG::Y h) :
    CUIListBox::Row(w, h, "BasesListBoxRow")
{}

void BasesListBox::BasesListBoxRow::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::FlatRectangle(ul, lr, ClientUI::WndColor(), GG::CLR_WHITE, 1);
}

void BasesListBox::BasesListBoxRow::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    CUIListBox::Row::SizeMove(ul, lr);
    if (!empty() && old_size != Size())
        at(0)->Resize(Size());
}

BasesListBox::HullPanel::HullPanel(GG::X w, GG::Y h, const std::string& hull) :
    GG::Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
    m_graphic(0),
    m_name(0)
{
    m_graphic = new GG::StaticGraphic(ClientUI::HullIcon(hull),
                                      GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC);
    m_graphic->Resize(GG::Pt(w, h));
    AttachChild(m_graphic);
    m_name = new CUILabel(UserString(hull), GG::FORMAT_NOWRAP);
    AttachChild(m_name);
}

void BasesListBox::HullPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Control::SizeMove(ul, lr);
    if (m_graphic)
        m_graphic->Resize(Size());
    if (m_name)
        m_name->Resize(GG::Pt(Width(), m_name->Height()));
}

BasesListBox::SavedDesignPanel::SavedDesignPanel(GG::X w, GG::Y h, const std::string& saved_design_name) :
    GG::Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
    m_graphic(0),
    m_name(0)
{
    const ShipDesign* design = GetSavedDesignsManager().GetDesign(saved_design_name);
    if (design) {
        m_graphic = new GG::StaticGraphic(ClientUI::HullIcon(design->Hull()),
                                          GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC);
        m_graphic->Resize(GG::Pt(w, h));
        AttachChild(m_graphic);

        m_name = new CUILabel(design->Name(), GG::FORMAT_NOWRAP);
        AttachChild(m_name);
    }
}

void BasesListBox::SavedDesignPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Control::SizeMove(ul, lr);
    if (m_graphic)
        m_graphic->Resize(Size());
    if (m_name)
        m_name->Resize(GG::Pt(Width(), m_name->Height()));
}

BasesListBox::HullAndPartsListBoxRow::HullAndPartsListBoxRow(GG::X w, GG::Y h) :
    BasesListBoxRow(w, h)
{}

BasesListBox::HullAndPartsListBoxRow::HullAndPartsListBoxRow(GG::X w, GG::Y h, const std::string& hull,
                                                             const std::vector<std::string>& parts) :
    BasesListBoxRow(w, h),
    m_hull(hull),
    m_parts(parts)
{
    if (m_parts.empty() && !m_hull.empty()) {
        // contents are just a hull
        push_back(new HullPanel(w, h, m_hull));
    } else if (!m_parts.empty() && !m_hull.empty()) {
        // contents are a hull and parts  TODO: make a HullAndPartsPanel
        GG::StaticGraphic* icon = new GG::StaticGraphic(ClientUI::HullTexture(hull), GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC);
        icon->Resize(GG::Pt(w, h));
        push_back(icon);
    }
    SetDragDropDataType(HULL_PARTS_ROW_DROP_TYPE_STRING);
}

BasesListBox::CompletedDesignListBoxRow::CompletedDesignListBoxRow(GG::X w, GG::Y h,
                                                                   int design_id) :
    BasesListBoxRow(w, h),
    m_design_id(design_id)
{
    std::string hull;
    if (const ShipDesign* ship_design = GetShipDesign(design_id))
        hull = ship_design->Hull();
    push_back(new ShipDesignPanel(w, h, design_id));
    SetDragDropDataType(COMPLETE_DESIGN_ROW_DROP_STRING);
}

BasesListBox::SavedDesignListBoxRow::SavedDesignListBoxRow(GG::X w, GG::Y h,
                                                           const std::string& design_name) :
    HullAndPartsListBoxRow(w, h),
    m_design_name(design_name)
{
    push_back(new SavedDesignPanel(w, h, m_design_name));
    SetDragDropDataType(SAVED_DESIGN_ROW_DROP_STRING);
}

const std::string& BasesListBox::SavedDesignListBoxRow::DesignName() const
{ return m_design_name; }

const std::string& BasesListBox::SavedDesignListBoxRow::Description() const {
    SavedDesignsManager& manager = GetSavedDesignsManager();
    const ShipDesign* design = manager.GetDesign(m_design_name);
    if (!design)
        return EMPTY_STRING;
    return design->Description();
}

bool BasesListBox::SavedDesignListBoxRow::LookupInStringtable() const {
    SavedDesignsManager& manager = GetSavedDesignsManager();
    const ShipDesign* design = manager.GetDesign(m_design_name);
    if (!design)
        return false;
    return design->LookupInStringtable();
}

BasesListBox::BasesListBox() :
    CUIListBox(),
    m_empire_id_shown(ALL_EMPIRES),
    m_availabilities_shown(std::make_pair(false, false)),
    m_showing_empty_hulls(false),
    m_showing_completed_designs(false),
    m_showing_saved_designs(false),
    m_showing_monsters(false)
{
    InitRowSizes();
    SetStyle(GG::LIST_NOSEL);

    GG::Connect(DoubleClickedSignal,    &BasesListBox::BaseDoubleClicked,   this);
    GG::Connect(LeftClickedSignal,      &BasesListBox::BaseLeftClicked,     this);
    GG::Connect(RightClickedSignal,     &BasesListBox::BaseRightClicked,    this);
}

const std::pair<bool, bool>& BasesListBox::GetAvailabilitiesShown() const
{ return m_availabilities_shown; }

void BasesListBox::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    CUIListBox::SizeMove(ul, lr);
    if (old_size != Size()) {
        const GG::Pt row_size = ListRowSize();
        for (GG::ListBox::iterator it = begin(); it != end(); ++it)
            (*it)->Resize(row_size);
    }
}

void BasesListBox::ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination) {
    if (wnds.empty())
        return;
    if (!wnds.size() == 1)
        ErrorLogger() << "BasesListBox::ChildrenDraggedAway unexpected informed that multiple Wnds were dragged away...";
    const GG::Wnd* wnd = wnds.front();  // should only be one wnd in list as BasesListBost doesn't allow selection, so dragging is only one-at-a-time
    const GG::Control* control = dynamic_cast<const GG::Control*>(wnd);
    if (!control)
        return;

    // remove dragged-away row from this ListBox
    CUIListBox::ChildrenDraggedAway(wnds, destination);

    // replace dragged-away control with new copy
    const GG::Pt row_size = ListRowSize();
    std::vector<std::string> empty_parts_vec;

    if (wnd->DragDropDataType() == COMPLETE_DESIGN_ROW_DROP_STRING) {
        // find design that was dragged away, and replace
        const BasesListBox::CompletedDesignListBoxRow* design_row =
            boost::polymorphic_downcast<const BasesListBox::CompletedDesignListBoxRow*>(wnd);

        int design_id = design_row->DesignID();
        CompletedDesignListBoxRow* row = new CompletedDesignListBoxRow(row_size.x, row_size.y,
                                                                       design_id);
        Insert(row);
        row->Resize(row_size);
        m_designs_in_list.insert(design_id);// should be redundant

    } else if (wnd->DragDropDataType() == HULL_PARTS_ROW_DROP_TYPE_STRING) {
        // find type of hull that was dragged away, and replace
        const BasesListBox::HullAndPartsListBoxRow* design_row =
            boost::polymorphic_downcast<const BasesListBox::HullAndPartsListBoxRow*>(wnd);

        const std::string& hull_name = design_row->Hull();
        HullAndPartsListBoxRow* row = new HullAndPartsListBoxRow(row_size.x, row_size.y,
                                                                 hull_name, empty_parts_vec);
        Insert(row);
        row->Resize(row_size);
        m_hulls_in_list.insert(hull_name);  // should be redundant

    } else if (wnd->DragDropDataType() == SAVED_DESIGN_ROW_DROP_STRING) {
        // find name of design that was dragged away, and replace
        const BasesListBox::SavedDesignListBoxRow* design_row =
            boost::polymorphic_downcast<const BasesListBox::SavedDesignListBoxRow*>(wnd);

        const std::string& design_name = design_row->DesignName();
        SavedDesignListBoxRow* row = new SavedDesignListBoxRow(row_size.x, row_size.y,
                                                               design_name);
        Insert(row);
        row->Resize(row_size);
        m_saved_desgins_in_list.insert(design_name);
    }
}

void BasesListBox::SetEmpireShown(int empire_id, bool refresh_list) {
    m_empire_id_shown = empire_id;

    // disconnect old signal
    m_empire_designs_changed_signal.disconnect();

    // connect signal to update this list if the empire's designs change
    if (const Empire* empire = GetEmpire(m_empire_id_shown))
        m_empire_designs_changed_signal = GG::Connect(empire->ShipDesignsChangedSignal, &BasesListBox::Populate,    this);

    if (refresh_list)
        Populate();
}

void BasesListBox::Populate() {
    //// abort of not visible to see results
    //if (!Empty() && !Visible())
    //    return;

    DebugLogger() << "BasesListBox::Populate";

    // populate list as appropriate for types of bases shown
    if (m_showing_empty_hulls)
        PopulateWithEmptyHulls();

    if (m_showing_completed_designs) {
        // make note of first visible row to preserve state
        std::size_t first_visible_row = std::distance(begin(), FirstRowShown());
        PopulateWithCompletedDesigns();
        if (!Empty())
            BringRowIntoView(--end());
        if (first_visible_row < NumRows())
            BringRowIntoView(boost::next(begin(), first_visible_row));
    }

    if (m_showing_saved_designs)
        PopulateWithSavedDesigns();

    if (m_showing_monsters)
        PopulateWithMonsters();
}

GG::Pt BasesListBox::ListRowSize()
{ return GG::Pt(Width() - ClientUI::ScrollWidth() - 5, BASES_LIST_BOX_ROW_HEIGHT); }

void BasesListBox::InitRowSizes() {
    // preinitialize listbox/row column widths, because what
    // ListBox::Insert does on default is not suitable for this case
    SetNumCols(1);
    SetColWidth(0, GG::X0);
    LockColWidths();
}

void BasesListBox::PopulateWithEmptyHulls() {
    ScopedTimer scoped_timer("BasesListBox::PopulateWithEmptyHulls");

    const bool showing_available = m_availabilities_shown.first;
    const bool showing_unavailable = m_availabilities_shown.second;
    DebugLogger() << "BasesListBox::PopulateWithEmptyHulls showing available (t, f):  " << showing_available << ", " << showing_unavailable;
    const Empire* empire = GetEmpire(m_empire_id_shown); // may return 0
    DebugLogger() << "BasesListBox::PopulateWithEmptyHulls m_empire_id_shown: " << m_empire_id_shown;

    //DebugLogger() << "... hulls in list: ";
    //for (std::set<std::string>::const_iterator it = m_hulls_in_list.begin(); it != m_hulls_in_list.end(); ++it)
    //    DebugLogger() << "... ... hull: " << *it;

    // loop through all hulls, determining if they need to be added to list
    std::set<std::string> hulls_to_add;
    std::set<std::string> hulls_to_remove;

    const HullTypeManager& manager = GetHullTypeManager();
    for (HullTypeManager::iterator it = manager.begin(); it != manager.end(); ++it) {
        const std::string& hull_name = it->first;

        const HullType* hull_type = manager.GetHullType(hull_name);
        if (!hull_type || !hull_type->Producible())
            continue;

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

    //DebugLogger() << "... hulls to remove from list: ";
    //for (std::set<std::string>::const_iterator it = hulls_to_remove.begin(); it != hulls_to_remove.end(); ++it)
    //    DebugLogger() << "... ... hull: " << *it;
    //DebugLogger() << "... hulls to add to list: ";
    //for (std::set<std::string>::const_iterator it = hulls_to_add.begin(); it != hulls_to_add.end(); ++it)
    //    DebugLogger() << "... ... hull: " << *it;


    // loop through list, removing rows as appropriate
    for (iterator it = begin(); it != end(); ) {
        iterator temp_it = it++;
        //DebugLogger() << " row index: " << i;
        if (const HullAndPartsListBoxRow* row = dynamic_cast<const HullAndPartsListBoxRow*>(*temp_it)) {
            const std::string& current_row_hull = row->Hull();
            //DebugLogger() << " current row hull: " << current_row_hull;
            if (hulls_to_remove.find(current_row_hull) != hulls_to_remove.end()) {
                //DebugLogger() << " ... removing";
                m_hulls_in_list.erase(current_row_hull);    // erase from set before deleting row, so as to not invalidate current_row_hull reference to deleted row's member string
                delete Erase(temp_it);
            }
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
    ScopedTimer scoped_timer("BasesListBox::PopulateWithCompletedDesigns");

    const bool showing_available = m_availabilities_shown.first;
    const bool showing_unavailable = m_availabilities_shown.second;
    const Empire* empire = GetEmpire(m_empire_id_shown); // may return 0
    const Universe& universe = GetUniverse();

    DebugLogger() << "BasesListBox::PopulateWithCompletedDesigns for empire " << m_empire_id_shown;

    // remove preexisting rows
    Clear();
    const GG::Pt row_size = ListRowSize();

    if (empire) {
        // add rows for designs this empire is keeping
        for (Empire::ShipDesignItr it = empire->ShipDesignBegin(); it != empire->ShipDesignEnd(); ++it) {
            int design_id = *it;
            bool available = empire->ShipDesignAvailable(design_id);
            const ShipDesign* design = GetShipDesign(design_id);
            if (!design || !design->Producible())
                continue;
            if ((available && showing_available) || (!available && showing_unavailable)) {
                CompletedDesignListBoxRow* row = new CompletedDesignListBoxRow(row_size.x, row_size.y, design_id);
                Insert(row);
                row->Resize(row_size);
            }
        }
    } else if (showing_available) {
        // add all known / existing designs
        for (Universe::ship_design_iterator it = universe.beginShipDesigns();
             it != universe.endShipDesigns(); ++it)
        {
            int design_id = it->first;
            const ShipDesign* design = it->second;
            if (!design->Producible())
                continue;
            CompletedDesignListBoxRow* row = new CompletedDesignListBoxRow(row_size.x, row_size.y, design_id);
            Insert(row);
            row->Resize(row_size);
        }
    }
}

void BasesListBox::PopulateWithSavedDesigns() {
    ScopedTimer scoped_timer("BasesListBox::PopulateWithSavedDesigns");

    DebugLogger() << "BasesListBox::PopulateWithSavedDesigns";

    // remove preexisting rows
    Clear();
    const GG::Pt row_size = ListRowSize();

    SavedDesignsManager& manager = GetSavedDesignsManager();
    for (std::map<std::string, ShipDesign*>::iterator it = manager.begin();
         it != manager.end(); ++it)
    {
        SavedDesignListBoxRow* row = new SavedDesignListBoxRow(row_size.x, row_size.y, it->first);
        Insert(row);
        row->Resize(row_size);
    }
}

void BasesListBox::PopulateWithMonsters() {
    ScopedTimer scoped_timer("BasesListBox::PopulateWithMonsters");

    const Universe& universe = GetUniverse();

    // remove preexisting rows
    Clear();
    const GG::Pt row_size = ListRowSize();

    for (Universe::ship_design_iterator it = universe.beginShipDesigns();
            it != universe.endShipDesigns(); ++it)
    {
        int design_id = it->first;
        const ShipDesign* design = it->second;
        if (!design->IsMonster())
            continue;
        CompletedDesignListBoxRow* row = new CompletedDesignListBoxRow(row_size.x, row_size.y, design_id);
        Insert(row);
        row->Resize(row_size);
    }
}

void BasesListBox::BaseLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt) {
    // determine type of row that was clicked, and emit appropriate signal

    CompletedDesignListBoxRow* design_row = dynamic_cast<CompletedDesignListBoxRow*>(*it);
    if (design_row) {
        int id = design_row->DesignID();
        const ShipDesign* design = GetShipDesign(id);
        if (design)
            DesignClickedSignal(design);
        return;
    }

    SavedDesignListBoxRow* saved_design_row = dynamic_cast<SavedDesignListBoxRow*>(*it);
    if (saved_design_row) {
        const std::string& design_name = saved_design_row->DesignName();
        const ShipDesign* design = GetSavedDesignsManager().GetDesign(design_name);
        if (design)
            DesignClickedSignal(design);
        return;
    }

    HullAndPartsListBoxRow* hull_parts_row = dynamic_cast<HullAndPartsListBoxRow*>(*it);
    if (hull_parts_row) {
        const std::string& hull_name = hull_parts_row->Hull();
        const HullType* hull_type = GetHullType(hull_name);
        const std::vector<std::string>& parts = hull_parts_row->Parts();
        if (hull_type && parts.empty())
            HullClickedSignal(hull_type);
    }
}

void BasesListBox::BaseRightClicked(GG::ListBox::iterator it, const GG::Pt& pt) {
    // determine type of row that was clicked, and emit appropriate signal

    if (CompletedDesignListBoxRow* design_row = dynamic_cast<CompletedDesignListBoxRow*>(*it)) {
        int design_id = design_row->DesignID();
        const ShipDesign* design = GetShipDesign(design_id);
        if (!design)
            return;

        DesignRightClickedSignal(design);
        // TODO: Subsequent code assumes we have a design, so we may want to do something about that...

        int client_empire_id = HumanClientApp::GetApp()->EmpireID();

        DebugLogger() << "BasesListBox::BaseRightClicked on design id : " << design_id;

        // create popup menu with a commands in it
        GG::MenuItem menu_contents;
        if (client_empire_id != ALL_EMPIRES)
            menu_contents.next_level.push_back(GG::MenuItem(UserString("DESIGN_DELETE"), 1, false, false));

        if (design->DesignedByEmpire() == client_empire_id)
            menu_contents.next_level.push_back(GG::MenuItem(UserString("DESIGN_RENAME"), 2, false, false));

        menu_contents.next_level.push_back(GG::MenuItem(UserString("DESIGN_SAVE"),       3, false, false));

        GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor(),
                            ClientUI::WndOuterBorderColor(), ClientUI::WndColor(), ClientUI::EditHiliteColor());

        if (popup.Run()) {
            switch (popup.MenuID()) {

            case 1: {   // delete design
                HumanClientApp::GetApp()->Orders().IssueOrder(
                    OrderPtr(new ShipDesignOrder(client_empire_id, design_id, true)));
                break;
            }

            case 2: {   // rename design
                CUIEditWnd edit_wnd(GG::X(350), UserString("DESIGN_ENTER_NEW_DESIGN_NAME"), design->Name());
                edit_wnd.Run();
                const std::string& result = edit_wnd.Result();
                if (result != "" && result != design->Name()) {
                    HumanClientApp::GetApp()->Orders().IssueOrder(
                        OrderPtr(new ShipDesignOrder(client_empire_id, design_id, result)));
                    ShipDesignPanel* design_panel = dynamic_cast<ShipDesignPanel*>((*design_row)[0]);
                    design_panel->Update();
                }
                break;
            }

            case 3: {   // save design
                ShowSaveDesignDialog(design_id);
                break;
            }

            default:
                break;
            }
        }
    } else if (SavedDesignListBoxRow* design_row = dynamic_cast<SavedDesignListBoxRow*>(*it)) {
        std::string design_name = design_row->DesignName();
        SavedDesignsManager& manager = GetSavedDesignsManager();
        const ShipDesign* design = manager.GetDesign(design_name);
        if (!design)
            return;

        int empire_id = HumanClientApp::GetApp()->EmpireID();
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return;

        DesignRightClickedSignal(design);

        DebugLogger() << "BasesListBox::BaseRightClicked on design name : " << design_name;

        // create popup menu with a commands in it
        GG::MenuItem menu_contents;
        menu_contents.next_level.push_back(GG::MenuItem(UserString("DESIGN_ADD"),       1, false, false));
        menu_contents.next_level.push_back(GG::MenuItem(UserString("DESIGN_ADD_ALL"),   2, false, false));

        GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor(),
                            ClientUI::WndOuterBorderColor(), ClientUI::WndColor(), ClientUI::EditHiliteColor());

        if (popup.Run()) {
            switch (popup.MenuID()) {

            case 1: {   // add design
                DebugLogger() << "BasesListBox::BaseRightClicked Add Saved Design" << design_name;
                int new_design_id = HumanClientApp::GetApp()->GetNewDesignID();
                HumanClientApp::GetApp()->Orders().IssueOrder(
                    OrderPtr(new ShipDesignOrder(empire_id, new_design_id, *design)));
                break;
            }

            case 2: {   // add all saved designs
                DebugLogger() << "BasesListBox::BaseRightClicked LoadAllSavedDesigns";
                manager.LoadAllSavedDesigns();
                break;
            }

            default:
                break;
            }
        }

    }
}

void BasesListBox::BaseDoubleClicked(GG::ListBox::iterator it) {
    // determine type of row that was clicked, and emit appropriate signal

    CompletedDesignListBoxRow* cd_row = dynamic_cast<CompletedDesignListBoxRow*>(*it);
    if (cd_row) {
        if (cd_row->DesignID() != ShipDesign::INVALID_DESIGN_ID)
            DesignSelectedSignal(cd_row->DesignID());
        return;
    }

    SavedDesignListBoxRow* sd_row = dynamic_cast<SavedDesignListBoxRow*>(*it);
    if (sd_row) {
        const std::string& design_name = sd_row->DesignName();
        if (!design_name.empty())
            SavedDesignSelectedSignal(design_name);
        return;
    }

    HullAndPartsListBoxRow* hp_row = dynamic_cast<HullAndPartsListBoxRow*>(*it);
    if (hp_row) {
        if (!hp_row->Hull().empty() || !hp_row->Parts().empty())
            DesignComponentsSelectedSignal(hp_row->Hull(), hp_row->Parts());
        return;
    }
}

void BasesListBox::ShowEmptyHulls(bool refresh_list) {
    m_showing_empty_hulls = true;
    m_showing_completed_designs = false;
    m_showing_saved_designs = false;
    m_showing_monsters = false;
    if (refresh_list)
        Populate();
}

void BasesListBox::ShowCompletedDesigns(bool refresh_list) {
    m_showing_empty_hulls = false;
    m_showing_completed_designs = true;
    m_showing_saved_designs = false;
    m_showing_monsters = false;
    if (refresh_list)
        Populate();
}

void BasesListBox::ShowSavedDesigns(bool refresh_list) {
    m_showing_empty_hulls = false;
    m_showing_completed_designs = false;
    m_showing_saved_designs = true;
    m_showing_monsters = false;
    if (refresh_list)
        Populate();
}

void BasesListBox::ShowMonsters(bool refresh_list) {
    m_showing_empty_hulls = false;
    m_showing_completed_designs = false;
    m_showing_saved_designs = false;
    m_showing_monsters = true;
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
    BaseSelector(GG::X w, GG::Y h);
    //@}

    /** \name Mutators */ //@{
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    void            Reset();
    void            ToggleAvailability(bool available, bool refresh_list);
    void            SetEmpireShown(int empire_id, bool refresh_list);
    void            ShowAvailability(bool available, bool refresh_list);
    void            HideAvailability(bool available, bool refresh_list);
    //@}

    mutable boost::signals2::signal<void (int)>                 DesignSelectedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::vector<std::string>&)>
                                                                DesignComponentsSelectedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::vector<std::string>&,
                                          const std::string&, const std::string&)>
                                                                SavedDesignSelectedSignal;

    mutable boost::signals2::signal<void (const ShipDesign*)>   DesignClickedSignal;
    mutable boost::signals2::signal<void (const HullType*)>     HullClickedSignal;

private:
    void            DoLayout();
    void            WndSelected(std::size_t index);
    void            SavedDesignSelectedSlot(const std::string& design_name);

    GG::TabWnd*     m_tabs;
    BasesListBox*   m_hulls_list;           // empty hulls on which a new design can be based
    BasesListBox*   m_designs_list;         // designs this empire has created or learned how to make
    BasesListBox*   m_saved_designs_list;   // designs saved to files
    BasesListBox*   m_monsters_list;        // monster designs
    std::pair<CUIButton*, CUIButton*>   m_availability_buttons;
};

DesignWnd::BaseSelector::BaseSelector(GG::X w, GG::Y h) :
    CUIWnd(UserString("DESIGN_WND_STARTS"), GG::X0, GG::Y0, w, h, GG::INTERACTIVE | GG::RESIZABLE | GG::ONTOP | GG::DRAGABLE | PINABLE ),
    m_tabs(0),
    m_hulls_list(0),
    m_designs_list(0),
    m_saved_designs_list(0),
    m_monsters_list(0)
{
    m_availability_buttons.first = new CUIButton(UserString("PRODUCTION_WND_AVAILABILITY_AVAILABLE"), ClientUI::ButtonHiliteColor(), ClientUI::ButtonHiliteBorderColor());
    AttachChild(m_availability_buttons.first);
    GG::Connect(m_availability_buttons.first->LeftClickedSignal,
                boost::bind(&DesignWnd::BaseSelector::ToggleAvailability, this, true, true));

    m_availability_buttons.second = new CUIButton(UserString("PRODUCTION_WND_AVAILABILITY_UNAVAILABLE"), ClientUI::ButtonHiliteColor(), ClientUI::ButtonHiliteBorderColor());
    AttachChild(m_availability_buttons.second);
    GG::Connect(m_availability_buttons.second->LeftClickedSignal,
                boost::bind(&DesignWnd::BaseSelector::ToggleAvailability, this, false, true));

    m_tabs = new GG::TabWnd(GG::X(5), GG::Y(2), GG::X(10), GG::Y(10), ClientUI::GetFont(), ClientUI::WndColor(), ClientUI::TextColor(), GG::TAB_BAR_DETACHED);
    GG::Connect(m_tabs->WndChangedSignal,                       &DesignWnd::BaseSelector::WndSelected,      this);
    AttachChild(m_tabs);

    m_hulls_list = new BasesListBox();
    m_hulls_list->Resize(GG::Pt(GG::X(10), GG::Y(10)));
    m_tabs->AddWnd(m_hulls_list, UserString("DESIGN_WND_HULLS"));
    m_hulls_list->ShowEmptyHulls(false);
    GG::Connect(m_hulls_list->DesignComponentsSelectedSignal,   DesignWnd::BaseSelector::DesignComponentsSelectedSignal);
    GG::Connect(m_hulls_list->HullClickedSignal,                DesignWnd::BaseSelector::HullClickedSignal);

    m_designs_list = new BasesListBox();
    m_designs_list->Resize(GG::Pt(GG::X(10), GG::Y(10)));
    m_tabs->AddWnd(m_designs_list, UserString("DESIGN_WND_FINISHED_DESIGNS"));
    m_designs_list->ShowCompletedDesigns(false);
    GG::Connect(m_designs_list->DesignSelectedSignal,           DesignWnd::BaseSelector::DesignSelectedSignal);
    GG::Connect(m_designs_list->DesignClickedSignal,            DesignWnd::BaseSelector::DesignClickedSignal);

    m_saved_designs_list = new BasesListBox();
    m_saved_designs_list->Resize(GG::Pt(GG::X(10), GG::Y(10)));
    m_tabs->AddWnd(m_saved_designs_list, UserString("DESIGN_WND_SAVED_DESIGNS"));
    m_saved_designs_list->ShowSavedDesigns(true);
    GG::Connect(m_saved_designs_list->SavedDesignSelectedSignal,&DesignWnd::BaseSelector::SavedDesignSelectedSlot,  this);
    GG::Connect(m_saved_designs_list->DesignClickedSignal,      DesignWnd::BaseSelector::DesignClickedSignal);

    m_monsters_list = new BasesListBox();
    m_monsters_list->Resize(GG::Pt(GG::X(10), GG::Y(10)));
    m_tabs->AddWnd(m_monsters_list, UserString("DESIGN_WND_MONSTERS"));
    m_monsters_list->ShowMonsters(false);
    GG::Connect(m_monsters_list->DesignSelectedSignal,          DesignWnd::BaseSelector::DesignSelectedSignal);
    GG::Connect(m_monsters_list->DesignClickedSignal,           DesignWnd::BaseSelector::DesignClickedSignal);


    DoLayout();
    ShowAvailability(true, false);   // default to showing available unavailable bases.
}

void DesignWnd::BaseSelector::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    CUIWnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void DesignWnd::BaseSelector::Reset() {
    ScopedTimer scoped_timer("BaseSelector::Reset");

    const int empire_id = HumanClientApp::GetApp()->EmpireID();
    SetEmpireShown(empire_id, false);

    if (!m_tabs)
        return;

    if (GG::Wnd* wnd = m_tabs->CurrentWnd()) {
        if (BasesListBox* base_box = dynamic_cast<BasesListBox*>(wnd))
            base_box->Populate();
    }
}

void DesignWnd::BaseSelector::SetEmpireShown(int empire_id, bool refresh_list) {
    if (m_hulls_list)
        m_hulls_list->SetEmpireShown(empire_id, refresh_list);
    if (m_designs_list)
        m_designs_list->SetEmpireShown(empire_id, refresh_list);
}

void DesignWnd::BaseSelector::ShowAvailability(bool available, bool refresh_list) {
    if (m_hulls_list)
        m_hulls_list->ShowAvailability(available, refresh_list);
    if (m_designs_list)
        m_designs_list->ShowAvailability(available, refresh_list);
    if (available)
        m_availability_buttons.first->SetCheck();
    else
        m_availability_buttons.second->SetCheck();
}

void DesignWnd::BaseSelector::HideAvailability(bool available, bool refresh_list) {
    if (m_hulls_list)
        m_hulls_list->HideAvailability(available, refresh_list);
    if (m_designs_list)
        m_designs_list->HideAvailability(available, refresh_list);
    if (available)
        m_availability_buttons.first->SetCheck(false);
    else
        m_availability_buttons.second->SetCheck(false);
}

void DesignWnd::BaseSelector::ToggleAvailability(bool available, bool refresh_list) {
    const std::pair<bool, bool>& avail_shown = m_hulls_list->GetAvailabilitiesShown();
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

void DesignWnd::BaseSelector::DoLayout() {
    const GG::X LEFT_PAD(5);
    const GG::Y TOP_PAD(2);
    const GG::X AVAILABLE_WIDTH = ClientWidth() - 2*LEFT_PAD;
    const int BUTTON_SEPARATION = 3;
    const GG::X BUTTON_WIDTH = (AVAILABLE_WIDTH - BUTTON_SEPARATION) / 2;
    const int PTS = ClientUI::Pts();
    const GG::Y BUTTON_HEIGHT(PTS * 2);

    GG::Y top(TOP_PAD);
    GG::X left(LEFT_PAD);

    m_availability_buttons.first->SizeMove(GG::Pt(left, top), GG::Pt(left + BUTTON_WIDTH, top + BUTTON_HEIGHT));
    left = left + BUTTON_WIDTH + BUTTON_SEPARATION;
    m_availability_buttons.second->SizeMove(GG::Pt(left, top), GG::Pt(left + BUTTON_WIDTH, top + BUTTON_HEIGHT));
    left = LEFT_PAD;
    top = top + BUTTON_HEIGHT + BUTTON_SEPARATION;

    m_tabs->SizeMove(GG::Pt(left, top), ClientSize() - GG::Pt(LEFT_PAD, TOP_PAD));
}

void DesignWnd::BaseSelector::WndSelected(std::size_t index)
{ Reset(); }

void DesignWnd::BaseSelector::SavedDesignSelectedSlot(const std::string& design_name) {
    if (design_name.empty())
        return;
    const ShipDesign* design = GetSavedDesignsManager().GetDesign(design_name);
    if (!design)
        return;

    const std::string& name = design->Name();       // should automatically look up name and description in stringtable if design->LookupInStringtable() is true
    const std::string& desc = design->Description();
    const std::string& hull = design->Hull();
    const std::vector<std::string>& parts = design->Parts();

    SavedDesignSelectedSignal(hull, parts, name, desc);
}


//////////////////////////////////////////////////
// SlotControl                                  //
//////////////////////////////////////////////////
/** UI representation and drop-target for slots of a design.  PartControl may
  * be dropped into slots to add the corresponding parts to the ShipDesign, or
  * the part may be set programmatically with SetPart(). */
class SlotControl : public GG::Control {
public:
    /** \name Structors */ //@{
    SlotControl();
    SlotControl(double x, double y, ShipSlotType slot_type);
    //@}

    /** \name Accessors */ //@{
    virtual void    DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last, const GG::Pt& pt) const;

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

    /** emitted when the contents of a slot are altered by the dragging
      * a PartControl in or out of the slot.  signal should be caught and the
      * slot contents set using SetPart accordingly */
    mutable boost::signals2::signal<void (const PartType*)> SlotContentsAlteredSignal;

    mutable boost::signals2::signal<void (const PartType*)> PartTypeClickedSignal;

private:
    /** emits SlotContentsAlteredSignal with PartType* = 0.  needed because
      * boost::signals2::signal is noncopyable, so boost::bind can't be used
      * to bind the parameter 0 to SlotContentsAlteredSignal::operator() */
    void            EmitNullSlotContentsAlteredSignal();

    bool                m_highlighted;
    ShipSlotType        m_slot_type;
    double              m_x_position_fraction, m_y_position_fraction;   //!< position on hull image where slot should be shown, as a fraction of that image's size
    PartControl*        m_part_control;
    GG::StaticGraphic*  m_background;
};

SlotControl::SlotControl() :
    GG::Control(GG::X0, GG::Y0, SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT, GG::INTERACTIVE),
    m_highlighted(false),
    m_slot_type(INVALID_SHIP_SLOT_TYPE),
    m_x_position_fraction(0.4),
    m_y_position_fraction(0.4),
    m_part_control(0),
    m_background(0)
{}

SlotControl::SlotControl(double x, double y, ShipSlotType slot_type) :
    GG::Control(GG::X0, GG::Y0, SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT, GG::INTERACTIVE),
    m_highlighted(false),
    m_slot_type(slot_type),
    m_x_position_fraction(x),
    m_y_position_fraction(y),
    m_part_control(0),
    m_background(0)
{
    m_background = new GG::StaticGraphic(SlotBackgroundTexture(m_slot_type), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_background->Resize(GG::Pt(SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT));
    m_background->Show();
    AttachChild(m_background);

    SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));

    // set up empty slot tool tip
    std::string title_text;
    if (slot_type == SL_EXTERNAL)
        title_text = UserString("SL_EXTERNAL");
    else if (slot_type == SL_INTERNAL)
        title_text = UserString("SL_INTERNAL");
    else if (slot_type == SL_CORE)
        title_text = UserString("SL_CORE");

    SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
        new IconTextBrowseWnd(SlotBackgroundTexture(m_slot_type), title_text, UserString("SL_TOOLTIP_DESC"))));
}

void SlotControl::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last, const GG::Pt& pt) const {
    for (DropsAcceptableIter it = first; it != last; ++it)
        it->second = false;
    if (std::distance(first, last) != 1)
        return;

    bool acceptable_part_found = false;
    for (DropsAcceptableIter it = first; it != last; ++it) {
        if (!acceptable_part_found && it->first->DragDropDataType() == PART_CONTROL_DROP_TYPE_STRING) {
            const PartControl* part_control = boost::polymorphic_downcast<const PartControl*>(it->first);
            const PartType* part_type = part_control->Part();
            if (part_type &&
                part_type->CanMountInSlotType(m_slot_type) &&
                part_control != m_part_control)
            {
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

ShipSlotType SlotControl::SlotType() const
{ return m_slot_type; }

double SlotControl::XPositionFraction() const
{ return m_x_position_fraction; }

double SlotControl::YPositionFraction() const
{ return m_y_position_fraction; }

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
    if (wnds.size() != 1) {
        // delete any extra wnds that won't be processed below
        std::vector<GG::Wnd*>::const_iterator it = wnds.begin();
        ++it;
        for (; it != wnds.end(); ++it)
            delete *it;
        ErrorLogger() << "SlotControl::AcceptDrops given multiple wnds unexpectedly...";
    }

    const GG::Wnd* wnd = *(wnds.begin());
    const PartControl* control = boost::polymorphic_downcast<const PartControl*>(wnd);
    const PartType* part_type = control ? control->Part() : 0;

    delete wnd;

    if (part_type)
        SlotContentsAlteredSignal(part_type);
}

void SlotControl::ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination) {
    if (wnds.empty())
        return;
    const GG::Wnd* wnd = wnds.front();
    const PartControl* part_control = dynamic_cast<const PartControl*>(wnd);
    if (part_control != m_part_control)
        return;
    // SlotContentsAlteredSignal is connected to this->SetPart, which will
    // delete m_part_control if it is not null.  The drop-accepting Wnd is
    // responsible for deleting the accepted Wnd, so setting m_part_control = 0
    // here prevents this->SetPart from deleting it prematurely
    m_part_control = 0;
    SlotContentsAlteredSignal(0);
}

void SlotControl::DragDropEnter(const GG::Pt& pt, const std::map<GG::Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys)
{}

void SlotControl::DragDropLeave() {
    if (m_part_control)
        m_part_control->Hide();
}

void SlotControl::Render()
{}

void SlotControl::Highlight(bool actually)
{ m_highlighted = actually; }

void SlotControl::SetPart(const std::string& part_name)
{ SetPart(GetPartType(part_name)); }

void SlotControl::SetPart(const PartType* part_type) {
    // remove existing part control, if any
    if (m_part_control) {
        delete m_part_control;
        m_part_control = 0;
    }

    // create new part control for passed in part_type
    if (part_type) {
        m_part_control = new PartControl(part_type);
        AttachChild(m_part_control);

        // single click shows encyclopedia data
        GG::Connect(m_part_control->ClickedSignal, PartTypeClickedSignal);

        // double click clears slot
        GG::Connect(m_part_control->DoubleClickedSignal,
                    boost::bind(&SlotControl::EmitNullSlotContentsAlteredSignal, this));
        SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));

        // set part occupying slot's tool tip to say slot type
        std::string title_text;
        if (m_slot_type == SL_EXTERNAL)
            title_text = UserString("SL_EXTERNAL");
        else if (m_slot_type == SL_INTERNAL)
            title_text = UserString("SL_INTERNAL");
        else if (m_slot_type == SL_CORE)
            title_text = UserString("SL_CORE");

        m_part_control->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
            new IconTextBrowseWnd(ClientUI::PartIcon(part_type->Name()),
                                  UserString(part_type->Name()) + " (" + title_text + ")",
                                  UserString(part_type->Description()))));
    }
}

void SlotControl::EmitNullSlotContentsAlteredSignal()
{ SlotContentsAlteredSignal(0); }


//////////////////////////////////////////////////
// DesignWnd::MainPanel                         //
//////////////////////////////////////////////////
class DesignWnd::MainPanel : public CUIWnd {
public:
    /** \name Structors */ //@{
    MainPanel(GG::X w, GG::Y h);
    //@}

    /** \name Accessors */ //@{
    virtual void                        DropsAcceptable(DropsAcceptableIter first,
                                                        DropsAcceptableIter last,
                                                        const GG::Pt& pt) const;

    const std::vector<std::string>      Parts() const;              //!< returns vector of names of parts in slots of current shown design.  empty slots are represented with empty stri
    const std::string&                  Hull() const;               //!< returns name of hull of current shown design
    const std::string&                  DesignName() const;         //!< returns name currently entered for design
    const std::string&                  DesignDescription() const;  //!< returns description currently entered for design

    boost::shared_ptr<const ShipDesign> GetIncompleteDesign() const;//!< returns a pointer to the design currently being modified (if any).  may return an empty pointer if not currently modifying a design.
    int                                 GetCompleteDesignID() const;//!< returns ID of complete design currently being shown in this panel.  returns ShipDesign::INVALID_DESIGN_ID if not showing a complete design

    bool                                CurrentDesignIsRegistered(std::string& design_name);//!< returns true iff a design with the same hull and parts is already registered with thsi empire; if so, also populates design_name with the name of that design
    //@}

    /** \name Mutators */ //@{
    virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt);

    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    void            Sanitize();

    void            SetPart(const std::string& part_name, unsigned int slot);   //!< puts specified part in specified slot.  does nothing if slot is out of range of available slots for current hull
    void            SetPart(const PartType* part, unsigned int slot, bool emit_signal = false);
    void            SetParts(const std::vector<std::string>& parts);            //!< puts specified parts in slots.  attempts to put each part into the slot corresponding to its place in the passed vector.  if a part cannot be placed, it is ignored.  more parts than there are slots available are ignored, and slots for which there are insufficient parts in the passed vector are unmodified

    /** Attempts to add the specified part to the design, if possible.  will
      * first attempt to add part to an empty slot of the appropriate type, and
      * if no appropriate slots are available, may or may not move other parts
      * around within the design to open up a compatible slot in which to add
      * this part (and then add it).  may also do nothing. */
    void            AddPart(const PartType* part);
    bool            CanPartBeAdded(const PartType* part);

    void            ClearParts();                                               //!< removes all parts from design.  hull is not altered
    void            SetHull(const std::string& hull_name);                      //!< sets the design hull to the specified hull, displaying appropriate background image and creating appropriate SlotControls
    void            SetHull(const HullType* hull);
    void            SetDesign(const ShipDesign* ship_design);                   //!< sets the displayed design by setting the appropriate hull and parts
    void            SetDesign(int design_id);                                   //!< sets the displayed design by setting the appropriate hull and parts

    /** sets design hull and parts to those specified */
    void            SetDesignComponents(const std::string& hull,
                                        const std::vector<std::string>& parts);
    void            SetDesignComponents(const std::string& hull,
                                        const std::vector<std::string>& parts,
                                        const std::string& name,
                                        const std::string& desc);

    void            HighlightSlotType(std::vector<ShipSlotType>& slot_types);   //!< renders slots of the indicated types differently, perhaps to indicate that that those slots can be drop targets for a particular part?
    void            RegisterCompletedDesignDump(std::string design_dump);
    void            ReregisterDesigns();                                        //!< resets m_completed_designs and reregisters all current empire designs. is used w/r/t m_confirm_button status
    //@}

    /** emitted when the design is changed (by adding or removing parts, not
      * name or description changes) */
    mutable boost::signals2::signal<void ()>                DesignChangedSignal;

    /** emitted when the design name is changed */
    mutable boost::signals2::signal<void ()>                DesignNameChangedSignal;

    /** propegates signals from contained SlotControls that signal that a part
      * has been clicked */
    mutable boost::signals2::signal<void (const PartType*)> PartTypeClickedSignal;

    mutable boost::signals2::signal<void (const HullType*)> HullTypeClickedSignal;

    /** emitted when the user clicks the m_confirm_button to add the new
      * design to the player's empire */
    mutable boost::signals2::signal<void ()>                DesignConfirmedSignal;

    /** emitted when the user clicks on the background of this main panel and
      * a completed design is showing */
    mutable boost::signals2::signal<void (int)>             CompleteDesignClickedSignal;

private:
    // disambiguate overloaded SetPart function, because otherwise boost::bind wouldn't be able to tell them apart
    typedef void (DesignWnd::MainPanel::*SetPartFuncPtrType)(const PartType* part, unsigned int slot, bool emit_signal);
    static SetPartFuncPtrType const s_set_part_func_ptr;

    void            Populate();                         //!< creates and places SlotControls for current hull
    void            DoLayout();                         //!< positions buttons, text entry boxes and SlotControls
    void            DesignChanged();                    //!< responds to the design being changed
    void            DesignNameChanged();                //!< responds to the design name being changed
    void            RefreshIncompleteDesign() const;
    std::string     GetCleanDesignDump(const ShipDesign* ship_design);  //!< similar to ship design dump but without 'lookup_strings', icon and model entries

    bool            AddPartEmptySlot(const PartType* part, int slot_number);                            //!< Adds part to slot number
    bool            AddPartWithSwapping(const PartType* part, std::pair<int, int> swap_and_empty_slot); //!< Swaps part in slot # pair.first to slot # pair.second, adds given part to slot # pair.first
    int             FindEmptySlotForPart(const PartType* part);                                         //!< Determines if a part can be added to any empty slot, returns the slot index if possible, otherwise -1   

    void            DesignNameEditedSlot(const std::string& new_name);  //!< triggered when m_design_name's AfterTextChangedSignal fires. Used for basic name validation.

    std::pair<int, int> FindSlotForPartWithSwapping(const PartType* part);                              //!< Determines if a part can be added to a slot with swapping, returns a pair containing the slot to swap and an empty slot, otherwise a pair with -1
                                                                                                        //!< This function only tries to find a way to add the given part by swapping a part already in a slot to an empty slot
                                                                                                        //!< If theres an open slot that the given part could go into but all of the occupied slots contain parts that can't swap into the open slot
                                                                                                        //!< This function will indicate that it could not add the part, even though adding the part is possible
    const HullType*                         m_hull;
    std::vector<SlotControl*>               m_slots;
    int                                     m_complete_design_id;
    mutable boost::shared_ptr<ShipDesign>   m_incomplete_design;
    std::set<std::string>                   m_completed_design_dump_strings;

    GG::StaticGraphic*  m_background_image;
    CUILabel*           m_design_name_label;
    GG::Edit*           m_design_name;
    CUILabel*           m_design_description_label;
    GG::Edit*           m_design_description;
    GG::Button*         m_confirm_button;
    GG::Button*         m_clear_button;
    bool                m_disabled_by_name; // if the design confirm button is currently disabled due to empty name

    boost::signals2::connection             m_empire_designs_changed_signal;
};

// static
DesignWnd::MainPanel::SetPartFuncPtrType const DesignWnd::MainPanel::s_set_part_func_ptr = &DesignWnd::MainPanel::SetPart;

DesignWnd::MainPanel::MainPanel(GG::X w, GG::Y h) :
    CUIWnd(UserString("DESIGN_WND_MAIN_PANEL_TITLE"), GG::X0, GG::Y0, w, h, GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE),
    m_hull(0),
    m_slots(),
    m_complete_design_id(ShipDesign::INVALID_DESIGN_ID),
    m_incomplete_design(),
    m_completed_design_dump_strings(),
    m_background_image(0),
    m_design_name_label(0),
    m_design_name(0),
    m_design_description_label(0),
    m_design_description(0),
    m_confirm_button(0),
    m_clear_button(0),
    m_disabled_by_name(false)
{
    SetChildClippingMode(ClipToClient);

    m_design_name_label = new CUILabel(UserString("DESIGN_WND_DESIGN_NAME"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_design_name = new CUIEdit(UserString("DESIGN_NAME_DEFAULT"));
    m_design_description_label = new CUILabel(UserString("DESIGN_WND_DESIGN_DESCRIPTION"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_design_description = new CUIEdit(UserString("DESIGN_DESCRIPTION_DEFAULT"));
    m_confirm_button = new CUIButton(UserString("DESIGN_WND_CONFIRM"));
    m_clear_button = new CUIButton(UserString("DESIGN_WND_CLEAR"));

    m_confirm_button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));

    AttachChild(m_design_name_label);
    AttachChild(m_design_name);
    AttachChild(m_design_description_label);
    AttachChild(m_design_description);
    AttachChild(m_confirm_button);
    AttachChild(m_clear_button);

    GG::Connect(m_clear_button->LeftClickedSignal, &DesignWnd::MainPanel::ClearParts, this);
    GG::Connect(m_design_name->EditedSignal, &DesignWnd::MainPanel::DesignNameEditedSlot, this);
    GG::Connect(m_confirm_button->LeftClickedSignal, DesignConfirmedSignal);
    GG::Connect(this->DesignChangedSignal, &DesignWnd::MainPanel::DesignChanged, this);

    DesignChanged(); // Initialize components that rely on the current state of the design.

    DoLayout();
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

const std::string& DesignWnd::MainPanel::DesignName() const
{ return m_design_name->Text(); }

const std::string& DesignWnd::MainPanel::DesignDescription() const
{ return m_design_description->Text(); }

boost::shared_ptr<const ShipDesign> DesignWnd::MainPanel::GetIncompleteDesign() const {
    RefreshIncompleteDesign();
    return m_incomplete_design;
}

int DesignWnd::MainPanel::GetCompleteDesignID() const
{ return m_complete_design_id; }

bool DesignWnd::MainPanel::CurrentDesignIsRegistered(std::string& design_name) {
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire* empire = GetEmpire(empire_id); // Had better not return 0 if we're designing a ship.
    if (!empire) {
        ErrorLogger() << "DesignWnd::MainPanel::CurrentDesignIsRegistered couldn't get the current empire.";
        return false;
    }

    if (boost::shared_ptr<const ShipDesign> cur_design = GetIncompleteDesign()) {
        for (Empire::ShipDesignItr it = empire->ShipDesignBegin();
             it != empire->ShipDesignEnd(); ++it)
        {
            const ShipDesign& ship_design = *GetShipDesign(*it);
            if (ship_design == *cur_design.get()) {
                design_name = ship_design.Name();
                return true;
            }
        }
    }
    return false;
}

void DesignWnd::MainPanel::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (m_hull)
        HullTypeClickedSignal(m_hull);
    CUIWnd::LClick(pt, mod_keys);
}

void DesignWnd::MainPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    CUIWnd::SizeMove(ul, lr);
    DoLayout();
}

void DesignWnd::MainPanel::ReregisterDesigns() {
    DebugLogger() << "DesignWnd::MainPanel::ReregisterDesigns";
    m_completed_design_dump_strings.clear();
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire* empire = GetEmpire(empire_id); // may return 0
    if (empire) {
        for (Empire::ShipDesignItr it = empire->ShipDesignBegin(); it != empire->ShipDesignEnd(); ++it) {
            if (const ShipDesign* design = GetShipDesign(*it)) {
                std::string dump_str = GetCleanDesignDump(design);
                m_completed_design_dump_strings.insert(dump_str); //no need to validate
            }
        }
    }
}

void DesignWnd::MainPanel::Sanitize() {
    SetHull(0);
    m_design_name->SetText(UserString("DESIGN_NAME_DEFAULT"));
    m_design_description->SetText(UserString("DESIGN_DESCRIPTION_DEFAULT"));
    // disconnect old empire design signal
    m_empire_designs_changed_signal.disconnect();
    // connect signal to update this list if the empire's designs change
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    if (const Empire* empire = GetEmpire(empire_id)) {
        DebugLogger() << "DesignWnd::MainPanel::Sanitize";
        if ((CurrentTurn() == 1) && GetOptionsDB().Get<bool>("auto-add-saved-designs")) { // otherwise can be manually triggered by right click context menu
            GetSavedDesignsManager().LoadAllSavedDesigns(); 
        }
        m_empire_designs_changed_signal = GG::Connect(empire->ShipDesignsChangedSignal, &MainPanel::ReregisterDesigns,    this); // not apparent if this is working, but in typical use is unnecessary
    }
    ReregisterDesigns();
}

void DesignWnd::MainPanel::SetPart(const std::string& part_name, unsigned int slot)
{ SetPart(GetPartType(part_name), slot); }

void DesignWnd::MainPanel::SetPart(const PartType* part, unsigned int slot, bool emit_signal /* = false */) {
    //DebugLogger() << "DesignWnd::MainPanel::SetPart(" << (part ? part->Name() : "no part") << ", slot " << slot << ")";
    if (slot > m_slots.size()) {
        ErrorLogger() << "DesignWnd::MainPanel::SetPart specified nonexistant slot";
        return;
    }
    m_slots[slot]->SetPart(part);
    if (emit_signal)  // to avoid unnecessary signal repetition.
        DesignChangedSignal();  
}

void DesignWnd::MainPanel::SetParts(const std::vector<std::string>& parts) {
    unsigned int num_parts = std::min(parts.size(), m_slots.size());
    for (unsigned int i = 0; i < num_parts; ++i)
        m_slots[i]->SetPart(parts[i]);
    DesignChangedSignal();
}

void DesignWnd::MainPanel::AddPart(const PartType* part) {
    if (AddPartEmptySlot(part, FindEmptySlotForPart(part)))
        return;

    if (!AddPartWithSwapping(part, FindSlotForPartWithSwapping(part)))
        DebugLogger() << "DesignWnd::MainPanel::AddPart("
                               << (part ? part->Name() : "no part")
                               << ") couldn't find a slot for the part";
}

bool DesignWnd::MainPanel::CanPartBeAdded(const PartType* part) {
    std::pair<int, int> swap_result = FindSlotForPartWithSwapping(part);
    return (FindEmptySlotForPart(part) >= 0 || (swap_result.first >= 0 && swap_result.second >= 0));
}

bool DesignWnd::MainPanel::AddPartEmptySlot(const PartType* part, int slot_number) {
    if (!part || slot_number < 0)
        return false;
    SetPart(part, slot_number);
    DesignChangedSignal();
    return true;
}

bool DesignWnd::MainPanel::AddPartWithSwapping(const PartType* part, std::pair<int, int> swap_and_empty_slot) {
    if (!part || swap_and_empty_slot.first < 0 || swap_and_empty_slot.second < 0)
        return false;
    // Move the flexible part to the first open spot
    SetPart(m_slots[swap_and_empty_slot.first]->GetPart(), swap_and_empty_slot.second);
    // Move replacement part into the newly opened slot
    SetPart(part, swap_and_empty_slot.first);
    DesignChangedSignal();
    return true;
}

int DesignWnd::MainPanel::FindEmptySlotForPart(const PartType* part) {
    int result = -1;
    if (!part)
        return result;

    for (unsigned int i = 0; i < m_slots.size(); ++i) {             // scan through slots to find one that can mount part
        const ShipSlotType slot_type = m_slots[i]->SlotType();
        const PartType* part_type = m_slots[i]->GetPart();          // check if this slot is empty

        if (!part_type && part->CanMountInSlotType(slot_type)) {    // ... and if the part can mount here
            result = i;
            return result;
        }
    }
    return result;
}

void DesignWnd::MainPanel::DesignNameEditedSlot(const std::string& new_name) {
    DesignNameChanged();  // Check whether the confirmation button should be enabled or disabled each time the name changes.
}

std::pair<int, int> DesignWnd::MainPanel::FindSlotForPartWithSwapping(const PartType* part) {
    // result.first = swap_slot, result.second = empty_slot
    // if any of the pair == -1, no swap!

    if (!part)
        return std::make_pair(-1, -1);

    // first search for an empty compatible slot for the new part
    for (unsigned int i = 0; i < m_slots.size(); ++i) {
        if (!part->CanMountInSlotType(m_slots[i]->SlotType()))
            continue;   // skip incompatible slots

        if (!m_slots[i]->GetPart())
            return std::make_pair(-1, -1);  // empty slot that can hold part. no swapping needed.
    }


    // second, scan for a slot containing a part that can be moved to another
    // slot to make room for the new part
    for (unsigned int i = 0; i < m_slots.size(); ++i) {
        if (!part->CanMountInSlotType(m_slots[i]->SlotType()))
            continue;   // skip incompatible slots

        // can now assume m_slots[i] has a part, as if it didn't, it would have
        // been found in the first loop

        // see if we can move the part in the candidate slot to an empty slot elsewhere
        for (unsigned int j = 0; j < m_slots.size(); ++j) {
            if (m_slots[j]->GetPart())
                continue;   // only consider moving into empty slots

            if (m_slots[i]->GetPart()->CanMountInSlotType(m_slots[j]->SlotType()))
                return std::make_pair(i, j);    // other slot can hold current part to make room for new part
        }
    }

    return std::make_pair(-1, -1);
}

void DesignWnd::MainPanel::ClearParts() {
    for (unsigned int i = 0; i < m_slots.size(); ++i)
        m_slots[i]->SetPart(0);
    DesignChangedSignal();
}

void DesignWnd::MainPanel::SetHull(const std::string& hull_name)
{ SetHull(GetHullType(hull_name)); }

void DesignWnd::MainPanel::SetHull(const HullType* hull) {
    m_hull = hull;
    DeleteChild(m_background_image);
    m_background_image = 0;
    if (m_hull) {
        boost::shared_ptr<GG::Texture> texture = ClientUI::HullTexture(hull->Name());
        m_background_image = new GG::StaticGraphic(texture, GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC);
        AttachChild(m_background_image);
        MoveChildDown(m_background_image);
    }
    Populate();
    DoLayout();
    if (hull)
        DesignChangedSignal();
}

void DesignWnd::MainPanel::SetDesign(const ShipDesign* ship_design) {
    m_incomplete_design.reset();

    if (!ship_design) {
        SetHull(0);
        return;
    }

    m_complete_design_id = ship_design->ID();

    m_design_name->SetText(ship_design->Name());
    m_design_description->SetText(ship_design->Description());

    const HullType* hull_type = ship_design->GetHull();
    SetHull(hull_type);

    const std::vector<std::string>& parts_vec = ship_design->Parts();
    for (unsigned int i = 0; i < parts_vec.size() && i < m_slots.size(); ++i)
        m_slots[i]->SetPart(GetPartType(parts_vec[i]));
    DesignChangedSignal();
}

void DesignWnd::MainPanel::SetDesign(int design_id)
{ SetDesign(GetShipDesign(design_id)); }

void DesignWnd::MainPanel::SetDesignComponents(const std::string& hull,
                                               const std::vector<std::string>& parts)
{
    SetHull(hull);
    SetParts(parts);
}

void DesignWnd::MainPanel::SetDesignComponents(const std::string& hull,
                                               const std::vector<std::string>& parts,
                                               const std::string& name,
                                               const std::string& desc)
{
    SetHull(hull);
    SetParts(parts);
    m_design_name->SetText(name);
    m_design_description->SetText(desc);
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
            boost::bind(DesignWnd::MainPanel::s_set_part_func_ptr, this, _1, i, true);
        GG::Connect(slot_control->SlotContentsAlteredSignal, set_part_func);
        GG::Connect(slot_control->PartTypeClickedSignal, PartTypeClickedSignal);
    }
}

void DesignWnd::MainPanel::DoLayout() {
    // position labels and text edit boxes for name and description and buttons to clear and confirm design

    const int PTS = ClientUI::Pts();
    const GG::X PTS_WIDE(PTS / 2);           // guess at how wide per character the font needs
    const GG::Y BUTTON_HEIGHT(PTS * 2);
    const GG::X LABEL_WIDTH = PTS_WIDE * 15;
    const int PAD = 6;
    const int GUESSTIMATE_NUM_CHARS_IN_BUTTON_TEXT = 25;    // rough guesstimate... avoid overly long part class names
    const GG::X BUTTON_WIDTH = PTS_WIDE*GUESSTIMATE_NUM_CHARS_IN_BUTTON_TEXT;

    GG::X edit_right = ClientWidth();
    GG::X confirm_right = ClientWidth() - PAD;

    GG::Pt lr = GG::Pt(confirm_right, BUTTON_HEIGHT) + GG::Pt(GG::X0, GG::Y(PAD));
    GG::Pt ul = lr - GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_confirm_button->SizeMove(ul, lr);
    edit_right = ul.x - PAD;

    lr = ClientSize() + GG::Pt(-GG::X(PAD), -GG::Y(PAD));
    ul = lr - GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_clear_button->SizeMove(ul, lr);

    ul = GG::Pt(GG::X(PAD), GG::Y(PAD));
    lr = ul + GG::Pt(LABEL_WIDTH, m_design_name->MinUsableSize().y);
    m_design_name_label->SizeMove(ul, lr);

    ul.x += lr.x;
    lr.x = edit_right;
    m_design_name->SizeMove(ul, lr);

    ul.x = GG::X(PAD);
    ul.y += (m_design_name->Height() + PAD);
    lr = ul + GG::Pt(LABEL_WIDTH, m_design_name->MinUsableSize().y);
    m_design_description_label->SizeMove(ul, lr);

    ul.x = lr.x + PAD;
    lr.x = confirm_right;
    m_design_description->SizeMove(ul, lr);

    // place background image of hull
    ul.x = GG::X0;
    ul.y += m_design_name->Height();
    GG::Rect background_rect = GG::Rect(ul, ClientLowerRight());

    if (m_background_image) {
        GG::Pt ul = background_rect.UpperLeft();
        GG::Pt lr = ClientSize();
        m_background_image->SizeMove(ul, lr);
        background_rect = m_background_image->RenderedArea();
    }

    // place slot controls over image of hull
    for (std::vector<SlotControl*>::iterator it = m_slots.begin(); it != m_slots.end(); ++it) {
        SlotControl* slot = *it;
        GG::X x(background_rect.Left() - slot->Width()/2 - ClientUpperLeft().x + slot->XPositionFraction() * background_rect.Width());
        GG::Y y(background_rect.Top() - slot->Height()/2 - ClientUpperLeft().y + slot->YPositionFraction() * background_rect.Height());
        slot->MoveTo(GG::Pt(x, y));
    }
}

void DesignWnd::MainPanel::DesignChanged() {
    m_confirm_button->ClearBrowseInfoWnd();

    m_complete_design_id = ShipDesign::INVALID_DESIGN_ID;
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    std::string design_name;
    m_disabled_by_name = false;

    if (!m_hull) {
        m_confirm_button->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
            new TextBrowseWnd(UserString("DESIGN_INVALID"), UserString("DESIGN_INV_NO_HULL"))));

        m_confirm_button->Disable(true);
    }
    else if (client_empire_id == ALL_EMPIRES) {
        m_confirm_button->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
            new TextBrowseWnd(UserString("DESIGN_INVALID"), UserString("DESIGN_INV_MODERATOR"))));

        m_confirm_button->Disable(true);
    }
    else if (m_design_name->Text().empty()) { // Whitespace probably shouldn't be OK either.
        m_confirm_button->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
            new TextBrowseWnd(UserString("DESIGN_INVALID"), UserString("DESIGN_INV_NO_NAME"))));
        m_disabled_by_name = true;
        m_confirm_button->Disable(true);
    }
    else if (!ShipDesign::ValidDesign(m_hull->Name(), Parts())) {
        // I have no idea how this would happen, so I'm not going to display a tooltip for it. ~ Bigjoe5
        m_confirm_button->Disable(true);
    }
    else if (CurrentDesignIsRegistered(design_name)) {
        m_confirm_button->SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
            new TextBrowseWnd(UserString("DESIGN_KNOWN"),
            boost::io::str(FlexibleFormat(UserString("DESIGN_KNOWN_DETAIL")) % design_name))));

        m_confirm_button->Disable(true);
    }
    else {
        m_confirm_button->Disable(false);
    }
}

void DesignWnd::MainPanel::DesignNameChanged() {
    if (m_disabled_by_name || (m_design_name->Text().empty() && !m_confirm_button->Disabled()))
        DesignChangedSignal();
    else if (GetOptionsDB().Get<bool>("UI.design-pedia-dynamic"))
        DesignNameChangedSignal();
    else
        RefreshIncompleteDesign();
}

std::string DesignWnd::MainPanel::GetCleanDesignDump(const ShipDesign* ship_design) {
    std::string retval = "ShipDesign\n";
    retval += ship_design->Name() + "\"\n";
    retval += ship_design->Hull() + "\"\n";
    const std::vector<std::string> part_list = ship_design->Parts();
    for (std::vector<std::string>::const_iterator it = part_list.begin(); it != part_list.end(); ++it) {
        retval += "\"" + *it + "\"\n";
    }
    return retval; 
}

void DesignWnd::MainPanel::RefreshIncompleteDesign() const {
    if (ShipDesign* design = m_incomplete_design.get()) {
        if (design->Hull() ==           this->Hull() &&
            design->Name() ==           this->DesignName() &&
            design->Description() ==    this->DesignDescription() &&
            design->Parts() ==          this->Parts())
        {
            // nothing has changed, so don't need to update
            return;
        }
    }

    // assemble and check info for new design
    const std::string& hull =           this->Hull();
    std::vector<std::string> parts =    this->Parts();

    if (!ShipDesign::ValidDesign(hull, parts)) {
        ErrorLogger() << "DesignWnd::MainPanel::RefreshIncompleteDesign attempting to create an invalid design.";
        m_incomplete_design.reset();
        return;
    }

    // make sure name isn't blank.  TODO: prevent duplicate names?
    std::string name = this->DesignName();
    if (name.empty())
        name = UserString("DESIGN_NAME_DEFAULT");

    const std::string& description = this->DesignDescription();

    const std::string& icon = m_hull ? m_hull->Icon() : EMPTY_STRING;

    // update stored design
    try {
        m_incomplete_design.reset(new ShipDesign(name, description, CurrentTurn(), ClientApp::GetApp()->EmpireID(),
                                                 hull, parts, icon, ""));
    } catch (const std::exception& e) {
        // had a weird crash in the above call a few times, but I can't seem to
        // replicate it now.  hopefully catching any exception here will
        // prevent crashes and instead just cause the incomplete design details
        // to not update when expected.
        ErrorLogger() << "DesignWnd::MainPanel::RefreshIncompleteDesign caught exception: " << e.what();
    }
}

void DesignWnd::MainPanel::DropsAcceptable(DropsAcceptableIter first,
                                           DropsAcceptableIter last,
                                           const GG::Pt& pt) const
{
    for (DropsAcceptableIter it = first; it != last; ++it)
        it->second = false;
    if (std::distance(first, last) != 1)
        return;

    bool accepted_something = false;
    for (DropsAcceptableIter it = first; it != last; ++it) {
        if (!accepted_something && it->first->DragDropDataType() == COMPLETE_DESIGN_ROW_DROP_STRING) {
            accepted_something = true;
            it->second = true;

        } else if (!accepted_something && it->first->DragDropDataType() == HULL_PARTS_ROW_DROP_TYPE_STRING) {
            accepted_something = true;
            it->second = true;

        } else if (!accepted_something && it->first->DragDropDataType() == SAVED_DESIGN_ROW_DROP_STRING) {
            accepted_something = true;
            it->second = true;

        } else {
            it->second = false;
        }
    }
}

void DesignWnd::MainPanel::AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt) {
    if (wnds.size() != 1) {
        // delete any extra wnds that won't be processed below
        std::vector<GG::Wnd*>::const_iterator it = wnds.begin();
        ++it;
        for (; it != wnds.end(); ++it)
            delete *it;
        ErrorLogger() << "DesignWnd::MainPanel::AcceptDrops given multiple wnds unexpectedly...";
    }

    const GG::Wnd* wnd = *(wnds.begin());
    if (!wnd)
        return;

    if (wnd->DragDropDataType() == COMPLETE_DESIGN_ROW_DROP_STRING) {
        const BasesListBox::CompletedDesignListBoxRow* control =
            boost::polymorphic_downcast<const BasesListBox::CompletedDesignListBoxRow*>(wnd);
        if (control) {
            int design_id = control->DesignID();
            if (design_id != ShipDesign::INVALID_DESIGN_ID)
                SetDesign(design_id);
        }
    }
    else if (wnd->DragDropDataType() == HULL_PARTS_ROW_DROP_TYPE_STRING) {
        const BasesListBox::HullAndPartsListBoxRow* control =
            boost::polymorphic_downcast<const BasesListBox::HullAndPartsListBoxRow*>(wnd);
        if (control) {
            const std::string& hull = control->Hull();
            const std::vector<std::string>& parts = control->Parts();

            SetDesignComponents(hull, parts);
        }
    }
    else if (wnd->DragDropDataType() == SAVED_DESIGN_ROW_DROP_STRING) {
        const BasesListBox::SavedDesignListBoxRow* control =
            boost::polymorphic_downcast<const BasesListBox::SavedDesignListBoxRow*>(wnd);
        if (control) {
            const std::string& name = control->DesignName();
            const ShipDesign* design = GetSavedDesignsManager().GetDesign(name);
            if (design) {
                SetDesignComponents(design->Hull(), design->Parts(), 
                                    design->Name(), design->Description());
            }
        }
    }
    delete wnd;
}


//////////////////////////////////////////////////
// DesignWnd                                    //
//////////////////////////////////////////////////
DesignWnd::DesignWnd(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::ONTOP | GG::INTERACTIVE),
    m_detail_panel(0),
    m_base_selector(0),
    m_part_palette(0),
    m_main_panel(0)
{
    Sound::TempUISoundDisabler sound_disabler;
    SetChildClippingMode(ClipToClient);

    GG::X base_selector_width(250);
    GG::X most_panels_left = base_selector_width;
    GG::X most_panels_width = ClientWidth() - most_panels_left;
    GG::X detail_width = 5*most_panels_width/11;
    GG::X part_palette_left = base_selector_width + detail_width;
    GG::X part_palette_width = most_panels_width - detail_width;
    GG::Y detail_top = GG::Y0;
    GG::Y detail_height = 2*ClientHeight()/5;
    GG::Y main_top = detail_top + detail_height;
    GG::Y part_palette_height = detail_height;
    GG::Y part_palette_top = detail_top;
    GG::Y main_height = ClientHeight() - main_top;

    m_detail_panel = new EncyclopediaDetailPanel(detail_width, detail_height, GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | PINABLE );
    AttachChild(m_detail_panel);
    m_detail_panel->MoveTo(GG::Pt(most_panels_left, detail_top));

    m_main_panel = new MainPanel(most_panels_width, main_height);
    AttachChild(m_main_panel);
    GG::Connect(m_main_panel->PartTypeClickedSignal,            static_cast<void (EncyclopediaDetailPanel::*)(const PartType*)>(&EncyclopediaDetailPanel::SetItem),  m_detail_panel);
    GG::Connect(m_main_panel->HullTypeClickedSignal,            static_cast<void (EncyclopediaDetailPanel::*)(const HullType*)>(&EncyclopediaDetailPanel::SetItem),  m_detail_panel);
    GG::Connect(m_main_panel->DesignConfirmedSignal,            &DesignWnd::AddDesign,              this);
    GG::Connect(m_main_panel->DesignChangedSignal,              boost::bind(&EncyclopediaDetailPanel::SetIncompleteDesign,
                                                                            m_detail_panel,
                                                                            boost::bind(&DesignWnd::MainPanel::GetIncompleteDesign,
                                                                                        m_main_panel)));
    GG::Connect(m_main_panel->DesignNameChangedSignal,          boost::bind(&EncyclopediaDetailPanel::SetIncompleteDesign,
                                                                            m_detail_panel,
                                                                            boost::bind(&DesignWnd::MainPanel::GetIncompleteDesign,
                                                                                        m_main_panel)));
    GG::Connect(m_main_panel->CompleteDesignClickedSignal,      static_cast<void (EncyclopediaDetailPanel::*)(int)>(&EncyclopediaDetailPanel::SetDesign),m_detail_panel);
    m_main_panel->MoveTo(GG::Pt(most_panels_left, main_top));
    m_main_panel->Sanitize();

    m_part_palette = new PartPalette(part_palette_width, part_palette_height);
    AttachChild(m_part_palette);
    GG::Connect(m_part_palette->PartTypeClickedSignal,          static_cast<void (EncyclopediaDetailPanel::*)(const PartType*)>(&EncyclopediaDetailPanel::SetItem),  m_detail_panel);
    GG::Connect(m_part_palette->PartTypeDoubleClickedSignal,    &DesignWnd::MainPanel::AddPart,     m_main_panel);
    m_part_palette->MoveTo(GG::Pt(part_palette_left, part_palette_top));

    m_base_selector = new BaseSelector(base_selector_width, ClientHeight());
    AttachChild(m_base_selector);

    GG::Connect(m_base_selector->DesignSelectedSignal,          static_cast<void (MainPanel::*)(int)>(&MainPanel::SetDesign),
                m_main_panel);
    GG::Connect(m_base_selector->DesignComponentsSelectedSignal,&MainPanel::SetDesignComponents,
                m_main_panel);
    GG::Connect(m_base_selector->SavedDesignSelectedSignal,     &MainPanel::SetDesignComponents,
                m_main_panel);

    GG::Connect(m_base_selector->DesignClickedSignal,           static_cast<void (EncyclopediaDetailPanel::*)(const ShipDesign*)>(&EncyclopediaDetailPanel::SetItem),
                m_detail_panel);
    GG::Connect(m_base_selector->HullClickedSignal,             static_cast<void (EncyclopediaDetailPanel::*)(const HullType*)>(&EncyclopediaDetailPanel::SetItem),
                m_detail_panel);

    m_base_selector->MoveTo(GG::Pt());
}

void DesignWnd::Reset() {
    m_part_palette->Reset();
    m_base_selector->Reset();
    m_detail_panel->Refresh();
    m_main_panel->Sanitize();
}

void DesignWnd::Sanitize()
{ m_main_panel->Sanitize(); }

void DesignWnd::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    // use GL to draw the lines
    glDisable(GL_TEXTURE_2D);

    // draw background
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndColor());
        glVertex(ul.x, ul.y);
        glVertex(lr.x, ul.y);
        glVertex(lr.x, lr.y);
        glVertex(ul.x, lr.y);
        glVertex(ul.x, ul.y);
    glEnd();

    glEnable(GL_TEXTURE_2D);
}

void DesignWnd::ShowPartTypeInEncyclopedia(const std::string& part_type)
{ m_detail_panel->SetPartType(part_type); }

void DesignWnd::ShowHullTypeInEncyclopedia(const std::string& hull_type)
{ m_detail_panel->SetHullType(hull_type); }

void DesignWnd::ShowShipDesignInEncyclopedia(int design_id)
{ m_detail_panel->SetDesign(design_id); }

void DesignWnd::AddDesign() {
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire* empire = GetEmpire(empire_id);
    if (!empire) return;

    std::vector<std::string> parts = m_main_panel->Parts();
    const std::string& hull_name = m_main_panel->Hull();

    if (!ShipDesign::ValidDesign(hull_name, parts)) {
        ErrorLogger() << "DesignWnd::AddDesign tried to add an invalid ShipDesign";
        return;
    }

    // make sure name isn't blank.  TODO: prevent duplicate names?
    std::string name = m_main_panel->DesignName();
    if (name == "")
        name = UserString("DESIGN_NAME_DEFAULT");

    const std::string& description = m_main_panel->DesignDescription();

    std::string icon = "ship_hulls/generic_hull.png";
    if (const HullType* hull = GetHullType(hull_name))
        icon = hull->Icon();

    // create design from stuff chosen in UI
    ShipDesign design(name, description, CurrentTurn(), ClientApp::GetApp()->EmpireID(),
                      hull_name, parts, icon, "some model");

    int new_design_id = HumanClientApp::GetApp()->GetNewDesignID();
    HumanClientApp::GetApp()->Orders().IssueOrder(
        OrderPtr(new ShipDesignOrder(empire_id, new_design_id, design)));
    m_main_panel->ReregisterDesigns();
    m_main_panel->DesignChangedSignal();

    DebugLogger() << "Added new design: " << design.Name();
}

void DesignWnd::EnableOrderIssuing(bool enable/* = true*/)
{}
