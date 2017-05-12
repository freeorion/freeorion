#include "DesignWnd.h"

#include "ClientUI.h"
#include "CUIWnd.h"
#include "CUIControls.h"
#include "QueueListBox.h"
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
#include "../universe/Condition.h"
#include "../universe/UniverseObject.h"
#include "../universe/ShipDesign.h"
#include "../universe/Enums.h"
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
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <unordered_map>
#include <functional>

// Provide a hash function for boost::UUID
namespace std {
    template <> struct hash<boost::uuids::uuid> {
        std::size_t operator()(const boost::uuids::uuid& x) const
        { return boost::hash<boost::uuids::uuid>{}(x); }
    };
}

FO_COMMON_API extern const int INVALID_DESIGN_ID;

namespace {
    const std::string   PART_CONTROL_DROP_TYPE_STRING = "Part Control";
    const std::string   HULL_PARTS_ROW_DROP_TYPE_STRING = "Hull and Parts Row";
    const std::string   COMPLETE_DESIGN_ROW_DROP_STRING = "Complete Design Row";
    const std::string   SAVED_DESIGN_ROW_DROP_STRING = "Saved Design Row";
    const std::string   EMPTY_STRING = "";
    const std::string   DES_PEDIA_WND_NAME = "design.pedia";
    const std::string   DES_MAIN_WND_NAME = "design.main-panel";
    const std::string   DES_BASE_SELECTOR_WND_NAME = "design.base-selector";
    const std::string   DES_PART_PALETTE_WND_NAME = "design.part-palette";
    const GG::Y         BASES_LIST_BOX_ROW_HEIGHT(100);
    const GG::X         PART_CONTROL_WIDTH(54);
    const GG::Y         PART_CONTROL_HEIGHT(54);
    const GG::X         SLOT_CONTROL_WIDTH(60);
    const GG::Y         SLOT_CONTROL_HEIGHT(60);
    const int           PAD(3);

    /** Returns texture with which to render a SlotControl, depending on \a slot_type. */
    std::shared_ptr<GG::Texture> SlotBackgroundTexture(ShipSlotType slot_type) {
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
    std::shared_ptr<GG::Texture> PartBackgroundTexture(const PartType* part) {
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
            case PC_DIRECT_WEAPON:
            case PC_FIGHTER_BAY:
            case PC_FIGHTER_HANGAR:
            case PC_SHIELD:
            case PC_DETECTION:
            case PC_STEALTH:
            case PC_FUEL:
            case PC_COLONY:
            case PC_ARMOUR:
            case PC_SPEED:
            case PC_TROOPS:
            case PC_RESEARCH:
            case PC_INDUSTRY:
            case PC_TRADE:
                return part_type->Capacity();
                break;
            case PC_GENERAL:
            case PC_BOMBARD:
            case PC_PRODUCTION_LOCATION:
            default:
                return 0.0f;
        }
    }
    typedef std::map<std::pair<ShipPartClass, ShipSlotType>,
                     std::vector<const PartType*>>              PartGroupsType;

    const std::string DESIGN_FILENAME_PREFIX = "ShipDesign-";
    const std::string DESIGN_FILENAME_EXTENSION = ".focs.txt";
    const std::string DESIGN_MANIFEST_PREFIX = "ShipDesignManifest";
    const std::string UNABLE_TO_OPEN_FILE = "Unable to open file";
    boost::filesystem::path SavedDesignsDir() { return GetUserDataDir() / "shipdesigns/"; }

    void ReportFileError(const boost::filesystem::path& file) {
        std::string msg = boost::io::str(FlexibleFormat(UserString("ERROR_UNABLE_TO_WRITE_FILE")) % file);
        ErrorLogger() << msg;
        ClientUI::MessageBox(msg, true);
    }

    void WriteToFile(const boost::filesystem::path& file, const std::string& ss) {
        try {
            boost::filesystem::ofstream ofs(file);
            if (!ofs)
                return ReportFileError(file);

            ofs << ss;
            TraceLogger() << "Wrote to " << PathString(file);

        } catch (const boost::filesystem::filesystem_error& e) {
            ErrorLogger() << "Error writing to file.  Exception: " << ": " << e.what();
            ReportFileError(file);
        }
    }

    boost::filesystem::path GetDesignsDir() {
        // ensure directory present
        boost::filesystem::path designs_dir_path(SavedDesignsDir());
        if (!exists(designs_dir_path))
            boost::filesystem::create_directories(designs_dir_path);
        return designs_dir_path;
    }

    boost::filesystem::path CreateSaveFileNameForDesign(const ShipDesign& design) {
        boost::filesystem::path designs_dir_path = GetDesignsDir();

        // Since there is no easy way to guarantee that an arbitrary design name with possibly
        // embedded decorator code is a safe file name, use the UUID. The users will never interact
        // with this filename.
        std::string file_name =
            DESIGN_FILENAME_PREFIX + boost::uuids::to_string(design.UUID()) + DESIGN_FILENAME_EXTENSION;

        return boost::filesystem::absolute(PathString(designs_dir_path / file_name));
    }

    class SavedDesignsManager {
    public:
        const std::list<boost::uuids::uuid>& GetOrderedDesignUUIDs() const
        { return m_ordered_uuids; }

        static SavedDesignsManager& GetSavedDesignsManager() {
            static SavedDesignsManager manager;
            return manager;
        }

        void ClearDesigns()
        { m_saved_designs.clear(); }

        void RefreshDesigns() {
            using namespace boost::filesystem;
            ClearDesigns();

            path saved_designs_dir = SavedDesignsDir();
            if (!exists(saved_designs_dir))
                return;

            std::vector<std::pair<std::unique_ptr<ShipDesign>, boost::filesystem::path>> designs_and_paths;
            std::vector<boost::uuids::uuid> ordering;
            parse::ship_designs(saved_designs_dir, designs_and_paths, ordering);

            m_saved_designs.clear();
            for (auto&& design_and_path : designs_and_paths) {
                auto& design = design_and_path.first;

                // If the UUID is nil this is a legacy design that needs a new UUID
                if(design->UUID() == boost::uuids::uuid{{0}}) {
                    design->SetUUID(boost::uuids::random_generator()());
                    DebugLogger() << "Converted legacy ship design file by adding  UUID " << design->UUID()
                                  << " for name " << design->Name();

                }

                if (!m_saved_designs.count(design->UUID())) {
                    TraceLogger() << "Added saved design UUID " << design->UUID()
                                  << " with name " << design->Name();
                    m_saved_designs[design->UUID()] = std::move(design_and_path);
                } else {
                    WarnLogger() << "Duplicate ship design UUID " << design->UUID()
                                 << " found for ship design " << design->Name()
                                 << " in " << saved_designs_dir;
                }
            }

            // Verify that all UUIDs in ordering exist
            m_ordered_uuids.clear();
            bool ship_manifest_inconsistent = false;
            for (auto& uuid: ordering) {
                // Skip the nil UUID.
                if(uuid == boost::uuids::uuid{{0}})
                    continue;

                if (!m_saved_designs.count(uuid)) {
                    WarnLogger() << "UUID " << uuid << " is in ship design manifest for "
                                 << "a ship design that does not exist.";
                    ship_manifest_inconsistent = true;
                    continue;
                }
                m_ordered_uuids.push_back(uuid);
            }

            // Verify that every design in m_saved_designs is in m_ordered_uuids.
            if (m_ordered_uuids.size() != m_saved_designs.size()) {
                // Add any missing designs in alphabetical order to the end of the list
                std::unordered_set<boost::uuids::uuid> uuids_in_ordering{m_ordered_uuids.begin(), m_ordered_uuids.end()};
                std::map<std::string, boost::uuids::uuid> missing_uuids_sorted_by_name;
                for (auto& uuid_to_design_and_filename: m_saved_designs) {
                    if (uuids_in_ordering.count(uuid_to_design_and_filename.first))
                        continue;
                    ship_manifest_inconsistent = true;
                    missing_uuids_sorted_by_name.insert(
                        std::make_pair(uuid_to_design_and_filename.second.first->Name(),
                                       uuid_to_design_and_filename.first));
                }

                for (auto& name_and_uuid: missing_uuids_sorted_by_name) {
                    WarnLogger() << "Missing ship design " << name_and_uuid.second
                                 << " called " << name_and_uuid.first
                                 << " added to the manifest.";
                    m_ordered_uuids.push_back(name_and_uuid.second);
                }
            }

            // Write the corrected ordering back to disk.
            if (ship_manifest_inconsistent) {
                DebugLogger() << "Writing corrected ship designs back to saved designs.";
                SaveManifest();
                for (auto& uuid: m_ordered_uuids)
                    SaveDesign(uuid);
            }
        }

        const ShipDesign* GetDesign(const boost::uuids::uuid& uuid) {
            const auto& it = m_saved_designs.find(uuid);
            if (it == m_saved_designs.end())
                return nullptr;
            return it->second.first.get();
        }

         /* Causes the human client Empire to add all saved designs. */
        void LoadAllSavedDesigns() {
            int empire_id = HumanClientApp::GetApp()->EmpireID();
            if (const Empire* empire = GetEmpire(empire_id)) {
                DebugLogger() << "SavedDesignsManager::LoadAllSavedDesigns";
                for (const auto& entry : m_saved_designs) {
                    const auto& ship_design_on_disk = *(entry.second.first);
                    bool already_got = false;
                    for (int id : empire->OrderedShipDesigns()) {
                        const ShipDesign& ship_design = *GetShipDesign(id);
                        if (ship_design == ship_design_on_disk) {
                            already_got = true;
                            break;
                        }
                    }
                    if (!already_got) {
                        DebugLogger() << "SavedDesignsManager::LoadAllSavedDesigns adding saved design: " << ship_design_on_disk.Name();
                        int new_design_id = HumanClientApp::GetApp()->GetNewDesignID();
                        HumanClientApp::GetApp()->Orders().IssueOrder(
                            std::make_shared<ShipDesignOrder>(empire_id, new_design_id, ship_design_on_disk));
                    } else {
                        DebugLogger() << "SavedDesignsManager::LoadAllSavedDesigns saved design already present: " << ship_design_on_disk.Name();
                    }
                }
            } else {
                DebugLogger() << "SavedDesignsManager::LoadAllSavedDesigns HumanClient Does Not Control an Empire";
            }
        }

        void SaveManifest() {
            boost::filesystem::path designs_dir_path = GetDesignsDir();

            std::string file_name = DESIGN_MANIFEST_PREFIX + DESIGN_FILENAME_EXTENSION;

            boost::filesystem::path file =
                boost::filesystem::absolute(PathString(designs_dir_path / file_name));

            std::stringstream ss;
            ss << "ShipDesignManifest\n";
            for (const auto uuid: m_ordered_uuids)
                ss << "    uuid = \"" << uuid << "\"\n";
            WriteToFile(file, ss.str());
        }

        std::list<boost::uuids::uuid>::const_iterator InsertBefore(
            const ShipDesign& design,
            std::list<boost::uuids::uuid>::const_iterator next)
        {
            if(design.UUID() == boost::uuids::uuid{{0}}) {
                ErrorLogger() << "Ship design has a nil UUID for " << design.Name() << ". Not saving.";
                return next;
            }

            if (m_saved_designs.count(design.UUID())) {
                // UUID already exists so this is a move.  Remove the old UUID location
                const auto existing_it = std::find(m_ordered_uuids.begin(), m_ordered_uuids.end(), design.UUID());
                if (existing_it != m_ordered_uuids.end())
                    m_ordered_uuids.erase(existing_it);

            } else {
                // Add the new saved design.
                // TODO:: Use std::make_unique when switching to C++14
                std::unique_ptr<ShipDesign> design_copy{
                    new ShipDesign(design.Name(false), design.Description(false),
                                   design.Hull(), design.Parts(), design.Icon(),
                                   design.Model(), design.LookupInStringtable(),
                                   design.IsMonster(), design.UUID())};

                const auto save_path = CreateSaveFileNameForDesign(design);

                m_saved_designs.insert(
                    std::make_pair(design.UUID(), std::make_pair(std::move(design_copy), save_path)));
                SaveDesign(design.UUID());
            }

            // Insert in the list.
            const auto retval = m_ordered_uuids.insert(next, design.UUID());
            SaveManifest();
            return retval;
        }

        bool MoveBefore(const boost::uuids::uuid& moved_uuid, const boost::uuids::uuid& next_uuid) {
            if (!m_saved_designs.count(moved_uuid)) {
                ErrorLogger() << "Unable to move saved design because moved design is missing.";
                return false;
            }

            if (next_uuid != boost::uuids::uuid{{0}} && !m_saved_designs.count(next_uuid)) {
                ErrorLogger() << "Unable to move saved design because target design is missing.";
                return false;
            }

            const auto moved_it = std::find(m_ordered_uuids.begin(), m_ordered_uuids.end(), moved_uuid);
            if (moved_it == m_ordered_uuids.end()) {
                ErrorLogger() << "Unable to move saved design because moved design is missing.";
                return false;
            }

            m_ordered_uuids.erase(moved_it);

            const auto next_it = std::find(m_ordered_uuids.begin(), m_ordered_uuids.end(), next_uuid);

            // Insert in the list.
            m_ordered_uuids.insert(next_it, moved_uuid);
            SaveManifest();
            return true;
        }

        std::list<boost::uuids::uuid>::const_iterator Erase(std::list<boost::uuids::uuid>::const_iterator erasee)
        {
            if (erasee == m_ordered_uuids.end())
                return erasee;

            const auto& saved_design_it = m_saved_designs.find(*erasee);
            if (saved_design_it != m_saved_designs.end()) {
                const auto& file = saved_design_it->second.second;
                boost::filesystem::remove(file);
                m_saved_designs.erase(*erasee);
            }

            return m_ordered_uuids.erase(erasee);
        }

    private:
        SavedDesignsManager() {
            if (s_instance)
                throw std::runtime_error("Attempted to create more than one SavedDesignsManager.");
            s_instance = this;
            RefreshDesigns();
        }

        /** Save the design with the original filename or throw out_of_range..*/
        void SaveDesign(const boost::uuids::uuid &uuid) {
            const auto& design_and_filename = m_saved_designs.at(uuid);

            WriteToFile(design_and_filename.second, design_and_filename.first->Dump());
        }

        void SaveDesign(int design_id) {
            const ShipDesign* design = GetShipDesign(design_id);
            if (!design)
                return;

            // Save with the original filename if possible
            if (m_saved_designs.count(design->UUID())) {
                SaveDesign(design->UUID());
                return;
            }

            const auto save_path = CreateSaveFileNameForDesign(*design);

            WriteToFile(save_path, design->Dump());
        }

        std::list<boost::uuids::uuid> m_ordered_uuids;
        /// Saved designs with filename
        std::unordered_map<boost::uuids::uuid,
                           std::pair<std::unique_ptr<ShipDesign>,
                                     boost::filesystem::path>>  m_saved_designs;
        static SavedDesignsManager*         s_instance;
    };
    SavedDesignsManager* SavedDesignsManager::s_instance = nullptr;

    SavedDesignsManager& GetSavedDesignsManager()
    { return SavedDesignsManager::GetSavedDesignsManager(); }
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
    void Render() override;

    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
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
    m_icon(nullptr),
    m_background(nullptr),
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
    SetBrowseInfoWnd(std::make_shared<IconTextBrowseWnd>(
        ClientUI::PartIcon(m_part->Name()),
        UserString(m_part->Name()),
        UserString(m_part->Description()) + "\n" + m_part->CapacityDescription()
    ));
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
        void ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination) override;
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
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void AcceptDrops(const GG::Pt& pt, const std::vector<GG::Wnd*>& wnds, GG::Flags<GG::ModKey> mod_keys) override;

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

protected:
    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const override;

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

    GG::Control* dragged_control = nullptr;

    // find control in row
    unsigned int i = -1;
    for (i = 0; i < size(); ++i) {
        dragged_control = !empty() ? at(i) : nullptr;
        if (dragged_control == control)
            break;
        else
            dragged_control = nullptr;
    }

    if (!dragged_control)
        return;

    PartControl* part_control = dynamic_cast<PartControl*>(dragged_control);
    const PartType* part_type = nullptr;
    if (part_control)
        part_type = part_control->Part();

    RemoveCell(i);  // Wnd that accepts drop takes ownership of dragged-away control

    if (part_type) {
        part_control = new PartControl(part_type);
        const PartsListBox* parent = dynamic_cast<const PartsListBox*>(Parent());
        if (parent) {
            part_control->ClickedSignal.connect(
                parent->PartTypeClickedSignal);
            part_control->DoubleClickedSignal.connect(
                parent->PartTypeDoubleClickedSignal);
        }
        SetCell(i, part_control);
    }
}

PartsListBox::PartsListBox(void) :
    CUIListBox(),
    m_part_classes_shown(),
    m_availabilities_shown{false, false},
    m_show_superfluous_parts(true),
    m_previous_num_columns(-1)
{
    ManuallyManageColProps();
    NormalizeRowsOnInsert(false);
    SetStyle(GG::LIST_NOSEL);
}

const std::set<ShipPartClass>& PartsListBox::GetClassesShown() const
{ return m_part_classes_shown; }

const std::pair<bool, bool>& PartsListBox::GetAvailabilitiesShown() const
{ return m_availabilities_shown; }

void PartsListBox::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    // maybe later do something interesting with docking
    CUIListBox::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size()) {
        // determine how many columns can fit in the box now...
        const GG::X TOTAL_WIDTH = Size().x - ClientUI::ScrollWidth();
        const int NUM_COLUMNS = std::max(1, Value(TOTAL_WIDTH / (SLOT_CONTROL_WIDTH + GG::X(PAD))));

        if (NUM_COLUMNS != m_previous_num_columns)
            Populate();
    }
}

/** Accept parts being discarded from the ship under design.*/
void PartsListBox::AcceptDrops(const GG::Pt& pt, const std::vector<GG::Wnd*>& wnds, GG::Flags<GG::ModKey> mod_keys) {
    if (wnds.size() != 1) {
        // delete any extra wnds that won't be processed below
        std::vector<GG::Wnd*>::const_iterator it = wnds.begin();
        ++it;
        for (; it != wnds.end(); ++it)
            delete *it;
        ErrorLogger() << "PartsListBox::AcceptDrops given multiple wnds unexpectedly...";
    }

    const GG::Wnd* wnd = *(wnds.begin());
    delete wnd;
}

PartGroupsType PartsListBox::GroupAvailableDisplayableParts(const Empire* empire) {
    PartGroupsType part_groups;
    // loop through all possible parts
    for (const std::map<std::string, PartType*>::value_type& entry : GetPartTypeManager()) {
        const PartType* part = entry.second;
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

        for (ShipSlotType slot_type : part->MountableSlotTypes())
        { part_groups[{part_class, slot_type}].push_back(part); }
    }
    return part_groups;
}

// Checks if the Location condition of the check_part totally contains the Location condition of ref_part
// i,e,, the ref_part condition is met anywhere the check_part condition is
bool LocationASubsumesLocationB(const Condition::ConditionBase* check_part_loc, const Condition::ConditionBase* ref_part_loc) {
    //const Condition::ConditionBase* check_part_loc = check_part->Location();
    //const Condition::ConditionBase* ref_part_loc = ref_part->Location();
    if (dynamic_cast<const Condition::All*>(ref_part_loc))
        return true;
    if (!check_part_loc || !ref_part_loc)
        return false;
    if (*check_part_loc == *ref_part_loc)
        return true;
    // could do more involved checking for And conditions & Or, etc,
    // for now, will simply be conservative
    return false;
}

bool PartALocationSubsumesPartB(const PartType* check_part, const PartType* ref_part) {
    static std::map<std::pair<std::string, std::string>, bool> part_loc_comparison_map;

    std::pair<std::string, std::string> part_pair = {check_part->Name(), ref_part->Name()};
    std::map<std::pair<std::string, std::string>, bool>::iterator map_it = part_loc_comparison_map.find(part_pair);
    if (map_it != part_loc_comparison_map.end())
        return map_it->second;

    bool result = true;
    if (check_part->Name() == "SH_MULTISPEC" || ref_part->Name() == "SH_MULTISPEC")
        result = false;

    const Condition::ConditionBase* check_part_loc = check_part->Location();
    const Condition::ConditionBase* ref_part_loc = ref_part->Location();
    result = result && LocationASubsumesLocationB(check_part_loc, ref_part_loc);
    part_loc_comparison_map[part_pair] = result;
    //if (result && check_part_loc && ref_part_loc) {
    //    DebugLogger() << "Location for partA, " << check_part->Name() << ", subsumes that for partB, " << ref_part->Name();
    //    DebugLogger() << "   ...PartA Location is " << check_part_loc->Description();
    //    DebugLogger() << "   ...PartB Location is " << ref_part_loc->Description();
    //}
    return result;
}

void PartsListBox::CullSuperfluousParts(std::vector<const PartType* >& this_group,
                                        ShipPartClass pclass, int empire_id, int loc_id)
{
    /// This is not merely a check for obsolescence; see PartsListBox::Populate for more info
    static float min_bargain_ratio = -1.0;
    static float max_cost_ratio = -1.0;
    static float max_time_ratio = -1.0;

    if (min_bargain_ratio == -1.0) {
        min_bargain_ratio = 1.0;
        try {
            if (UserStringExists("FUNCTIONAL_MIN_BARGAIN_RATIO")) {
                float new_bargain_ratio = std::atof(UserString("FUNCTIONAL_MIN_BARGAIN_RATIO").c_str());
                if (new_bargain_ratio > 1.0f)
                    min_bargain_ratio = new_bargain_ratio;
            }
        } catch (...) {}
    }

    if (max_cost_ratio == -1.0) {
        max_cost_ratio = 1.0;
        try {
            if (UserStringExists("FUNCTIONAL_MAX_COST_RATIO")) {
                float new_cost_ratio = std::atof(UserString("FUNCTIONAL_MAX_COST_RATIO").c_str());
                if (new_cost_ratio > 1.0f)
                    max_cost_ratio = new_cost_ratio;
            }
        } catch (...) {}
    }

    if (max_time_ratio == -1.0) {
        max_time_ratio = 1.0;
        try {
            if (UserStringExists("FUNCTIONAL_MAX_TIME_RATIO")) {
                float new_time_ratio = std::atof(UserString("FUNCTIONAL_MAX_TIME_RATIO").c_str());
                if (new_time_ratio > 1.0f)
                    max_time_ratio = new_time_ratio;
            }
        } catch (...) {}
    }

    for (std::vector<const PartType* >::iterator part_it = this_group.begin();
         part_it != this_group.end(); ++part_it)
    {
        const PartType* checkPart = *part_it;
        for (const PartType* ref_part : this_group) {
            float cap_check = GetMainStat(checkPart);
            float cap_ref = GetMainStat(ref_part);
            if ((cap_check < 0.0f) || (cap_ref < 0.0f))
                continue;  // not intended to handle such cases
            float cap_ratio = cap_ref / std::max(cap_check, 1e-4f) ;  // some part types currently have zero capacity, but need to reject if both are zero
            float cost_check = checkPart->ProductionCost(empire_id, loc_id);
            float cost_ref = ref_part->ProductionCost(empire_id, loc_id);
            if ((cost_check < 0.0f) || (cost_ref < 0.0f))
                continue;  // not intended to handle such cases
            float cost_ratio = (cost_ref + 1e-4) / (cost_check + 1e-4);  // can accept if somehow they both have cost zero
            float bargain_ratio = cap_ratio / std::max(cost_ratio, 1e-4f);
            float time_ratio = float(std::max(1, ref_part->ProductionTime(empire_id, loc_id))) / std::max(1, checkPart->ProductionTime(empire_id, loc_id));
            // adjusting the max cost ratio to 1.4 or higher, will allow, for example, for
            // Zortium armor to make Standard armor redundant.  Setting a min_bargain_ratio higher than one can keep
            // trivial bargains from blocking lower valued parts.
            // TODO: move these values into default/customizations/common_user_customizations.txt  once that is supported

            if ((cap_ratio > 1.0) && ((cost_ratio <= 1.0) || ((bargain_ratio >= min_bargain_ratio) && (cost_ratio <= max_cost_ratio))) &&
                (time_ratio <= max_time_ratio) && PartALocationSubsumesPartB(checkPart, ref_part))
            {
                //DebugLogger() << "Filtering " << checkPart->Name() << " because of " << ref_part->Name();
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
    PartsListBoxRow* cur_row = nullptr;

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
        std::shared_ptr<const UniverseObject> location = GetUniverseObject(empire->CapitalID());
        loc_id = location ? location->ID() : INVALID_OBJECT_ID;
    }

    // if showing parts for a particular empire, cull redundant parts (if enabled)
    if (empire) {
        for (PartGroupsType::value_type& part_group : part_groups) {
            ShipPartClass pclass = part_group.first.first;
            if (!m_show_superfluous_parts)
                CullSuperfluousParts(part_group.second, pclass, empire_id, loc_id);
        }
    }

    // now sort the parts within each group according to main stat, via weak
    // sorting in a multimap also, if a part was in multiple groups due to being
    // compatible with multiple slot types, ensure it is only displayed once
    std::set<const PartType* > already_added;
    for (PartGroupsType::value_type& part_group : part_groups) {
        std::multimap<double, const PartType*> sorted_group;
        for (const PartType* part : part_group.second) {
            if (already_added.find(part) != already_added.end())
                continue;
            already_added.insert(part);
            sorted_group.insert({GetMainStat(part), part});
        }

        // take the sorted parts and make UI elements (technically rows) for the PartsListBox
        for (std::multimap<double, const PartType*>::value_type& group : sorted_group) {
            const PartType* part = group.second;
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
            control->ClickedSignal.connect(
                PartsListBox::PartTypeClickedSignal);
            control->DoubleClickedSignal.connect(
                PartsListBox::PartTypeDoubleClickedSignal);
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
    PartPalette(const std::string& config_name);
    //@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

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

    std::map<ShipPartClass, CUIStateButton*>    m_class_buttons;
    std::pair<CUIStateButton*, CUIStateButton*> m_availability_buttons;
    CUIStateButton*                             m_superfluous_parts_button;
};

DesignWnd::PartPalette::PartPalette(const std::string& config_name) :
    CUIWnd(UserString("DESIGN_WND_PART_PALETTE_TITLE"),
           GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE,
           config_name),
    m_parts_list(nullptr),
    m_superfluous_parts_button(nullptr)
{
    //TempUISoundDisabler sound_disabler;     // should be redundant with disabler in DesignWnd::DesignWnd.  uncomment if this is not the case
    SetChildClippingMode(ClipToClient);

    m_parts_list = new PartsListBox();
    AttachChild(m_parts_list);
    m_parts_list->PartTypeClickedSignal.connect(
        PartTypeClickedSignal);
    m_parts_list->PartTypeDoubleClickedSignal.connect(
        PartTypeDoubleClickedSignal);

    const PartTypeManager& part_manager = GetPartTypeManager();

    // class buttons
    for (ShipPartClass part_class = ShipPartClass(0); part_class != NUM_SHIP_PART_CLASSES; part_class = ShipPartClass(part_class + 1)) {
        // are there any parts of this class?
        bool part_of_this_class_exists = false;
        for (const std::map<std::string, PartType*>::value_type& entry : part_manager) {
            if (const PartType* part = entry.second) {
                if (part->Class() == part_class) {
                    part_of_this_class_exists = true;
                    break;
                }
            }
        }
        if (!part_of_this_class_exists)
            continue;

        m_class_buttons[part_class] = new CUIStateButton(UserString(boost::lexical_cast<std::string>(part_class)), GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
        AttachChild(m_class_buttons[part_class]);
        m_class_buttons[part_class]->CheckedSignal.connect(
            boost::bind(&DesignWnd::PartPalette::ToggleClass, this, part_class, true));
    }

    // availability buttons
    m_availability_buttons.first = new CUIStateButton(UserString("PRODUCTION_WND_AVAILABILITY_AVAILABLE"), GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(m_availability_buttons.first);
    m_availability_buttons.first->CheckedSignal.connect(
        boost::bind(&DesignWnd::PartPalette::ToggleAvailability, this, true, true));
    m_availability_buttons.second = new CUIStateButton(UserString("PRODUCTION_WND_AVAILABILITY_UNAVAILABLE"), GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(m_availability_buttons.second);
    m_availability_buttons.second->CheckedSignal.connect(
        boost::bind(&DesignWnd::PartPalette::ToggleAvailability, this, false, true));

    // superfluous parts button
    m_superfluous_parts_button = new CUIStateButton(UserString("PRODUCTION_WND_REDUNDANT"), GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(m_superfluous_parts_button);
    m_superfluous_parts_button->CheckedSignal.connect(
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
    for (std::map<ShipPartClass, CUIStateButton*>::value_type& entry : m_class_buttons) {
        if (col >= NUM_CLASS_BUTTONS_PER_ROW) {
            col = 0;
            ++row;
        }
        GG::Pt ul(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
        GG::Pt lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
        entry.second->SizeMove(ul, lr);
        ++col;
    }

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
    for (std::map<ShipPartClass, CUIStateButton*>::value_type& entry : m_class_buttons)
        entry.second->SetCheck();
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
    for (std::map<ShipPartClass, CUIStateButton*>::value_type& entry : m_class_buttons)
        entry.second->SetCheck(false);
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
class BasesListBox : public QueueListBox {
public:

    /** A class to allow the storage of availabilty in a common location to all
        the various BasesListBoxes. */
    class AvailabilityManager {
        private:
        // A tuple of bools for the state of the 3 availability filters:
        // Obsolete, Available and Unavailable
        std::tuple<bool, bool, bool> m_availabilities;

        public:
        enum Availability {Obsolete, Available, Unavailable};

        AvailabilityManager(bool obsolete, bool available, bool unavailable);

        bool GetAvailability(const Availability type) const;
        void SetAvailability(const Availability type, const bool state);
        void ToggleAvailability(const Availability type);
    };

    static const std::string BASES_LIST_BOX_DROP_TYPE;

    /** \name Structors */ //@{
    BasesListBox(const BasesListBox::AvailabilityManager& availabilities_state,
                 const boost::optional<std::string>& drop_type = boost::none);
    //@}

    /** \name Accessors */ //@{
    //@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination) override;

    void QueueItemMoved(const GG::ListBox::iterator& row_it, const GG::ListBox::iterator& original_position_it);

    void                            SetEmpireShown(int empire_id, bool refresh_list = true);

    virtual void                    Populate();

    //@}

    mutable boost::signals2::signal<void (int)>                 DesignSelectedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::vector<std::string>&)>
                                                                DesignComponentsSelectedSignal;
    mutable boost::signals2::signal<void (const boost::uuids::uuid&)>  SavedDesignSelectedSignal;

    mutable boost::signals2::signal<void (const ShipDesign*)>   DesignClickedSignal;
    mutable boost::signals2::signal<void (const HullType*)>     HullClickedSignal;
    mutable boost::signals2::signal<void (const ShipDesign*)>   DesignRightClickedSignal;

    class HullPanel : public GG::Control {
    public:
        HullPanel(GG::X w, GG::Y h, const std::string& hull);

        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

        void Render() override
        {}

    private:
        GG::StaticGraphic*              m_graphic;
        GG::Label*                      m_name;
    };

    class SavedDesignPanel : public GG::Control {
    public:
        SavedDesignPanel(GG::X w, GG::Y h, const boost::uuids::uuid& saved_design_uuid);

        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

        void Render() override
        {}

    private:
        GG::StaticGraphic*              m_graphic;
        GG::Label*                      m_name;
    };

    class BasesListBoxRow : public CUIListBox::Row {
    public:
        BasesListBoxRow(GG::X w, GG::Y h);

        void Render() override;

        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;
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
        SavedDesignListBoxRow(GG::X w, GG::Y h, const boost::uuids::uuid& design_uuid);
        const boost::uuids::uuid        DesignUUID() const;
        const std::string&              DesignName() const;
        const std::string&              Description() const;
        bool                            LookupInStringtable() const;

    private:
        boost::uuids::uuid              m_design_uuid;
    };

protected:
    void ItemRightClickedImpl(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) override;

    /** An implementation of BasesListBox provides a PopulateCore to fill itself.*/
    virtual void PopulateCore() = 0;

    /** If \p wnd is a valid dragged child return a replacement row.  Otherwise return nullptr. */
    virtual Row* ChildrenDraggedAwayCore(const GG::Wnd* const wnd) = 0;

    /** \name Accessors for derived classes. */ //@{
    int EmpireID() const { return m_empire_id_shown; }

    const BasesListBox::AvailabilityManager& AvailabilityState() const
    { return m_availabilities_state; }

    GG::Pt  ListRowSize();
    //@}


private:
    void    BaseDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);
    void    BaseLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);
    void    BaseRightClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);

    void    InitRowSizes();

    int                         m_empire_id_shown;
    const BasesListBox::AvailabilityManager& m_availabilities_state;

    std::set<std::string>       m_saved_desgins_in_list;

    boost::signals2::connection m_empire_designs_changed_signal;
};

BasesListBox::BasesListBoxRow::BasesListBoxRow(GG::X w, GG::Y h) :
    CUIListBox::Row(w, h, BASES_LIST_BOX_DROP_TYPE)
{}

void BasesListBox::BasesListBoxRow::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::Pt ul_adjusted_for_drop_indicator = GG::Pt(ul.x, ul.y + GG::Y(1));
    GG::Pt lr_adjusted_for_drop_indicator = GG::Pt(lr.x, lr.y - GG::Y(2));
    GG::FlatRectangle(ul_adjusted_for_drop_indicator, lr_adjusted_for_drop_indicator,
                      ClientUI::WndColor(), GG::CLR_WHITE, 1);
}

void BasesListBox::BasesListBoxRow::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    CUIListBox::Row::SizeMove(ul, lr);
    if (!empty() && old_size != Size())
        at(0)->Resize(Size());
}

BasesListBox::HullPanel::HullPanel(GG::X w, GG::Y h, const std::string& hull) :
    GG::Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
    m_graphic(nullptr),
    m_name(nullptr)
{
    SetChildClippingMode(ClipToClient);
    m_graphic = new GG::StaticGraphic(ClientUI::HullIcon(hull),
                                      GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC);
    m_graphic->Resize(GG::Pt(w, h));
    AttachChild(m_graphic);
    m_name = new CUILabel(UserString(hull), GG::FORMAT_WORDBREAK | GG::FORMAT_CENTER | GG::FORMAT_TOP);
    AttachChild(m_name);
}

void BasesListBox::HullPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Control::SizeMove(ul, lr);
    if (m_graphic)
        m_graphic->Resize(Size());
    if (m_name)
        m_name->Resize(Size());
}

BasesListBox::SavedDesignPanel::SavedDesignPanel(GG::X w, GG::Y h, const boost::uuids::uuid& saved_design_uuid) :
    GG::Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
    m_graphic(nullptr),
    m_name(nullptr)
{
    SetChildClippingMode(ClipToClient);
    const ShipDesign* design = GetSavedDesignsManager().GetDesign(saved_design_uuid);
    if (design) {
        m_graphic = new GG::StaticGraphic(ClientUI::HullIcon(design->Hull()),
                                          GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC);
        m_graphic->Resize(GG::Pt(w, h));
        AttachChild(m_graphic);

        m_name = new CUILabel(design->Name(), GG::FORMAT_WORDBREAK | GG::FORMAT_CENTER | GG::FORMAT_TOP);
        AttachChild(m_name);
    }
}

void BasesListBox::SavedDesignPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Control::SizeMove(ul, lr);
    if (m_graphic)
        m_graphic->Resize(Size());
    if (m_name)
        m_name->Resize(Size());
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
    push_back(new ShipDesignPanel(w, h, design_id));
    SetDragDropDataType(COMPLETE_DESIGN_ROW_DROP_STRING);
}

BasesListBox::SavedDesignListBoxRow::SavedDesignListBoxRow(GG::X w, GG::Y h,
                                                           const boost::uuids::uuid& design_uuid) :
    HullAndPartsListBoxRow(w, h),
    m_design_uuid(design_uuid)
{
    push_back(new SavedDesignPanel(w, h, m_design_uuid));
    SetDragDropDataType(SAVED_DESIGN_ROW_DROP_STRING);

    SavedDesignsManager& manager = GetSavedDesignsManager();
    const ShipDesign* design = manager.GetDesign(m_design_uuid);
    if (!design)
        WarnLogger() << "Design added to SavedDesignListBoxRow is not a valid saved design, uuid = " << design_uuid;
}

const boost::uuids::uuid BasesListBox::SavedDesignListBoxRow::DesignUUID() const {
    SavedDesignsManager& manager = GetSavedDesignsManager();
    const ShipDesign* design = manager.GetDesign(m_design_uuid);
    if (!design)
        return boost::uuids::uuid{};
    return design->UUID();
}

const std::string& BasesListBox::SavedDesignListBoxRow::DesignName() const {
    SavedDesignsManager& manager = GetSavedDesignsManager();
    const ShipDesign* design = manager.GetDesign(m_design_uuid);
    if (!design)
        return EMPTY_STRING;
    return design->Name();
}

const std::string& BasesListBox::SavedDesignListBoxRow::Description() const {
    SavedDesignsManager& manager = GetSavedDesignsManager();
    const ShipDesign* design = manager.GetDesign(m_design_uuid);
    if (!design)
        return EMPTY_STRING;
    return design->Description();
}

bool BasesListBox::SavedDesignListBoxRow::LookupInStringtable() const {
    SavedDesignsManager& manager = GetSavedDesignsManager();
    const ShipDesign* design = manager.GetDesign(m_design_uuid);
    if (!design)
        return false;
    return design->LookupInStringtable();
}

const std::string BasesListBox::BASES_LIST_BOX_DROP_TYPE = "BasesListBoxRow";

BasesListBox::BasesListBox(const BasesListBox::AvailabilityManager& availabilities_state,
                           const boost::optional<std::string>& drop_type) :
    QueueListBox(drop_type,  UserString("ADD_FIRST_DESIGN_DESIGN_QUEUE_PROMPT")),
    m_empire_id_shown(ALL_EMPIRES),
    m_availabilities_state(availabilities_state)
{
    InitRowSizes();
    SetStyle(GG::LIST_NOSEL | GG::LIST_NOSORT);

    DoubleClickedRowSignal.connect(
        boost::bind(&BasesListBox::BaseDoubleClicked, this, _1, _2, _3));
    LeftClickedRowSignal.connect(
        boost::bind(&BasesListBox::BaseLeftClicked, this, _1, _2, _3));
    MovedRowSignal.connect(
        boost::bind(&BasesListBox::QueueItemMoved, this, _1, _2));

    EnableOrderIssuing(false);
}

void BasesListBox::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    CUIListBox::SizeMove(ul, lr);
    if (old_size != Size()) {
        const GG::Pt row_size = ListRowSize();
        for (GG::ListBox::Row* row : *this)
            row->Resize(row_size);
    }
}

void BasesListBox::ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination) {
    if (MatchesOrContains(this, destination))
        return;
    if (wnds.empty())
        return;
    if (wnds.size() != 1)
        ErrorLogger() << "BasesListBox::ChildrenDraggedAway unexpected informed that multiple Wnds were dragged away...";
    const GG::Wnd* wnd = wnds.front();  // should only be one wnd in list as BasesListBost doesn't allow selection, so dragging is only one-at-a-time
    const GG::Control* control = dynamic_cast<const GG::Control*>(wnd);
    if (!control)
        return;

    Row* original_row = boost::polymorphic_downcast<Row*>(*wnds.begin());
    iterator insertion_point = std::find(begin(), end(), original_row);
    if (insertion_point != end())
        ++insertion_point;

    // replace dragged-away control with new copy
    auto row = ChildrenDraggedAwayCore(wnd);
    if (row) {
        Insert(row, insertion_point);
        row->Resize(ListRowSize());
    }

    // remove dragged-away row from this ListBox
    CUIListBox::ChildrenDraggedAway(wnds, destination);
    DetachChild(wnds.front());
}

void BasesListBox::QueueItemMoved(const GG::ListBox::iterator& row_it,
                                  const GG::ListBox::iterator& original_position_it)
{
    if (BasesListBox::CompletedDesignListBoxRow* control =
        dynamic_cast<BasesListBox::CompletedDesignListBoxRow*>(*row_it))
    {
        if (!GetEmpire(m_empire_id_shown))
           return;

        int design_id = control->DesignID();

        iterator insert_before_row = std::next(row_it);

        const BasesListBox::CompletedDesignListBoxRow* insert_before_control = (insert_before_row == end()) ? nullptr :
            boost::polymorphic_downcast<const BasesListBox::CompletedDesignListBoxRow*>(*insert_before_row);
        int insert_before_id = insert_before_control
            ? insert_before_control->DesignID() : INVALID_DESIGN_ID;

        control->Resize(ListRowSize());
        HumanClientApp::GetApp()->Orders()
            .IssueOrder(std::make_shared<ShipDesignOrder>(m_empire_id_shown, design_id, insert_before_id));
        return;
    }

    if (BasesListBox::SavedDesignListBoxRow* control =
        dynamic_cast<BasesListBox::SavedDesignListBoxRow*>(*row_it))
    {
        const auto& uuid = control->DesignUUID();

        iterator insert_before_row = std::next(row_it);

        const BasesListBox::SavedDesignListBoxRow* insert_before_control = (insert_before_row == end()) ? nullptr :
            boost::polymorphic_downcast<const BasesListBox::SavedDesignListBoxRow*>(*insert_before_row);
        const auto& next_uuid = insert_before_control
            ? insert_before_control->DesignUUID() : boost::uuids::uuid{{0}};

        if (uuid == next_uuid)
            return;

        if (GetSavedDesignsManager().MoveBefore(uuid, next_uuid))
            control->Resize(ListRowSize());
    }
}

void BasesListBox::SetEmpireShown(int empire_id, bool refresh_list) {
    m_empire_id_shown = empire_id;

    // disconnect old signal
    m_empire_designs_changed_signal.disconnect();

    // connect signal to update this list if the empire's designs change
    if (const Empire* empire = GetEmpire(m_empire_id_shown))
        m_empire_designs_changed_signal = empire->ShipDesignsChangedSignal.connect(
                                            boost::bind(&BasesListBox::Populate, this));

    if (refresh_list)
        Populate();
}

void BasesListBox::Populate() {
    DebugLogger() << "BasesListBox::Populate";

    // make note of first visible row to preserve state
    auto init_first_row_shown = FirstRowShown();
    std::size_t init_first_row_offset = std::distance(begin(), init_first_row_shown);

    this->PopulateCore();

    if (!Empty())
        BringRowIntoView(--end());
    if (init_first_row_offset < NumRows())
        BringRowIntoView(std::next(begin(), init_first_row_offset));
}

GG::Pt BasesListBox::ListRowSize()
{ return GG::Pt(Width() - ClientUI::ScrollWidth() - 5, BASES_LIST_BOX_ROW_HEIGHT); }

void BasesListBox::InitRowSizes() {
    // preinitialize listbox/row column widths, because what
    // ListBox::Insert does on default is not suitable for this case
    ManuallyManageColProps();
    NormalizeRowsOnInsert(false);
}

void BasesListBox::BaseLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
    // determine type of row that was clicked, and emit appropriate signal

    CompletedDesignListBoxRow* design_row = dynamic_cast<CompletedDesignListBoxRow*>(*it);
    if (design_row) {
        int id = design_row->DesignID();
        const ShipDesign* design = GetShipDesign(id);
        if (!design)
            return;
        if (modkeys & GG::MOD_KEY_CTRL)
            HumanClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<ShipDesignOrder>(HumanClientApp::GetApp()->EmpireID(), id, true));
        else
            DesignClickedSignal(design);
        return;
    }

    SavedDesignListBoxRow* saved_design_row = dynamic_cast<SavedDesignListBoxRow*>(*it);
    if (saved_design_row) {
        const auto design_uuid = saved_design_row->DesignUUID();
        const ShipDesign* design = GetSavedDesignsManager().GetDesign(design_uuid);
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

void BasesListBox::ItemRightClickedImpl(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys)
{ BaseRightClicked(it, pt, modkeys); }

void BasesListBox::BaseRightClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
    // determine type of row that was clicked, and emit appropriate signal

    if (CompletedDesignListBoxRow* design_row = dynamic_cast<CompletedDesignListBoxRow*>(*it)) {
        int design_id = design_row->DesignID();
        const ShipDesign* design = GetShipDesign(design_id);
        if (!design)
            return;

        DesignRightClickedSignal(design);

        int client_empire_id = HumanClientApp::GetApp()->EmpireID();

        DebugLogger() << "BasesListBox::BaseRightClicked on design id : " << design_id;


        // Context menu actions
        auto delete_design_action = [&client_empire_id, &design_id]() {
            HumanClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<ShipDesignOrder>(client_empire_id, design_id, true));
        };

        auto rename_design_action = [&client_empire_id, &design_id, design, &design_row]() {
            CUIEditWnd edit_wnd(GG::X(350), UserString("DESIGN_ENTER_NEW_DESIGN_NAME"), design->Name());
            edit_wnd.Run();
            const std::string& result = edit_wnd.Result();
            if (result != "" && result != design->Name()) {
                HumanClientApp::GetApp()->Orders().IssueOrder(
                    std::make_shared<ShipDesignOrder>(client_empire_id, design_id, result));
                if (!design_row->empty())
                    if (ShipDesignPanel* design_panel = dynamic_cast<ShipDesignPanel*>(design_row->at(0)))
                        design_panel->Update();
            }
        };

        auto save_design_action = [&design]() {
            auto saved_design = *design;
            saved_design.SetUUID(boost::uuids::random_generator()());
            GetSavedDesignsManager().InsertBefore(saved_design, GetSavedDesignsManager().GetOrderedDesignUUIDs().begin());
        };

        // create popup menu with a commands in it
        CUIPopupMenu popup(pt.x, pt.y);

        // delete design
        if (client_empire_id != ALL_EMPIRES)
            popup.AddMenuItem(GG::MenuItem(UserString("DESIGN_OBSOLETE"), false, false, delete_design_action));

        // rename design
        if (design->DesignedByEmpire() == client_empire_id)
            popup.AddMenuItem(GG::MenuItem(UserString("DESIGN_RENAME"), false, false, rename_design_action));

        // save design
        popup.AddMenuItem(GG::MenuItem(UserString("DESIGN_SAVE"), false, false, save_design_action));

        popup.Run();

    } else if (SavedDesignListBoxRow* design_row = dynamic_cast<SavedDesignListBoxRow*>(*it)) {
        const auto design_uuid = design_row->DesignUUID();
        SavedDesignsManager& manager = GetSavedDesignsManager();
        const ShipDesign* design = manager.GetDesign(design_uuid);
        if (!design)
            return;

        int empire_id = HumanClientApp::GetApp()->EmpireID();
        const Empire* empire = GetEmpire(empire_id);
        if (!empire)
            return;

        DesignRightClickedSignal(design);

        DebugLogger() << "BasesListBox::BaseRightClicked on design name : " << design->Name();;

        // Context menu actions
        // add design
        auto add_design_action = [&design, empire_id]() {
            DebugLogger() << "BasesListBox::BaseRightClicked Add Saved Design" << design->Name();
            int new_design_id = HumanClientApp::GetApp()->GetNewDesignID();
            HumanClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<ShipDesignOrder>(empire_id, new_design_id, *design));
        };

        // add all saved designs
        auto add_all_saved_designs_action = [&manager]() {
            DebugLogger() << "BasesListBox::BaseRightClicked LoadAllSavedDesigns";
            manager.LoadAllSavedDesigns();
        };

        // create popup menu with a commands in it
        CUIPopupMenu popup(pt.x, pt.y);
        popup.AddMenuItem(GG::MenuItem(UserString("DESIGN_ADD"),       false, false, add_design_action));
        popup.AddMenuItem(GG::MenuItem(UserString("DESIGN_ADD_ALL"),   false, false, add_all_saved_designs_action));
        popup.Run();
    }
}

void BasesListBox::BaseDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
    // determine type of row that was clicked, and emit appropriate signal

    CompletedDesignListBoxRow* cd_row = dynamic_cast<CompletedDesignListBoxRow*>(*it);
    if (cd_row) {
        if (cd_row->DesignID() != INVALID_DESIGN_ID)
            DesignSelectedSignal(cd_row->DesignID());
        return;
    }

    if (SavedDesignListBoxRow* sd_row = dynamic_cast<SavedDesignListBoxRow*>(*it)) {
        SavedDesignSelectedSignal(sd_row->DesignUUID());
        return;
    }

    HullAndPartsListBoxRow* hp_row = dynamic_cast<HullAndPartsListBoxRow*>(*it);
    if (hp_row) {
        if (!hp_row->Hull().empty() || !hp_row->Parts().empty())
            DesignComponentsSelectedSignal(hp_row->Hull(), hp_row->Parts());
        return;
    }
}

BasesListBox::AvailabilityManager::AvailabilityManager(bool obsolete, bool available, bool unavailable) :
    m_availabilities{obsolete, available, unavailable}
{}

bool BasesListBox::AvailabilityManager::GetAvailability(const Availability type) const {
    switch(type) {
    case Obsolete:
        return std::get<Obsolete>(m_availabilities);
    case Available:
        return std::get<Available>(m_availabilities);
    case Unavailable:
        return std::get<Unavailable>(m_availabilities);
    }
    return std::get<Unavailable>(m_availabilities);
}

void BasesListBox::AvailabilityManager::SetAvailability(const Availability type, const bool state) {
    switch(type) {
    case Obsolete:
        std::get<Obsolete>(m_availabilities) = state;
        break;
    case Available:
        std::get<Available>(m_availabilities) = state;
        break;
    case Unavailable:
        std::get<Unavailable>(m_availabilities) = state;
        break;
    }
}

void BasesListBox::AvailabilityManager::ToggleAvailability(const Availability type) {
    SetAvailability(type, !GetAvailability(type));
}

//////////////////////////////////////////////////
// BasesListBox derived classes                 //
//////////////////////////////////////////////////

class EmptyHullsListBox : public BasesListBox {
    public:
    EmptyHullsListBox(const BasesListBox::AvailabilityManager& availabilities_state,
                      const boost::optional<std::string>& drop_type = boost::none) :
        BasesListBox::BasesListBox(availabilities_state, drop_type)
    {};

    void EnableOrderIssuing(bool enable = true) override;

    protected:
    void PopulateCore() override;
    Row* ChildrenDraggedAwayCore(const GG::Wnd* const wnd) override;

    private:
    std::set<std::string>       m_hulls_in_list;
};

class CompletedDesignsListBox : public BasesListBox {
    public:
    CompletedDesignsListBox(const BasesListBox::AvailabilityManager& availabilities_state,
                            const boost::optional<std::string>& drop_type = boost::none) :
        BasesListBox::BasesListBox(availabilities_state, drop_type)
    {};

    protected:
    void PopulateCore() override;
    Row* ChildrenDraggedAwayCore(const GG::Wnd* const wnd) override;
};

class SavedDesignsListBox : public BasesListBox {
    public:
    SavedDesignsListBox(const BasesListBox::AvailabilityManager& availabilities_state,
                        const boost::optional<std::string>& drop_type = boost::none) :
        BasesListBox::BasesListBox(availabilities_state, drop_type)
    {};

    protected:
    void PopulateCore() override;
    Row* ChildrenDraggedAwayCore(const GG::Wnd* const wnd) override;
};

class MonstersListBox : public BasesListBox {
    public:
    MonstersListBox(const BasesListBox::AvailabilityManager& availabilities_state,
                    const boost::optional<std::string>& drop_type = boost::none) :
        BasesListBox::BasesListBox(availabilities_state, drop_type)
    {};

    void EnableOrderIssuing(bool enable = true) override;

    protected:
    void PopulateCore() override;
    Row* ChildrenDraggedAwayCore(const GG::Wnd* const wnd) override;
};


void EmptyHullsListBox::PopulateCore() {
    ScopedTimer scoped_timer("EmptyHulls::PopulateCore");
    const bool showing_available = AvailabilityState().GetAvailability(AvailabilityManager::Available);
    const bool showing_unavailable = AvailabilityState().GetAvailability(AvailabilityManager::Unavailable);
    DebugLogger() << "EmptyHulls::PopulateCore showing available (t, f):  " << showing_available << ", " << showing_unavailable;
    const Empire* empire = GetEmpire(EmpireID()); // may return 0
    DebugLogger() << "EmptyHulls::PopulateCore EmpireID(): " << EmpireID();

    //DebugLogger() << "... hulls in list: ";
    //for (const std::string& hull : m_hulls_in_list)
    //    DebugLogger() << "... ... hull: " << hull;

    // loop through all hulls, determining if they need to be added to list
    std::set<std::string> hulls_to_add;
    std::set<std::string> hulls_to_remove;

    const HullTypeManager& manager = GetHullTypeManager();
    for (const std::map<std::string, HullType*>::value_type& entry : manager) {
        const std::string& hull_name = entry.first;

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
    //for (const std::string& hull_name : hulls_to_remove)
    //    DebugLogger() << "... ... hull: " << hull_name;
    //DebugLogger() << "... hulls to add to list: ";
    //for (const std::string& hull_name : hulls_to_add)
    //    DebugLogger() << "... ... hull: " << hull_name;


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
    for (const std::string& hull_name : hulls_to_add) {
        HullAndPartsListBoxRow* row = new HullAndPartsListBoxRow(row_size.x, row_size.y, hull_name, empty_parts_vec);
        Insert(row);
        row->Resize(row_size);

        m_hulls_in_list.insert(hull_name);
    }
}

void CompletedDesignsListBox::PopulateCore() {
    ScopedTimer scoped_timer("CompletedDesignsListBox::PopulateCore");

    const bool showing_available = AvailabilityState().GetAvailability(AvailabilityManager::Available);
    const bool showing_unavailable = AvailabilityState().GetAvailability(AvailabilityManager::Unavailable);
    const Empire* empire = GetEmpire(EmpireID()); // may return 0
    const Universe& universe = GetUniverse();

    DebugLogger() << "CompletedDesignsListBox::PopulateCore for empire " << EmpireID();

    // remove preexisting rows
    Clear();
    const GG::Pt row_size = ListRowSize();

    if (empire) {
        // add rows for designs this empire is keeping
        for (int design_id : empire->OrderedShipDesigns()) {
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

void SavedDesignsListBox::PopulateCore() {
    ScopedTimer scoped_timer("CompletedDesigns::PopulateCore");

    DebugLogger() << "CompletedDesigns::PopulateCore";

    // remove preexisting rows
    Clear();
    const GG::Pt row_size = ListRowSize();

    for (const auto& entry : GetSavedDesignsManager().GetOrderedDesignUUIDs()) {
        SavedDesignListBoxRow* row = new SavedDesignListBoxRow(row_size.x, row_size.y, entry);
        Insert(row);
        row->Resize(row_size);
    }
}

void MonstersListBox::PopulateCore() {
    ScopedTimer scoped_timer("Monsters::PopulateCore");

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


BasesListBox::Row* EmptyHullsListBox::ChildrenDraggedAwayCore(const GG::Wnd* const wnd) {
    if (wnd->DragDropDataType() != HULL_PARTS_ROW_DROP_TYPE_STRING)
        return nullptr;

    // find type of hull that was dragged away, and replace
    const BasesListBox::HullAndPartsListBoxRow* design_row =
        boost::polymorphic_downcast<const BasesListBox::HullAndPartsListBoxRow*>(wnd);

    const std::string& hull_name = design_row->Hull();
    const auto row_size = ListRowSize();
    std::vector<std::string> empty_parts_vec;
    return new HullAndPartsListBoxRow(row_size.x, row_size.y, hull_name, empty_parts_vec);
}

BasesListBox::Row* CompletedDesignsListBox::ChildrenDraggedAwayCore(const GG::Wnd* const wnd) {
    if (wnd->DragDropDataType() != COMPLETE_DESIGN_ROW_DROP_STRING)
        return nullptr;
    // find design that was dragged away, and replace

    const BasesListBox::CompletedDesignListBoxRow* design_row =
        boost::polymorphic_downcast<const BasesListBox::CompletedDesignListBoxRow*>(wnd);

    int design_id = design_row->DesignID();

    const auto row_size = ListRowSize();
    return new CompletedDesignListBoxRow(row_size.x, row_size.y, design_id);
}

BasesListBox::Row* SavedDesignsListBox::ChildrenDraggedAwayCore(const GG::Wnd* const wnd) {
    if (wnd->DragDropDataType() != SAVED_DESIGN_ROW_DROP_STRING)
        return nullptr;
    // find name of design that was dragged away, and replace
    const BasesListBox::SavedDesignListBoxRow* design_row =
        boost::polymorphic_downcast<const BasesListBox::SavedDesignListBoxRow*>(wnd);

    const auto row_size = ListRowSize();
    return new SavedDesignListBoxRow(row_size.x, row_size.y, design_row->DesignUUID());
}

BasesListBox::Row* MonstersListBox::ChildrenDraggedAwayCore(const GG::Wnd* const wnd)
{ return nullptr; }


void EmptyHullsListBox::EnableOrderIssuing(bool enable/* = true*/)
{ QueueListBox::EnableOrderIssuing(false); }

void MonstersListBox::EnableOrderIssuing(bool)
{ QueueListBox::EnableOrderIssuing(false); }



//////////////////////////////////////////////////
// DesignWnd::BaseSelector                      //
//////////////////////////////////////////////////
class DesignWnd::BaseSelector : public CUIWnd {
public:
    /** \name Structors */ //@{
    BaseSelector(const std::string& config_name);
    //@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void            Reset();
    void            ToggleAvailability(const BasesListBox::AvailabilityManager::Availability type);
    void            SetEmpireShown(int empire_id, bool refresh_list);
    void            ShowAvailability(bool available, bool refresh_list);
    void            HideAvailability(bool available, bool refresh_list);
    void            EnableOrderIssuing(bool enable/* = true*/);
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
    void            SavedDesignSelectedSlot(const boost::uuids::uuid& design_name);

    GG::TabWnd*                m_tabs;
    EmptyHullsListBox*         m_hulls_list;           // empty hulls on which a new design can be based
    CompletedDesignsListBox*   m_designs_list;         // designs this empire has created or learned how to make
    SavedDesignsListBox*       m_saved_designs_list;   // designs saved to files
    MonstersListBox*           m_monsters_list;        // monster designs

    // Holds the state of the availabilities filter.
    BasesListBox::AvailabilityManager m_availabilities_state;
    std::tuple<CUIStateButton*, CUIStateButton*, CUIStateButton*> m_availabilities_buttons;
};

DesignWnd::BaseSelector::BaseSelector(const std::string& config_name) :
    CUIWnd(UserString("DESIGN_WND_STARTS"),
           GG::INTERACTIVE | GG::RESIZABLE | GG::ONTOP | GG::DRAGABLE | PINABLE,
           config_name),
    m_tabs(nullptr),
    m_hulls_list(nullptr),
    m_designs_list(nullptr),
    m_saved_designs_list(nullptr),
    m_monsters_list(nullptr),
    m_availabilities_state{false, true, false}
{

    auto& m_available_button = std::get<BasesListBox::AvailabilityManager::Available>(m_availabilities_buttons);
    m_available_button = new CUIStateButton(UserString("PRODUCTION_WND_AVAILABILITY_AVAILABLE"), GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(m_available_button);
    m_available_button->CheckedSignal.connect(
        boost::bind(&DesignWnd::BaseSelector::ToggleAvailability, this, BasesListBox::AvailabilityManager::Available));
    m_available_button->SetCheck(m_availabilities_state.GetAvailability(BasesListBox::AvailabilityManager::Available));

    auto& m_unavailable_button = std::get<BasesListBox::AvailabilityManager::Unavailable>(m_availabilities_buttons);
    m_unavailable_button = new CUIStateButton(UserString("PRODUCTION_WND_AVAILABILITY_UNAVAILABLE"), GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(m_unavailable_button);
    m_unavailable_button->CheckedSignal.connect(
        boost::bind(&DesignWnd::BaseSelector::ToggleAvailability, this, BasesListBox::AvailabilityManager::Unavailable));
    m_unavailable_button->SetCheck(m_availabilities_state.GetAvailability(BasesListBox::AvailabilityManager::Unavailable));

    m_tabs = new GG::TabWnd(GG::X(5), GG::Y(2), GG::X(10), GG::Y(10), ClientUI::GetFont(), ClientUI::WndColor(), ClientUI::TextColor());
    m_tabs->TabChangedSignal.connect(
        boost::bind(&DesignWnd::BaseSelector::Reset, this));
    AttachChild(m_tabs);

    m_hulls_list = new EmptyHullsListBox(m_availabilities_state);
    m_hulls_list->Resize(GG::Pt(GG::X(10), GG::Y(10)));
    m_tabs->AddWnd(m_hulls_list, UserString("DESIGN_WND_HULLS"));
    m_hulls_list->DesignComponentsSelectedSignal.connect(
        DesignWnd::BaseSelector::DesignComponentsSelectedSignal);
    m_hulls_list->HullClickedSignal.connect(
        DesignWnd::BaseSelector::HullClickedSignal);

    m_designs_list = new CompletedDesignsListBox(m_availabilities_state, COMPLETE_DESIGN_ROW_DROP_STRING);
    m_designs_list->Resize(GG::Pt(GG::X(10), GG::Y(10)));
    m_tabs->AddWnd(m_designs_list, UserString("DESIGN_WND_FINISHED_DESIGNS"));
    m_designs_list->DesignSelectedSignal.connect(
        DesignWnd::BaseSelector::DesignSelectedSignal);
    m_designs_list->DesignClickedSignal.connect(
        DesignWnd::BaseSelector::DesignClickedSignal);

    m_saved_designs_list = new SavedDesignsListBox(m_availabilities_state, SAVED_DESIGN_ROW_DROP_STRING);
    m_saved_designs_list->Resize(GG::Pt(GG::X(10), GG::Y(10)));
    m_tabs->AddWnd(m_saved_designs_list, UserString("DESIGN_WND_SAVED_DESIGNS"));
    m_saved_designs_list->SavedDesignSelectedSignal.connect(
        boost::bind(&DesignWnd::BaseSelector::SavedDesignSelectedSlot, this, _1));
    m_saved_designs_list->DesignClickedSignal.connect(
        DesignWnd::BaseSelector::DesignClickedSignal);

    m_monsters_list = new MonstersListBox(m_availabilities_state);
    m_monsters_list->Resize(GG::Pt(GG::X(10), GG::Y(10)));
    m_tabs->AddWnd(m_monsters_list, UserString("DESIGN_WND_MONSTERS"));
    m_monsters_list->DesignSelectedSignal.connect(
        DesignWnd::BaseSelector::DesignSelectedSignal);
    m_monsters_list->DesignClickedSignal.connect(
        DesignWnd::BaseSelector::DesignClickedSignal);

    DoLayout();
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

void DesignWnd::BaseSelector::ToggleAvailability(BasesListBox::AvailabilityManager::Availability type) {
    CUIStateButton* button;
    bool state;
    switch(type) {
    case BasesListBox::AvailabilityManager::Obsolete:
        m_availabilities_state.ToggleAvailability(BasesListBox::AvailabilityManager::Obsolete);
        state = m_availabilities_state.GetAvailability(BasesListBox::AvailabilityManager::Obsolete);
        button = std::get<BasesListBox::AvailabilityManager::Obsolete>(m_availabilities_buttons);
        break;
    case BasesListBox::AvailabilityManager::Available:
        m_availabilities_state.ToggleAvailability(BasesListBox::AvailabilityManager::Available);
        state = m_availabilities_state.GetAvailability(BasesListBox::AvailabilityManager::Available);
        button = std::get<BasesListBox::AvailabilityManager::Available>(m_availabilities_buttons);
        break;
    case BasesListBox::AvailabilityManager::Unavailable:
        m_availabilities_state.ToggleAvailability(BasesListBox::AvailabilityManager::Unavailable);
        state = m_availabilities_state.GetAvailability(BasesListBox::AvailabilityManager::Unavailable);
        button = std::get<BasesListBox::AvailabilityManager::Unavailable>(m_availabilities_buttons);
        break;
    }

    button->SetCheck(state);

    if (m_hulls_list)
        m_hulls_list->Populate();
    if (m_designs_list)
        m_designs_list->Populate();
}

void DesignWnd::BaseSelector::EnableOrderIssuing(bool enable/* = true*/) {
    if (m_hulls_list)
        m_hulls_list->EnableOrderIssuing(enable);
    if (m_designs_list)
        m_designs_list->EnableOrderIssuing(enable);
    if (m_saved_designs_list)
        m_saved_designs_list->EnableOrderIssuing(enable);
    if (m_monsters_list)
        m_monsters_list->EnableOrderIssuing(enable);
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

    auto& m_available_button = std::get<BasesListBox::AvailabilityManager::Available>(m_availabilities_buttons);
    auto& m_unavailable_button = std::get<BasesListBox::AvailabilityManager::Unavailable>(m_availabilities_buttons);

    m_available_button->SizeMove(GG::Pt(left, top), GG::Pt(left + BUTTON_WIDTH, top + BUTTON_HEIGHT));
    left = left + BUTTON_WIDTH + BUTTON_SEPARATION;
    m_unavailable_button->SizeMove(GG::Pt(left, top), GG::Pt(left + BUTTON_WIDTH, top + BUTTON_HEIGHT));
    left = LEFT_PAD;
    top = top + BUTTON_HEIGHT + BUTTON_SEPARATION;

    m_tabs->SizeMove(GG::Pt(left, top), ClientSize() - GG::Pt(LEFT_PAD, TOP_PAD));
}

void DesignWnd::BaseSelector::SavedDesignSelectedSlot(const boost::uuids::uuid& design_uuid) {
    const ShipDesign* design = GetSavedDesignsManager().GetDesign(design_uuid);
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
    ShipSlotType    SlotType() const;
    double          XPositionFraction() const;
    double          YPositionFraction() const;
    const PartType* GetPart() const;
    //@}

    /** \name Mutators */ //@{
    void StartingChildDragDrop(const GG::Wnd* wnd, const GG::Pt& offset) override;

    void CancellingChildDragDrop(const std::vector<const GG::Wnd*>& wnds) override;

    void AcceptDrops(const GG::Pt& pt, const std::vector<GG::Wnd*>& wnds, GG::Flags<GG::ModKey> mod_keys) override;

    void ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination) override;

    void DragDropEnter(const GG::Pt& pt, std::map<const GG::Wnd*, bool>& drop_wnds_acceptable,
                       GG::Flags<GG::ModKey> mod_keys) override;

    void DragDropLeave() override;

    void Render() override;

    void            Highlight(bool actually = true);

    void            SetPart(const std::string& part_name);  //!< used to programmatically set the PartControl in this slot.  Does not emit signal
    void            SetPart(const PartType* part_type = nullptr); //!< used to programmatically set the PartControl in this slot.  Does not emit signal
    //@}

    /** emitted when the contents of a slot are altered by the dragging
      * a PartControl in or out of the slot.  signal should be caught and the
      * slot contents set using SetPart accordingly */
    mutable boost::signals2::signal<void (const PartType*)> SlotContentsAlteredSignal;

    mutable boost::signals2::signal<void (const PartType*)> PartTypeClickedSignal;

protected:
    bool EventFilter(GG::Wnd* w, const GG::WndEvent& event) override;

    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const override;

private:
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
    m_part_control(nullptr),
    m_background(nullptr)
{}

SlotControl::SlotControl(double x, double y, ShipSlotType slot_type) :
    GG::Control(GG::X0, GG::Y0, SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT, GG::INTERACTIVE),
    m_highlighted(false),
    m_slot_type(slot_type),
    m_x_position_fraction(x),
    m_y_position_fraction(y),
    m_part_control(nullptr),
    m_background(nullptr)
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

    SetBrowseInfoWnd(std::make_shared<IconTextBrowseWnd>(
        SlotBackgroundTexture(m_slot_type),
        title_text,
        UserString("SL_TOOLTIP_DESC")
    ));
}

bool SlotControl::EventFilter(GG::Wnd* w, const GG::WndEvent& event) {
    //std::cout << "SlotControl::EventFilter " << EventTypeName(event) << std::endl << std::flush;

    if (w == this)
        return false;

    switch (event.Type()) {
    case GG::WndEvent::DragDropEnter:
    case GG::WndEvent::DragDropHere:
    case GG::WndEvent::CheckDrops:
    case GG::WndEvent::DragDropLeave:
    case GG::WndEvent::DragDroppedOn:
        if (w == this) {
            ErrorLogger() << "SlotControl::EventFilter w == this";
            return false;
        }
        HandleEvent(event);
        return true;
        break;
    default:
        return false;
    }
}

void SlotControl::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                                  const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const
{
    for (DropsAcceptableIter it = first; it != last; ++it)
        it->second = false;

    // if more than one control dropped somehow, reject all
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
        return nullptr;
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

    for (const GG::Wnd* wnd : wnds) {
        const PartControl* control = dynamic_cast<const PartControl*>(wnd);
        if (!control)
            continue;

        if (control == m_part_control)
            m_part_control->Show();
    }
}

void SlotControl::AcceptDrops(const GG::Pt& pt, const std::vector<GG::Wnd*>& wnds, GG::Flags<GG::ModKey> mod_keys) {
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
    const PartType* part_type = control ? control->Part() : nullptr;

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
    // responsible for deleting the accepted Wnd, so setting m_part_control = nullptr
    // here prevents this->SetPart from deleting it prematurely
    m_part_control = nullptr;
    SlotContentsAlteredSignal(nullptr);
}

void SlotControl::DragDropEnter(const GG::Pt& pt, std::map<const GG::Wnd*, bool>& drop_wnds_acceptable,
                                GG::Flags<GG::ModKey> mod_keys) {

    if (drop_wnds_acceptable.empty())
        return;

    DropsAcceptable(drop_wnds_acceptable.begin(), drop_wnds_acceptable.end(), pt, mod_keys);

    if (drop_wnds_acceptable.begin()->second && m_part_control)
        m_part_control->Hide();
}

void SlotControl::DragDropLeave() {
    if (m_part_control)
        m_part_control->Show();
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
        m_part_control = nullptr;
    }

    // create new part control for passed in part_type
    if (part_type) {
        m_part_control = new PartControl(part_type);
        AttachChild(m_part_control);
        m_part_control->InstallEventFilter(this);

        // single click shows encyclopedia data
        m_part_control->ClickedSignal.connect(
            PartTypeClickedSignal);

        // double click clears slot
        m_part_control->DoubleClickedSignal.connect(
            [this](const PartType*){ this->SlotContentsAlteredSignal(nullptr); });
        SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));

        // set part occupying slot's tool tip to say slot type
        std::string title_text;
        if (m_slot_type == SL_EXTERNAL)
            title_text = UserString("SL_EXTERNAL");
        else if (m_slot_type == SL_INTERNAL)
            title_text = UserString("SL_INTERNAL");
        else if (m_slot_type == SL_CORE)
            title_text = UserString("SL_CORE");

        m_part_control->SetBrowseInfoWnd(std::make_shared<IconTextBrowseWnd>(
            ClientUI::PartIcon(part_type->Name()),
            UserString(part_type->Name()) + " (" + title_text + ")",
            UserString(part_type->Description())
        ));
    }
}

/** PartsListBox accepts parts that are being removed from a SlotControl.*/
void PartsListBox::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                                   const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const
{
    for (DropsAcceptableIter it = first; it != last; ++it)
        it->second = false;

    // if more than one control dropped somehow, reject all
    if (std::distance(first, last) != 1)
        return;

    const GG::Wnd* parent = first->first->Parent();
    if (first->first->DragDropDataType() == PART_CONTROL_DROP_TYPE_STRING
        && parent
        && dynamic_cast<const SlotControl*>(parent))
    {
        first->second = true;
    }
}

//////////////////////////////////////////////////
// DesignWnd::MainPanel                         //
//////////////////////////////////////////////////
class DesignWnd::MainPanel : public CUIWnd {
public:
    /** \name Structors */ //@{
    MainPanel(const std::string& config_name);
    //@}

    /** \name Accessors */ //@{
    const std::vector<std::string>      Parts() const;              //!< returns vector of names of parts in slots of current shown design.  empty slots are represented with empty stri
    const std::string&                  Hull() const;               //!< returns name of hull of current shown design
    bool                                IsDesignNameValid() const;  //!< checks design name validity
    const std::string                   ValidatedDesignName() const;//!< returns name currently entered for design or valid default
    const std::string&                  DesignDescription() const;  //!< returns description currently entered for design

    /** Returns a pointer to the design currently being modified (if any).  May
        return an empty pointer if not currently modifying a design. */
    std::shared_ptr<const ShipDesign> GetIncompleteDesign() const;
    int                                 GetCompleteDesignID() const;//!< returns ID of complete design currently being shown in this panel.  returns INVALID_DESIGN_ID if not showing a complete design
    int                                 GetReplacedDesignID() const;//!< returns ID of completed design selected to be replaced.

    bool                                CurrentDesignIsRegistered(std::string& design_name);//!< returns true iff a design with the same hull and parts is already registered with thsi empire; if so, also populates design_name with the name of that design
    //@}

    /** \name Mutators */ //@{
    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void AcceptDrops(const GG::Pt& pt, const std::vector<GG::Wnd*>& wnds, GG::Flags<GG::ModKey> mod_keys) override;

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

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

    /** propagates signals from contained SlotControls that signal that a part
      * has been clicked */
    mutable boost::signals2::signal<void (const PartType*)> PartTypeClickedSignal;

    mutable boost::signals2::signal<void (const HullType*)> HullTypeClickedSignal;

    /** emitted when the user clicks the m_replace_button to replace the currently selected
      * design with the new design in the player's empire */
    mutable boost::signals2::signal<void ()>                DesignReplacedSignal;

    /** emitted when the user clicks the m_confirm_button to add the new
      * design to the player's empire */
    mutable boost::signals2::signal<void ()>                DesignConfirmedSignal;

    /** emitted when the user clicks on the background of this main panel and
      * a completed design is showing */
    mutable boost::signals2::signal<void (int)>             CompleteDesignClickedSignal;

protected:
    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const override;

private:
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
    int                                     m_replaced_design_id;
    mutable std::shared_ptr<ShipDesign> m_incomplete_design;
    std::set<std::string>                   m_completed_design_dump_strings;

    GG::StaticGraphic*  m_background_image;
    GG::Label*          m_design_name_label;
    GG::Edit*           m_design_name;
    GG::Label*          m_design_description_label;
    GG::Edit*           m_design_description;
    GG::Button*         m_replace_button;
    GG::Button*         m_confirm_button;
    GG::Button*         m_clear_button;
    bool                m_disabled_by_name; // if the design confirm button is currently disabled due to empty name
    bool                m_disabled_by_part_conflict;

    boost::signals2::connection             m_empire_designs_changed_signal;
};


DesignWnd::MainPanel::MainPanel(const std::string& config_name) :
    CUIWnd(UserString("DESIGN_WND_MAIN_PANEL_TITLE"),
           GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE,
           config_name),
    m_hull(nullptr),
    m_slots(),
    m_complete_design_id(INVALID_DESIGN_ID),
    m_replaced_design_id(INVALID_DESIGN_ID),
    m_incomplete_design(),
    m_completed_design_dump_strings(),
    m_background_image(nullptr),
    m_design_name_label(nullptr),
    m_design_name(nullptr),
    m_design_description_label(nullptr),
    m_design_description(nullptr),
    m_replace_button(nullptr),
    m_confirm_button(nullptr),
    m_clear_button(nullptr),
    m_disabled_by_name(false),
    m_disabled_by_part_conflict(false)
{
    SetChildClippingMode(ClipToClient);

    m_design_name_label = new CUILabel(UserString("DESIGN_WND_DESIGN_NAME"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_design_name = new CUIEdit(UserString("DESIGN_NAME_DEFAULT"));
    m_design_description_label = new CUILabel(UserString("DESIGN_WND_DESIGN_DESCRIPTION"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_design_description = new CUIEdit(UserString("DESIGN_DESCRIPTION_DEFAULT"));
    m_replace_button = new CUIButton(UserString("DESIGN_WND_UPDATE"));
    m_confirm_button = new CUIButton(UserString("DESIGN_WND_ADD"));
    m_clear_button = new CUIButton(UserString("DESIGN_WND_CLEAR"));

    m_replace_button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    m_confirm_button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));

    AttachChild(m_design_name_label);
    AttachChild(m_design_name);
    AttachChild(m_design_description_label);
    AttachChild(m_design_description);
    AttachChild(m_replace_button);
    AttachChild(m_confirm_button);
    AttachChild(m_clear_button);

    m_clear_button->LeftClickedSignal.connect(
        boost::bind(&DesignWnd::MainPanel::ClearParts, this));
    m_design_name->EditedSignal.connect(
        boost::bind(&DesignWnd::MainPanel::DesignNameEditedSlot, this, _1));
    m_replace_button->LeftClickedSignal.connect(
        DesignReplacedSignal);
    m_confirm_button->LeftClickedSignal.connect(
        DesignConfirmedSignal);
    this->DesignChangedSignal.connect(
        boost::bind(&DesignWnd::MainPanel::DesignChanged, this));

    DesignChanged(); // Initialize components that rely on the current state of the design.

    DoLayout();
}

const std::vector<std::string> DesignWnd::MainPanel::Parts() const {
    std::vector<std::string> retval;
    for (const SlotControl* slot : m_slots) {
        const PartType* part_type = slot->GetPart();
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

bool DesignWnd::MainPanel::IsDesignNameValid() const {
    // Whitespace probably shouldn't be OK either.
    // make sure name isn't blank.  TODO: prevent duplicate names?
    return !m_design_name->Text().empty();
}

const std::string DesignWnd::MainPanel::ValidatedDesignName() const
{ return (IsDesignNameValid()) ? m_design_name->Text() : UserString("DESIGN_NAME_DEFAULT"); }

const std::string& DesignWnd::MainPanel::DesignDescription() const
{ return m_design_description->Text(); }

std::shared_ptr<const ShipDesign> DesignWnd::MainPanel::GetIncompleteDesign() const {
    RefreshIncompleteDesign();
    return m_incomplete_design;
}

int DesignWnd::MainPanel::GetCompleteDesignID() const
{ return m_complete_design_id; }

int DesignWnd::MainPanel::GetReplacedDesignID() const
{ return m_replaced_design_id; }

bool DesignWnd::MainPanel::CurrentDesignIsRegistered(std::string& design_name) {
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const Empire* empire = GetEmpire(empire_id); // Had better not return 0 if we're designing a ship.
    if (!empire) {
        ErrorLogger() << "DesignWnd::MainPanel::CurrentDesignIsRegistered couldn't get the current empire.";
        return false;
    }

    if (std::shared_ptr<const ShipDesign> cur_design = GetIncompleteDesign()) {
        for (int design_id : empire->OrderedShipDesigns()) {
            const ShipDesign& ship_design = *GetShipDesign(design_id);
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
        for (int design_id : empire->OrderedShipDesigns()) {
            if (const ShipDesign* design = GetShipDesign(design_id)) {
                std::string dump_str = GetCleanDesignDump(design);
                m_completed_design_dump_strings.insert(dump_str); //no need to validate
            }
        }
    }
}

void DesignWnd::MainPanel::Sanitize() {
    SetHull(nullptr);
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

        // not apparent if this is working, but in typical use is unnecessary
        m_empire_designs_changed_signal = empire->ShipDesignsChangedSignal.connect(
                                            boost::bind(&MainPanel::ReregisterDesigns, this));
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
        DebugLogger() << "DesignWnd::MainPanel::AddPart(" << (part ? part->Name() : "no part")
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

    if (part->Class() == PC_FIGHTER_HANGAR) {
        // give up if part is a hangar and there is already a hangar of another type
        std::string already_seen_hangar_name;
        for (const SlotControl* slot : m_slots) {
            const PartType* part_type = slot->GetPart();
            if (!part_type || part_type->Class() != PC_FIGHTER_HANGAR)
                continue;
            if (part_type->Name() != part->Name())
                return result;
        }
    }

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
        return {-1, -1};

    // check if adding the part would cause the design to have multiple different types of hangar (which is not allowed)
    if (part->Class() == PC_FIGHTER_HANGAR) {
        for (const SlotControl* slot : m_slots) {
            const PartType* existing_part = slot->GetPart();
            if (!existing_part || existing_part->Class() != PC_FIGHTER_HANGAR)
                continue;
            if (existing_part->Name() != part->Name())
                return {-1, -1};  // conflict; new part can't be added
        }
    }

    // first search for an empty compatible slot for the new part
    for (const SlotControl* slot : m_slots) {
        if (!part->CanMountInSlotType(slot->SlotType()))
            continue;   // skip incompatible slots

        if (!slot->GetPart())
            return {-1, -1};  // empty slot that can hold part. no swapping needed.
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
                return {i, j};    // other slot can hold current part to make room for new part
        }
    }

    return {-1, -1};
}

void DesignWnd::MainPanel::ClearParts() {
    for (SlotControl* slot : m_slots)
        slot->SetPart(nullptr);
    DesignChangedSignal();
}

void DesignWnd::MainPanel::SetHull(const std::string& hull_name)
{ SetHull(GetHullType(hull_name)); }

void DesignWnd::MainPanel::SetHull(const HullType* hull) {
    m_hull = hull;
    DeleteChild(m_background_image);
    m_background_image = nullptr;
    if (m_hull) {
        std::shared_ptr<GG::Texture> texture = ClientUI::HullTexture(hull->Name());
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
        SetHull(nullptr);
        return;
    }

    m_complete_design_id = ship_design->ID();
    m_replaced_design_id = ship_design->IsMonster() ? INVALID_DESIGN_ID : ship_design->ID();

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
    m_replaced_design_id = INVALID_DESIGN_ID;
    SetHull(hull);
    SetParts(parts);
}

void DesignWnd::MainPanel::SetDesignComponents(const std::string& hull,
                                               const std::vector<std::string>& parts,
                                               const std::string& name,
                                               const std::string& desc)
{
    SetDesignComponents(hull, parts);
    m_design_name->SetText(name);
    m_design_description->SetText(desc);
}

void DesignWnd::MainPanel::HighlightSlotType(std::vector<ShipSlotType>& slot_types) {
    for (SlotControl* control : m_slots) {
        ShipSlotType slot_type = control->SlotType();
        if (std::find(slot_types.begin(), slot_types.end(), slot_type) != slot_types.end())
            control->Highlight(true);
        else
            control->Highlight(false);
    }
}

void DesignWnd::MainPanel::Populate(){
    for (SlotControl* control : m_slots)
        delete control;
    m_slots.clear();

    if (!m_hull)
        return;

    const std::vector<HullType::Slot>& hull_slots = m_hull->Slots();

    for (std::vector<HullType::Slot>::size_type i = 0; i != hull_slots.size(); ++i) {
        const HullType::Slot& slot = hull_slots[i];
        SlotControl* slot_control = new SlotControl(slot.x, slot.y, slot.type);
        m_slots.push_back(slot_control);
        AttachChild(slot_control);
        slot_control->SlotContentsAlteredSignal.connect(
            boost::bind(static_cast<void (DesignWnd::MainPanel::*)(const PartType*, unsigned int, bool)>(&DesignWnd::MainPanel::SetPart), this, _1, i, true));
        slot_control->PartTypeClickedSignal.connect(
            PartTypeClickedSignal);
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

    lr = lr - GG::Pt(BUTTON_WIDTH, GG::Y(0))- GG::Pt(GG::X(PAD),GG::Y(0));
    ul = lr - GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_replace_button->SizeMove(ul, lr);

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
    for (SlotControl* slot : m_slots) {
        GG::X x(background_rect.Left() - slot->Width()/2 - ClientUpperLeft().x + slot->XPositionFraction() * background_rect.Width());
        GG::Y y(background_rect.Top() - slot->Height()/2 - ClientUpperLeft().y + slot->YPositionFraction() * background_rect.Height());
        slot->MoveTo(GG::Pt(x, y));
    }
}

void DesignWnd::MainPanel::DesignChanged() {
    m_replace_button->ClearBrowseInfoWnd();
    m_confirm_button->ClearBrowseInfoWnd();

    m_complete_design_id = INVALID_DESIGN_ID;
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    std::string design_name;
    m_disabled_by_name = false;
    m_disabled_by_part_conflict = false;

    m_replace_button->Disable(true);
    m_confirm_button->Disable(true);

    if (!m_hull) {
        m_replace_button->SetBrowseInfoWnd(std::make_shared<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_UPDATE_INVALID_NO_CANDIDATE")));
        m_confirm_button->SetBrowseInfoWnd(std::make_shared<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_INV_NO_HULL")));
    }
    else if (client_empire_id == ALL_EMPIRES) {
        m_replace_button->SetBrowseInfoWnd(std::make_shared<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_INV_MODERATOR")));
        m_confirm_button->SetBrowseInfoWnd(std::make_shared<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_INV_MODERATOR")));
    }
    else if (!IsDesignNameValid()) {
        m_disabled_by_name = true;

        m_replace_button->SetBrowseInfoWnd(std::make_shared<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_INV_NO_NAME")));
        m_confirm_button->SetBrowseInfoWnd(std::make_shared<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_INV_NO_NAME")));
    }
    else if (!ShipDesign::ValidDesign(m_hull->Name(), Parts())) {
        // if a design has exclusion violations between parts and hull, highlight these and indicate it on the button

        std::pair<std::string, std::string> problematic_components;

        // check hull exclusions against all parts...
        const std::set<std::string>& hull_exclusions = m_hull->Exclusions();
        for (const std::string& part_name : Parts()) {
            if (part_name.empty())
                continue;
            if (hull_exclusions.find(part_name) != hull_exclusions.end()) {
                m_disabled_by_part_conflict = true;
                problematic_components.first = m_hull->Name();
                problematic_components.second = part_name;
            }
        }

        // check part exclusions against other parts and hull
        std::set<std::string> already_seen_component_names;
        already_seen_component_names.insert(m_hull->Name());
        for (const std::string& part_name : Parts()) {
            if (m_disabled_by_part_conflict)
                break;
            const PartType* part_type = GetPartType(part_name);
            if (!part_type)
                continue;
            for (const std::string& excluded_part : part_type->Exclusions()) {
                if (already_seen_component_names.find(excluded_part) != already_seen_component_names.end()) {
                    m_disabled_by_part_conflict = true;
                    problematic_components.first = part_name;
                    problematic_components.second = excluded_part;
                    break;
                }
            }
            already_seen_component_names.insert(part_name);
        }


        if (m_disabled_by_part_conflict) {
            m_replace_button->SetBrowseInfoWnd(std::make_shared<TextBrowseWnd>(
                UserString("DESIGN_COMPONENT_CONFLICT"),
                boost::io::str(FlexibleFormat(UserString("DESIGN_COMPONENT_CONFLICT_DETAIL"))
                               % UserString(problematic_components.first)
                               % UserString(problematic_components.second))));
            m_confirm_button->SetBrowseInfoWnd(std::make_shared<TextBrowseWnd>(
                UserString("DESIGN_COMPONENT_CONFLICT"),
                boost::io::str(FlexibleFormat(UserString("DESIGN_COMPONENT_CONFLICT_DETAIL"))
                               % UserString(problematic_components.first)
                               % UserString(problematic_components.second))));

            // todo: mark conflicting parts somehow
        }
    }
    else if (CurrentDesignIsRegistered(design_name)) {
        m_replace_button->SetBrowseInfoWnd(std::make_shared<TextBrowseWnd>(
            UserString("DESIGN_KNOWN"),
            boost::io::str(FlexibleFormat(UserString("DESIGN_KNOWN_DETAIL"))
                           % design_name)));
        m_confirm_button->SetBrowseInfoWnd(std::make_shared<TextBrowseWnd>(
            UserString("DESIGN_KNOWN"),
            boost::io::str(FlexibleFormat(UserString("DESIGN_KNOWN_DETAIL"))
                           % design_name)));
    }
    else {
        std::string new_design_name = ValidatedDesignName();
        const ShipDesign* replaced_ship_design = GetShipDesign(m_replaced_design_id);

        if (m_replaced_design_id != INVALID_DESIGN_ID && replaced_ship_design) {
            m_replace_button->SetBrowseInfoWnd(std::make_shared<TextBrowseWnd>(
                UserString("DESIGN_WND_UPDATE"),
                boost::io::str(FlexibleFormat(UserString("DESIGN_WND_UPDATE_DETAIL"))
                                              % replaced_ship_design->Name()
                                              % new_design_name)));
            m_replace_button->Disable(false);
        }
        m_confirm_button->SetBrowseInfoWnd(std::make_shared<TextBrowseWnd>(
                UserString("DESIGN_WND_ADD"),
                boost::io::str(FlexibleFormat(UserString("DESIGN_WND_ADD_DETAIL"))
                                              % new_design_name)));
        m_confirm_button->Disable(false);
    }
}

void DesignWnd::MainPanel::DesignNameChanged() {
    if (m_disabled_by_name || (!IsDesignNameValid() && !m_confirm_button->Disabled()))
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
    for (const std::string& part_name : ship_design->Parts()) {
        retval += "\"" + part_name + "\"\n";
    }
    return retval;
}

void DesignWnd::MainPanel::RefreshIncompleteDesign() const {
    if (ShipDesign* design = m_incomplete_design.get()) {
        if (design->Hull() ==           this->Hull() &&
            design->Name() ==           this->m_design_name->Text() &&
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

    std::string name = this->ValidatedDesignName();

    const std::string& description = this->DesignDescription();

    const std::string& icon = m_hull ? m_hull->Icon() : EMPTY_STRING;

    const auto uuid = boost::uuids::random_generator()();

    // update stored design
    m_incomplete_design.reset();
    try {
        m_incomplete_design.reset(new ShipDesign(std::invalid_argument(""),
                                                 name, description, CurrentTurn(), ClientApp::GetApp()->EmpireID(),
                                                 hull, parts, icon, "", false, false, uuid));
    } catch (const std::invalid_argument& e) {
        ErrorLogger() << "DesignWnd::MainPanel::RefreshIncompleteDesign " << e.what();
    }
}

void DesignWnd::MainPanel::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                                           const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const
{
    for (DropsAcceptableIter it = first; it != last; ++it)
        it->second = false;

    // if multiple things dropped simultaneously somehow, reject all
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

void DesignWnd::MainPanel::AcceptDrops(const GG::Pt& pt, const std::vector<GG::Wnd*>& wnds, GG::Flags<GG::ModKey> mod_keys) {
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
            if (design_id != INVALID_DESIGN_ID)
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
            const auto& uuid = control->DesignUUID();
            const ShipDesign* design = GetSavedDesignsManager().GetDesign(uuid);
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
    m_detail_panel(nullptr),
    m_base_selector(nullptr),
    m_part_palette(nullptr),
    m_main_panel(nullptr)
{
    Sound::TempUISoundDisabler sound_disabler;
    SetChildClippingMode(ClipToClient);

    m_detail_panel = new EncyclopediaDetailPanel(GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | PINABLE, DES_PEDIA_WND_NAME);
    m_main_panel = new MainPanel(DES_MAIN_WND_NAME);
    m_part_palette = new PartPalette(DES_PART_PALETTE_WND_NAME);
    m_base_selector = new BaseSelector(DES_BASE_SELECTOR_WND_NAME);
    InitializeWindows();
    HumanClientApp::GetApp()->RepositionWindowsSignal.connect(
        boost::bind(&DesignWnd::InitializeWindows, this));

    AttachChild(m_detail_panel);

    AttachChild(m_main_panel);
    m_main_panel->PartTypeClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(const PartType*)>(&EncyclopediaDetailPanel::SetItem), m_detail_panel, _1));
    m_main_panel->HullTypeClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(const HullType*)>(&EncyclopediaDetailPanel::SetItem), m_detail_panel, _1));
    m_main_panel->DesignReplacedSignal.connect(
        boost::bind(&DesignWnd::ReplaceDesign, this));
    m_main_panel->DesignConfirmedSignal.connect(
        boost::bind(&DesignWnd::AddDesign, this));
    m_main_panel->DesignChangedSignal.connect(
        boost::bind(&DesignWnd::DesignChanged, this));
    m_main_panel->DesignNameChangedSignal.connect(
        boost::bind(&DesignWnd::DesignNameChanged, this));
    m_main_panel->CompleteDesignClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(int)>(&EncyclopediaDetailPanel::SetDesign), m_detail_panel, _1));
    m_main_panel->Sanitize();

    AttachChild(m_part_palette);
    m_part_palette->PartTypeClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(const PartType*)>(&EncyclopediaDetailPanel::SetItem), m_detail_panel, _1));
    m_part_palette->PartTypeDoubleClickedSignal.connect(
        boost::bind(&DesignWnd::MainPanel::AddPart, m_main_panel, _1));

    AttachChild(m_base_selector);

    m_base_selector->DesignSelectedSignal.connect(
        boost::bind(static_cast<void (MainPanel::*)(int)>(&MainPanel::SetDesign), m_main_panel, _1));
    m_base_selector->DesignComponentsSelectedSignal.connect(
        boost::bind(&MainPanel::SetDesignComponents, m_main_panel, _1, _2));
    m_base_selector->SavedDesignSelectedSignal.connect(
        boost::bind(&MainPanel::SetDesignComponents, m_main_panel, _1, _2));

    m_base_selector->DesignClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(const ShipDesign*)>(&EncyclopediaDetailPanel::SetItem), m_detail_panel, _1));
    m_base_selector->HullClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(const HullType*)>(&EncyclopediaDetailPanel::SetItem), m_detail_panel, _1));
}

void DesignWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    GG::Wnd::SizeMove(ul, lr);
    if (old_size != Size()) {
        m_detail_panel->ValidatePosition();
        m_base_selector->ValidatePosition();
        m_part_palette->ValidatePosition();
        m_main_panel->ValidatePosition();
    }
}

void DesignWnd::Reset() {
    m_part_palette->Reset();
    m_base_selector->Reset();
    m_detail_panel->Refresh();
    m_main_panel->Sanitize();
}

void DesignWnd::Sanitize()
{ m_main_panel->Sanitize(); }

void DesignWnd::Render()
{ GG::FlatRectangle(UpperLeft(), LowerRight(), ClientUI::WndColor(), GG::CLR_ZERO, 0); }

void DesignWnd::InitializeWindows() {
    const GG::X selector_width = GG::X(250);
    const GG::X main_width = ClientWidth() - selector_width;

    const GG::Pt pedia_ul(selector_width, GG::Y0);
    const GG::Pt pedia_wh(5*main_width/11, 2*ClientHeight()/5);

    const GG::Pt main_ul(selector_width, pedia_ul.y + pedia_wh.y);
    const GG::Pt main_wh(main_width, ClientHeight() - main_ul.y);

    const GG::Pt palette_ul(selector_width + pedia_wh.x, pedia_ul.y);
    const GG::Pt palette_wh(main_width - pedia_wh.x, pedia_wh.y);

    const GG::Pt selector_ul(GG::X0, GG::Y0);
    const GG::Pt selector_wh(selector_width, ClientHeight());

    m_detail_panel-> InitSizeMove(pedia_ul,     pedia_ul + pedia_wh);
    m_main_panel->   InitSizeMove(main_ul,      main_ul + main_wh);
    m_part_palette-> InitSizeMove(palette_ul,   palette_ul + palette_wh);
    m_base_selector->InitSizeMove(selector_ul,  selector_ul + selector_wh);
}

void DesignWnd::ShowPartTypeInEncyclopedia(const std::string& part_type)
{ m_detail_panel->SetPartType(part_type); }

void DesignWnd::ShowHullTypeInEncyclopedia(const std::string& hull_type)
{ m_detail_panel->SetHullType(hull_type); }

void DesignWnd::ShowShipDesignInEncyclopedia(int design_id)
{ m_detail_panel->SetDesign(design_id); }

int DesignWnd::AddDesign() {
    try {
        int empire_id = HumanClientApp::GetApp()->EmpireID();
        const Empire* empire = GetEmpire(empire_id);
        if (!empire) return INVALID_DESIGN_ID;

        std::vector<std::string> parts = m_main_panel->Parts();
        const std::string& hull_name = m_main_panel->Hull();

        if (!ShipDesign::ValidDesign(hull_name, parts)) {
            ErrorLogger() << "DesignWnd::AddDesign tried to add an invalid ShipDesign";
            return INVALID_DESIGN_ID;
        }

        std::string name = m_main_panel->ValidatedDesignName();

        const std::string& description = m_main_panel->DesignDescription();

        std::string icon = "ship_hulls/generic_hull.png";
        if (const HullType* hull = GetHullType(hull_name))
            icon = hull->Icon();

        const auto uuid = boost::uuids::random_generator()();

        // create design from stuff chosen in UI
        ShipDesign design(std::invalid_argument(""), name, description, CurrentTurn(), ClientApp::GetApp()->EmpireID(),
                          hull_name, parts, icon, "some model", false, false, uuid);

        int new_design_id = HumanClientApp::GetApp()->GetNewDesignID();
        HumanClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<ShipDesignOrder>(empire_id, new_design_id, design));
        m_main_panel->ReregisterDesigns();
        m_main_panel->DesignChangedSignal();

        DebugLogger() << "Added new design: " << design.Name();

        return new_design_id;
    } catch (std::invalid_argument&) {
        ErrorLogger() << "DesignWnd::AddDesign tried to add an invalid ShipDesign";
        return INVALID_DESIGN_ID;
    }
}

void DesignWnd::ReplaceDesign() {
    int new_design_id = AddDesign();
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    int replaced_id = m_main_panel->GetReplacedDesignID();

    if (new_design_id == INVALID_DESIGN_ID) return;

    //move it to before the replaced design
    HumanClientApp::GetApp()->Orders().IssueOrder(std::make_shared<ShipDesignOrder>(empire_id, new_design_id, replaced_id ));
    //remove old design
    HumanClientApp::GetApp()->Orders().IssueOrder(std::make_shared<ShipDesignOrder>(empire_id, replaced_id, true));
    DebugLogger() << "Replaced design #" << replaced_id << " with #" << new_design_id ;
}

void DesignWnd::DesignChanged()
{ m_detail_panel->SetIncompleteDesign(m_main_panel->GetIncompleteDesign()); }

void DesignWnd::DesignNameChanged()
{ m_detail_panel->SetIncompleteDesign(m_main_panel->GetIncompleteDesign()); }

void DesignWnd::EnableOrderIssuing(bool enable/* = true*/)
{ m_base_selector->EnableOrderIssuing(enable); }
