#include "DesignWnd.h"

#include "ClientUI.h"
#include "CUIWnd.h"
#include "CUIControls.h"
#include "QueueListBox.h"
#include "EncyclopediaDetailPanel.h"
#include "IconTextBrowseWnd.h"
#include "Sound.h"
#include "TextBrowseWnd.h"
#include "../parse/Parse.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Order.h"
#include "../util/OptionsDB.h"
#include "../util/ScopedTimer.h"
#include "../util/Directories.h"
#include "../Empire/Empire.h"
#include "../client/human/GGHumanClientApp.h"
#include "../universe/ConditionAll.h"
#include "../universe/UniverseObject.h"
#include "../universe/ShipDesign.h"
#include "../universe/ShipPart.h"
#include "../universe/ShipHull.h"
#include "../universe/ValueRef.h"

#include <GG/StaticGraphic.h>
#include <GG/TabWnd.h>

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

struct Availability {
    // Declaring an enum inside a struct makes the syntax when using the enum 
    // with tuples simpler, without polluting the global namespace with 3
    // generic names.
    enum Enum {
        Obsolete,  // A design/part is researched/known by the player has marked it obsolete
        Available, // A design/part is researched/known and currently available
        Future     // A design/part is unresearched and hence not available
    };
};

class CUIEdit;

namespace {
#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    constexpr std::string EMPTY_STRING;
#else
    const std::string EMPTY_STRING;
#endif

    constexpr std::string_view PART_CONTROL_DROP_TYPE_STRING = "Part Control";
    constexpr std::string_view HULL_PARTS_ROW_DROP_TYPE_STRING = "Hull and Parts Row";
    constexpr std::string_view COMPLETE_DESIGN_ROW_DROP_STRING = "Complete Design Row";
    constexpr std::string_view SAVED_DESIGN_ROW_DROP_STRING = "Saved Design Row";
    constexpr std::string_view DES_PEDIA_WND_NAME = "design.pedia";
    constexpr std::string_view DES_MAIN_WND_NAME = "design.edit";
    constexpr std::string_view DES_BASE_SELECTOR_WND_NAME = "design.selector";
    constexpr std::string_view DES_PART_PALETTE_WND_NAME = "design.parts";
    constexpr GG::Y            BASES_LIST_BOX_ROW_HEIGHT{100};
    constexpr GG::X            PART_CONTROL_WIDTH{54};
    constexpr GG::Y            PART_CONTROL_HEIGHT{54};
    constexpr GG::X            SLOT_CONTROL_WIDTH{60};
    constexpr GG::Y            SLOT_CONTROL_HEIGHT{60};
    constexpr GG::Pt           PALETTE_MIN_SIZE{GG::X{450}, GG::Y{400}};
    constexpr GG::Pt           MAIN_PANEL_MIN_SIZE{GG::X{400}, GG::Y{160}};
    constexpr GG::Pt           BASES_MIN_SIZE{GG::X{160}, GG::Y{160}};
    constexpr int              PAD{3};

    /** Returns texture with which to render a SlotControl, depending on \a slot_type. */
    std::shared_ptr<GG::Texture> SlotBackgroundTexture(ShipSlotType slot_type) {
        if (slot_type == ShipSlotType::SL_EXTERNAL)
            return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "ship_parts" / "external_slot.png", true);
        else if (slot_type == ShipSlotType::SL_INTERNAL)
            return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "ship_parts" / "internal_slot.png", true);
        else if (slot_type == ShipSlotType::SL_CORE)
            return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "ship_parts" / "core_slot.png", true);
        else
            return ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "missing.png", true);
    }

    //! Returns background texture with which to render a PartControl,
    //! depending on the types of slot that the indicated @a part can be put
    //! into.
    std::shared_ptr<GG::Texture> PartBackgroundTexture(const ShipPart* part) {
        if (part) {
            bool ex = part->CanMountInSlotType(ShipSlotType::SL_EXTERNAL);
            bool in = part->CanMountInSlotType(ShipSlotType::SL_INTERNAL);
            bool co = part->CanMountInSlotType(ShipSlotType::SL_CORE);

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

    float GetMainStat(const ShipPart* part)  {
        if (!part)
            return 0.0f;
        switch (part->Class()) {
            case ShipPartClass::PC_DIRECT_WEAPON:
            case ShipPartClass::PC_FIGHTER_BAY:
            case ShipPartClass::PC_FIGHTER_HANGAR:
            case ShipPartClass::PC_SHIELD:
            case ShipPartClass::PC_DETECTION:
            case ShipPartClass::PC_STEALTH:
            case ShipPartClass::PC_FUEL:
            case ShipPartClass::PC_COLONY:
            case ShipPartClass::PC_ARMOUR:
            case ShipPartClass::PC_SPEED:
            case ShipPartClass::PC_TROOPS:
            case ShipPartClass::PC_RESEARCH:
            case ShipPartClass::PC_INDUSTRY:
            case ShipPartClass::PC_INFLUENCE:
                return part->Capacity();
                break;
            case ShipPartClass::PC_GENERAL:
            case ShipPartClass::PC_BOMBARD:
            case ShipPartClass::PC_PRODUCTION_LOCATION:
            default:
                return 0.0f;
        }
    }

    typedef std::map<std::pair<ShipPartClass, ShipSlotType>, std::vector<const ShipPart*>> PartGroupsType;

    constexpr std::string_view formatting_chars = "<>;:,.@#$%&*(){}'\"/?\\`[]|\a\b\f\n\r\t\b";

    constexpr std::string_view DESIGN_FILENAME_PREFIX = "ShipDesign-";
    constexpr std::string_view DESIGN_FILENAME_EXTENSION = ".focs.txt";
    constexpr std::string_view DESIGN_MANIFEST_PREFIX = "ShipDesignOrdering";
    boost::filesystem::path SavedDesignsDir() { return GetUserDataDir() / "shipdesigns/"; }

    void ReportFileError(const boost::filesystem::path& file) {
        std::string msg = boost::io::str(FlexibleFormat(UserString("ERROR_UNABLE_TO_WRITE_FILE"))
                                         % PathToString(file));
        ErrorLogger() << msg;
        ClientUI::MessageBox(msg, true);
    }

    void WriteToFile(const boost::filesystem::path& file, const std::string& ss) {
        try {
            boost::filesystem::ofstream ofs(file);
            if (!ofs)
                return ReportFileError(file);

            ofs << ss;
            TraceLogger() << "Wrote to " << PathToString(file);

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

    boost::filesystem::path CreateSavePathForDesign(const ShipDesign& design) {
        boost::filesystem::path designs_dir_path = GetDesignsDir();

        // Since there is no easy way to guarantee that an arbitrary design name with possibly
        // embedded decorator code is a safe file name, use the UUID. The users will never interact
        // with this filename.
        const auto file_name{std::string(DESIGN_FILENAME_PREFIX)
            .append(boost::uuids::to_string(design.UUID())).append(DESIGN_FILENAME_EXTENSION)};

        return boost::filesystem::absolute(designs_dir_path / file_name);
    }


    /** DisplayedShipDesignManager allows for the storage and manipulation of an
      * ordered list of design ids that are used to order the display of
      * ShipDesigns in the DesignWnd and the ProductionWnd. */
    class DisplayedShipDesignManager : public ShipDesignManager::Designs {
    public:
        /** Return non-obsolete available ordered ids. */
        std::vector<int> OrderedIDs() const override;

        /** Return all ids including obsoleted designs. */
        std::vector<int> AllOrderedIDs() const;

        /** Return non-obsolete available hulls. */
        std::vector<std::string> OrderedHulls() const;

        template <typename T>
        void InsertOrderedIDs(const T& new_order);

        void InsertBefore(const int id, const int next_id);
        bool MoveBefore(const int moved_id, const int next_id);
        void Remove(const int id);

        void InsertHullBefore(const std::string& id, const std::string& next_id = EMPTY_STRING);

        bool IsKnown(const int id) const;

        /** Return true if design \p id is obsolete or boost::none if \p id is not in
            the manager. */
        boost::optional<bool> IsObsolete(const int id, const ScriptingContext& context) const;
        /** Return UI event number that obsoletes \p hull if it is obsolete. */
        boost::optional<int> IsHullObsolete(const std::string& hull) const;
        /** Return UI event number that obsoletes \p part if it is obsolete. */
        boost::optional<int> IsPartObsolete(const std::string& part) const;

    private:
        /* Increment and return the obsolete ui event counter. */
        int NextUIObsoleteEvent();

    public:
        /** If \p id is in manager, set \p id's obsolescence to \p obsolete. */
        void SetObsolete(const int id, const bool obsolete);
        /** Set \p hull's obsolescence to \p obsolete. */
        void SetHullObsolete(const std::string& hull, const bool obsolete);
        /** Set \p part's obsolescence to \p obsolete. */
        void SetPartObsolete(const std::string& part, const bool obsolete);

        void Load(const int obsolete_ui_event_count,
                  const std::vector<std::pair<int, boost::optional<std::pair<bool, int>>>>& design_ids_and_obsoletes,
                  const std::vector<std::pair<std::string, std::pair<bool, int>>>& hulls_and_obsoletes,
                  const std::unordered_map<std::string, int>& obsolete_parts);

        /** Save modifies each of its parameters to store obsolete and
            ordering data in the UI save data.*/
        void Save(int& obsolete_ui_event_count,
                  std::vector<std::pair<int, boost::optional<std::pair<bool, int>>>>& design_ids_and_obsoletes,
                  std::vector<std::pair<std::string, std::pair<bool, int>>>& hulls_and_obsoletes,
                  std::unordered_map<std::string, int>& obsolete_parts);

    private:
        std::list<int>          m_ordered_design_ids;
        std::list<std::string>  m_ordered_hulls;

        // A counter to track the number of times a player has (un)obsoleted a part,
        // hull or design.  This allows the ui to determine if a design was obsoleted
        // before/after its hull/parts.
        int m_obsolete_ui_event_count = 1;

        // An index from the id to the obsolescence state and the location in the
        // m_ordered_design_ids list.
        // For the state information ((false, ui_event), (true, ui_event), none)
        // correspond to (not obsolete, obsolete and defer to parts and hull obsolescence)
        boost::unordered_map<int,
                             std::pair<boost::optional<std::pair<bool, int>>,
                                       std::list<int>::const_iterator>> m_id_to_obsolete_and_loc;

        // An index from the hull name to the obsolescence state and the location in m_ordered_hull_ids list.
        boost::unordered_map<std::string,
                             std::pair<std::pair<bool, int>,
                                       std::list<std::string>::const_iterator>> m_hull_to_obsolete_and_loc;

        // A map from obsolete part name to the UI event count that changed it.
        std::unordered_map<std::string, int> m_obsolete_parts;
    };

    class SavedDesignsManager : public ShipDesignManager::Designs {
    public:
        SavedDesignsManager() = default;

        const std::list<boost::uuids::uuid>& OrderedDesignUUIDs() const;
        std::vector<int> OrderedIDs() const override;

        void StartParsingDesignsFromFileSystem(bool is_new_game);
        void CheckPendingDesigns() const;

        const ShipDesign* GetDesign(boost::uuids::uuid uuid) const;

        void SaveManifest();

        std::list<boost::uuids::uuid>::const_iterator InsertBefore(
            const ShipDesign& design, std::list<boost::uuids::uuid>::const_iterator next);
        bool MoveBefore(boost::uuids::uuid moved_uuid, boost::uuids::uuid next_uuid);
        void Erase(boost::uuids::uuid erased_uuid);

    private:
        /** Save the design with the original filename or throw out_of_range. */
        void SaveDesign(boost::uuids::uuid uuid);

        /** SaveDesignConst allows CheckPendingDesigns to correct the designs
            in the saved directory.*/
        void SaveDesignConst(boost::uuids::uuid uuid) const;

        /** A const version of SaveManifest to allow CheckPendingDesigns to
            correct and save the loaded designs. */
        void SaveManifestConst() const;

        /** Future ship design type being parsed by parser.  mutable so that it can
        be assigned to m_saved_designs when completed.*/
        mutable boost::optional<std::future<PredefinedShipDesignManager::ParsedShipDesignsType>>
            m_pending_designs = boost::none;

        mutable std::list<boost::uuids::uuid> m_ordered_uuids;
        /// Saved designs with filename
        mutable std::unordered_map<boost::uuids::uuid,
                                   std::pair<std::unique_ptr<ShipDesign>,
                                             boost::filesystem::path>,
                                   boost::hash<boost::uuids::uuid>> m_saved_designs;

        mutable bool m_is_new_game = false;
    };


    //////////////////////////////////////////////////
    // Common Reused Actions                        //
    //////////////////////////////////////////////////
    DisplayedShipDesignManager& GetDisplayedDesignsManager() {
        auto designs = dynamic_cast<DisplayedShipDesignManager*>(
            ClientUI::GetClientUI()->GetShipDesignManager()->DisplayedDesigns());
        return *designs;
    }

    SavedDesignsManager& GetSavedDesignsManager() {
        auto designs = dynamic_cast<SavedDesignsManager*>(
            ClientUI::GetClientUI()->GetShipDesignManager()->SavedDesigns());
        return *designs;
    }

    bool CanAddDesignToDisplayedDesigns(const ShipDesign* design) {
        if (!design)
            return false;

        const ScriptingContext& context = GGHumanClientApp::GetApp()->GetContext();
        const auto is_same_design = [design, &context](const int id) {
            auto const current_design = context.ContextUniverse().GetShipDesign(id);
            return current_design && *current_design == *design;
        };

        const auto& current_ids = GetDisplayedDesignsManager().AllOrderedIDs();
        return std::none_of(current_ids.begin(), current_ids.end(), is_same_design);
    }

    /** Add \p design to the \p is_front of \p empire_id's list of current designs. */
    void AddSavedDesignToDisplayedDesigns(boost::uuids::uuid uuid, int empire_id, bool is_front = true) {
        auto* app = GGHumanClientApp::GetApp();
        ScriptingContext& context = app->GetContext();

        const auto empire = std::as_const(context).GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "AddSavedDesignsToDisplayedDesigns HumanClient Does Not Control an Empire";
            return;
        }

        const auto design = GetSavedDesignsManager().GetDesign(uuid);
        if (!design) {
            ErrorLogger() << "AddSavedDesignsToDisplayedDesigns missing expected uuid " << uuid;
            return;
        }

        // Is it the same as an existing design
        if (!CanAddDesignToDisplayedDesigns(design)) {
            DebugLogger() << "AddSavedDesignsToDisplayedDesigns saved design already present: "
                          << design->Name();
            return;
        }

        TraceLogger() << "Add saved design " << design->Name() << " to current designs.";

        // Give it a new UUID so that the empire design is distinct.
        auto new_current_design{*design};
        new_current_design.SetUUID(boost::uuids::random_generator()());

        auto order = app->Orders().IssueOrder<ShipDesignOrder>(context, empire_id, new_current_design);

        auto& current_manager = GetDisplayedDesignsManager();
        const auto& all_ids = current_manager.AllOrderedIDs();
        const int before_id = (all_ids.empty() || !is_front) ? INVALID_OBJECT_ID : all_ids.front();
        current_manager.InsertBefore(order ? order->DesignID() : INVALID_DESIGN_ID, before_id);
    }

    /** Set whether a currently known design is obsolete or not. Not obsolete
      * means that it is known to the empire (on the server) and will appear in
      * the production list.  Obsolete means that it will only appear in the
      * list of obsolete designs. */
    void SetObsoleteInDisplayedDesigns(const int design_id, bool obsolete) {
        auto& manager = GetDisplayedDesignsManager();
        if (!manager.IsKnown(design_id)) {
            WarnLogger() << "Attempted to toggle obsolete state of design id "
                         << design_id << " which is unknown to the empire";
            return;
        }
        manager.SetObsolete(design_id, obsolete);

        auto* app = GGHumanClientApp::GetApp();
        const auto empire_id = app->EmpireID();
        ScriptingContext& context = app->GetContext();

        if (obsolete) {
            // make empire forget on the server
            app->Orders().IssueOrder<ShipDesignOrder>(context, empire_id, design_id, true);
        } else {
            const auto design = std::as_const(context).ContextUniverse().GetShipDesign(design_id);
            if (!design) {
                ErrorLogger() << "Attempted to toggle obsolete state of design id "
                              << design_id << " which is unknown to the server";
                return;
            }

            //make known to empire on server
            app->Orders().IssueOrder<ShipDesignOrder>(context, empire_id, design_id);
        }
    }

    /** Remove design from DisplayedDesigns. */
    void DeleteFromDisplayedDesigns(const int design_id) {
        auto& manager = GetDisplayedDesignsManager();
        auto* app = GGHumanClientApp::GetApp();
        ScriptingContext& context = app->GetContext();

        const auto maybe_obsolete = manager.IsObsolete(design_id, context); // purpose of this obsolescence check is unclear... author didn't comment
        if (maybe_obsolete && !*maybe_obsolete)
            app->Orders().IssueOrder<ShipDesignOrder>(  // erase design id order : empire should forget this design
                context, app->EmpireID(), design_id, true);
        manager.Remove(design_id);
    }


    //////////////////////////////////////////////////
    // SavedDesignsManager implementations          
    //////////////////////////////////////////////////
    const std::list<boost::uuids::uuid>& SavedDesignsManager::OrderedDesignUUIDs() const {
        CheckPendingDesigns();
        return m_ordered_uuids;
    }

    std::vector<int> SavedDesignsManager::OrderedIDs() const {
        CheckPendingDesigns();
        std::vector<int> retval;
        for (const auto uuid : m_ordered_uuids) {
            const auto it = m_saved_designs.find(uuid);
            if (it == m_saved_designs.end())
                continue;
            retval.push_back(it->second.first->ID());
        }
        return retval;
    }

    void SavedDesignsManager::StartParsingDesignsFromFileSystem(bool is_new_game) {
        auto saved_designs_dir = SavedDesignsDir();
        if (!exists(saved_designs_dir))
            return;

        m_is_new_game = is_new_game;

        TraceLogger() << "Start parsing saved designs from directory";
        m_pending_designs = std::async(std::launch::async, parse::ship_designs, saved_designs_dir);
    }

    void SavedDesignsManager::CheckPendingDesigns() const {
        if (!m_pending_designs)
            return;

        TraceLogger() << "Waiting for pending saved designs";

        // Only print waiting message if not immediately ready
        while (m_pending_designs->wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
            DebugLogger() << "Waiting for saved ShipDesigns to parse.";
        }

        bool inconsistent;
        std::vector<boost::uuids::uuid> ordering;

        try {
            TraceLogger() << "Receive parsed saved designs.";

            auto parsed_designs = m_pending_designs->get();

            std::tie(inconsistent, m_saved_designs, ordering) =
                LoadShipDesignsAndManifestOrderFromParseResults(parsed_designs);

        } catch (const std::exception& e) {
            ErrorLogger() << "Failed parsing designs: error: " << e.what();
            throw;
        }

        m_pending_designs = boost::none;

        m_ordered_uuids = std::list<boost::uuids::uuid>(ordering.begin(), ordering.end());

        // Write any corrected ordering back to disk.
        if (inconsistent) {
            WarnLogger() << "Writing corrected ship designs back to saved designs.";
            SaveManifestConst();

            // Correct file name extension
            // Nothing fancy, just convert '.txt' to '.txt.focs.txt' which the scripting system is
            // happy with and there is minimal risk of botching the file name;
            for (auto& uuid_and_design_and_path: m_saved_designs) {
                auto& path = uuid_and_design_and_path.second.second;
                auto extension = path.extension();
                bool extension_wrong = extension.empty() || extension != ".txt";

                auto stem_extension = path.stem().extension();
                bool stem_wrong = extension_wrong || stem_extension.empty() || stem_extension != ".focs";

                if (extension_wrong || stem_wrong)
                    path += DESIGN_FILENAME_EXTENSION.data();
            }

            for (const auto uuid: m_ordered_uuids)
                SaveDesignConst(uuid);
        }

        if (!m_is_new_game)
            return;

        m_is_new_game = false;

        // If requested on the first turn copy all of the saved designs to the client empire.
        if (GetOptionsDB().Get<bool>("resource.shipdesign.saved.enabled")) {
            const auto empire_id = GGHumanClientApp::GetApp()->EmpireID();
            TraceLogger() << "Adding saved designs to empire.";
            // assume the saved designs are preferred by the user: add them to the front.
            // note that this also ensures correct ordering.
            for (const auto uuid : m_ordered_uuids)
                AddSavedDesignToDisplayedDesigns(uuid, empire_id, true);
        }
    }

    const ShipDesign* SavedDesignsManager::GetDesign(boost::uuids::uuid uuid) const {
        CheckPendingDesigns();
        const auto it = m_saved_designs.find(uuid);
        if (it == m_saved_designs.end())
            return nullptr;
        return it->second.first.get();
    }

    void SavedDesignsManager::SaveManifest()
    { SaveManifestConst(); }

    void SavedDesignsManager::SaveManifestConst() const {
        CheckPendingDesigns();
        boost::filesystem::path designs_dir_path = GetDesignsDir();

        const auto file_name{std::string{DESIGN_MANIFEST_PREFIX}.append(DESIGN_FILENAME_EXTENSION)};
        auto file = boost::filesystem::absolute(PathToString(designs_dir_path / file_name));

        std::stringstream ss;
        ss << DESIGN_MANIFEST_PREFIX << "\n";
        for (const auto uuid : m_ordered_uuids)
            ss << "    uuid = \"" << uuid << "\"\n";
        WriteToFile(file, ss.str());
    }

    std::list<boost::uuids::uuid>::const_iterator SavedDesignsManager::InsertBefore(
        const ShipDesign& design,
        std::list<boost::uuids::uuid>::const_iterator next)
    {
        if (design.UUID().is_nil()) {
            ErrorLogger() << "Ship design has a nil UUID for " << design.Name() << ". Not saving.";
            return next;
        }

        CheckPendingDesigns();
        if (m_saved_designs.contains(design.UUID())) {
            // UUID already exists so this is a move.  Remove the old UUID location
            const auto existing_it = std::find(m_ordered_uuids.begin(), m_ordered_uuids.end(), design.UUID());
            if (existing_it != m_ordered_uuids.end())
                m_ordered_uuids.erase(existing_it);

        } else {
            // Add the new saved design.
            m_saved_designs.emplace(design.UUID(),
                                    std::pair{std::make_unique<ShipDesign>(design),
                                              CreateSavePathForDesign(design)});
            SaveDesign(design.UUID());
        }

        // Insert in the list.
        const auto retval = m_ordered_uuids.insert(next, design.UUID());
        SaveManifest();
        return retval;
    }

    bool SavedDesignsManager::MoveBefore(boost::uuids::uuid moved_uuid, boost::uuids::uuid next_uuid) {
        if (moved_uuid == next_uuid)
            return false;

        CheckPendingDesigns();
        if (!m_saved_designs.contains(moved_uuid)) {
            ErrorLogger() << "Unable to move saved design because moved design is missing.";
            return false;
        }

        if (!next_uuid.is_nil() && !m_saved_designs.contains(next_uuid)) {
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

    void SavedDesignsManager::Erase(boost::uuids::uuid erased_uuid) {
        CheckPendingDesigns();
        const auto& saved_design_it = m_saved_designs.find(erased_uuid);
        if (saved_design_it != m_saved_designs.end()) {
            const auto& file = saved_design_it->second.second;
            boost::filesystem::remove(file);
            m_saved_designs.erase(erased_uuid);
        }

        const auto uuid_it = std::find(m_ordered_uuids.begin(), m_ordered_uuids.end(), erased_uuid);
        m_ordered_uuids.erase(uuid_it);
    }

    void SavedDesignsManager::SaveDesign(boost::uuids::uuid uuid) { SaveDesignConst(uuid); }

    /** Save the design with the original filename or throw out_of_range..*/
    void SavedDesignsManager::SaveDesignConst(boost::uuids::uuid uuid) const {
        CheckPendingDesigns();
        const auto& design_and_path = m_saved_designs.at(uuid);

        WriteToFile(design_and_path.second, design_and_path.first->Dump());
    }


    //////////////////////////////////////////////////
    // CurrentShipDesignsManager implementations
    //////////////////////////////////////////////////
    std::vector<int> DisplayedShipDesignManager::OrderedIDs() const {
        // Make sure that saved designs are included.
        // Only OrderedIDs is part of the Designs base class and
        // accessible outside this file.
        GetSavedDesignsManager().CheckPendingDesigns();
        const ScriptingContext& context = GGHumanClientApp::GetApp()->GetContext();

        // Remove all obsolete ids from the list
        std::vector<int> retval;
        std::copy_if(m_ordered_design_ids.begin(), m_ordered_design_ids.end(), std::back_inserter(retval),
                     [this, &context](const int id) {
                         const auto maybe_obsolete = IsObsolete(id, context);
                         const auto known_and_not_obsolete = maybe_obsolete ? !*maybe_obsolete : false;
                         return known_and_not_obsolete;
                     });
        return retval;
    }

    std::vector<int> DisplayedShipDesignManager::AllOrderedIDs() const
    { return std::vector<int>(m_ordered_design_ids.begin(), m_ordered_design_ids.end()); }

    std::vector<std::string> DisplayedShipDesignManager::OrderedHulls() const
    { return std::vector<std::string>(m_ordered_hulls.begin(), m_ordered_hulls.end()); }

    template <typename T>
    void DisplayedShipDesignManager::InsertOrderedIDs(const T& new_order) {
        for (const auto id : new_order)
            InsertBefore(id, INVALID_DESIGN_ID);
    }

    void DisplayedShipDesignManager::InsertBefore(const int id, const int next_id) {
        if (id == INVALID_DESIGN_ID) {
            ErrorLogger() << "Ship design is invalid";
            return;
        }

        if (id == next_id) {
            ErrorLogger() << "Ship design " << id << " is the same as next_id";
            return;
        }

        // if id already exists so this is a move.  Remove the old location
        if (IsKnown(id)) {
            WarnLogger() << "DisplayedShipDesignManager::InsertBefore id = " << id
                         << " already inserted.  Removing and reinserting at new location";
            Remove(id);
        }

        // Insert in the list, either before next_id or at the end of the list.
        const auto next_it = m_id_to_obsolete_and_loc.find(next_id);
        bool is_valid_next_id = (next_id != INVALID_DESIGN_ID
                                 && next_it != m_id_to_obsolete_and_loc.end());
        const auto insert_before_it = (is_valid_next_id ? next_it->second.second :m_ordered_design_ids.end());
        const auto inserted_it = m_ordered_design_ids.insert(insert_before_it, id);

        m_id_to_obsolete_and_loc[id] = std::pair(boost::none, inserted_it);
    }

    bool DisplayedShipDesignManager::MoveBefore(const int moved_id, const int next_id) {
        if (moved_id == next_id)
            return false;

        auto existing_it = m_id_to_obsolete_and_loc.find(moved_id);
        if (existing_it == m_id_to_obsolete_and_loc.end()) {
            ErrorLogger() << "Unable to move design because moved design is missing.";
            return false;
        }

        m_ordered_design_ids.erase(existing_it->second.second);

        const auto next_it = m_id_to_obsolete_and_loc.find(next_id);
        bool is_valid_next_id = (next_id != INVALID_DESIGN_ID
                                 && next_it != m_id_to_obsolete_and_loc.end());
        const auto insert_before_it = (is_valid_next_id ? next_it->second.second :m_ordered_design_ids.end());
        const auto inserted_it = m_ordered_design_ids.insert(insert_before_it, moved_id);

        existing_it->second.second = inserted_it;
        return true;
    }

    void DisplayedShipDesignManager::Remove(const int id) {
        auto it = m_id_to_obsolete_and_loc.find(id);
        if (it == m_id_to_obsolete_and_loc.end())
            return;

        m_ordered_design_ids.erase(it->second.second);
        m_id_to_obsolete_and_loc.erase(it);
    }

    void DisplayedShipDesignManager::InsertHullBefore(const std::string& hull, const std::string& next_hull) {
        if (hull.empty()) {
            ErrorLogger() << "DisplayedShipDesignManager::InsertHullBefore: Hull name is empty()";
            return;
        }
        if (hull == next_hull)
            return;

        // if hull already exists, this is a move.  Remove the old location
        auto existing_it = m_hull_to_obsolete_and_loc.find(hull);
        if (existing_it != m_hull_to_obsolete_and_loc.end()) {
            m_ordered_hulls.erase(existing_it->second.second);
            m_hull_to_obsolete_and_loc.erase(existing_it);
        }

        // Insert in the list, either before next_id or at the end of the list.
        const auto next_it = (!next_hull.empty()
                              ? m_hull_to_obsolete_and_loc.find(next_hull)
                              : m_hull_to_obsolete_and_loc.end());
        bool is_valid_next_hull = (!next_hull.empty()
                                   && next_it != m_hull_to_obsolete_and_loc.end());
        const auto insert_before_it = (is_valid_next_hull
                                       ? next_it->second.second
                                       : m_ordered_hulls.end());
        const auto inserted_it = m_ordered_hulls.insert(insert_before_it, hull);

        m_hull_to_obsolete_and_loc[hull] = {std::pair{false, NextUIObsoleteEvent()}, inserted_it};
    }

    [[nodiscard]] bool FlexibleContains(const auto& container, const auto& val) {
        if constexpr (requires { container.contains(val); })
            return container.contains(val);
        else if constexpr (requires { container.count(val); })
            return container.count(val) > 0;
        else if constexpr (requires { container.find(val) != container.end(); })
            return container.find(val) != container.end();
        else
            return std::any_of(container.begin(), container.end(), [&val](const auto& cv) { return val == cv; });
    }

    [[nodiscard]] bool DisplayedShipDesignManager::IsKnown(const int id) const
    { return FlexibleContains(m_id_to_obsolete_and_loc, id); };

    boost::optional<bool> DisplayedShipDesignManager::IsObsolete(
        const int id, const ScriptingContext& context) const
    {
        // A non boost::none value for a specific design overrides the hull and part values
        auto it_id = m_id_to_obsolete_and_loc.find(id);

        // Unknown design
        if (it_id == m_id_to_obsolete_and_loc.end())
            return boost::none;

        const auto design = context.ContextUniverse().GetShipDesign(id);
        if (!design) {
            ErrorLogger() << "DisplayedShipDesignManager::IsObsolete design id "
                          << id << " is unknown to the server";
            return boost::none;
        }

        // Check the UI (un)obsoleting events for design, hull and parts.  Events
        // with a later/higher/more recent stamp supercede older instructions.
        // If there are no instructions from the user the design is not obsolete.
        int latest_obsolete_event = -1;
        int latest_unobsolete_event = 0;

        if (const auto& maybe_obsolete_design = it_id->second.first) {
            if (maybe_obsolete_design->first)
                latest_obsolete_event = maybe_obsolete_design->second;
            else
                latest_unobsolete_event = maybe_obsolete_design->second;
        }

        if (const auto maybe_hull_obsolete = IsHullObsolete(design->Hull()))
            latest_obsolete_event = std::max(latest_obsolete_event, *maybe_hull_obsolete);

        for (const auto& part: design->Parts()) {
            if (const auto maybe_part_obsolete = IsPartObsolete(part))
                latest_obsolete_event = std::max(latest_obsolete_event, *maybe_part_obsolete);
        }

        // Default to false if the player has not obsoleted the design, its hull or its parts
        return latest_obsolete_event > latest_unobsolete_event;
    }

    boost::optional<int> DisplayedShipDesignManager::IsHullObsolete(const std::string& hull) const {
        auto it_hull = m_hull_to_obsolete_and_loc.find(hull);
        if (it_hull == m_hull_to_obsolete_and_loc.end())
            return boost::none;

        return (it_hull->second.first.first ?  boost::optional<int>(it_hull->second.first.second) : boost::none);
    }

    boost::optional<int> DisplayedShipDesignManager::IsPartObsolete(const std::string& part) const {
        auto it_part = m_obsolete_parts.find(part);
        return (it_part != m_obsolete_parts.end()) ?  boost::optional<int>(it_part->second) : boost::none ;
    }

    int DisplayedShipDesignManager::NextUIObsoleteEvent() {
        ++m_obsolete_ui_event_count;
        if (m_obsolete_ui_event_count < 0)
            // Report but don't fix so the error appears more than once in the log
            ErrorLogger() << "The counter of UI obsoletion events has wrapped to negative values.";
        return m_obsolete_ui_event_count > 0 ? m_obsolete_ui_event_count : 0;
    }

    void DisplayedShipDesignManager::SetObsolete(const int id, const bool obsolete) {
        auto it = m_id_to_obsolete_and_loc.find(id);
        if (it != m_id_to_obsolete_and_loc.end())
            it->second.first = std::pair{obsolete, NextUIObsoleteEvent()};
    }

    void DisplayedShipDesignManager::SetHullObsolete(const std::string& name, const bool obsolete) {
        auto it = m_hull_to_obsolete_and_loc.find(name);
        if (it == m_hull_to_obsolete_and_loc.end()) {
            // All hulls should be known so tack it on the end and try again.
            ErrorLogger() << "Hull " << name << " is missing in DisplayedShipDesignManager.  Adding...";
            InsertHullBefore(name);
            return SetHullObsolete(name, obsolete);
        }
        it->second.first = {obsolete, NextUIObsoleteEvent()};
    }

    void DisplayedShipDesignManager::SetPartObsolete(const std::string& name, const bool obsolete) {
        if (obsolete)
            m_obsolete_parts[name] = NextUIObsoleteEvent();
        else
            m_obsolete_parts.erase(name);
    }

    void DisplayedShipDesignManager::Load(
        const int obsolete_ui_event_count,
        const std::vector<std::pair<int, boost::optional<std::pair<bool, int>>>>& design_ids_and_obsoletes,
        const std::vector<std::pair<std::string, std::pair<bool, int>>>& hulls_and_obsoletes,
        const std::unordered_map<std::string, int>& obsolete_parts)
    {
        m_obsolete_ui_event_count = obsolete_ui_event_count;
        if (m_obsolete_ui_event_count < 0)
            ErrorLogger() << "DisplayedShipDesignManager::Load obsolete_ui_event_count = "
                          << obsolete_ui_event_count << " < 0 ";

        // Clear and load the ship design ids
        m_id_to_obsolete_and_loc.clear();
        m_ordered_design_ids.clear();
        for (const auto& [id, obsolete] : design_ids_and_obsoletes) {
            if (FlexibleContains(m_id_to_obsolete_and_loc, id)) {
                ErrorLogger() << "DisplayedShipDesignManager::Load duplicate design id = " << id;
                continue;
            }
            if (obsolete && obsolete->first &&
                (obsolete->second < 0 || obsolete->second >= m_obsolete_ui_event_count))
            {
                ErrorLogger() << "DisplayedShipDesignManager::Load design with id = " << id
                              << " has an obsolete_ui_event_count = " << obsolete->second
                              << " which does not satisfy 0 < obsolete_ui_event_count < m_obsolete_ui_event_count = "
                              << m_obsolete_ui_event_count;
            }
            m_ordered_design_ids.push_back(id);
            m_id_to_obsolete_and_loc.emplace(std::piecewise_construct,
                                             std::make_tuple(id),
                                             std::make_tuple(obsolete, --m_ordered_design_ids.end()));
        }

        // Clear and load the ship hulls
        m_hull_to_obsolete_and_loc.clear();
        m_ordered_hulls.clear();
        for (const auto& [name, obsolete] : hulls_and_obsoletes) {
            if (FlexibleContains(m_hull_to_obsolete_and_loc, name)) {
                ErrorLogger() << "DisplayedShipDesignManager::Load duplicate hull name = " << name;
                continue;
            }
            if (obsolete.first && (obsolete.second < 0 || obsolete.second >= m_obsolete_ui_event_count))
                ErrorLogger() << "DisplayedShipDesignManager::Load hull \"" << name
                              << "\" has an obsolete_ui_event_count = " << obsolete.second
                              << " which does not satisfy 0 < obsolete_ui_event_count < m_obsolete_ui_event_count = "
                              << m_obsolete_ui_event_count;
            m_ordered_hulls.push_back(name);
            m_hull_to_obsolete_and_loc.emplace(std::piecewise_construct,
                                               std::make_tuple(name),
                                               std::make_tuple(obsolete, --m_ordered_hulls.end()));
        }

        // Clear and load the ship parts
        m_obsolete_parts = obsolete_parts;
        for (const auto& [name, count] : m_obsolete_parts) {
            if (count < 0 || count >= m_obsolete_ui_event_count)
                ErrorLogger() << "DisplayedShipDesignManager::Load part \"" << name
                              << "\" has an obsolete_ui_event_count = " << count
                              << " which does not satisfy 0 < obsolete_ui_event_count < m_obsolete_ui_event_count = "
                              << m_obsolete_ui_event_count;

        }
    }

    void DisplayedShipDesignManager::Save(
        int& obsolete_ui_event_count,
        std::vector<std::pair<int, boost::optional<std::pair<bool, int>>>>& design_ids_and_obsoletes,
        std::vector<std::pair<std::string, std::pair<bool, int>>>& hulls_and_obsoletes,
        std::unordered_map<std::string, int>& obsolete_parts)
    {
        obsolete_ui_event_count = m_obsolete_ui_event_count;

        design_ids_and_obsoletes.clear();
        for (const auto id : m_ordered_design_ids) {
            try {
                design_ids_and_obsoletes.emplace_back(id, m_id_to_obsolete_and_loc.at(id).first);
            } catch (const std::out_of_range&) {
                ErrorLogger() << "DisplayedShipDesignManager::Save missing id = " << id;
                continue;
            }
        }

        hulls_and_obsoletes.clear();
        for (const auto& name : m_ordered_hulls) {
            try {
               hulls_and_obsoletes.emplace_back(name, m_hull_to_obsolete_and_loc.at(name).first);
            } catch (const std::out_of_range&) {
                ErrorLogger() << "DisplayedShipDesignManager::Save missing hull = " << name;
                continue;
            }
        }

        obsolete_parts = m_obsolete_parts;
    }

    //////////////////////////////////////////////////
    //  AvailabilityManager                         //
    //////////////////////////////////////////////////
    /** A class to allow the storage of the state of a GUI availabilty filter
        and the querying of that state WRT a ship design. */
    class AvailabilityManager {
    public:
        // DisplayedAvailabilies is indexed by Availability::Enum
        using DisplayedAvailabilies = std::tuple<bool, bool, bool>;

        [[nodiscard]] constexpr AvailabilityManager(bool obsolete, bool available, bool unavailable) noexcept :
            m_availabilities{obsolete, available, unavailable}
        {}

        [[nodiscard]] constexpr auto GetAvailabilities() const noexcept { return m_availabilities; };
        [[nodiscard]] constexpr bool GetAvailability(const Availability::Enum type) const noexcept;
        constexpr void SetAvailability(const Availability::Enum type, const bool state) noexcept;
        constexpr void ToggleAvailability(const Availability::Enum type) noexcept;

        /** Given the GUI's displayed availabilities as stored in this
            AvailabilityManager, return the displayed state of the \p design.
            Return none if the \p design should not be displayed. */
        boost::optional<DisplayedAvailabilies> DisplayedDesignAvailability(const ShipDesign& design) const;
        /** Given the GUI's displayed availabilities as stored in this
            AvailabilityManager, return the displayed state of the hull \p
            name. Return none if the hull should not be displayed. */
        boost::optional<DisplayedAvailabilies> DisplayedHullAvailability(const std::string& name) const;
        /** Given the GUI's displayed availabilities as stored in this
            AvailabilityManager, return the displayed state of the part \p
            name. Return none if the part should not be displayed. */
        boost::optional<DisplayedAvailabilies> DisplayedPartAvailability(const std::string& name) const;

    private:
        /** Given the GUI's displayed availabilities as stored in this
            AvailabilityManager and that the X is \p available and \p obsolete,
            return the displayed state of the X. Return none if the X should
            not be displayed. */
        boost::optional<DisplayedAvailabilies> DisplayedXAvailability(bool available, bool obsolete) const;

        // A tuple of the toogle state of the 3-tuple of coupled
        // availability filters in the GUI:
        // Obsolete, Available and Unavailable
        DisplayedAvailabilies m_availabilities = {false, false, false};
    };

    [[nodiscard]] constexpr bool AvailabilityManager::GetAvailability(const Availability::Enum type) const noexcept {
        switch (type) {
        case Availability::Obsolete:
            return std::get<Availability::Obsolete>(m_availabilities);
        case Availability::Available:
            return std::get<Availability::Available>(m_availabilities);
        case Availability::Future:
            return std::get<Availability::Future>(m_availabilities);
        }
        return std::get<Availability::Future>(m_availabilities);
    }

    constexpr void AvailabilityManager::SetAvailability(const Availability::Enum type, const bool state) noexcept {
        switch (type) {
        case Availability::Obsolete:
            std::get<Availability::Obsolete>(m_availabilities) = state;
            break;
        case Availability::Available:
            std::get<Availability::Available>(m_availabilities) = state;
            break;
        case Availability::Future:
            std::get<Availability::Future>(m_availabilities) = state;
            break;
        }
    }

    constexpr void AvailabilityManager::ToggleAvailability(const Availability::Enum type) noexcept
    { SetAvailability(type, !GetAvailability(type)); }

    [[nodiscard]] boost::optional<AvailabilityManager::DisplayedAvailabilies>
    AvailabilityManager::DisplayedDesignAvailability(const ShipDesign& design) const {
        const auto* app = GGHumanClientApp::GetApp();
        const ScriptingContext& context = app->GetContext();
        auto empire = context.GetEmpire(app->EmpireID());
        bool available = empire ? empire->ShipDesignAvailable(design) : true;

        const auto& manager = GetDisplayedDesignsManager();
        const auto maybe_obsolete = manager.IsObsolete(design.ID(), context);
        bool is_obsolete = maybe_obsolete && *maybe_obsolete;

        return DisplayedXAvailability(available, is_obsolete);
    }

    [[nodiscard]] boost::optional<AvailabilityManager::DisplayedAvailabilies>
    AvailabilityManager::DisplayedHullAvailability(const std::string& id) const {
        int empire_id = GGHumanClientApp::GetApp()->EmpireID();
        const Empire* empire = GetEmpire(empire_id);  // may be nullptr
        bool available = empire ? empire->ShipHullAvailable(id) : true;

        const auto& manager = GetDisplayedDesignsManager();
        const auto obsolete = bool(manager.IsHullObsolete(id));

        return DisplayedXAvailability(available, obsolete);
    }

    [[nodiscard]] boost::optional<AvailabilityManager::DisplayedAvailabilies>
    AvailabilityManager::DisplayedPartAvailability(const std::string& id) const {
        int empire_id = GGHumanClientApp::GetApp()->EmpireID();
        const Empire* empire = GetEmpire(empire_id);  // may be nullptr
        bool available = empire ? empire->ShipPartAvailable(id) : true;

        const auto& manager = GetDisplayedDesignsManager();
        const auto obsolete = bool(manager.IsPartObsolete(id));

        return DisplayedXAvailability(available, obsolete);
    }

    [[nodiscard]] boost::optional<AvailabilityManager::DisplayedAvailabilies>
    AvailabilityManager::DisplayedXAvailability(bool available, bool obsolete) const {
        const auto& [showing_obsolete, showing_available, showing_future] = m_availabilities;

        auto show = (
            (showing_obsolete && obsolete && showing_available && available)
            || (showing_obsolete && obsolete && showing_future && !available)
            || (showing_obsolete && obsolete && !showing_available && !showing_future)
            || (showing_available && available && !obsolete)
            || (showing_future && !available && !obsolete));

        if (!show)
            return boost::none;

        return std::make_tuple(showing_obsolete && obsolete,
                               showing_available && available,
                               showing_future && !available);
    }
}

//////////////////////////////////////////////////
// ShipDesignManager                            //
//////////////////////////////////////////////////
ShipDesignManager::ShipDesignManager() :
    m_displayed_designs(std::make_unique<DisplayedShipDesignManager>()),
    m_saved_designs(std::make_unique<SavedDesignsManager>())
{}

ShipDesignManager::~ShipDesignManager() = default;

void ShipDesignManager::StartGame(int empire_id, bool is_new_game) {
    auto* app = GGHumanClientApp::GetApp();
    ScriptingContext& context = app->GetContext();
    const auto empire = std::as_const(context).GetEmpire(empire_id);
    if (!empire) {
        ErrorLogger() << "Unable to initialize ShipDesignManager because empire id, " << empire_id << ", is invalid";
        return;
    }

    DebugLogger() << "ShipDesignManager initializing. New game " << is_new_game;

    m_displayed_designs = std::make_unique<DisplayedShipDesignManager>();
    auto* const displayed_designs = dynamic_cast<DisplayedShipDesignManager*>(m_displayed_designs.get());

    m_saved_designs = std::make_unique<SavedDesignsManager>();
    auto saved_designs = dynamic_cast<SavedDesignsManager*>(m_saved_designs.get());
    saved_designs->StartParsingDesignsFromFileSystem(is_new_game);

    // Only setup saved and current designs for new games
    if (!is_new_game)
        return;

    // Initialize the hull ordering from the ShipHullManager
    for (const auto& [hull_name, ship_hull] : GetShipHullManager()) {
        if (ship_hull && ship_hull->Producible())
            displayed_designs->InsertHullBefore(hull_name);
    }

    // If requested, initialize the current designs to all designs known by the empire
    if (GetOptionsDB().Get<bool>("resource.shipdesign.default.enabled")) {
        // While initializing a new game, before sending info to players, the
        // server should have added the default design ids to an empire's known
        // designs. Loop over these, and add them to "current" designs.
        DebugLogger() << "Add default designs to empire's current designs";
        displayed_designs->InsertOrderedIDs(empire->ShipDesigns());

    } else {
        // Remove the default designs from the empire's current designs.
        // Purpose and logic of this is unclear... author didn't comment upon inquiry, but having this here reportedly fixes some issues...
        DebugLogger() << "Remove default designs from empire";
        for (const auto design_id : empire->ShipDesigns())
            app->Orders().IssueOrder<ShipDesignOrder>(context, empire_id, design_id, true);
    }

    TraceLogger() << "ShipDesignManager initialized";
}

void ShipDesignManager::Save(SaveGameUIData& data) const {
    GetDisplayedDesignsManager().Save(data.obsolete_ui_event_count,
                                      data.ordered_ship_design_ids_and_obsolete,
                                      data.ordered_ship_hull_and_obsolete,
                                      data.obsolete_ship_parts);
}

void ShipDesignManager::Load(const SaveGameUIData& data) {
    GetDisplayedDesignsManager().Load(data.obsolete_ui_event_count,
                                      data.ordered_ship_design_ids_and_obsolete,
                                      data.ordered_ship_hull_and_obsolete,
                                      data.obsolete_ship_parts);
}

ShipDesignManager::Designs* ShipDesignManager::DisplayedDesigns() {
    auto* retval = m_displayed_designs.get();
    if (!retval) {
        ErrorLogger() << "ShipDesignManager m_displayed_designs was not correctly initialized "
                      << "with ShipDesignManager::GameStart().";
        m_displayed_designs = std::make_unique<DisplayedShipDesignManager>();
        return m_displayed_designs.get();
    }
    return retval;
}

ShipDesignManager::Designs* ShipDesignManager::SavedDesigns() {
    auto retval = m_saved_designs.get();
    if (!retval) {
        ErrorLogger() << "ShipDesignManager m_saved_designs was not correctly initialized "
                      << "with ShipDesignManager::GameStart().";
        m_saved_designs = std::make_unique<SavedDesignsManager>();
        return m_saved_designs.get();
    }
    return retval;
}


//////////////////////////////////////////////////
// PartControl                                  //
//////////////////////////////////////////////////
/** UI representation of a ship part.  Displayed in the PartPalette, and can be
  * dragged onto SlotControls to add parts to the design. */
class PartControl : public GG::Control {
public:
    PartControl(const ShipPart* part);
    void CompleteConstruction() override;

    const ShipPart*     Part() const { return m_part; }
    const std::string&  PartName() const { return m_part ? m_part->Name() : EMPTY_STRING; }

    void Render() override;
    void LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void LDoubleClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void SetAvailability(const AvailabilityManager::DisplayedAvailabilies& type);

    mutable boost::signals2::signal<void (const ShipPart*, GG::Flags<GG::ModKey>)> ClickedSignal;
    mutable boost::signals2::signal<void (const ShipPart*, GG::Pt pt)> RightClickedSignal;
    mutable boost::signals2::signal<void (const ShipPart*)> DoubleClickedSignal;

private:
    std::shared_ptr<GG::StaticGraphic> m_icon;
    std::shared_ptr<GG::StaticGraphic> m_background;
    const ShipPart*                    m_part = nullptr;
};

PartControl::PartControl(const ShipPart* part) :
    GG::Control(GG::X0, GG::Y0, SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT, GG::INTERACTIVE),
    m_part(part)
{}

void PartControl::CompleteConstruction() {
    GG::Control::CompleteConstruction();
    if (!m_part)
        return;

    m_background = GG::Wnd::Create<GG::StaticGraphic>(
        PartBackgroundTexture(m_part), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_background->Resize(GG::Pt(SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT));
    m_background->Show();
    AttachChild(m_background);


    // position of part image centred within part control.  control is size of a slot, but the
    // part image is smaller
    GG::X part_left = (Width() - PART_CONTROL_WIDTH) / 2;
    GG::Y part_top = (Height() - PART_CONTROL_HEIGHT) / 2;

    //DebugLogger() << "PartControl::PartControl this: " << this << " part: " << part << " named: " << (part ? part->Name() : "no part");
    m_icon = GG::Wnd::Create<GG::StaticGraphic>(
        ClientUI::PartIcon(m_part->Name()), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_icon->MoveTo(GG::Pt(part_left, part_top));
    m_icon->Resize(GG::Pt(PART_CONTROL_WIDTH, PART_CONTROL_HEIGHT));
    m_icon->Show();
    AttachChild(m_icon);

    SetDragDropDataType(PART_CONTROL_DROP_TYPE_STRING);

    //DebugLogger() << "PartControl::PartControl part name: " << m_part->Name();
    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
        ClientUI::PartIcon(m_part->Name()),
        UserString(m_part->Name()),
        UserString(m_part->Description()) + "\n" + m_part->CapacityDescription()
    ));
}

void PartControl::Render() {}

void PartControl::LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ ClickedSignal(m_part, mod_keys); }

void PartControl::LDoubleClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ DoubleClickedSignal(m_part); }

void PartControl::RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ RightClickedSignal(m_part, pt); }

void PartControl::SetAvailability(const AvailabilityManager::DisplayedAvailabilies& type) {
    auto disabled = std::get<Availability::Obsolete>(type);
    m_icon->Disable(disabled);
    m_background->Disable(disabled);
}

//////////////////////////////////////////////////
// PartsListBox                                 //
//////////////////////////////////////////////////
/** Arrangement of PartControls that can be dragged onto SlotControls */
class PartsListBox : public CUIListBox {
public:
    class PartsListBoxRow : public CUIListBox::Row {
    public:
        PartsListBoxRow(GG::X w, GG::Y h, const AvailabilityManager& availabilities_state);
        void ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds,
                                 const GG::Wnd* destination) override;
    private:
        const AvailabilityManager& m_availabilities_state;
    };

    explicit PartsListBox(const AvailabilityManager& availabilities_state);

    auto& GetClassesShown() const noexcept { return m_part_classes_shown; }
    auto& AvailabilityState() const noexcept { return m_availabilities_state; }
    bool  GetShowingSuperfluous() const noexcept { return m_show_superfluous_parts; }

    void SizeMove(GG::Pt ul, GG::Pt lr) override;
    void AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds,
                     GG::Flags<GG::ModKey> mod_keys) override;
    void Populate();

    void ShowClass(ShipPartClass part_class, bool refresh_list = true);
    void ShowAllClasses(bool refresh_list = true);
    void HideClass(ShipPartClass part_class, bool refresh_list = true);
    void HideAllClasses(bool refresh_list = true);
    void ShowSuperfluousParts(bool refresh_list = true);
    void HideSuperfluousParts(bool refresh_list = true);

    mutable boost::signals2::signal<void (const ShipPart*, GG::Flags<GG::ModKey>)>  ShipPartClickedSignal;
    mutable boost::signals2::signal<void (const ShipPart*)>                         ShipPartDoubleClickedSignal;
    mutable boost::signals2::signal<void (const ShipPart*, GG::Pt pt)>              ShipPartRightClickedSignal;
    mutable boost::signals2::signal<void (const std::string&)>                      ClearPartSignal;

protected:
    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) const override;

private:
    PartGroupsType GroupAvailableDisplayableParts(const Empire* empire) const;
    void CullSuperfluousParts(std::vector<const ShipPart*>& this_group,
                              ShipPartClass part_class, int empire_id, int loc_id) const;

    std::set<ShipPartClass>     m_part_classes_shown;   // which part classes should be shown
    bool                        m_show_superfluous_parts = true;
    int                         m_previous_num_columns = -1;
    const AvailabilityManager&  m_availabilities_state;
};

PartsListBox::PartsListBoxRow::PartsListBoxRow(GG::X w, GG::Y h, const AvailabilityManager& availabilities_state) :
    CUIListBox::Row(w, h),
    m_availabilities_state(availabilities_state)
{}

void PartsListBox::PartsListBoxRow::ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination) {
    if (wnds.empty())
        return;
    // should only be one wnd in list because PartControls doesn't allow selection, so dragging is
    // only one-at-a-time
    auto control = dynamic_cast<GG::Control*>(wnds.front());
    if (!control || empty())
        return;

    // find control in row
    unsigned int ii = 0;
    for (; ii < size(); ++ii) {
        if (at(ii) != control)
            continue;
    }

    if (ii == size())
        return;

    RemoveCell(ii);  // Wnd that accepts drop takes ownership of dragged-away control

    auto part_control = dynamic_cast<PartControl*>(control);
    if (!part_control)
        return;

    const auto part = part_control->Part();
    if (!part)
        return;

    auto new_part_control = GG::Wnd::Create<PartControl>(part);
    const auto parent = dynamic_cast<const PartsListBox*>(Parent().get());
    if (parent) {
        new_part_control->ClickedSignal.connect(
            parent->ShipPartClickedSignal);
        new_part_control->DoubleClickedSignal.connect(
            parent->ShipPartDoubleClickedSignal);
        new_part_control->RightClickedSignal.connect(
            parent->ShipPartRightClickedSignal);
    }

    // set availability shown
    auto shown = m_availabilities_state.DisplayedPartAvailability(part->Name());
    if (shown)
        new_part_control->SetAvailability(*shown);

    SetCell(ii, new_part_control);
}

PartsListBox::PartsListBox(const AvailabilityManager& availabilities_state) :
    CUIListBox(),
    m_availabilities_state(availabilities_state)
{
    ManuallyManageColProps();
    NormalizeRowsOnInsert(false);
    SetStyle(GG::LIST_NOSEL);
}

void PartsListBox::SizeMove(GG::Pt ul, GG::Pt lr) {
    GG::Pt old_size = GG::Wnd::Size();

    // maybe later do something interesting with docking
    CUIListBox::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size()) {
        // determine how many columns can fit in the box now...
        const GG::X TOTAL_WIDTH = Size().x - ClientUI::ScrollWidth();
        const int NUM_COLUMNS = std::max(1, TOTAL_WIDTH / (SLOT_CONTROL_WIDTH + GG::X{PAD}));

        if (NUM_COLUMNS != m_previous_num_columns)
            Populate();
    }
}

void PartsListBox::AcceptDrops(GG::Pt, std::vector<std::shared_ptr<GG::Wnd>> wnds,
                               GG::Flags<GG::ModKey> mod_keys)
{
    // Accept parts being discarded from the ship under design

    // If ctrl is pressed then signal all parts of the same type to be cleared.
    if (!(mod_keys & GG::MOD_KEY_CTRL))
        return;

    if (wnds.empty())
        return;

    if (auto* control = dynamic_cast<const PartControl*>(wnds.begin()->get()))
        if (auto* part = control->Part())
            ClearPartSignal(part->Name());
}

PartGroupsType PartsListBox::GroupAvailableDisplayableParts(const Empire* empire) const {
    PartGroupsType part_groups;

    // loop through all possible parts
    for (const auto& [part_name, part] : GetShipPartManager()) {
        if (!part->Producible())
            continue;

        // check whether this part should be shown in list
        ShipPartClass part_class = part->Class();
        if (!m_part_classes_shown.contains(part_class))
            continue;   // part of this class is not requested to be shown

        // Check if part satisfies availability and obsolecense
        auto shown = m_availabilities_state.DisplayedPartAvailability(part_name);
        if (!shown)
            continue;

        for (ShipSlotType slot_type : part->MountableSlotTypes())
            part_groups[{part_class, slot_type}].push_back(part.get());
    }
    return part_groups;
}

namespace {
    // Checks if the Location condition of the check_part totally contains the Location condition of ref_part
    // i,e,, the ref_part condition is met anywhere the check_part condition is
    bool LocationASubsumesLocationB(const Condition::Condition* check_part_loc,
                                    const Condition::Condition* ref_part_loc)
    {
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

    bool PartALocationSubsumesPartB(const ShipPart* check_part, const ShipPart* ref_part) {
        static std::map<std::pair<std::string, std::string>, bool> part_loc_comparison_map;

        auto part_pair = std::pair(check_part->Name(), ref_part->Name());
        auto map_it = part_loc_comparison_map.find(part_pair);
        if (map_it != part_loc_comparison_map.end())
            return map_it->second;

        bool result = true;
        if (check_part->Name() == "SH_MULTISPEC" || ref_part->Name() == "SH_MULTISPEC")
            result = false;

        auto check_part_loc = check_part->Location();
        auto ref_part_loc = ref_part->Location();
        result = result && LocationASubsumesLocationB(check_part_loc, ref_part_loc);
        part_loc_comparison_map[part_pair] = result;
        //if (result && check_part_loc && ref_part_loc) {
        //    DebugLogger() << "Location for partA, " << check_part->Name() << ", subsumes that for partB, " << ref_part->Name();
        //    DebugLogger() << "   ...PartA Location is " << check_part_loc->Description();
        //    DebugLogger() << "   ...PartB Location is " << ref_part_loc->Description();
        //}
        return result;
    }
}

void PartsListBox::CullSuperfluousParts(std::vector<const ShipPart*>& this_group,
                                        ShipPartClass part_class, int empire_id,
                                        int loc_id) const
{
    // This is not merely a check for obsolescence; see PartsListBox::Populate
    // for more info
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

    const ScriptingContext& context = GGHumanClientApp::GetApp()->GetContext();

    for (auto part_it = this_group.begin(); part_it != this_group.end(); ++part_it) {
        const ShipPart* checkPart = *part_it;
        for (const ShipPart* ref_part : this_group) {
            float cap_check = GetMainStat(checkPart);
            float cap_ref = GetMainStat(ref_part);
            if ((cap_check < 0.0f) || (cap_ref < 0.0f))
                continue;  // not intended to handle such cases
            float cap_ratio = cap_ref / std::max(cap_check, 1e-4f) ;  // some part types currently have zero capacity, but need to reject if both are zero
            float cost_check = checkPart->ProductionCost(empire_id, loc_id, context);
            float cost_ref = ref_part->ProductionCost(empire_id, loc_id, context);
            if ((cost_check < 0.0f) || (cost_ref < 0.0f))
                continue;  // not intended to handle such cases
            float cost_ratio = (cost_ref + 1e-4) / (cost_check + 1e-4);  // can accept if somehow they both have cost zero
            float bargain_ratio = cap_ratio / std::max(cost_ratio, 1e-4f);
            float time_ratio = float(std::max(1, ref_part->ProductionTime(empire_id, loc_id, context))) /
                               std::max(1, checkPart->ProductionTime(empire_id, loc_id, context));
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
    const int NUM_COLUMNS = std::max(1, TOTAL_WIDTH / (SLOT_CONTROL_WIDTH + GG::X{PAD}));

    const auto* app = GGHumanClientApp::GetApp();
    const auto& context = app->GetContext();
    const int empire_id = app->EmpireID();
    const auto empire = context.GetEmpire(empire_id);  // may be nullptr

    int cur_col = NUM_COLUMNS;
    std::shared_ptr<PartsListBoxRow> cur_row;
    int num_parts = 0;

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
    PartGroupsType part_groups = GroupAvailableDisplayableParts(empire.get());

    // get empire id and location to use for cost and time comparisons
    int loc_id = INVALID_OBJECT_ID;
    if (empire) {
        auto location = context.ContextObjects().get(empire->CapitalID());
        loc_id = location ? location->ID() : INVALID_OBJECT_ID;
    }

    // if showing parts for a particular empire, cull redundant parts (if enabled)
    if (empire) {
        for (auto& [part_class_slot, parts] : part_groups) {
            ShipPartClass part_class = part_class_slot.first;
            if (!m_show_superfluous_parts)
                CullSuperfluousParts(parts, part_class, empire_id, loc_id);
        }
    }

    // now sort the parts within each group according to main stat, via weak
    // sorting in a multimap also, if a part was in multiple groups due to being
    // compatible with multiple slot types, ensure it is only displayed once
    std::set<const ShipPart*> already_added;
    for (const auto& parts : part_groups | range_values) {
        std::multimap<double, const ShipPart*> sorted_group;
        for (const ShipPart* part : parts) {
            if (already_added.contains(part))
                continue;
            already_added.insert(part);
            sorted_group.emplace(GetMainStat(part), part);
        }

        // take the sorted parts and make UI elements (technically rows) for the PartsListBox
        for (const auto* part : sorted_group | range_values) {
            // check if current row is full, and make a new row if necessary
            if (cur_col >= NUM_COLUMNS) {
                if (cur_row)
                    Insert(cur_row);
                cur_col = 0;
                cur_row = GG::Wnd::Create<PartsListBoxRow>(
                    TOTAL_WIDTH, SLOT_CONTROL_HEIGHT + GG::Y(PAD), m_availabilities_state);
            }
            ++cur_col;
            ++num_parts;

            // make new part control and add to row
            auto control = GG::Wnd::Create<PartControl>(part);
            control->ClickedSignal.connect(PartsListBox::ShipPartClickedSignal);
            control->DoubleClickedSignal.connect(PartsListBox::ShipPartDoubleClickedSignal);
            control->RightClickedSignal.connect(PartsListBox::ShipPartRightClickedSignal);

            if (auto shown = m_availabilities_state.DisplayedPartAvailability(part->Name()))
                control->SetAvailability(*shown);

            cur_row->push_back(std::move(control));
        }
    }
    // add any incomplete rows
    if (cur_row)
        Insert(std::move(cur_row));

    // keep track of how many columns are present now
    m_previous_num_columns = NUM_COLUMNS;

    // If there are no parts add a prompt to suggest a solution.
    if (num_parts == 0)
        Insert(GG::Wnd::Create<PromptRow>(TOTAL_WIDTH,
                                          UserString("ALL_AVAILABILITY_FILTERS_BLOCKING_PROMPT")),
               begin(), false);

}

void PartsListBox::ShowClass(ShipPartClass part_class, bool refresh_list) {
    if (!m_part_classes_shown.contains(part_class)) {
        m_part_classes_shown.insert(part_class);
        if (refresh_list)
            Populate();
    }
}

void PartsListBox::ShowAllClasses(bool refresh_list) {
    for (ShipPartClass part_class = ShipPartClass(0);
         part_class != ShipPartClass::NUM_SHIP_PART_CLASSES;
         part_class = ShipPartClass(int(part_class) + 1))
    { m_part_classes_shown.insert(part_class); }
    if (refresh_list)
        Populate();
}

void PartsListBox::HideClass(ShipPartClass part_class, bool refresh_list) {
    auto it = m_part_classes_shown.find(part_class);
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
    PartPalette(std::string_view config_name);
    void CompleteConstruction() override;

    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    void ShowClass(ShipPartClass part_class, bool refresh_list = true);
    void ShowAllClasses(bool refresh_list = true);
    void HideClass(ShipPartClass part_class, bool refresh_list = true);
    void HideAllClasses(bool refresh_list = true);
    void ToggleClass(ShipPartClass part_class, bool refresh_list = true);
    void ToggleAllClasses(bool refresh_list = true);

    void ToggleAvailability(const Availability::Enum type);

    void ShowSuperfluous(bool refresh_list = true);
    void HideSuperfluous(bool refresh_list = true);
    void ToggleSuperfluous(bool refresh_list = true);

    void Populate();

    mutable boost::signals2::signal<void (const ShipPart*, GG::Flags<GG::ModKey>)> ShipPartClickedSignal;
    mutable boost::signals2::signal<void (const ShipPart*)> ShipPartDoubleClickedSignal;
    mutable boost::signals2::signal<void (const ShipPart*, GG::Pt pt)> ShipPartRightClickedSignal;
    mutable boost::signals2::signal<void ()> PartObsolescenceChangedSignal;
    mutable boost::signals2::signal<void (const std::string&)> ClearPartSignal;

private:
    void DoLayout();

    /** A part type click with ctrl obsoletes part. */
    void HandleShipPartClicked(const ShipPart*, GG::Flags<GG::ModKey>);
    void HandleShipPartRightClicked(const ShipPart*, GG::Pt pt);

    std::shared_ptr<PartsListBox>                            m_parts_list;
    std::map<ShipPartClass, std::shared_ptr<CUIStateButton>> m_class_buttons;
    std::shared_ptr<CUIStateButton>                          m_superfluous_parts_button;

    // Holds the state of the availabilities filter.
    AvailabilityManager                         m_availabilities_state;
    std::tuple<std::shared_ptr<CUIStateButton>,
               std::shared_ptr<CUIStateButton>,
               std::shared_ptr<CUIStateButton>> m_availabilities_buttons;

};

DesignWnd::PartPalette::PartPalette(std::string_view config_name) :
    CUIWnd(UserString("DESIGN_WND_PART_PALETTE_TITLE"),
           GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE,
           config_name),
    m_availabilities_state(false, true, false)
{}

void DesignWnd::PartPalette::CompleteConstruction() {
    //TempUISoundDisabler sound_disabler;     // should be redundant with disabler in DesignWnd::DesignWnd.  uncomment if this is not the case
    SetChildClippingMode(ChildClippingMode::ClipToClient);

    m_parts_list = GG::Wnd::Create<PartsListBox>(m_availabilities_state);
    AttachChild(m_parts_list);
    m_parts_list->ShipPartClickedSignal.connect([this](auto* p, auto modkeys) { HandleShipPartClicked(p, modkeys); });
    m_parts_list->ShipPartDoubleClickedSignal.connect(ShipPartDoubleClickedSignal);
    m_parts_list->ShipPartRightClickedSignal.connect([this](auto* p, auto pt) { HandleShipPartRightClicked(p, pt); });
    m_parts_list->ClearPartSignal.connect(ClearPartSignal);

    const ShipPartManager& part_manager = GetShipPartManager();

    // class buttons
    for (ShipPartClass part_class = ShipPartClass(0);
         part_class != ShipPartClass::NUM_SHIP_PART_CLASSES;
         part_class = ShipPartClass(int(part_class) + 1))
    {
        // are there any parts of this class?
        bool part_of_this_class_exists = std::any_of(part_manager.begin(), part_manager.end(),
                                                     [part_class](auto& name_part) {
                                                         return name_part.second &&
                                                             name_part.second->Class() == part_class;
                                                     });
        if (!part_of_this_class_exists)
            continue;

        m_class_buttons[part_class] = GG::Wnd::Create<CUIStateButton>(
            UserString(to_string(part_class)), GG::FORMAT_CENTER,
            std::make_shared<CUILabelButtonRepresenter>());
        AttachChild(m_class_buttons[part_class]);
        m_class_buttons[part_class]->CheckedSignal.connect(
            [part_class, this](bool) { ToggleClass(part_class, true); });
    }


    // availability buttons
    auto& [obsolete_button, available_button, unavailable_button] = m_availabilities_buttons;

    obsolete_button = GG::Wnd::Create<CUIStateButton>(UserString("PRODUCTION_WND_AVAILABILITY_OBSOLETE"),
                                                      GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(obsolete_button);
    obsolete_button->CheckedSignal.connect([this](bool) { ToggleAvailability(Availability::Obsolete); });
    obsolete_button->SetCheck(m_availabilities_state.GetAvailability(Availability::Obsolete));

    available_button = GG::Wnd::Create<CUIStateButton>(UserString("PRODUCTION_WND_AVAILABILITY_AVAILABLE"),
                                                       GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(available_button);
    available_button->CheckedSignal.connect([this](bool) { ToggleAvailability(Availability::Available); });
    available_button->SetCheck(m_availabilities_state.GetAvailability(Availability::Available));

    unavailable_button = GG::Wnd::Create<CUIStateButton>(UserString("PRODUCTION_WND_AVAILABILITY_UNAVAILABLE"),
                                                         GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(unavailable_button);
    unavailable_button->CheckedSignal.connect([this](bool) { ToggleAvailability(Availability::Future); });
    unavailable_button->SetCheck(m_availabilities_state.GetAvailability(Availability::Future));


    // superfluous parts button
    m_superfluous_parts_button = GG::Wnd::Create<CUIStateButton>(UserString("PRODUCTION_WND_REDUNDANT"), GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(m_superfluous_parts_button);
    m_superfluous_parts_button->CheckedSignal.connect([this](bool) { ToggleSuperfluous(true); });

    // default to showing nothing
    ShowAllClasses(false);
    ShowSuperfluous(false);
    Populate();

    CUIWnd::CompleteConstruction();

    SetMinSize(PALETTE_MIN_SIZE);

    DoLayout();
    SaveDefaultedOptions();
}

void DesignWnd::PartPalette::SizeMove(GG::Pt ul, GG::Pt lr) {
    CUIWnd::SizeMove(ul, lr);
    DoLayout();
}

void DesignWnd::PartPalette::DoLayout() {
    const int PTS = ClientUI::Pts();
    const GG::X PTS_WIDE{PTS/2};         // guess at how wide per character the font needs
    const GG::Y BUTTON_HEIGHT{PTS*3/2};
    static constexpr int BUTTON_SEPARATION = 3; // vertical or horizontal sepration between adjacent buttons
    static constexpr int BUTTON_EDGE_PAD = 2;   // distance from edges of control to buttons
    static constexpr GG::X RIGHT_EDGE_PAD{8};   // to account for border of CUIWnd

    const GG::X USABLE_WIDTH = std::max(ClientWidth() - RIGHT_EDGE_PAD, GG::X1);   // space in which to fit buttons
    static constexpr int GUESSTIMATE_NUM_CHARS_IN_BUTTON_LABEL = 14;                   // rough guesstimate... avoid overly long part class names
    const GG::X MIN_BUTTON_WIDTH = PTS_WIDE*GUESSTIMATE_NUM_CHARS_IN_BUTTON_LABEL;
    const int MAX_BUTTONS_PER_ROW = std::max(USABLE_WIDTH / (MIN_BUTTON_WIDTH + BUTTON_SEPARATION), 1);

    const int NUM_CLASS_BUTTONS = std::max(1, static_cast<int>(m_class_buttons.size()));
    static constexpr int NUM_SUPERFLUOUS_CULL_BUTTONS = 1;
    static constexpr int NUM_AVAILABILITY_BUTTONS = 3;
    static constexpr int NUM_NON_CLASS_BUTTONS = NUM_SUPERFLUOUS_CULL_BUTTONS + NUM_AVAILABILITY_BUTTONS;

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
    for (auto& button : m_class_buttons | range_values) {
        if (col >= NUM_CLASS_BUTTONS_PER_ROW) {
            col = 0;
            ++row;
        }
        GG::Pt ul(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
        GG::Pt lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
        button->SizeMove(ul, lr);
        ++col;
    }

    // place parts list.  note: assuming at least as many rows of class buttons as availability buttons, as should
    //                          be the case given how num_non_class_buttons_per_row is determined
    m_parts_list->SizeMove(GG::Pt(GG::X0, BUTTON_EDGE_PAD + ROW_OFFSET*(row + 1)),
                           ClientSize() - GG::Pt(GG::X(BUTTON_SEPARATION), GG::Y(BUTTON_SEPARATION)));

    // place slot type buttons
    col = NUM_CLASS_BUTTONS_PER_ROW;
    row = 0;
    const auto ul = GG::Pt(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
    const auto lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_superfluous_parts_button->SizeMove(ul, lr);

    // a function to place availability buttons either in a single column below the
    // superfluous button or to complete a 2X2 grid left of the class buttons.
    auto place_avail_button_adjacent =
        [&col, &row, &num_non_class_buttons_per_row, NUM_CLASS_BUTTONS_PER_ROW,
         COL_OFFSET, ROW_OFFSET, BUTTON_WIDTH, BUTTON_HEIGHT]
        (GG::Wnd* avail_btn)
        {
            if (num_non_class_buttons_per_row == 1) {
                ++row;
            } else {
                if (col >= NUM_CLASS_BUTTONS_PER_ROW + num_non_class_buttons_per_row - 1) {
                    col = NUM_CLASS_BUTTONS_PER_ROW - 1;
                    ++row;
                }
                ++col;
            }

            auto ul = GG::Pt(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
            auto lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
            avail_btn->SizeMove(ul, lr);
        };

    //place availability buttons
    auto& [obsolete_button, available_button, unavailable_button] = m_availabilities_buttons;
    place_avail_button_adjacent(obsolete_button.get());
    place_avail_button_adjacent(available_button.get());
    place_avail_button_adjacent(unavailable_button.get());
}

void DesignWnd::PartPalette::HandleShipPartClicked(const ShipPart* part, GG::Flags<GG::ModKey> modkeys) {
    // Toggle obsolete for a control click.
    if (modkeys & GG::MOD_KEY_CTRL) {
        auto& manager = GetDisplayedDesignsManager();
        const auto obsolete = manager.IsPartObsolete(part->Name());
        manager.SetPartObsolete(part->Name(), !obsolete);

        PartObsolescenceChangedSignal();
        Populate();
    } else {
        ShipPartClickedSignal(part, modkeys);
    }
}

void DesignWnd::PartPalette::HandleShipPartRightClicked(const ShipPart* part, GG::Pt pt) {
    // Context menu actions
    auto& manager = GetDisplayedDesignsManager();
    const auto& part_name = part->Name();
    auto is_obsolete = manager.IsPartObsolete(part_name);
    auto toggle_obsolete_design_action = [&manager, &part_name, is_obsolete, this]() {
        manager.SetPartObsolete(part_name, !is_obsolete);
        PartObsolescenceChangedSignal();
        Populate();
    };

    // create popup menu with a commands in it
    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

    const auto empire_id = GGHumanClientApp::GetApp()->EmpireID();
    if (empire_id != ALL_EMPIRES)
        popup->AddMenuItem(GG::MenuItem(
                               (is_obsolete
                                ? UserString("DESIGN_WND_UNOBSOLETE_PART")
                                : UserString("DESIGN_WND_OBSOLETE_PART")),
                               false, false, toggle_obsolete_design_action));

    popup->Run();

    ShipPartRightClickedSignal(part, pt);
}

void DesignWnd::PartPalette::ShowClass(ShipPartClass part_class, bool refresh_list) {
    if (part_class >= ShipPartClass(0) && part_class < ShipPartClass::NUM_SHIP_PART_CLASSES) {
        m_parts_list->ShowClass(part_class, refresh_list);
        m_class_buttons[part_class]->SetCheck();
    } else {
        throw std::invalid_argument("PartPalette::ShowClass was passed an invalid ShipPartClass");
    }
}

void DesignWnd::PartPalette::ShowAllClasses(bool refresh_list) {
    m_parts_list->ShowAllClasses(refresh_list);
    for (auto& button : m_class_buttons | range_values)
        button->SetCheck();
}

void DesignWnd::PartPalette::HideClass(ShipPartClass part_class, bool refresh_list) {
    if (part_class >= ShipPartClass(0) && part_class < ShipPartClass::NUM_SHIP_PART_CLASSES) {
        m_parts_list->HideClass(part_class, refresh_list);
        m_class_buttons[part_class]->SetCheck(false);
    } else {
        throw std::invalid_argument("PartPalette::HideClass was passed an invalid ShipPartClass");
    }
}

void DesignWnd::PartPalette::HideAllClasses(bool refresh_list) {
    m_parts_list->HideAllClasses(refresh_list);
    for (auto& button : m_class_buttons | range_values)
        button->SetCheck(false);
}

void DesignWnd::PartPalette::ToggleClass(ShipPartClass part_class, bool refresh_list) {
    if (part_class >= ShipPartClass(0) && part_class < ShipPartClass::NUM_SHIP_PART_CLASSES) {
        const auto& classes_shown = m_parts_list->GetClassesShown();
        if (!classes_shown.contains(part_class))
            ShowClass(part_class, refresh_list);
        else
            HideClass(part_class, refresh_list);
    } else {
        throw std::invalid_argument("PartPalette::ToggleClass was passed an invalid ShipPartClass");
    }
}

void DesignWnd::PartPalette::ToggleAllClasses(bool refresh_list)
{
    const auto& classes_shown = m_parts_list->GetClassesShown();
    if (classes_shown.size() == int(ShipPartClass::NUM_SHIP_PART_CLASSES))
        HideAllClasses(refresh_list);
    else
        ShowAllClasses(refresh_list);
}

void DesignWnd::PartPalette::ToggleAvailability(Availability::Enum type) {
    std::shared_ptr<CUIStateButton> button;
    bool state = false;
    switch (type) {
    case Availability::Obsolete:
        m_availabilities_state.ToggleAvailability(Availability::Obsolete);
        state = m_availabilities_state.GetAvailability(Availability::Obsolete);
        button = std::get<Availability::Obsolete>(m_availabilities_buttons);
        break;
    case Availability::Available:
        m_availabilities_state.ToggleAvailability(Availability::Available);
        state = m_availabilities_state.GetAvailability(Availability::Available);
        button = std::get<Availability::Available>(m_availabilities_buttons);
        break;
    case Availability::Future:
        m_availabilities_state.ToggleAvailability(Availability::Future);
        state = m_availabilities_state.GetAvailability(Availability::Future);
        button = std::get<Availability::Future>(m_availabilities_buttons);
        break;
    }

    button->SetCheck(state);

    Populate();
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

void DesignWnd::PartPalette::Populate()
{ m_parts_list->Populate(); }


//////////////////////////////////////////////////
// BasesListBox                                  //
//////////////////////////////////////////////////
/** List of starting points for designs, such as empty hulls, existing designs
  * kept by this empire or seen elsewhere in the universe, design template
  * scripts or saved (on disk) designs from previous games. */
class BasesListBox : public QueueListBox {
public:
    static constexpr std::string_view BASES_LIST_BOX_DROP_TYPE = "BasesListBoxRow";

    BasesListBox(const AvailabilityManager& availabilities_state,
                 boost::optional<std::string_view> drop_type = boost::none,
                 boost::optional<std::string_view> empty_prompt = boost::none);
    BasesListBox(const AvailabilityManager& availabilities_state,
                 boost::optional<std::string_view> drop_type,
                 const std::string& empty_prompt) :
        BasesListBox(availabilities_state, std::move(drop_type),
                     boost::optional<std::string_view>{empty_prompt})
    {}
    void CompleteConstruction() override;

    void SizeMove(GG::Pt ul, GG::Pt lr) override;
    void ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds,
                             const GG::Wnd* destination) override;
    virtual void QueueItemMoved(const GG::ListBox::iterator row_it,
                                const GG::ListBox::iterator original_position_it) {}
    void SetEmpireShown(int empire_id, bool refresh_list = true);
    virtual void Populate();

    mutable boost::signals2::signal<void (int)>                 DesignSelectedSignal;
    mutable boost::signals2::signal<void (int)>                 DesignUpdatedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::vector<std::string>&)>
                                                                DesignComponentsSelectedSignal;
    mutable boost::signals2::signal<void (const boost::uuids::uuid&)>  SavedDesignSelectedSignal;

    mutable boost::signals2::signal<void (const ShipDesign*)>   DesignClickedSignal;
    mutable boost::signals2::signal<void (const ShipHull*)>     HullClickedSignal;
    mutable boost::signals2::signal<void (const ShipDesign*)>   DesignRightClickedSignal;

    class HullAndNamePanel : public GG::Control {
    public:
        HullAndNamePanel(GG::X w, GG::Y h, const std::string& hull, std::string name);

        void CompleteConstruction() override;
        void SizeMove(GG::Pt ul, GG::Pt lr) override;

        void Render() override {}

        void SetAvailability(const AvailabilityManager::DisplayedAvailabilies& type);
        void SetDisplayName(std::string name);

    private:
        std::shared_ptr<GG::StaticGraphic>  m_graphic;
        std::shared_ptr<GG::Label>          m_name;
    };

    class BasesListBoxRow : public CUIListBox::Row {
    public:
        BasesListBoxRow(GG::X w, GG::Y h, const std::string& hull, std::string name);

        void CompleteConstruction() override;
        void Render() override;

        void SizeMove(GG::Pt ul, GG::Pt lr) override;

        virtual void SetAvailability(const AvailabilityManager::DisplayedAvailabilies& type);
        virtual void SetDisplayName(std::string name);

    private:
        std::shared_ptr<HullAndNamePanel> m_hull_panel;
    };

    class HullAndPartsListBoxRow : public BasesListBoxRow {
    public:
        HullAndPartsListBoxRow(GG::X w, GG::Y h, std::string hull,
                               std::vector<std::string> parts);
        void CompleteConstruction() override;
        const auto& Hull() const noexcept  { return m_hull_name; }
        const auto& Parts() const noexcept { return m_parts; }

    protected:
        std::string              m_hull_name;
        std::vector<std::string> m_parts;
    };

    class CompletedDesignListBoxRow : public BasesListBoxRow {
    public:
        CompletedDesignListBoxRow(GG::X w, GG::Y h, const ShipDesign& design);
        void CompleteConstruction() override;
        int DesignID() const noexcept { return m_design_id; }
    private:
        int m_design_id = INVALID_DESIGN_ID;
    };

protected:
    void ItemRightClickedImpl(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) override;

    /** An implementation of BasesListBox provides a PopulateCore to fill itself.*/
    virtual void PopulateCore() = 0;

    /** Reset the empty list prompt. */
    virtual void ResetEmptyListPrompt();

    /** If \p wnd is a valid dragged child return a replacement row.  Otherwise return nullptr. */
    virtual std::shared_ptr<Row> ChildrenDraggedAwayCore(const GG::Wnd* const wnd) = 0;

    int EmpireID() const noexcept { return m_empire_id_shown; }

    const AvailabilityManager& AvailabilityState() const noexcept { return m_availabilities_state; }

    GG::Pt ListRowSize();

    virtual void BaseDoubleClicked(GG::ListBox::iterator, GG::Pt, GG::Flags<GG::ModKey>) {}
    virtual void BaseLeftClicked(GG::ListBox::iterator, GG::Pt, GG::Flags<GG::ModKey>) {}
    virtual void BaseRightClicked(GG::ListBox::iterator, GG::Pt, GG::Flags<GG::ModKey>) {}

private:
    void InitRowSizes();

    int                                m_empire_id_shown = ALL_EMPIRES;
    const AvailabilityManager&         m_availabilities_state;
    boost::signals2::scoped_connection m_empire_designs_changed_signal;
};

BasesListBox::HullAndNamePanel::HullAndNamePanel(GG::X w, GG::Y h, const std::string& hull,
                                                 std::string name) :
    GG::Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS)
{
    SetChildClippingMode(ChildClippingMode::ClipToClient);

    m_graphic = GG::Wnd::Create<GG::StaticGraphic>(ClientUI::HullIcon(hull),
                                                   GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC);
    m_graphic->Resize(GG::Pt(w, h));
    m_name = GG::Wnd::Create<CUILabel>(std::move(name),
                                       GG::FORMAT_WORDBREAK | GG::FORMAT_CENTER | GG::FORMAT_TOP);
}

void BasesListBox::HullAndNamePanel::CompleteConstruction() {
    GG::Control::CompleteConstruction();
    AttachChild(m_graphic);
    AttachChild(m_name);
}

void BasesListBox::HullAndNamePanel::SizeMove(GG::Pt ul, GG::Pt lr) {
    GG::Control::SizeMove(ul, lr);
    m_graphic->Resize(Size());
    m_name->Resize(Size());
}

void BasesListBox::HullAndNamePanel::SetAvailability(
    const AvailabilityManager::DisplayedAvailabilies& type)
{
    auto disabled = std::get<Availability::Obsolete>(type);
    m_graphic->Disable(disabled);
    m_name->Disable(disabled);
}

void BasesListBox::HullAndNamePanel::SetDisplayName(std::string name) {
    m_name->SetText(std::move(name));
    m_name->Resize(GG::Pt(Width(), m_name->Height()));
}

BasesListBox::BasesListBoxRow::BasesListBoxRow(GG::X w, GG::Y h, const std::string& hull,
                                               std::string name) :
    CUIListBox::Row(w, h)
{
    SetDragDropDataType(BASES_LIST_BOX_DROP_TYPE);
    if (hull.empty()) {
        ErrorLogger() << "No hull name provided for ship row display.";
        return;
    }

    m_hull_panel = GG::Wnd::Create<HullAndNamePanel>(w, h, hull, std::move(name));

    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
}

void BasesListBox::BasesListBoxRow::CompleteConstruction() {
    CUIListBox::Row::CompleteConstruction();
    push_back(m_hull_panel);
}

void BasesListBox::BasesListBoxRow::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::Pt ul_adjusted_for_drop_indicator = GG::Pt(ul.x, ul.y + GG::Y1);
    GG::Pt lr_adjusted_for_drop_indicator = GG::Pt(lr.x, lr.y - GG::Y{2});
    GG::FlatRectangle(ul_adjusted_for_drop_indicator, lr_adjusted_for_drop_indicator,
                      ClientUI::WndColor(),
                      (Disabled() ? DisabledColor(GG::CLR_WHITE) : GG::CLR_WHITE), 1);
}

void BasesListBox::BasesListBoxRow::SizeMove(GG::Pt ul, GG::Pt lr) {
    const auto old_size = Size();
    CUIListBox::Row::SizeMove(ul, lr);
    if (!empty() && old_size != Size())
        at(0)->Resize(Size());
}

void BasesListBox::BasesListBoxRow::SetAvailability(const AvailabilityManager::DisplayedAvailabilies& type) {
    if (std::get<Availability::Obsolete>(type) && std::get<Availability::Future>(type))
        SetBrowseText(UserString("PRODUCTION_WND_AVAILABILITY_OBSOLETE_AND_UNAVAILABLE"));
    else if (std::get<Availability::Obsolete>(type))
        SetBrowseText(UserString("PRODUCTION_WND_AVAILABILITY_OBSOLETE"));
    else if (std::get<Availability::Future>(type))
        SetBrowseText(UserString("PRODUCTION_WND_AVAILABILITY_UNAVAILABLE"));
    else
        ClearBrowseInfoWnd();

    auto disabled = std::get<Availability::Obsolete>(type);
    Disable(disabled);
    if (m_hull_panel)
        m_hull_panel->SetAvailability(type);
}

void BasesListBox::BasesListBoxRow::SetDisplayName(std::string name) {
    if (m_hull_panel)
        m_hull_panel->SetDisplayName(std::move(name));
}

BasesListBox::HullAndPartsListBoxRow::HullAndPartsListBoxRow(GG::X w, GG::Y h, std::string hull,
                                                             std::vector<std::string> parts) :
    BasesListBoxRow(w, h, hull, UserString(hull)),
    m_hull_name(std::move(hull)),
    m_parts(std::move(parts))
{}

void BasesListBox::HullAndPartsListBoxRow::CompleteConstruction() {
    BasesListBoxRow::CompleteConstruction();
    SetDragDropDataType(HULL_PARTS_ROW_DROP_TYPE_STRING);
}

BasesListBox::CompletedDesignListBoxRow::CompletedDesignListBoxRow(
    GG::X w, GG::Y h, const ShipDesign &design) :
    BasesListBoxRow(w, h, design.Hull(), design.Name()),
    m_design_id(design.ID())
{}

void BasesListBox::CompletedDesignListBoxRow::CompleteConstruction() {
    BasesListBoxRow::CompleteConstruction();
    SetDragDropDataType(COMPLETE_DESIGN_ROW_DROP_STRING);
}

BasesListBox::BasesListBox(const AvailabilityManager& availabilities_state,
                           boost::optional<std::string_view> drop_type,
                           boost::optional<std::string_view> empty_prompt) :
    QueueListBox(std::move(drop_type),
                 empty_prompt ? std::string{*empty_prompt} : UserString("ADD_FIRST_DESIGN_DESIGN_QUEUE_PROMPT")),
    m_empire_id_shown(ALL_EMPIRES),
    m_availabilities_state(availabilities_state)
{}

void BasesListBox::CompleteConstruction() {
    QueueListBox::CompleteConstruction();

    InitRowSizes();
    SetStyle(GG::LIST_NOSEL | GG::LIST_NOSORT);

    // iterator, Pt, GG::Flags<GG::ModKey>
    DoubleClickedRowSignal.connect([this](auto it, auto pt, auto modkeys) { BaseDoubleClicked(it, pt, modkeys); });
    LeftClickedRowSignal.connect([this](auto it, auto pt, auto modkeys) { BaseLeftClicked(it, pt, modkeys); });
    MovedRowSignal.connect([this](auto new_it, auto old_it) { QueueItemMoved(new_it, old_it); });

    EnableOrderIssuing(false);
}

void BasesListBox::SizeMove(GG::Pt ul, GG::Pt lr) {
    const auto old_size = Size();
    CUIListBox::SizeMove(ul, lr);
    if (old_size != Size()) {
        const GG::Pt row_size = ListRowSize();
        for (auto& row : *this)
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
    auto* original_row = dynamic_cast<const Row*>(wnd);
    if (!original_row)
        return;

    iterator insertion_point = std::find_if(
        begin(), end(), [&original_row](const auto& xx){return xx.get() == original_row;});
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

void BasesListBox::SetEmpireShown(int empire_id, bool refresh_list) {
    m_empire_id_shown = empire_id;

    // disconnect old signal
    m_empire_designs_changed_signal.disconnect();

    // connect signal to update this list if the empire's designs change
    if (const Empire* empire = GetEmpire(m_empire_id_shown))
        m_empire_designs_changed_signal = empire->ShipDesignsChangedSignal.connect([this]() { Populate(); });

    if (refresh_list)
        Populate();
}

void BasesListBox::Populate() {
    const auto avail_state{m_availabilities_state.GetAvailabilities()};

    // Provide conditional reminder text when the list is empty
    static constexpr auto all_false{AvailabilityManager::DisplayedAvailabilies{false, false, false}};
    if (avail_state == all_false)
        SetEmptyPromptText(UserString("ALL_AVAILABILITY_FILTERS_BLOCKING_PROMPT"));
    else
        this->ResetEmptyListPrompt();

    // make note of first visible row to preserve state
    auto init_first_row_shown = FirstRowShown();
    std::size_t init_first_row_offset = std::distance(begin(), init_first_row_shown);

    Clear();

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

void BasesListBox::ItemRightClickedImpl(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys)
{ this->BaseRightClicked(it, pt, modkeys); }


//////////////////////////////////////////////////
// BasesListBox derived classes                 //
//////////////////////////////////////////////////
class EmptyHullsListBox : public BasesListBox {
public:
    EmptyHullsListBox(const AvailabilityManager& availabilities_state,
                      const boost::optional<std::string_view>& drop_type = boost::none) :
        BasesListBox::BasesListBox(availabilities_state, drop_type, UserString("ALL_AVAILABILITY_FILTERS_BLOCKING_PROMPT"))
    {}

    void EnableOrderIssuing(bool enable = true) override;

protected:
    void PopulateCore() override;
    std::shared_ptr<Row> ChildrenDraggedAwayCore(const GG::Wnd* const wnd) override;
    void QueueItemMoved(const GG::ListBox::iterator row_it,
                        const GG::ListBox::iterator original_position_it) override;

    void BaseDoubleClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) override;
    void BaseLeftClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) override;
    void BaseRightClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) override;
};

class CompletedDesignsListBox : public BasesListBox {
public:
    CompletedDesignsListBox(const AvailabilityManager& availabilities_state,
                            const boost::optional<std::string_view>& drop_type = boost::none) :
        BasesListBox::BasesListBox(availabilities_state, drop_type)
    {};

protected:
    void PopulateCore() override;
    void ResetEmptyListPrompt() override;
    std::shared_ptr<Row> ChildrenDraggedAwayCore(const GG::Wnd* const wnd) override;
    void QueueItemMoved(const GG::ListBox::iterator row_it,
                        const GG::ListBox::iterator original_position_it) override;
    void BaseDoubleClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) override;
    void BaseLeftClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) override;
    void BaseRightClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) override;
};

class SavedDesignsListBox : public BasesListBox {
public:
    SavedDesignsListBox(const AvailabilityManager& availabilities_state,
                        const boost::optional<std::string_view>& drop_type = boost::none) :
        BasesListBox::BasesListBox(availabilities_state, drop_type, UserString("ADD_FIRST_SAVED_DESIGN_QUEUE_PROMPT"))
    {};

    class SavedDesignListBoxRow : public BasesListBoxRow {
        public:
        SavedDesignListBoxRow(GG::X w, GG::Y h, const ShipDesign& design);
        void CompleteConstruction() override;
        const boost::uuids::uuid        DesignUUID() const;
        const std::string&              DesignName() const;
        const std::string&              Description() const;
        bool                            LookupInStringtable() const;

        private:
        boost::uuids::uuid              m_design_uuid;
    };

protected:
    void PopulateCore() override;
    void ResetEmptyListPrompt() override;
    std::shared_ptr<Row> ChildrenDraggedAwayCore(const GG::Wnd* const wnd) override;
    void QueueItemMoved(const GG::ListBox::iterator row_it,
                        const GG::ListBox::iterator original_position_it) override;

    void BaseDoubleClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) override;
    void BaseLeftClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) override;
    void BaseRightClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) override;
};

class MonstersListBox : public BasesListBox {
public:
    MonstersListBox(const AvailabilityManager& availabilities_state,
                    const boost::optional<std::string_view>& drop_type = boost::none) :
        BasesListBox::BasesListBox(availabilities_state, drop_type)
    {}

    void EnableOrderIssuing(bool enable = true) override;

protected:
    void PopulateCore() override;
    std::shared_ptr<Row> ChildrenDraggedAwayCore(const GG::Wnd* const wnd) override;

    void BaseDoubleClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) override;
    void BaseLeftClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) override;
};

class AllDesignsListBox : public BasesListBox {
public:
    AllDesignsListBox(const AvailabilityManager& availabilities_state,
                      const boost::optional<std::string_view>& drop_type = boost::none) :
        BasesListBox::BasesListBox(availabilities_state, drop_type)
    {}

protected:
    void PopulateCore() override;
    std::shared_ptr<Row> ChildrenDraggedAwayCore(const GG::Wnd* const wnd) override;

    void BaseDoubleClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) override;
    void BaseLeftClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) override;
};

void EmptyHullsListBox::PopulateCore() {
    ScopedTimer scoped_timer("EmptyHulls::PopulateCore");
    DebugLogger() << "EmptyHulls::PopulateCore EmpireID(): " << EmpireID();

    const GG::Pt row_size = ListRowSize();

    const auto& manager = GetDisplayedDesignsManager();

    auto hulls = manager.OrderedHulls();
    if (hulls.size() < GetShipHullManager().size()) {
        ErrorLogger() << "EmptyHulls::PopulateCoreordered has fewer than expected entries...";
        for (auto& hull : GetShipHullManager()) {
            auto it = std::find(hulls.begin(), hulls.end(), hull.first);
            if (it == hulls.end())
                hulls.push_back(hull.first);    // O(N^2) in loop, but I don't care...
        }
    }

    for (auto& hull_name : hulls) {
        const auto& ship_hull =  GetShipHullManager().GetShipHull(hull_name);

        if (!ship_hull || !ship_hull->Producible())
            continue;

        auto shown = AvailabilityState().DisplayedHullAvailability(hull_name);
        if (!shown)
            continue;
        auto row = GG::Wnd::Create<HullAndPartsListBoxRow>(row_size.x, row_size.y,
                                                           std::move(hull_name),
                                                           std::vector<std::string>{});
        row->SetAvailability(*shown);
        row->Resize(row_size);  // TODO: should this and following be swapped?
        Insert(std::move(row));
    }
}

void CompletedDesignsListBox::PopulateCore() {
    ScopedTimer scoped_timer("CompletedDesignsListBox::PopulateCore");
    DebugLogger() << "CompletedDesignsListBox::PopulateCore for empire " << EmpireID();

    const bool showing_available = AvailabilityState().GetAvailability(Availability::Available);
    const Universe& universe = GetUniverse();
    const GG::Pt row_size = ListRowSize();

    if (const auto empire = GetEmpire(EmpireID())) {
        // add rows for designs this empire is keeping
        const auto& manager = GetDisplayedDesignsManager();
        for (int design_id : manager.AllOrderedIDs()) {
            try {
                const ShipDesign* design = GetUniverse().GetShipDesign(design_id);
                if (!design)
                    continue;

                auto shown = AvailabilityState().DisplayedDesignAvailability(*design);
                if (shown) {
                    auto row = GG::Wnd::Create<CompletedDesignListBoxRow>(row_size.x, row_size.y, *design);
                    row->SetAvailability(*shown);
                    Insert(row);
                    row->Resize(row_size);
                }
            } catch (const std::out_of_range&) {
                ErrorLogger() << "ship design with id " << design_id << " incorrectly stored in manager.";
            }
        }

    } else if (showing_available) {
        // add all known / existing designs
        for (const auto& design : universe.ShipDesigns() | range_values
             | range_filter([](const auto& d) { return d.Producible(); }))
        {
            auto row = GG::Wnd::Create<CompletedDesignListBoxRow>(row_size.x, row_size.y, design);
            Insert(row);
            row->Resize(row_size);
        }
    }
}

void SavedDesignsListBox::PopulateCore() {
    ScopedTimer scoped_timer("CompletedDesigns::PopulateCore");
    DebugLogger() << "CompletedDesigns::PopulateCore";

    const GG::Pt row_size = ListRowSize();

    for (const auto& uuid : GetSavedDesignsManager().OrderedDesignUUIDs()) {
        const auto design = GetSavedDesignsManager().GetDesign(uuid);
        auto shown = AvailabilityState().DisplayedDesignAvailability(*design);
        if (!shown)
            continue;

        auto row = GG::Wnd::Create<SavedDesignListBoxRow>(row_size.x, row_size.y, *design);
        Insert(row);
        row->Resize(row_size);
        row->SetAvailability(*shown);
    }
}

void MonstersListBox::PopulateCore() {
    ScopedTimer scoped_timer("Monsters::PopulateCore");

    const Universe& universe = GetUniverse();

    const GG::Pt row_size = ListRowSize();

    for (const auto& design : universe.ShipDesigns() | range_values
         | range_filter([](const auto& d) { return d.IsMonster(); }))
    {
        auto row = GG::Wnd::Create<CompletedDesignListBoxRow>(row_size.x, row_size.y, design);
        Insert(row);
        row->Resize(row_size);
    }
}

void AllDesignsListBox::PopulateCore() {
    ScopedTimer scoped_timer("All::PopulateCore");

    const Universe& universe = GetUniverse();
    const auto row_size = ListRowSize();

    for (const auto& design : universe.ShipDesigns() | range_values) {
        auto row = GG::Wnd::Create<CompletedDesignListBoxRow>(row_size.x, row_size.y, design);
        Insert(row);
        row->Resize(row_size);
    }
}


void BasesListBox::ResetEmptyListPrompt()
{ SetEmptyPromptText(UserString("ALL_AVAILABILITY_FILTERS_BLOCKING_PROMPT")); }

void CompletedDesignsListBox::ResetEmptyListPrompt() {
    if (!GetOptionsDB().Get<bool>("resource.shipdesign.saved.enabled")
        && !GetOptionsDB().Get<bool>("resource.shipdesign.default.enabled"))
    {
        SetEmptyPromptText(UserString("NO_SAVED_OR_DEFAULT_DESIGNS_ADDED_PROMPT"));
    } else {
        SetEmptyPromptText(UserString("ADD_FIRST_DESIGN_DESIGN_QUEUE_PROMPT"));
    }
}

void SavedDesignsListBox::ResetEmptyListPrompt()
{ SetEmptyPromptText(UserString("ADD_FIRST_SAVED_DESIGN_QUEUE_PROMPT")); }


std::shared_ptr<BasesListBox::Row> EmptyHullsListBox::ChildrenDraggedAwayCore(const GG::Wnd* const wnd) {
    // find type of hull that was dragged away, and replace
    const auto design_row = dynamic_cast<const BasesListBox::HullAndPartsListBoxRow*>(wnd);
    if (!design_row)
        return nullptr;

    const std::string& hull_name = design_row->Hull();
    const auto row_size = ListRowSize();
    auto row =  GG::Wnd::Create<HullAndPartsListBoxRow>(row_size.x, row_size.y,
                                                        hull_name, std::vector<std::string>{});

    if (auto shown = AvailabilityState().DisplayedHullAvailability(hull_name))
        row->SetAvailability(*shown);

    return row;
}

std::shared_ptr<BasesListBox::Row> CompletedDesignsListBox::ChildrenDraggedAwayCore(const GG::Wnd* const wnd) {
    // find design that was dragged away, and replace

    const auto design_row = dynamic_cast<const BasesListBox::CompletedDesignListBoxRow*>(wnd);
    if (!design_row)
        return nullptr;

    int design_id = design_row->DesignID();
    const ShipDesign* design = GetUniverse().GetShipDesign(design_id);
    if (!design) {
        ErrorLogger() << "Missing design with id " << design_id;
        return nullptr;
    }

    const auto row_size = ListRowSize();
    auto row = GG::Wnd::Create<CompletedDesignListBoxRow>(row_size.x, row_size.y, *design);
    if (auto shown = AvailabilityState().DisplayedDesignAvailability(*design))
        row->SetAvailability(*shown);
    return row;
}

std::shared_ptr<BasesListBox::Row> SavedDesignsListBox::ChildrenDraggedAwayCore(const GG::Wnd* const wnd) {
    // find name of design that was dragged away, and replace
    const auto design_row = dynamic_cast<const SavedDesignsListBox::SavedDesignListBoxRow*>(wnd);
    if (!design_row)
        return nullptr;

    SavedDesignsManager& manager = GetSavedDesignsManager();
    const auto design = manager.GetDesign(design_row->DesignUUID());
    if (!design) {
        ErrorLogger() << "Saved design missing with uuid " << design_row->DesignUUID();
        return nullptr;
    }

    const auto row_size = ListRowSize();
    auto row = GG::Wnd::Create<SavedDesignListBoxRow>(row_size.x, row_size.y, *design);

    if (auto shown = AvailabilityState().DisplayedDesignAvailability(*design))
        row->SetAvailability(*shown);

    return row;
}

std::shared_ptr<BasesListBox::Row> MonstersListBox::ChildrenDraggedAwayCore(const GG::Wnd* const wnd) {
    // Replace the design that was dragged away
    const auto design_row = dynamic_cast<const BasesListBox::CompletedDesignListBoxRow*>(wnd);
    if (!design_row)
        return nullptr;

    int design_id = design_row->DesignID();
    const ShipDesign* design = GetUniverse().GetShipDesign(design_id);
    if (!design) {
        ErrorLogger() << "Missing design with id " << design_id;
        return nullptr;
    }

    const auto row_size = ListRowSize();
    auto row = GG::Wnd::Create<CompletedDesignListBoxRow>(row_size.x, row_size.y, *design);
    return row;
}

std::shared_ptr<BasesListBox::Row> AllDesignsListBox::ChildrenDraggedAwayCore(const GG::Wnd* const wnd) {
    // Replace the design that was dragged away
    const auto design_row = dynamic_cast<const BasesListBox::CompletedDesignListBoxRow*>(wnd);
    if (!design_row)
        return nullptr;

    int design_id = design_row->DesignID();
    const ShipDesign* design = GetUniverse().GetShipDesign(design_id);
    if (!design) {
        ErrorLogger() << "Missing design with id " << design_id;
        return nullptr;
    }

    const auto row_size = ListRowSize();
    auto row = GG::Wnd::Create<CompletedDesignListBoxRow>(row_size.x, row_size.y, *design);
    return row;
}

void EmptyHullsListBox::EnableOrderIssuing(bool enable)
{ QueueListBox::EnableOrderIssuing(enable); }

void MonstersListBox::EnableOrderIssuing(bool)
{ QueueListBox::EnableOrderIssuing(false); }

void EmptyHullsListBox::BaseDoubleClicked(GG::ListBox::iterator it, GG::Pt pt,
                                          GG::Flags<GG::ModKey> modkeys)
{
    const auto hp_row = dynamic_cast<HullAndPartsListBoxRow*>(it->get());
    if (!hp_row)
        return;

    if (!hp_row->Hull().empty() || !hp_row->Parts().empty())
        DesignComponentsSelectedSignal(hp_row->Hull(), hp_row->Parts());
}

void CompletedDesignsListBox::BaseDoubleClicked(GG::ListBox::iterator it, GG::Pt pt,
                                                GG::Flags<GG::ModKey> modkeys)
{
    const auto cd_row = dynamic_cast<CompletedDesignListBoxRow*>(it->get());
    if (!cd_row || cd_row->DesignID() == INVALID_DESIGN_ID)
        return;

    DesignSelectedSignal(cd_row->DesignID());
}

void SavedDesignsListBox::BaseDoubleClicked(GG::ListBox::iterator it, GG::Pt pt,
                                            GG::Flags<GG::ModKey> modkeys)
{
    const auto sd_row = dynamic_cast<SavedDesignListBoxRow*>(it->get());

    if (!sd_row)
        return;
    SavedDesignSelectedSignal(sd_row->DesignUUID());
}

void MonstersListBox::BaseDoubleClicked(GG::ListBox::iterator it, GG::Pt pt,
                                        GG::Flags<GG::ModKey> modkeys)
{
    const auto cd_row = dynamic_cast<CompletedDesignListBoxRow*>(it->get());
    if (!cd_row || cd_row->DesignID() == INVALID_DESIGN_ID)
        return;

    DesignSelectedSignal(cd_row->DesignID());
}

void AllDesignsListBox::BaseDoubleClicked(GG::ListBox::iterator it, GG::Pt pt,
                                          GG::Flags<GG::ModKey> modkeys)
{
    const auto cd_row = dynamic_cast<CompletedDesignListBoxRow*>(it->get());
    if (!cd_row || cd_row->DesignID() == INVALID_DESIGN_ID)
        return;

    DesignSelectedSignal(cd_row->DesignID());
}

void EmptyHullsListBox::BaseLeftClicked(GG::ListBox::iterator it, GG::Pt pt,
                                        GG::Flags<GG::ModKey> modkeys)
{
    const auto hull_parts_row = dynamic_cast<HullAndPartsListBoxRow*>(it->get());
    if (!hull_parts_row)
        return;
    const std::string& hull_name = hull_parts_row->Hull();
    const ShipHull* ship_hull = GetShipHull(hull_name);
    const std::vector<std::string>& parts = hull_parts_row->Parts();

    if (modkeys & GG::MOD_KEY_CTRL) {
        // Toggle hull obsolete
        auto& manager = GetDisplayedDesignsManager();
        const auto is_obsolete = manager.IsHullObsolete(hull_name);
        manager.SetHullObsolete(hull_name, !is_obsolete);
        Populate();
    }
    else if (ship_hull && parts.empty())
        HullClickedSignal(ship_hull);
}

void CompletedDesignsListBox::BaseLeftClicked(GG::ListBox::iterator it, GG::Pt pt,
                                              GG::Flags<GG::ModKey> modkeys)
{
    const auto design_row = dynamic_cast<CompletedDesignListBoxRow*>(it->get());
    if (!design_row)
        return;
    int id = design_row->DesignID();

    const ScriptingContext& context = GGHumanClientApp::GetApp()->GetContext();
    const ShipDesign* design = context.ContextUniverse().GetShipDesign(id);
    if (!design)
        return;

    const auto& manager = GetDisplayedDesignsManager();

    if (modkeys & GG::MOD_KEY_CTRL && manager.IsKnown(id)) {
        const auto maybe_obsolete = manager.IsObsolete(id, context);
        bool is_obsolete = maybe_obsolete && *maybe_obsolete;
        SetObsoleteInDisplayedDesigns(id, !is_obsolete);
        Populate();

    } else {
        DesignClickedSignal(design);
    }
}

void SavedDesignsListBox::BaseLeftClicked(GG::ListBox::iterator it, GG::Pt pt,
                                          GG::Flags<GG::ModKey> modkeys)
{
    const auto saved_design_row = dynamic_cast<SavedDesignListBoxRow*>(it->get());
    if (!saved_design_row)
        return;
    const auto design_uuid = saved_design_row->DesignUUID();
    auto& manager = GetSavedDesignsManager();
    const auto design = manager.GetDesign(design_uuid);
    if (!design)
        return;
    if (modkeys & GG::MOD_KEY_CTRL)
        AddSavedDesignToDisplayedDesigns(design->UUID(), EmpireID());
    else
        DesignClickedSignal(design);
}

void MonstersListBox::BaseLeftClicked(GG::ListBox::iterator it, GG::Pt pt,
                                      GG::Flags<GG::ModKey> modkeys)
{
    const auto design_row = dynamic_cast<CompletedDesignListBoxRow*>(it->get());
    if (!design_row)
        return;
    int id = design_row->DesignID();
    const ShipDesign* design = GetUniverse().GetShipDesign(id);
    if (!design)
        return;

    DesignClickedSignal(design);
}

void AllDesignsListBox::BaseLeftClicked(GG::ListBox::iterator it, GG::Pt pt,
                                        GG::Flags<GG::ModKey> modkeys)
{
    const auto design_row = dynamic_cast<CompletedDesignListBoxRow*>(it->get());
    if (!design_row)
        return;
    int id = design_row->DesignID();
    const ShipDesign* design = GetUniverse().GetShipDesign(id);
    if (!design)
        return;

    DesignClickedSignal(design);
}

void EmptyHullsListBox::BaseRightClicked(GG::ListBox::iterator it, GG::Pt pt,
                                         GG::Flags<GG::ModKey> modkeys)
{
    const auto hull_parts_row = dynamic_cast<HullAndPartsListBoxRow*>(it->get());
    if (!hull_parts_row)
        return;
    const std::string& hull_name = hull_parts_row->Hull();

    // Context menu actions
    auto& manager = GetDisplayedDesignsManager();
    auto is_obsolete = manager.IsHullObsolete(hull_name);
    auto toggle_obsolete_design_action = [&manager, &hull_name, is_obsolete, this]() {
        manager.SetHullObsolete(hull_name, !is_obsolete);
        Populate();
    };

    // create popup menu with a commands in it
    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

    const auto empire_id = EmpireID();
    if (empire_id != ALL_EMPIRES)
        popup->AddMenuItem(GG::MenuItem(
                               (is_obsolete
                                ? UserString("DESIGN_WND_UNOBSOLETE_HULL")
                                : UserString("DESIGN_WND_OBSOLETE_HULL")),
                               false, false, toggle_obsolete_design_action));

    popup->Run();
}

void CompletedDesignsListBox::BaseRightClicked(GG::ListBox::iterator it, GG::Pt pt,
                                               GG::Flags<GG::ModKey> modkeys)
{
    const auto design_row = dynamic_cast<CompletedDesignListBoxRow*>(it->get());
    if (!design_row)
        return;

    const ScriptingContext& context = GGHumanClientApp::GetApp()->GetContext();

    const auto design_id = design_row->DesignID();
    const auto design = context.ContextUniverse().GetShipDesign(design_id);
    if (!design)
        return;

    DesignRightClickedSignal(design);

    DebugLogger() << "BasesListBox::BaseRightClicked on design id : " << design_id;

    if (design->UUID().is_nil())
        ErrorLogger() << "BasesListBox::BaseRightClicked Design UUID is nil";

    // Context menu actions
    const auto& manager = GetDisplayedDesignsManager();
    const auto maybe_obsolete = manager.IsObsolete(design_id, context);
    bool is_obsolete = maybe_obsolete && *maybe_obsolete;
    auto toggle_obsolete_design_action = [design_id, is_obsolete, this]() {
        SetObsoleteInDisplayedDesigns(design_id, !is_obsolete);
        Populate();
    };

    auto delete_design_action = [design_id, this]() {
        DeleteFromDisplayedDesigns(design_id);
        Populate();
        DesignUpdatedSignal(design_id);
    };

    const auto empire_id = EmpireID();

    auto rename_design_action = [empire_id, design_id, design, &design_row]() {
        auto edit_wnd = GG::Wnd::Create<CUIEditWnd>(
            GG::X(350), UserString("DESIGN_ENTER_NEW_DESIGN_NAME"), design->Name());
        edit_wnd->Run();
        const auto& result = edit_wnd->Result();
        if (!result.empty() && result != design->Name()) {
            auto* app = GGHumanClientApp::GetApp();
            app->Orders().IssueOrder<ShipDesignOrder>(
                app->GetContext(), empire_id, design_id, result, design->Description());
            design_row->SetDisplayName(design->Name());
        }
    };

    auto movetotop_design_action = [id{design->ID()}, this]() {
        GetDisplayedDesignsManager().MoveBefore(id, GetDisplayedDesignsManager().OrderedIDs().front());
        Populate();
    };

    auto movetobottom_design_action = [id{design->ID()}, this]() {
        GetDisplayedDesignsManager().MoveBefore(id, INVALID_DESIGN_ID);
        Populate();
    };

    auto save_design_action = [&design]() {
        auto saved_design{*design};
        saved_design.SetUUID(boost::uuids::random_generator()());
        GetSavedDesignsManager().InsertBefore(
            saved_design, GetSavedDesignsManager().OrderedDesignUUIDs().begin());
    };

    // toggle the option to add all saved designs at game start.
    const auto add_defaults = GetOptionsDB().Get<bool>("resource.shipdesign.default.enabled");
    auto toggle_add_default_designs_at_game_start_action = [add_defaults]()
    { GetOptionsDB().Set<bool>("resource.shipdesign.default.enabled", !add_defaults); };

    // create popup menu with a commands in it
    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

    // obsolete design
    if (empire_id != ALL_EMPIRES)
        popup->AddMenuItem(GG::MenuItem(
                              (is_obsolete
                               ? UserString("DESIGN_WND_UNOBSOLETE_DESIGN")
                               : UserString("DESIGN_WND_OBSOLETE_DESIGN")),
                              false, false, toggle_obsolete_design_action));

    // delete design
    if (empire_id != ALL_EMPIRES)
        popup->AddMenuItem(GG::MenuItem(UserString("DESIGN_WND_DELETE_DESIGN"),     false, false, delete_design_action));

    // rename design
    if (design->DesignedByEmpire() == empire_id && empire_id != ALL_EMPIRES)
        popup->AddMenuItem(GG::MenuItem(UserString("DESIGN_RENAME"),                false, false, rename_design_action));

    // save design
    popup->AddMenuItem(GG::MenuItem(UserString("DESIGN_SAVE"),                      false, false, save_design_action));

    // move design
    popup->AddMenuItem(GG::MenuItem(true)); // separator
    popup->AddMenuItem(GG::MenuItem(UserString("MOVE_UP_LIST_ITEM"),                false, false, movetotop_design_action));
    popup->AddMenuItem(GG::MenuItem(UserString("MOVE_DOWN_LIST_ITEM"),              false, false, movetobottom_design_action));

    popup->AddMenuItem(GG::MenuItem(true)); // separator
    popup->AddMenuItem(GG::MenuItem(UserString("DESIGN_WND_ADD_ALL_DEFAULT_START"), false, add_defaults,
                                    toggle_add_default_designs_at_game_start_action));

    popup->Run();
}

void SavedDesignsListBox::BaseRightClicked(GG::ListBox::iterator it, GG::Pt pt,
                                           GG::Flags<GG::ModKey> modkeys)
{
    const auto design_row = dynamic_cast<SavedDesignListBoxRow*>(it->get());
    if (!design_row)
        return;
    const auto design_uuid = design_row->DesignUUID();
    auto& manager = GetSavedDesignsManager();
    const auto design = manager.GetDesign(design_uuid);
    if (!design)
        return;
    const auto empire_id = EmpireID();

    DesignRightClickedSignal(design);

    DebugLogger() << "BasesListBox::BaseRightClicked on design name : " << design->Name();

    // Context menu actions
    // add design to empire
    auto add_design_action = [&design, &empire_id]() {
        AddSavedDesignToDisplayedDesigns(design->UUID(), empire_id);
    };

    // delete design from saved designs
    auto delete_saved_design_action = [&manager, &design, this]() {
        DebugLogger() << "BasesListBox::BaseRightClicked Delete Saved Design" << design->Name();
        manager.Erase(design->UUID());
        Populate();
    };

    auto movetotop_design_action = [&design, this]() {
        GetSavedDesignsManager().MoveBefore(design->UUID(), GetSavedDesignsManager().OrderedDesignUUIDs().front());
        Populate();
    };

    auto movetobottom_design_action = [&design, this]() {
        GetSavedDesignsManager().MoveBefore(design->UUID(), boost::uuids::nil_generator()());
        Populate();
    };

    // add all saved designs
    auto add_all_saved_designs_action = [&manager, &empire_id]() {
        DebugLogger() << "BasesListBox::BaseRightClicked AddAllSavedDesignsToDisplayedDesigns";
        // add the items to the end of the existing list, in correct order
        // TODO: think about adding them at the front.
        const auto design_uuids{manager.OrderedDesignUUIDs()};
        for (auto it = design_uuids.rbegin(); it != design_uuids.rend(); ++it)
            AddSavedDesignToDisplayedDesigns(*it, empire_id);
    };

    // toggle the option to add all saved designs at game start.
    const auto add_all = GetOptionsDB().Get<bool>("resource.shipdesign.saved.enabled");
    auto toggle_add_all_saved_game_start_action = [add_all]() {
        GetOptionsDB().Set<bool>("resource.shipdesign.saved.enabled", !add_all);
    };


    // create popup menu with a commands in it
    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
    if (design->Producible() && CanAddDesignToDisplayedDesigns(design))
        popup->AddMenuItem(GG::MenuItem(UserString("DESIGN_ADD"),                 false, false, add_design_action));
    popup->AddMenuItem(GG::MenuItem(UserString("DESIGN_WND_ADD_ALL_SAVED_NOW"),   false, false, add_all_saved_designs_action));
    popup->AddMenuItem(GG::MenuItem(true)); // separator
    popup->AddMenuItem(GG::MenuItem(UserString("DESIGN_WND_DELETE_SAVED"),        false, false, delete_saved_design_action));
    popup->AddMenuItem(GG::MenuItem(true)); // separator
    popup->AddMenuItem(GG::MenuItem(UserString("MOVE_UP_LIST_ITEM"),              false, false, movetotop_design_action));
    popup->AddMenuItem(GG::MenuItem(UserString("MOVE_DOWN_LIST_ITEM"),            false, false, movetobottom_design_action));
    popup->AddMenuItem(GG::MenuItem(true)); // separator
    popup->AddMenuItem(GG::MenuItem(UserString("DESIGN_WND_ADD_ALL_SAVED_START"), false, add_all,
                                   toggle_add_all_saved_game_start_action));

    popup->Run();
}

void EmptyHullsListBox::QueueItemMoved(const GG::ListBox::iterator row_it,
                                       const GG::ListBox::iterator original_position_it)
{
    const auto control = dynamic_cast<HullAndPartsListBoxRow*>(row_it->get());
    if (!control || !GetEmpire(EmpireID()))
        return;

    const std::string& hull_name = control->Hull();

    iterator insert_before_row = std::next(row_it);

    const auto insert_before_control = (insert_before_row == end()) ?
        nullptr : dynamic_cast<const HullAndPartsListBoxRow*>(insert_before_row->get());
    const auto& insert_before_hull = insert_before_control ? insert_before_control->Hull() : EMPTY_STRING;

    control->Resize(ListRowSize());

    GetDisplayedDesignsManager().InsertHullBefore(hull_name, insert_before_hull);
}

void CompletedDesignsListBox::QueueItemMoved(const GG::ListBox::iterator row_it,
                                             const GG::ListBox::iterator original_position_it)
{
    const auto control = dynamic_cast<BasesListBox::CompletedDesignListBoxRow*>(row_it->get());
    if (!control || !GetEmpire(EmpireID()))
        return;

    int design_id = control->DesignID();

    iterator insert_before_row = std::next(row_it);

    const auto insert_before_control = (insert_before_row == end()) ? nullptr :
        dynamic_cast<const BasesListBox::CompletedDesignListBoxRow*>(insert_before_row->get());
    int insert_before_id = insert_before_control
        ? insert_before_control->DesignID() : INVALID_DESIGN_ID;

    control->Resize(ListRowSize());

    GetDisplayedDesignsManager().MoveBefore(design_id, insert_before_id);
}

void SavedDesignsListBox::QueueItemMoved(const GG::ListBox::iterator row_it,
                                         const GG::ListBox::iterator original_position_it)
{
    const auto control = dynamic_cast<SavedDesignsListBox::SavedDesignListBoxRow*>(row_it->get());
    if (!control)
        return;

    const auto& uuid = control->DesignUUID();

    iterator insert_before_row = std::next(row_it);

    const auto insert_before_control = (insert_before_row == end()) ? nullptr :
        dynamic_cast<const SavedDesignsListBox::SavedDesignListBoxRow*>(insert_before_row->get());
    const auto next_uuid = insert_before_control
        ? insert_before_control->DesignUUID() : boost::uuids::nil_generator()();

    if (GetSavedDesignsManager().MoveBefore(uuid, next_uuid))
        control->Resize(ListRowSize());
}


//////////////////////////////////////////////////
// BasesListBox derived class rows              //
//////////////////////////////////////////////////
SavedDesignsListBox::SavedDesignListBoxRow::SavedDesignListBoxRow(
    GG::X w, GG::Y h, const ShipDesign& design) :
    BasesListBoxRow(w, h, design.Hull(), design.Name()),
    m_design_uuid(design.UUID())
{}

void SavedDesignsListBox::SavedDesignListBoxRow::CompleteConstruction() {
    BasesListBoxRow::CompleteConstruction();
    SetDragDropDataType(SAVED_DESIGN_ROW_DROP_STRING);
}

const boost::uuids::uuid SavedDesignsListBox::SavedDesignListBoxRow::DesignUUID() const {
    SavedDesignsManager& manager = GetSavedDesignsManager();
    const ShipDesign* design = manager.GetDesign(m_design_uuid);
    if (!design) {
        ErrorLogger() << "Saved ship design missing with uuid " << m_design_uuid;
        return boost::uuids::uuid{};
    }
    return design->UUID();
}

const std::string& SavedDesignsListBox::SavedDesignListBoxRow::DesignName() const {
    SavedDesignsManager& manager = GetSavedDesignsManager();
    const ShipDesign* design = manager.GetDesign(m_design_uuid);
    if (!design)
        return EMPTY_STRING;
    return design->Name();
}

const std::string& SavedDesignsListBox::SavedDesignListBoxRow::Description() const {
    SavedDesignsManager& manager = GetSavedDesignsManager();
    const ShipDesign* design = manager.GetDesign(m_design_uuid);
    if (!design)
        return EMPTY_STRING;
    return design->Description();
}

bool SavedDesignsListBox::SavedDesignListBoxRow::LookupInStringtable() const {
    SavedDesignsManager& manager = GetSavedDesignsManager();
    const ShipDesign* design = manager.GetDesign(m_design_uuid);
    if (!design)
        return false;
    return design->LookupInStringtable();
}


//////////////////////////////////////////////////
// DesignWnd::BaseSelector                      //
//////////////////////////////////////////////////
class DesignWnd::BaseSelector : public CUIWnd {
public:
    BaseSelector(std::string_view config_name);
    void CompleteConstruction() override;

    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    void Reset();
    void ToggleAvailability(const Availability::Enum type);
    void SetEmpireShown(int empire_id, bool refresh_list);
    void EnableOrderIssuing(bool enable);

    mutable boost::signals2::signal<void (int)>                         DesignSelectedSignal;
    mutable boost::signals2::signal<void (int)>                         DesignUpdatedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::vector<std::string>&)>
                                                                        DesignComponentsSelectedSignal;
    mutable boost::signals2::signal<void (const boost::uuids::uuid&)>   SavedDesignSelectedSignal;
    mutable boost::signals2::signal<void (const ShipDesign*)>           DesignClickedSignal;
    mutable boost::signals2::signal<void (const ShipHull*)>             HullClickedSignal;

    enum class BaseSelectorTab : std::size_t {Hull, Current, Saved, Monster, All};
    mutable boost::signals2::signal<void (const BaseSelectorTab)>       TabChangedSignal;

private:
    void DoLayout();

    std::shared_ptr<GG::TabWnd>                 m_tabs;
    std::shared_ptr<EmptyHullsListBox>          m_hulls_list;           // empty hulls on which a new design can be based
    std::shared_ptr<CompletedDesignsListBox>    m_designs_list;         // designs this empire has created or learned how to make
    std::shared_ptr<SavedDesignsListBox>        m_saved_designs_list;   // designs saved to files
    std::shared_ptr<MonstersListBox>            m_monsters_list;        // monster designs
    std::shared_ptr<AllDesignsListBox>          m_all_list;             // all designs known to empire

    // Holds the state of the availabilities filter.
    AvailabilityManager                         m_availabilities_state{false, true, false};

    std::tuple<std::shared_ptr<CUIStateButton>, std::shared_ptr<CUIStateButton>,
               std::shared_ptr<CUIStateButton>> m_availabilities_buttons;
};

DesignWnd::BaseSelector::BaseSelector(std::string_view config_name) :
    CUIWnd(UserString("DESIGN_WND_STARTS"),
           GG::INTERACTIVE | GG::RESIZABLE | GG::ONTOP | GG::DRAGABLE | PINABLE,
           config_name)
{}

void DesignWnd::BaseSelector::CompleteConstruction() {
    auto& [obsolete_button, available_button, unavailable_button] = m_availabilities_buttons;

    obsolete_button = GG::Wnd::Create<CUIStateButton>(UserString("PRODUCTION_WND_AVAILABILITY_OBSOLETE"),
                                                      GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(obsolete_button);
    obsolete_button->CheckedSignal.connect([this](auto) { ToggleAvailability(Availability::Obsolete); });
    obsolete_button->SetCheck(m_availabilities_state.GetAvailability(Availability::Obsolete));

    available_button = GG::Wnd::Create<CUIStateButton>(UserString("PRODUCTION_WND_AVAILABILITY_AVAILABLE"),
                                                       GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(available_button);
    available_button->CheckedSignal.connect([this](auto) { ToggleAvailability(Availability::Available); });
    available_button->SetCheck(m_availabilities_state.GetAvailability(Availability::Available));

    unavailable_button = GG::Wnd::Create<CUIStateButton>(UserString("PRODUCTION_WND_AVAILABILITY_UNAVAILABLE"),
                                                         GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(unavailable_button);
    unavailable_button->CheckedSignal.connect([this](auto) { ToggleAvailability(Availability::Future); });
    unavailable_button->SetCheck(m_availabilities_state.GetAvailability(Availability::Future));


    m_tabs = GG::Wnd::Create<GG::TabWnd>(GG::X(5), GG::Y(2), GG::X(10), GG::Y(10), ClientUI::GetFont(),
                                         ClientUI::WndColor(), ClientUI::TextColor());
    m_tabs->TabChangedSignal.connect([this](auto) { Reset(); });
    AttachChild(m_tabs);

    m_hulls_list = GG::Wnd::Create<EmptyHullsListBox>(m_availabilities_state, HULL_PARTS_ROW_DROP_TYPE_STRING);
    m_hulls_list->Resize(GG::Pt(GG::X(10), GG::Y(10)));
    m_tabs->AddWnd(m_hulls_list, UserString("DESIGN_WND_HULLS"));
    m_hulls_list->DesignComponentsSelectedSignal.connect(DesignWnd::BaseSelector::DesignComponentsSelectedSignal);
    m_hulls_list->HullClickedSignal.connect(DesignWnd::BaseSelector::HullClickedSignal);

    m_designs_list = GG::Wnd::Create<CompletedDesignsListBox>(m_availabilities_state, COMPLETE_DESIGN_ROW_DROP_STRING);
    m_designs_list->Resize(GG::Pt(GG::X(10), GG::Y(10)));
    m_tabs->AddWnd(m_designs_list, UserString("DESIGN_WND_FINISHED_DESIGNS"));
    m_designs_list->DesignSelectedSignal.connect(DesignWnd::BaseSelector::DesignSelectedSignal);
    m_designs_list->DesignUpdatedSignal.connect(DesignWnd::BaseSelector::DesignUpdatedSignal);
    m_designs_list->DesignClickedSignal.connect(DesignWnd::BaseSelector::DesignClickedSignal);

    m_saved_designs_list = GG::Wnd::Create<SavedDesignsListBox>(m_availabilities_state, SAVED_DESIGN_ROW_DROP_STRING);
    m_saved_designs_list->Resize(GG::Pt(GG::X(10), GG::Y(10)));
    m_tabs->AddWnd(m_saved_designs_list, UserString("DESIGN_WND_SAVED_DESIGNS"));
    m_saved_designs_list->SavedDesignSelectedSignal.connect(DesignWnd::BaseSelector::SavedDesignSelectedSignal);
    m_saved_designs_list->DesignClickedSignal.connect(DesignWnd::BaseSelector::DesignClickedSignal);

    m_monsters_list = GG::Wnd::Create<MonstersListBox>(m_availabilities_state);
    m_monsters_list->Resize(GG::Pt(GG::X(10), GG::Y(10)));
    m_tabs->AddWnd(m_monsters_list, UserString("DESIGN_WND_MONSTERS"));
    m_monsters_list->DesignSelectedSignal.connect(DesignWnd::BaseSelector::DesignSelectedSignal);
    m_monsters_list->DesignClickedSignal.connect(DesignWnd::BaseSelector::DesignClickedSignal);

    m_all_list = GG::Wnd::Create<AllDesignsListBox>(m_availabilities_state);
    m_all_list->Resize(GG::Pt(GG::X(10), GG::Y(10)));
    m_tabs->AddWnd(m_all_list, UserString("DESIGN_WND_ALL"));
    m_all_list->DesignSelectedSignal.connect(DesignWnd::BaseSelector::DesignSelectedSignal);
    m_all_list->DesignClickedSignal.connect(DesignWnd::BaseSelector::DesignClickedSignal);


    CUIWnd::CompleteConstruction();

    SetMinSize(BASES_MIN_SIZE);

    DoLayout();
    SaveDefaultedOptions();
}

void DesignWnd::BaseSelector::SizeMove(GG::Pt ul, GG::Pt lr) {
    const auto old_size = Size();
    CUIWnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void DesignWnd::BaseSelector::Reset() {
    ScopedTimer scoped_timer("BaseSelector::Reset");

    const int empire_id = GGHumanClientApp::GetApp()->EmpireID();
    SetEmpireShown(empire_id, false);

    if (auto base_box = dynamic_cast<BasesListBox*>(m_tabs->CurrentWnd()))
        base_box->Populate();

    // Signal the type of tab selected
    auto tab_type = BaseSelectorTab(m_tabs->CurrentWndIndex());
    switch (tab_type) {
    case BaseSelectorTab::Hull:
    case BaseSelectorTab::Current:
    case BaseSelectorTab::Saved:
    case BaseSelectorTab::Monster:
    case BaseSelectorTab::All:
        TabChangedSignal(tab_type);
        break;
    default:
        break;
    }
}

void DesignWnd::BaseSelector::SetEmpireShown(int empire_id, bool refresh_list) {
    m_hulls_list->SetEmpireShown(empire_id, refresh_list);
    m_designs_list->SetEmpireShown(empire_id, refresh_list);
    m_saved_designs_list->SetEmpireShown(empire_id, refresh_list);
}

void DesignWnd::BaseSelector::ToggleAvailability(Availability::Enum type) {
    if (type != Availability::Enum::Obsolete &&
        type != Availability::Enum::Available &&
        type != Availability::Enum::Future)
    { return; }
    m_availabilities_state.ToggleAvailability(type);

    const std::array<std::shared_ptr<CUIStateButton>, 3> buttons{
        std::get<0>(m_availabilities_buttons),
        std::get<1>(m_availabilities_buttons),
        std::get<2>(m_availabilities_buttons)};
    bool state = m_availabilities_state.GetAvailability(type);
    const auto& button = buttons[static_cast<std::size_t>(type)];

    button->SetCheck(state);

    m_hulls_list->Populate();
    m_designs_list->Populate();
    m_saved_designs_list->Populate();
}

void DesignWnd::BaseSelector::EnableOrderIssuing(bool enable) {
    m_hulls_list->EnableOrderIssuing(enable);
    m_designs_list->EnableOrderIssuing(enable);
    m_saved_designs_list->EnableOrderIssuing(enable);
    m_monsters_list->EnableOrderIssuing(enable);
}

void DesignWnd::BaseSelector::DoLayout() {
    static constexpr GG::X LEFT_PAD{5};
    static constexpr GG::Y TOP_PAD{2};
    const GG::X AVAILABLE_WIDTH = ClientWidth() - 2*LEFT_PAD;
    static constexpr int BUTTON_SEPARATION = 3;
    const GG::X BUTTON_WIDTH = (AVAILABLE_WIDTH - 2*BUTTON_SEPARATION) / 3;
    const int PTS = ClientUI::Pts();
    const GG::Y BUTTON_HEIGHT{PTS * 2};

    GG::Y top(TOP_PAD);
    GG::X left(LEFT_PAD);

    auto& [obsolete_button, available_button, unavailable_button] = m_availabilities_buttons;
    obsolete_button->SizeMove(GG::Pt(left, top), GG::Pt(left + BUTTON_WIDTH, top + BUTTON_HEIGHT));
    left = left + BUTTON_WIDTH + BUTTON_SEPARATION;
    available_button->SizeMove(GG::Pt(left, top), GG::Pt(left + BUTTON_WIDTH, top + BUTTON_HEIGHT));
    left = left + BUTTON_WIDTH + BUTTON_SEPARATION;
    unavailable_button->SizeMove(GG::Pt(left, top), GG::Pt(left + BUTTON_WIDTH, top + BUTTON_HEIGHT));
    left = LEFT_PAD;
    top = top + BUTTON_HEIGHT + BUTTON_SEPARATION;

    m_tabs->SizeMove(GG::Pt(left, top), ClientSize() - GG::Pt(LEFT_PAD, TOP_PAD));
}


//////////////////////////////////////////////////
// SlotControl                                  //
//////////////////////////////////////////////////
/** UI representation and drop-target for slots of a design.  PartControl may
  * be dropped into slots to add the corresponding parts to the ShipDesign, or
  * the part may be set programmatically with SetPart(). */
class SlotControl : public GG::Control {
public:
    SlotControl();
    SlotControl(double x, double y, ShipSlotType slot_type);
    void CompleteConstruction() override;

    ShipSlotType    SlotType() const;
    double          XPositionFraction() const;
    double          YPositionFraction() const;
    const ShipPart* GetPart() const;

    void StartingChildDragDrop(const GG::Wnd* wnd, GG::Pt offset) override;
    void CancellingChildDragDrop(const std::vector<const GG::Wnd*>& wnds) override;
    void AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds,
                     GG::Flags<GG::ModKey> mod_keys) override;
    void ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination) override;
    void DragDropEnter(GG::Pt pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                       GG::Flags<GG::ModKey> mod_keys) override;
    void DragDropLeave() override;

    void Render() override;
    void Highlight(bool actually = true);

    //! Used to programmatically set the ShipPart in this slot.
    //! Does not emit signal.
    void SetPart(const std::string& part_name);
    //! Used to programmatically set the ShipPart in this slot.
    //! Does not emit signal.
    void SetPart(const ShipPart* part = nullptr);

    /** emitted when the contents of a slot are altered by dragging a
      * PartControl in or out of the slot.  signal should be caught
      * and the slot contents set using SetPart accordingly. second
      * parameter indicates that CTRL was held while dropping. */
    mutable boost::signals2::signal<void (const ShipPart*, bool)> SlotContentsAlteredSignal;

    mutable boost::signals2::signal<void (const ShipPart*, GG::Flags<GG::ModKey>)> ShipPartClickedSignal;

protected:
    bool EventFilter(GG::Wnd* w, const GG::WndEvent& event) override;
    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) const override;

private:
    bool                                m_highlighted = false;
    ShipSlotType                        m_slot_type = ShipSlotType::INVALID_SHIP_SLOT_TYPE;
    double                              m_x_position_fraction = 0.4;    //!< position on hull image where slot should be shown, as a fraction of that image's size
    double                              m_y_position_fraction = 0.4;
    std::shared_ptr<PartControl>        m_part_control;
    std::shared_ptr<GG::StaticGraphic>  m_background;
};

SlotControl::SlotControl() :
    GG::Control(GG::X0, GG::Y0, SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT, GG::INTERACTIVE)
{}

SlotControl::SlotControl(double x, double y, ShipSlotType slot_type) :
    GG::Control(GG::X0, GG::Y0, SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT, GG::INTERACTIVE),
    m_slot_type(slot_type),
    m_x_position_fraction(x),
    m_y_position_fraction(y)
{}

void SlotControl::CompleteConstruction() {
    GG::Control::CompleteConstruction();

    m_background = GG::Wnd::Create<GG::StaticGraphic>(
        SlotBackgroundTexture(m_slot_type), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_background->Resize(GG::Pt(SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT));
    m_background->Show();
    AttachChild(m_background);

    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    // set up empty slot tool tip
    auto& title_text = (m_slot_type == ShipSlotType::SL_EXTERNAL) ? UserString("SL_EXTERNAL") :
                       (m_slot_type == ShipSlotType::SL_INTERNAL) ? UserString("SL_INTERNAL") :
                       (m_slot_type == ShipSlotType::SL_CORE) ? UserString("SL_CORE") : "";

    SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
        SlotBackgroundTexture(m_slot_type),
        title_text,
        UserString("SL_TOOLTIP_DESC")
    ));
}

bool SlotControl::EventFilter(GG::Wnd* w, const GG::WndEvent& event) {
    if (w == this)
        return false;

    switch (event.Type()) {
    case GG::WndEvent::EventType::DragDropEnter:
    case GG::WndEvent::EventType::DragDropHere:
    case GG::WndEvent::EventType::CheckDrops:
    case GG::WndEvent::EventType::DragDropLeave:
    case GG::WndEvent::EventType::DragDroppedOn:
        HandleEvent(event);
        return true;
        break;
    default:
        return false;
    }
}

void SlotControl::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                                  GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) const
{
    for (DropsAcceptableIter it = first; it != last; ++it)
        it->second = false;

    // if more than one control dropped somehow, reject all
    if (std::distance(first, last) != 1)
        return;

    for (DropsAcceptableIter it = first; it != last; ++it) {
        if (it->first->DragDropDataType() != PART_CONTROL_DROP_TYPE_STRING)
            continue;
        const auto part_control = dynamic_cast<const PartControl* const>(it->first);
        const ShipPart* part = part_control->Part();
        if (part &&
            part->CanMountInSlotType(m_slot_type) &&
            part_control != m_part_control.get())
        {
            it->second = true;
            return;
        }
    }
}

ShipSlotType SlotControl::SlotType() const
{ return m_slot_type; }

double SlotControl::XPositionFraction() const
{ return m_x_position_fraction; }

double SlotControl::YPositionFraction() const
{ return m_y_position_fraction; }

const ShipPart* SlotControl::GetPart() const {
    if (m_part_control)
        return m_part_control->Part();
    else
        return nullptr;
}

void SlotControl::StartingChildDragDrop(const GG::Wnd* wnd, GG::Pt offset) {
    if (!m_part_control)
        return;

    const auto control = dynamic_cast<const PartControl*>(wnd);
    if (!control)
        return;

    if (control == m_part_control.get())
        m_part_control->Hide();
}

void SlotControl::CancellingChildDragDrop(const std::vector<const GG::Wnd*>& wnds) {
    if (!m_part_control)
        return;

    for (const auto& wnd : wnds) {
        const auto control = dynamic_cast<const PartControl*>(wnd);
        if (!control)
            continue;

        if (control == m_part_control.get())
            m_part_control->Show();
    }
}

void SlotControl::AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds,
                              GG::Flags<GG::ModKey> mod_keys)
{
    if (wnds.size() != 1)
        ErrorLogger() << "SlotControl::AcceptDrops given multiple wnds unexpectedly...";

    if (const PartControl* control = dynamic_cast<const PartControl*>(wnds.front().get()))
        if (const ShipPart* part = control->Part())
            SlotContentsAlteredSignal(part, mod_keys & GG::MOD_KEY_CTRL);
}

void SlotControl::ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds,
                                      const GG::Wnd* destination)
{
    if (wnds.empty())
        return;
    const GG::Wnd* wnd = wnds.front();
    const auto part_control = dynamic_cast<const PartControl*>(wnd);
    if (part_control != m_part_control.get())
        return;
    DetachChildAndReset(m_part_control);
    SlotContentsAlteredSignal(nullptr, false);
}

void SlotControl::DragDropEnter(GG::Pt pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                                GG::Flags<GG::ModKey> mod_keys) {

    if (drop_wnds_acceptable.empty())
        return;

    DropsAcceptable(drop_wnds_acceptable.begin(), drop_wnds_acceptable.end(), pt, mod_keys);

    // Note:  If this SlotControl is being dragged over this indicates the dragged part would
    //        replace this part.
    if (drop_wnds_acceptable.begin()->second && m_part_control)
        m_part_control->Hide();
}

void SlotControl::DragDropLeave() {
    // Note:  If m_part_control is being dragged, this does nothing, because it is detached.
    //        If this SlotControl is being dragged over this indicates the dragged part would
    //        replace this part.
    if (m_part_control && !GG::GUI::GetGUI()->DragDropWnd(m_part_control.get()))
        m_part_control->Show();
}

void SlotControl::Render()
{}

void SlotControl::Highlight(bool actually)
{ m_highlighted = actually; }

void SlotControl::SetPart(const std::string& part_name)
{ SetPart(GetShipPart(part_name)); }

void SlotControl::SetPart(const ShipPart* part) {
    // remove existing part control, if any
    DetachChildAndReset(m_part_control);

    if (!part)
        return;

    // create new part control for passed in part
    m_part_control = GG::Wnd::Create<PartControl>(part);
    AttachChild(m_part_control);
    m_part_control->InstallEventFilter(shared_from_this());

    // single click shows encyclopedia data
    m_part_control->ClickedSignal.connect(ShipPartClickedSignal);

    // double click clears slot
    m_part_control->DoubleClickedSignal.connect(
        [this](const ShipPart*){ this->SlotContentsAlteredSignal(nullptr, false); });
    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    // set part occupying slot's tool tip to say slot type
    std::string title_text;
    if (m_slot_type == ShipSlotType::SL_EXTERNAL)
        title_text = UserString("SL_EXTERNAL");
    else if (m_slot_type == ShipSlotType::SL_INTERNAL)
        title_text = UserString("SL_INTERNAL");
    else if (m_slot_type == ShipSlotType::SL_CORE)
        title_text = UserString("SL_CORE");

    m_part_control->SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
        ClientUI::PartIcon(part->Name()),
        UserString(part->Name()) + " (" + title_text + ")",
        UserString(part->Description())
    ));
}

void PartsListBox::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                                   GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) const
{
    // PartsListBox accepts parts that are being removed from a SlotControl

    for (DropsAcceptableIter it = first; it != last; ++it)
        it->second = false;

    // if more than one control dropped somehow, reject all
    if (std::distance(first, last) != 1)
        return;

    const auto&& parent = first->first->Parent();
    if (first->first->DragDropDataType() == PART_CONTROL_DROP_TYPE_STRING
        && parent
        && dynamic_cast<const SlotControl*>(parent.get()))
    {
        first->second = true;
    }
}


//////////////////////////////////////////////////
// DesignWnd::MainPanel                         //
//////////////////////////////////////////////////
class DesignWnd::MainPanel : public CUIWnd {
public:

    /** I18nString stores a string that might be in the stringtable. */
    class I18nString {
    public:
        I18nString (bool is_in_stringtable, const std::string& text) :
            m_is_in_stringtable(is_in_stringtable), m_text(text)
        {}
        I18nString (bool is_in_stringtable, std::string&& text) :
            m_is_in_stringtable(is_in_stringtable), m_text(std::move(text))
        {}

        /** Return the text a displayed. */
        const std::string& DisplayText() const
        { return m_is_in_stringtable ? UserString(m_text) : m_text; }

        /** Return the text as stored. */
        const std::string& StoredString() const
        { return m_text; }

        bool IsInStringtable() const
        { return m_is_in_stringtable; }

    private:
        const bool m_is_in_stringtable;
        const std::string m_text;
    };

    MainPanel(std::string_view config_name);
    void CompleteConstruction() override;

    /** If editing a current design return a ShipDesign* otherwise boost::none. */
    boost::optional<const ShipDesign*> EditingCurrentDesign() const;
    /** If editing a saved design return a ShipDesign* otherwise boost::none. */
    boost::optional<const ShipDesign*> EditingSavedDesign() const;

    std::vector<std::string>            Parts() const;              //!< names of parts in slots of current shown design.  empty slots are represented with empty stri
    const std::string&                  Hull() const noexcept;      //!< name of hull of current shown design
    bool                                IsDesignNameValid() const;  //!< checks design name validity
    /** Validated name and description.  If the design is a saved design then either both
      * or neither will be stringtable values.*/
    std::pair<I18nString, I18nString>   ValidatedNameAndDescription() const;
    const I18nString                    ValidatedDesignName() const;//!< name currently entered for design or valid default
    const I18nString                    DesignDescription() const;  //!< description currently entered for design

    /** Returns a pointer to the design currently being modified (if any).  May
      * return an empty pointer if not currently modifying a design. */
    std::shared_ptr<const ShipDesign>   GetIncompleteDesign() const;
    boost::optional<int>                GetReplacedDesignID() const;//!< ID of completed design selected to be replaced.

    /** If a design with the same hull and parts is registered with the empire then return the
      * design, otherwise return boost::none. */
    boost::optional<const ShipDesign*>  CurrentDesignIsRegistered() const;

    void LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;

    void AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys) override;

    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    void Sanitize();

    //! Puts specified part in specified slot.  Does nothing if slot is out of
    //! range of available slots for current hull
    void SetPart(const std::string& part_name, unsigned int slot);

    //! Sets the part in @p slot to @p part and emits and signal if requested.
    //! Changes all similar parts if @p change_all_similar_parts.
    void SetPart(const ShipPart* part, unsigned int slot, bool emit_signal = false, bool change_all_similar_parts = false);

    //! Puts specified parts in slots.  Attempts to put each part into the slot
    //! corresponding to its place in the passed vector.  If a part cannot be
    //! placed, it is ignored.  More parts than there are slots available are
    //! ignored, and slots for which there are insufficient parts in the passed
    //! vector are unmodified.
    void SetParts(const std::vector<std::string>& parts);

    //! Attempts to add the specified part to the design, if possible.  Will
    //! first attempt to add part to an empty slot of the appropriate type, and
    //! if no appropriate slots are available, may or may not move other parts
    //! around within the design to open up a compatible slot in which to add
    //! this part (and then add it).  May also do nothing.
    void AddPart(const ShipPart* part);
    bool CanPartBeAdded(const ShipPart* part);

    void ClearParts();                                               //!< removes all parts from design.  hull is not altered
    /** Remove parts called \p part_name*/
    void ClearPart(const std::string& part_name);

    /** Set the design hull \p hull_name, displaying appropriate background image and creating
        appropriate SlotControls.  If \p signal is false do not emit the the
        DesignChangedSignal(). */
    void SetHull(const std::string& hull_name, bool signal = true);
    void SetHull(const ShipHull* hull, bool signal = true);
    void SetDesign(const ShipDesign* ship_design);                   //!< sets the displayed design by setting the appropriate hull and parts
    void SetDesign(int design_id);                                   //!< sets the displayed design by setting the appropriate hull and parts
    /** SetDesign to the design with \p uuid from the SavedDesignManager. */
    void SetDesign(const boost::uuids::uuid& uuid);

    /** sets design hull and parts to those specified */
    void SetDesignComponents(const std::string& hull,
                             const std::vector<std::string>& parts);
    void SetDesignComponents(const std::string& hull,
                             const std::vector<std::string>& parts,
                             const std::string& name,
                             const std::string& desc);

    /** Responds to the design being changed **/
    void DesignChanged();

    /** Add a design. */
    std::pair<int, boost::uuids::uuid> AddDesign();

    /** Replace an existing design.*/
    void ReplaceDesign();

    void ToggleDescriptionEditor();

    void HighlightSlotType(std::vector<ShipSlotType>& slot_types);   //!< renders slots of the indicated types differently, perhaps to indicate that that those slots can be drop targets for a particular part?

    /** Track changes in base type. */
    void HandleBaseTypeChange(const DesignWnd::BaseSelector::BaseSelectorTab base_type);

    /** emitted when the design is changed (by adding or removing parts, not
      * name or description changes) */
    mutable boost::signals2::signal<void ()>                DesignChangedSignal;

    /** emitted when the design name is changed */
    mutable boost::signals2::signal<void ()>                DesignNameChangedSignal;

    //! Propagates signals from contained SlotControls that signal that a part
    //! has been clicked.
    mutable boost::signals2::signal<void (const ShipPart*, GG::Flags<GG::ModKey>)> ShipPartClickedSignal;

    mutable boost::signals2::signal<void (const ShipHull*)> ShipHullClickedSignal;

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
                         GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) const override;

private:
    void            Populate();                         //!< creates and places SlotControls for current hull
    void            DoLayout();                         //!< positions buttons, text entry boxes and SlotControls
    void            DesignNameChanged();                //!< responds to the design name being changed
    void            RefreshIncompleteDesign() const;
    std::string     GetCleanDesignDump(const ShipDesign* ship_design);  //!< similar to ship design dump but without 'lookup_strings', icon and model entries

    //! Adds part to slot number
    bool AddPartEmptySlot(const ShipPart* part, int slot_number);

    //! Swaps part in slot # pair.first to slot # pair.second, adds given part
    //! to slot # pair.first
    bool AddPartWithSwapping(const ShipPart* part, std::pair<int, int> swap_and_empty_slot);

    //! Determines if a part can be added to any empty slot, returns the slot
    //! index if possible, otherwise -1
    int FindEmptySlotForPart(const ShipPart* part);

    void DesignNameEditedSlot(const std::string& new_name);  //!< triggered when m_design_name's AfterTextChangedSignal fires. Used for basic name validation.

    //! Determines if a part can be added to a slot with swapping, returns
    //! a pair containing the slot to swap and an empty slot, otherwise a pair
    //! with -1.
    //! This function only tries to find a way to add the given part by
    //! swapping a part already in a slot to an empty slot.
    //! If theres an open slot that the given part could go into but all of the
    //! occupied slots contain parts that can't swap into the open slot
    //! This function will indicate that it could not add the part, even though
    //! adding the part is possible
    std::pair<int, int> FindSlotForPartWithSwapping(const ShipPart* part);

    const ShipHull*                             m_hull = nullptr;
    std::vector<std::shared_ptr<SlotControl>>   m_slots;
    boost::optional<int>                        m_replaced_design_id = boost::none;     // The design id if this design is replacable
    boost::optional<boost::uuids::uuid>         m_replaced_design_uuid = boost::none;   // The design uuid if this design is replacable

    /// Whether to add new designs to current or saved designs
    /// This tracks the last relevant selected tab in the base selector
    DesignWnd::BaseSelector::BaseSelectorTab    m_type_to_create = DesignWnd::BaseSelector::BaseSelectorTab::Current;

    mutable std::shared_ptr<ShipDesign>         m_incomplete_design;

    std::shared_ptr<GG::StaticGraphic>          m_background_image;
    std::shared_ptr<GG::Label>                  m_design_name_label;
    std::shared_ptr<CUIEdit>                    m_design_name;
    std::shared_ptr<GG::StateButton>            m_design_description_toggle;
    std::shared_ptr<GG::MultiEdit>              m_design_description_edit;
    std::shared_ptr<GG::Button>                 m_replace_button;
    std::shared_ptr<GG::Button>                 m_confirm_button;
    std::shared_ptr<GG::Button>                 m_clear_button;
    bool                                        m_disabled_by_name = false; // if the design confirm button is currently disabled due to empty name
    bool                                        m_disabled_by_part_conflict = false;

    boost::signals2::scoped_connection          m_empire_designs_changed_signal;
};

DesignWnd::MainPanel::MainPanel(std::string_view config_name) :
    CUIWnd(UserString("DESIGN_WND_MAIN_PANEL_TITLE"),
           GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE,
           config_name)
{}

void DesignWnd::MainPanel::CompleteConstruction() {
    SetChildClippingMode(ChildClippingMode::ClipToClient);

    m_design_name_label = GG::Wnd::Create<CUILabel>(UserString("DESIGN_WND_DESIGN_NAME"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_design_name = GG::Wnd::Create<CUIEdit>(UserString("DESIGN_NAME_DEFAULT"));
    m_design_name->DisallowChars(formatting_chars);
    m_design_description_toggle = GG::Wnd::Create<CUIStateButton>(UserString("DESIGN_WND_DESIGN_DESCRIPTION"),GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    m_design_description_edit = GG::Wnd::Create<CUIMultiEdit>(UserString("DESIGN_DESCRIPTION_DEFAULT"));
    m_design_description_edit->SetTextFormat(m_design_description_edit->GetTextFormat() | GG::FORMAT_IGNORETAGS);
    m_replace_button = Wnd::Create<CUIButton>(UserString("DESIGN_WND_UPDATE_SAVED"));
    m_confirm_button = Wnd::Create<CUIButton>(UserString("DESIGN_WND_ADD_FINISHED"));
    m_clear_button = Wnd::Create<CUIButton>(UserString("DESIGN_WND_CLEAR"));

    m_replace_button->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_confirm_button->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    AttachChild(m_design_name_label);
    AttachChild(m_design_name);
    AttachChild(m_design_description_toggle);
    AttachChild(m_design_description_edit);
    AttachChild(m_replace_button);
    AttachChild(m_confirm_button);
    AttachChild(m_clear_button);

    m_clear_button->LeftClickedSignal.connect([this]() { ClearParts(); });
    m_design_name->EditedSignal.connect([this](auto& new_name) { DesignNameEditedSlot(new_name); });
    m_replace_button->LeftClickedSignal.connect(DesignReplacedSignal);
    m_confirm_button->LeftClickedSignal.connect(DesignConfirmedSignal);
    m_design_description_toggle->CheckedSignal.connect([this](auto) { ToggleDescriptionEditor(); });
    DesignChangedSignal.connect([this]() { DesignChanged(); });
    DesignReplacedSignal.connect([this]() { ReplaceDesign();});
    DesignConfirmedSignal.connect([this]() { AddDesign(); });

    DesignChanged(); // Initialize components that rely on the current state of the design.

    CUIWnd::CompleteConstruction();
    SetMinSize(MAIN_PANEL_MIN_SIZE);

    DoLayout();
    SaveDefaultedOptions();
}

boost::optional<const ShipDesign*> DesignWnd::MainPanel::EditingSavedDesign() const {
    // Is there a valid replaced_uuid that indexes a saved design?
    if (!m_replaced_design_uuid)
        return boost::none;

    if (auto* maybe_design = GetSavedDesignsManager().GetDesign(*m_replaced_design_uuid))
        return maybe_design;
    return boost::none;
}

boost::optional<const ShipDesign*> DesignWnd::MainPanel::EditingCurrentDesign() const {
    // Is there a valid replaced_uuid that indexes a saved design?
    if (!m_replaced_design_id || !GetDisplayedDesignsManager().IsKnown(*m_replaced_design_id))
        return boost::none;

    if (auto* maybe_design = GetUniverse().GetShipDesign(*m_replaced_design_id))
        return maybe_design;
    return boost::none;
}

std::vector<std::string> DesignWnd::MainPanel::Parts() const {
    std::vector<std::string> retval;
    retval.reserve(m_slots.size());
    for (const auto& slot : m_slots) {
        const ShipPart* part = slot->GetPart();
        retval.emplace_back(part ? part->Name() : EMPTY_STRING);
    }
    return retval;
}

const std::string& DesignWnd::MainPanel::Hull() const noexcept
{ return m_hull ? m_hull->Name() : EMPTY_STRING; }

bool DesignWnd::MainPanel::IsDesignNameValid() const {
    const auto& name = m_design_name->Text();
    if (name.empty())
        return false;

    // disallow formatting characters
    if (std::any_of(name.begin(), name.end(),
                    [](const auto c) {
                        return std::any_of(formatting_chars.begin(), formatting_chars.end(),
                                           [c](const auto f) { return f == c; });
                    }))
    { return false; }

    // disallow leading and trailing spaces
    if (name.front() == ' ' || name.back() == ' ')
        return false;

    return true;
}

std::pair<DesignWnd::MainPanel::I18nString, DesignWnd::MainPanel::I18nString>
DesignWnd::MainPanel::ValidatedNameAndDescription() const
{
    const auto maybe_saved = EditingSavedDesign();

    // Determine if the title and descrition could both be string table values.

    // Is the title a stringtable index or the same as the saved designs value
    const std::string name_index =
        (UserStringExists(m_design_name->Text()) ? m_design_name->Text() :
         ((maybe_saved && (*maybe_saved)->LookupInStringtable()
           && (m_design_name->Text() == (*maybe_saved)->Name())) ? (*maybe_saved)->Name(false) : ""));

    // Is the descrition a stringtable index or the same as the saved designs value
    const std::string desc_index =
        (UserStringExists(m_design_description_edit->Text()) ? m_design_description_edit->Text() :
         ((maybe_saved && (*maybe_saved)->LookupInStringtable()
           && (m_design_description_edit->Text() == (*maybe_saved)->Description())) ? (*maybe_saved)->Description(false) : ""));

    // Are both the title and the description string table lookup values
    if (!name_index.empty() && !desc_index.empty())
        return std::pair(
            I18nString(true, name_index),
            I18nString(true, desc_index));

    return std::pair(
        I18nString(false, (IsDesignNameValid()) ? m_design_name->Text() : UserString("DESIGN_NAME_DEFAULT")),
        I18nString(false, m_design_description_edit->Text()));
}

const DesignWnd::MainPanel::I18nString DesignWnd::MainPanel::ValidatedDesignName() const
{ return ValidatedNameAndDescription().first; }

const DesignWnd::MainPanel::I18nString DesignWnd::MainPanel::DesignDescription() const
{ return ValidatedNameAndDescription().second; }

std::shared_ptr<const ShipDesign> DesignWnd::MainPanel::GetIncompleteDesign() const {
    RefreshIncompleteDesign();
    return m_incomplete_design;
}

boost::optional<int> DesignWnd::MainPanel::GetReplacedDesignID() const
{ return m_replaced_design_id; }

boost::optional<const ShipDesign*> DesignWnd::MainPanel::CurrentDesignIsRegistered() const {
    int empire_id = GGHumanClientApp::GetApp()->EmpireID();
    const auto empire = GetEmpire(empire_id);
    if (!empire) {
        ErrorLogger() << "DesignWnd::MainPanel::CurrentDesignIsRegistered couldn't get the current empire.";
        return boost::none;
    }

    if (const auto& cur_design = GetIncompleteDesign()) {
        for (const auto design_id : empire->ShipDesigns()) {
            const auto ship_design = GetUniverse().GetShipDesign(design_id);
            if (*ship_design == *cur_design.get())
                return ship_design;
        }
    }
    return boost::none;
}

void DesignWnd::MainPanel::LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    if (m_hull)
        ShipHullClickedSignal(m_hull);
    CUIWnd::LClick(pt, mod_keys);
}

void DesignWnd::MainPanel::SizeMove(GG::Pt ul, GG::Pt lr) {
    CUIWnd::SizeMove(ul, lr);
    DoLayout();
}

void DesignWnd::MainPanel::Sanitize() {
    SetHull(nullptr, false);
    m_design_name->SetText(UserString("DESIGN_NAME_DEFAULT"));
    m_design_description_edit->SetText(UserString("DESIGN_DESCRIPTION_DEFAULT"));
    // disconnect old empire design signal
    m_empire_designs_changed_signal.disconnect();
}

void DesignWnd::MainPanel::SetPart(const std::string& part_name, unsigned int slot)
{ SetPart(GetShipPart(part_name), slot); }

void DesignWnd::MainPanel::SetPart(const ShipPart* part, unsigned int slot,
                                   bool emit_signal,
                                   bool change_all_similar_parts)
{
    //DebugLogger() << "DesignWnd::MainPanel::SetPart(" << (part ? part->Name() : "no part") << ", slot " << slot << ")";
    if (slot > m_slots.size()) {
        ErrorLogger() << "DesignWnd::MainPanel::SetPart specified nonexistant slot";
        return;
    }

    if (!change_all_similar_parts) {
        m_slots[slot]->SetPart(part);

    } else {
        const auto original_part = m_slots[slot]->GetPart();
        std::string original_part_name = original_part ? original_part->Name() : "";

        if (change_all_similar_parts) {
            for (auto& slot_control : m_slots) {
                // skip incompatible slots
                if (!part->CanMountInSlotType(slot_control->SlotType()))
                    continue;

                // skip different type parts
                const auto replaced_part = slot_control->GetPart();
                if (replaced_part && (replaced_part->Name() != original_part_name))
                    continue;

                slot_control->SetPart(part);
            }
        }
    }

    if (emit_signal)  // to avoid unnecessary signal repetition.
        DesignChangedSignal();
}

void DesignWnd::MainPanel::SetParts(const std::vector<std::string>& parts) {
    unsigned int num_parts = std::min(parts.size(), m_slots.size());
    for (unsigned int i = 0; i < num_parts; ++i)
        m_slots[i]->SetPart(parts[i]);

    DesignChangedSignal();
}

void DesignWnd::MainPanel::AddPart(const ShipPart* part) {
    if (AddPartEmptySlot(part, FindEmptySlotForPart(part)))
        return;

    if (!AddPartWithSwapping(part, FindSlotForPartWithSwapping(part)))
        DebugLogger() << "DesignWnd::MainPanel::AddPart(" << (part ? part->Name() : "no part")
                      << ") couldn't find a slot for the part";
}

bool DesignWnd::MainPanel::CanPartBeAdded(const ShipPart* part) {
    std::pair<int, int> swap_result = FindSlotForPartWithSwapping(part);
    return (FindEmptySlotForPart(part) >= 0 || (swap_result.first >= 0 && swap_result.second >= 0));
}

bool DesignWnd::MainPanel::AddPartEmptySlot(const ShipPart* part, int slot_number) {
    if (!part || slot_number < 0)
        return false;
    SetPart(part, slot_number);
    DesignChangedSignal();
    return true;
}

bool DesignWnd::MainPanel::AddPartWithSwapping(const ShipPart* part,
                                               std::pair<int, int> swap_and_empty_slot)
{
    if (!part || swap_and_empty_slot.first < 0 || swap_and_empty_slot.second < 0)
        return false;
    // Move the flexible part to the first open spot
    SetPart(m_slots[swap_and_empty_slot.first]->GetPart(), swap_and_empty_slot.second);
    // Move replacement part into the newly opened slot
    SetPart(part, swap_and_empty_slot.first);
    DesignChangedSignal();
    return true;
}

int DesignWnd::MainPanel::FindEmptySlotForPart(const ShipPart* part) {
    int result = -1;
    if (!part)
        return result;

    if (part->Class() == ShipPartClass::PC_FIGHTER_HANGAR) {
        // give up if part is a hangar and there is already a hangar of another type
        std::string already_seen_hangar_name;
        for (const auto& slot : m_slots) {
            const ShipPart* part = slot->GetPart();
            if (!part || part->Class() != ShipPartClass::PC_FIGHTER_HANGAR)
                continue;
            if (part->Name() != part->Name())
                return result;
        }
    }

    for (unsigned int i = 0; i < m_slots.size(); ++i) {             // scan through slots to find one that can mount part
        const ShipSlotType slot_type = m_slots[i]->SlotType();
        const ShipPart* slotted_part = m_slots[i]->GetPart();

        if (!slotted_part && part->CanMountInSlotType(slot_type)) {
            result = i;
            return result;
        }
    }
    return result;
}

void DesignWnd::MainPanel::DesignNameEditedSlot(const std::string& new_name) {
    DesignNameChanged();  // Check whether the confirmation button should be enabled or disabled each time the name changes.
}

std::pair<int, int> DesignWnd::MainPanel::FindSlotForPartWithSwapping(const ShipPart* part) {
    // result.first = swap_slot, result.second = empty_slot
    // if any of the pair == -1, no swap!

    if (!part)
        return {-1, -1};

    // check if adding the part would cause the design to have multiple different types of hangar (which is not allowed)
    if (part->Class() == ShipPartClass::PC_FIGHTER_HANGAR) {
        for (const auto& slot : m_slots) {
            const ShipPart* existing_part = slot->GetPart();
            if (!existing_part || existing_part->Class() != ShipPartClass::PC_FIGHTER_HANGAR)
                continue;
            if (existing_part->Name() != part->Name())
                return {-1, -1};  // conflict; new part can't be added
        }
    }

    // first search for an empty compatible slot for the new part
    for (const auto& slot : m_slots) {
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
    for (auto& slot : m_slots)
        slot->SetPart(nullptr);
    DesignChangedSignal();
}

void DesignWnd::MainPanel::ClearPart(const std::string& part_name) {
    bool changed = false;
    for (const auto& slot : m_slots) {
        const ShipPart* existing_part = slot->GetPart();
        if (!existing_part)
            continue;
        if (existing_part->Name() != part_name)
            continue;
        slot->SetPart(nullptr);
        changed = true;
    }

    if (changed)
        DesignChangedSignal();
}

void DesignWnd::MainPanel::SetHull(const std::string& hull_name, bool signal)
{ SetHull(GetShipHull(hull_name), signal); }

void DesignWnd::MainPanel::SetHull(const ShipHull* hull, bool signal) {
    m_hull = hull;
    DetachChild(m_background_image);
    m_background_image = nullptr;
    if (m_hull) {
        auto texture = ClientUI::HullTexture(hull->Name());
        m_background_image = GG::Wnd::Create<GG::StaticGraphic>(
            std::move(texture), GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC);
        AttachChild(m_background_image);
        MoveChildDown(m_background_image);
    }
    Populate();
    DoLayout();
    if (signal)
        DesignChangedSignal();
}

void DesignWnd::MainPanel::SetDesign(const ShipDesign* ship_design) {
    m_incomplete_design.reset();

    if (!ship_design) {
        SetHull(nullptr);
        return;
    }

    if (!ship_design->IsMonster()) {
        m_replaced_design_id = ship_design->ID();
        m_replaced_design_uuid = ship_design->UUID();
    } else {
        // Allow editing of monsters if the design is a saved design
        const auto is_saved_monster = GetSavedDesignsManager().GetDesign(ship_design->UUID());
        m_replaced_design_id = is_saved_monster ? ship_design->ID() : boost::optional<int>();
        m_replaced_design_uuid = is_saved_monster ? ship_design->UUID() : boost::optional<boost::uuids::uuid>();
    }

    m_design_name->SetText(ship_design->Name());
    m_design_description_edit->SetText(ship_design->Description());

    bool suppress_design_changed_signal = true;
    SetHull(ship_design->Hull(), !suppress_design_changed_signal);

    SetParts(ship_design->Parts());
    DesignChangedSignal();
}

void DesignWnd::MainPanel::SetDesign(int design_id)
{ SetDesign(GetUniverse().GetShipDesign(design_id)); }

void DesignWnd::MainPanel::SetDesign(const boost::uuids::uuid& uuid)
{ SetDesign(GetSavedDesignsManager().GetDesign(uuid)); }

void DesignWnd::MainPanel::SetDesignComponents(const std::string& hull,
                                               const std::vector<std::string>& parts)
{
    m_replaced_design_id = boost::none;
    m_replaced_design_uuid = boost::none;
    SetHull(hull, false);
    SetParts(parts);
}

void DesignWnd::MainPanel::SetDesignComponents(const std::string& hull,
                                               const std::vector<std::string>& parts,
                                               const std::string& name,
                                               const std::string& desc)
{
    SetDesignComponents(hull, parts);
    m_design_name->SetText(name);
    m_design_description_edit->SetText(desc);
}

void DesignWnd::MainPanel::HighlightSlotType(std::vector<ShipSlotType>& slot_types) {
    for (auto& control : m_slots) {
        if (std::count(slot_types.begin(), slot_types.end(), control->SlotType()))
            control->Highlight(true);
        else
            control->Highlight(false);
    }
}

void DesignWnd::MainPanel::HandleBaseTypeChange(DesignWnd::BaseSelector::BaseSelectorTab base_type) {
    if (m_type_to_create == base_type)
        return;
    switch (base_type) {
    case DesignWnd::BaseSelector::BaseSelectorTab::Current:
    case DesignWnd::BaseSelector::BaseSelectorTab::Saved:
        m_type_to_create = base_type;
        break;
    default:
        break;
    }
    DesignChanged();
}

void DesignWnd::MainPanel::Populate() {
    for (const auto& slot: m_slots)
        DetachChild(slot);
    m_slots.clear();

    if (!m_hull)
        return;

    const std::vector<ShipHull::Slot>& hull_slots = m_hull->Slots();

    for (std::size_t i = 0; i < hull_slots.size(); ++i) {
        const ShipHull::Slot& slot = hull_slots[i];
        auto slot_control = GG::Wnd::Create<SlotControl>(slot.x, slot.y, slot.type);
        m_slots.push_back(slot_control);
        AttachChild(slot_control);

        slot_control->SlotContentsAlteredSignal.connect(
            [this, slot{i}](auto* part, bool do_similar) { SetPart(part, slot, true, do_similar); });
        slot_control->ShipPartClickedSignal.connect(ShipPartClickedSignal);
    }
}

void DesignWnd::MainPanel::DoLayout() {
    // position labels and text edit boxes for name and description and buttons to clear and confirm design

    const int PTS = ClientUI::Pts();
    static constexpr int PAD = 6;

    const auto cl_sz = ClientSize();

    auto lr = cl_sz - GG::Pt(GG::X{PAD}, GG::Y{PAD});
    m_confirm_button->SizeMove(lr - m_confirm_button->MinUsableSize(), lr);

    auto mus = m_replace_button->MinUsableSize();
    auto ul = m_confirm_button->RelativeUpperLeft() - GG::Pt(mus.x+PAD, GG::Y0);
    m_replace_button->SizeMove(ul, ul+mus);

    const auto ll = GG::Pt(GG::X(PAD), cl_sz.y - PAD);
    mus = m_clear_button->MinUsableSize();
    ul = ll - GG::Pt(GG::X0, mus.y);
    m_clear_button->SizeMove(ul, ul + mus);

    ul = GG::Pt(GG::X(PAD), GG::Y(PAD));
    // adjust based on the (bigger) height of the edit bar 
    lr = ul + GG::Pt(m_design_name_label->MinUsableSize().x, m_design_name->MinUsableSize().y);
    m_design_name_label->SizeMove(ul, lr);

    ul = GG::Pt(m_design_name_label->RelativeLowerRight().x+PAD, GG::Y(PAD));
    m_design_name->SizeMove(ul, GG::Pt(cl_sz.x-PAD, ul.y+m_design_name->MinUsableSize().y));

    ul = GG::Pt(GG::X(PAD), GG::Y(m_design_name->RelativeLowerRight().y+PAD));
    // Apparently calling minuseablesize on the button itself doesn't work
    lr = ul + GG::Pt(m_design_description_toggle->GetLabel()->MinUsableSize().x + 10,
                     m_design_name->MinUsableSize().y);
    m_design_description_toggle->SizeMove(ul, lr);

    ul.x = m_design_description_toggle->RelativeLowerRight().x + PAD;
    m_design_description_edit->SizeMove(ul, GG::Pt(cl_sz.x-PAD, ul.y+PTS*4+8));
    if (m_design_description_toggle->Checked())
        m_design_description_edit->Show();
    else
        m_design_description_edit->Hide();

    // place background image of hull
    ul.x = GG::X0;
    ul.y += m_design_name->Height();

    auto bg_rect = GG::Rect(ul, ClientLowerRight());
    if (m_background_image) {
        m_background_image->SizeMove(bg_rect.UpperLeft(), ClientSize());
        bg_rect = m_background_image->RenderedArea();
    }

    // place slot controls over image of hull
    const auto slot_ref_ul = bg_rect.UpperLeft() - ClientUpperLeft();

    for (auto& slot : m_slots) {
        GG::X x = slot_ref_ul.x - slot->Width()/2 + GG::ToX(slot->XPositionFraction() * bg_rect.Width());
        GG::Y y = slot_ref_ul.y - slot->Height()/2 + GG::ToY(slot->YPositionFraction() * bg_rect.Height());
        slot->MoveTo(GG::Pt(x, y));
    }
}

void DesignWnd::MainPanel::DesignChanged() {
    m_replace_button->ClearBrowseInfoWnd();
    m_confirm_button->ClearBrowseInfoWnd();

    const int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();
    m_disabled_by_name = false;
    m_disabled_by_part_conflict = false;

    m_replace_button->Disable(true);
    m_confirm_button->Disable(true);

    m_replace_button->SetText(UserString("DESIGN_WND_UPDATE_FINISHED"));
    m_confirm_button->SetText(UserString("DESIGN_WND_ADD_FINISHED"));

    if (!m_hull) {
        m_replace_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_UPDATE_INVALID_NO_CANDIDATE")));
        m_confirm_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_INV_NO_HULL")));
        return;
    }

    if (client_empire_id == ALL_EMPIRES) {
        m_replace_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_INV_MODERATOR")));
        m_confirm_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_INV_MODERATOR")));
        return;
    }

    if (!IsDesignNameValid()) {
        m_disabled_by_name = true;

        m_replace_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_INV_NO_NAME")));
        m_confirm_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
            UserString("DESIGN_INVALID"), UserString("DESIGN_INV_NO_NAME")));
        return;
    }

    if (!ShipDesign::ValidDesign(m_hull->Name(), Parts())) {
        // if a design has exclusion violations between parts and hull, highlight these and indicate it on the button

        std::pair<std::string, std::string> problematic_components;

        // check hull exclusions against all parts...
        const auto& hull_exclusions = m_hull->Exclusions();
        for (const std::string& part_name : Parts()) { // TODO: could habe mutable ref and move?
            if (part_name.empty())
                continue;
            if (std::count(hull_exclusions.begin(), hull_exclusions.end(), part_name)) {
                m_disabled_by_part_conflict = true;
                problematic_components.first = m_hull->Name();
                problematic_components.second = part_name;
            }
        }

        // check part exclusions against other parts and hull
        std::set<std::string> already_seen_component_names;
        already_seen_component_names.insert(m_hull->Name());
        for (std::string& part_name : Parts()) {
            if (m_disabled_by_part_conflict)
                break;
            const ShipPart* part = GetShipPart(part_name);
            if (!part)
                continue;
            for (const std::string& excluded_part : part->Exclusions()) {
                if (already_seen_component_names.contains(excluded_part)) {
                    m_disabled_by_part_conflict = true;
                    problematic_components.first = part_name;
                    problematic_components.second = excluded_part;
                    break;
                }
            }
            already_seen_component_names.insert(std::move(part_name));
        }


        if (m_disabled_by_part_conflict) {
            m_replace_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
                UserString("DESIGN_WND_COMPONENT_CONFLICT"),
                boost::io::str(FlexibleFormat(UserString("DESIGN_WND_COMPONENT_CONFLICT_DETAIL"))
                               % UserString(problematic_components.first)
                               % UserString(problematic_components.second))));
            m_confirm_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
                UserString("DESIGN_WND_COMPONENT_CONFLICT"),
                boost::io::str(FlexibleFormat(UserString("DESIGN_WND_COMPONENT_CONFLICT_DETAIL"))
                               % UserString(problematic_components.first)
                               % UserString(problematic_components.second))));

            // todo: mark conflicting parts somehow
        }
        return;
    }

    const auto& cur_design = GetIncompleteDesign();

    if (!cur_design)
        return;

    const auto new_design_name{ValidatedDesignName().DisplayText()}; // TODO: can this be a const reference?

    // producible only matters for empire designs.
    // Monster designs can be edited as saved designs.
    bool producible = cur_design->Producible();

    // Current designs can not duplicate other designs, be already registered.
    const auto existing_design = CurrentDesignIsRegistered();
    const auto replaced_saved_design = EditingSavedDesign();
    const auto replaced_current_design = EditingCurrentDesign();

    // Choose text for the replace button: replace saved design, replace current design or already known.

    // A changed saved design can be replaced with an updated design
    if (replaced_saved_design) {
        if (cur_design && !(*cur_design == **replaced_saved_design)) {
            m_replace_button->SetText(UserString("DESIGN_WND_UPDATE_SAVED"));
            m_replace_button->SetBrowseInfoWnd(
                GG::Wnd::Create<TextBrowseWnd>(
                    UserString("DESIGN_WND_UPDATE_SAVED"),
                    boost::io::str(FlexibleFormat(UserString("DESIGN_WND_UPDATE_SAVED_DETAIL"))
                                   % (*replaced_saved_design)->Name()
                                   % new_design_name)));
            m_replace_button->Disable(false);
        }
    }

    if (producible) {
        if (existing_design
            && m_type_to_create == DesignWnd::BaseSelector::BaseSelectorTab::Current)
        {
            // Rename duplicate finished designs
            if ((*existing_design)->Name() != new_design_name) {
                m_replace_button->SetText(UserString("DESIGN_WND_RENAME_FINISHED"));
                m_replace_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
                    UserString("DESIGN_WND_RENAME_FINISHED"),
                    boost::io::str(FlexibleFormat(UserString("DESIGN_WND_RENAME_FINISHED_DETAIL"))
                                   % ((*existing_design)->Name())
                                   % new_design_name)));
                m_replace_button->Disable(false);

            // Otherwise mark it as known.
            } else {
                m_disabled_by_name = true;
                m_replace_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
                    UserString("DESIGN_WND_KNOWN"),
                    boost::io::str(FlexibleFormat(UserString("DESIGN_WND_KNOWN_DETAIL"))
                                   % (*existing_design)->Name())));
            }


        } else if (replaced_current_design) {
            // A current design can be replaced if it doesn't duplicate an existing design
            m_replace_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
                UserString("DESIGN_WND_UPDATE_FINISHED"),
                boost::io::str(FlexibleFormat(UserString("DESIGN_WND_UPDATE_FINISHED_DETAIL"))
                               % (*replaced_current_design)->Name()
                               % new_design_name)));
            m_replace_button->Disable(false);
        }
    }

    // Choose text for the add new design button: add saved design, add current design or already known.

    // Add a saved design if the saved base selector was visited more recently than the current tab.
    if (m_type_to_create == DesignWnd::BaseSelector::BaseSelectorTab::Saved) {
        // A new saved design can always be created
        m_confirm_button->SetText(UserString("DESIGN_WND_ADD_SAVED"));
        m_confirm_button->SetBrowseInfoWnd(
            GG::Wnd::Create<TextBrowseWnd>(
                UserString("DESIGN_WND_ADD_SAVED"),
                boost::io::str(FlexibleFormat(UserString("DESIGN_WND_ADD_SAVED_DETAIL"))
                               % new_design_name)));
        m_confirm_button->Disable(false);
    } else if (producible) {
        if (!existing_design) {
            // A new current can be added if it does not duplicate an existing design.
            m_confirm_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
                UserString("DESIGN_WND_ADD_FINISHED"),
                boost::io::str(FlexibleFormat(UserString("DESIGN_WND_ADD_FINISHED_DETAIL"))
                               % new_design_name)));
            m_confirm_button->Disable(false);

        } else {
            // Otherwise the design is already known.
            m_confirm_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(
                UserString("DESIGN_WND_KNOWN"),
                boost::io::str(FlexibleFormat(UserString("DESIGN_WND_KNOWN_DETAIL"))
                               % (*existing_design)->Name())));
        }
    }
}

void DesignWnd::MainPanel::DesignNameChanged() {
    if (m_disabled_by_name || (!IsDesignNameValid() && !m_confirm_button->Disabled()))
        DesignChangedSignal();
    else if (GetOptionsDB().Get<bool>("ui.design.pedia.title.dynamic.enabled"))
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
    auto [name, description] = ValidatedNameAndDescription();

    if (const ShipDesign* design = m_incomplete_design.get()) {
        if (design->Hull() ==             Hull() &&
            design->Name(false) ==        name.StoredString() &&
            design->Description(false) == description.StoredString() &&
            design->Parts() ==            Parts())
        {
            // nothing has changed, so don't need to update
            return;
        }
    }

    // assemble and check info for new design
    const std::string& hull = Hull();
    const std::string& icon = m_hull ? m_hull->Icon() : EMPTY_STRING;

    auto uuid = boost::uuids::random_generator()();

    // update stored design
    m_incomplete_design.reset();
    if (hull.empty())
        return;
    try {
        m_incomplete_design = std::make_shared<ShipDesign>(
            std::invalid_argument(""),
            name.StoredString(), description.StoredString(),
            ClientApp::GetApp()->CurrentTurn(), ClientApp::GetApp()->EmpireID(),
            hull, Parts(), icon, "", name.IsInStringtable(),
            false, std::move(uuid));
        m_incomplete_design->SetID(INCOMPLETE_DESIGN_ID);
    } catch (const std::invalid_argument& e) {
        ErrorLogger() << "DesignWnd::MainPanel::RefreshIncompleteDesign " << e.what();
    }
}

void DesignWnd::MainPanel::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                                           GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) const
{
    for (DropsAcceptableIter it = first; it != last; ++it)
        it->second = false;

    // if multiple things dropped simultaneously somehow, reject all
    if (std::distance(first, last) != 1)
        return;

    if (dynamic_cast<const BasesListBox::BasesListBoxRow*>(first->first))
        first->second = true;
}

void DesignWnd::MainPanel::AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds,
                                       GG::Flags<GG::ModKey> mod_keys)
{
    if (wnds.size() != 1)
        ErrorLogger() << "DesignWnd::MainPanel::AcceptDrops given multiple wnds unexpectedly...";

    const auto* wnd = wnds.front().get();
    if (!wnd)
        return;

    if (const auto completed_design_row = dynamic_cast<const BasesListBox::CompletedDesignListBoxRow*>(wnd)) {
        SetDesign(GetUniverse().GetShipDesign(completed_design_row->DesignID()));

    } else if (const auto hullandparts_row = dynamic_cast<const BasesListBox::HullAndPartsListBoxRow*>(wnd)) {
        const std::string& hull = hullandparts_row->Hull();
        const std::vector<std::string>& parts = hullandparts_row->Parts();

        SetDesignComponents(hull, parts);

    } else if (const auto saved_design_row = dynamic_cast<const SavedDesignsListBox::SavedDesignListBoxRow*>(wnd)) {
        const auto uuid = saved_design_row->DesignUUID();
        SetDesign(GetSavedDesignsManager().GetDesign(uuid));
    }
}

std::pair<int, boost::uuids::uuid> DesignWnd::MainPanel::AddDesign() {
    try {
        std::vector<std::string> parts = Parts();
        const std::string& hull_name = Hull();

        const auto name = ValidatedDesignName();

        const auto description = DesignDescription();

        std::string icon = "ship_hulls/generic_hull.png";
        if (const ShipHull* hull = GetShipHull(hull_name))
            icon = hull->Icon();

        auto new_uuid = boost::uuids::random_generator()();
        auto new_design_id = INVALID_DESIGN_ID;

        auto* app = GGHumanClientApp::GetApp();
        int empire_id = app->EmpireID();

        // create design from stuff chosen in UI
        ShipDesign design(std::invalid_argument(""),
                          name.StoredString(), description.StoredString(),
                          app->CurrentTurn(), empire_id,
                          hull_name, parts, icon, "some model", name.IsInStringtable(),
                          false, new_uuid);

        // If editing a saved design insert into saved designs
        if (m_type_to_create == DesignWnd::BaseSelector::BaseSelectorTab::Saved) {
            auto& manager = GetSavedDesignsManager();
            manager.InsertBefore(design, manager.OrderedDesignUUIDs().begin());
            new_uuid = manager.OrderedDesignUUIDs().front();

        // Otherwise insert into current empire designs
        } else {
            ScriptingContext& context = app->GetContext();
            auto empire = context.GetEmpire(empire_id);
            if (!empire) return {INVALID_DESIGN_ID, boost::uuids::nil_generator()()};

            const auto order = app->Orders().IssueOrder<ShipDesignOrder>(context, empire_id, design);
            new_design_id = order ? order->DesignID() : INVALID_DESIGN_ID;

            auto& manager = GetDisplayedDesignsManager();
            const auto& all_ids = manager.AllOrderedIDs();
            manager.InsertBefore(new_design_id, all_ids.empty() ? INVALID_DESIGN_ID : all_ids.front());
        }

        DesignChangedSignal();

        DebugLogger() << "Added new design: " << design.Name();

        return std::pair(new_design_id, new_uuid);

    } catch (const std::invalid_argument&) {
        ErrorLogger() << "DesignWnd::AddDesign tried to add an invalid ShipDesign";
        return {INVALID_DESIGN_ID, boost::uuids::nil_generator()()};
    }
}

void DesignWnd::MainPanel::ReplaceDesign() {
    auto old_m_type_to_create = m_type_to_create;
    m_type_to_create = EditingSavedDesign()
        ? DesignWnd::BaseSelector::BaseSelectorTab::Saved
        : DesignWnd::BaseSelector::BaseSelectorTab::Current;

    const auto new_id_and_uuid = AddDesign();
    const auto& new_uuid = new_id_and_uuid.second;
    const auto new_design_id = new_id_and_uuid.first;

    m_type_to_create = old_m_type_to_create;

    // If replacing a saved design
    if (const auto replaced_design = EditingSavedDesign()) {
        auto& manager = GetSavedDesignsManager();

        manager.MoveBefore(new_uuid, (*replaced_design)->UUID());
        manager.Erase((*replaced_design)->UUID());

        // Update the replaced design on the bench
        SetDesign(manager.GetDesign(new_uuid));

    } else {
        // If replacing or renaming a currect design
        const auto current_maybe_design = EditingCurrentDesign();
        const auto existing_design = CurrentDesignIsRegistered();
        if (current_maybe_design || existing_design) {
            auto& manager = GetDisplayedDesignsManager();
            auto* app = GGHumanClientApp::GetApp();
            int replaced_id = (*(current_maybe_design ? current_maybe_design : existing_design))->ID();

            if (new_design_id == INVALID_DESIGN_ID) return;

            // Remove the old id from the Empire.
            ScriptingContext& context = app->GetContext();
            const auto maybe_obsolete = manager.IsObsolete(replaced_id, context);
            bool is_obsolete = maybe_obsolete && *maybe_obsolete;
            if (!is_obsolete)
                app->Orders().IssueOrder<ShipDesignOrder>(context, app->EmpireID(), replaced_id, true);

            // Replace the old id in the manager.
            manager.MoveBefore(new_design_id, replaced_id);
            manager.Remove(replaced_id);

            // Update the replaced design on the bench
            SetDesign(new_design_id);

            DebugLogger() << "Replaced design #" << replaced_id << " with #" << new_design_id ;
        }
    }

    DesignChangedSignal();
}

void DesignWnd::MainPanel::ToggleDescriptionEditor() {
    if (m_design_description_toggle->Checked())
        m_design_description_edit->Show();
    else
        m_design_description_edit->Hide();
}

//////////////////////////////////////////////////
// DesignWnd                                    //
//////////////////////////////////////////////////
DesignWnd::DesignWnd(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::ONTOP | GG::INTERACTIVE)
{}

void DesignWnd::CompleteConstruction() {
    GG::Wnd::CompleteConstruction();

    Sound::TempUISoundDisabler sound_disabler;
    SetChildClippingMode(ChildClippingMode::ClipToClient);

    m_detail_panel = GG::Wnd::Create<EncyclopediaDetailPanel>(
        GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | PINABLE,
        DES_PEDIA_WND_NAME);
    m_main_panel = GG::Wnd::Create<MainPanel>(DES_MAIN_WND_NAME);
    m_part_palette = GG::Wnd::Create<PartPalette>(DES_PART_PALETTE_WND_NAME);
    m_base_selector = GG::Wnd::Create<BaseSelector>(DES_BASE_SELECTOR_WND_NAME);
    InitializeWindows();
    GGHumanClientApp::GetApp()->RepositionWindowsSignal.connect([this]() { InitializeWindows(); });

    AttachChild(m_detail_panel);

    AttachChild(m_main_panel);
    m_main_panel->ShipPartClickedSignal.connect([this](auto* p, auto) { m_detail_panel->SetItem(p); });
    m_main_panel->ShipHullClickedSignal.connect([this](auto* h) { m_detail_panel->SetItem(h); });
    m_main_panel->DesignChangedSignal.connect([this]() { DesignChanged(); });
    m_main_panel->DesignNameChangedSignal.connect([this]() { DesignNameChanged(); });
    m_main_panel->CompleteDesignClickedSignal.connect([this](int id) { m_detail_panel->SetDesign(id); });
    //m_main_panel->Sanitize();

    AttachChild(m_part_palette);
    m_part_palette->ShipPartClickedSignal.connect([this](auto* p, auto) { m_detail_panel->SetItem(p); });
    m_part_palette->ShipPartDoubleClickedSignal.connect([this](auto* p) { m_main_panel->AddPart(p); });
    m_part_palette->ClearPartSignal.connect([this](auto& p) { m_main_panel->ClearPart(p); });

    AttachChild(m_base_selector);
    m_base_selector->DesignSelectedSignal.connect([this](int id) { m_main_panel->SetDesign(id); });
    m_base_selector->DesignUpdatedSignal.connect([this](int id) { m_main_panel->DesignChanged(); });
    m_base_selector->DesignComponentsSelectedSignal.connect(
        [this](auto& hull, auto& parts) { m_main_panel->SetDesignComponents(hull, parts); });
    m_base_selector->SavedDesignSelectedSignal.connect(
        [this](const auto& uuid) { m_main_panel->SetDesign(uuid); });
    m_base_selector->DesignClickedSignal.connect([this](auto* d) { m_detail_panel->SetItem(d); });
    m_base_selector->HullClickedSignal.connect([this](auto* h) { m_detail_panel->SetItem(h); });
    m_base_selector->TabChangedSignal.connect([this](auto tab) { m_main_panel->HandleBaseTypeChange(tab); });

    // Connect signals to re-populate when part obsolescence changes
    m_part_palette->PartObsolescenceChangedSignal.connect([this]() { m_base_selector->Reset(); });
}

void DesignWnd::SizeMove(GG::Pt ul, GG::Pt lr) {
    const auto old_size = Size();
    GG::Wnd::SizeMove(ul, lr);
    if (old_size != Size()) {
        m_detail_panel->ValidatePosition();
        m_base_selector->ValidatePosition();
        m_part_palette->ValidatePosition();
        m_main_panel->ValidatePosition();
    }
}

void DesignWnd::Reset() {
    m_part_palette->Populate();
    m_base_selector->Reset();
    m_detail_panel->Refresh();
    m_main_panel->Sanitize();
}

void DesignWnd::Sanitize()
{ m_main_panel->Sanitize(); }

void DesignWnd::Render()
{ GG::FlatRectangle(UpperLeft(), LowerRight(), ClientUI::WndColor(), GG::CLR_ZERO, 0); }

void DesignWnd::InitializeWindows() {
    static constexpr GG::X selector_width = GG::X(300);
    const GG::X main_width = ClientWidth() - selector_width;

    static constexpr GG::Pt pedia_ul(selector_width, GG::Y0);
    const GG::Pt pedia_wh(5*main_width/11, 2*ClientHeight()/5);

    const GG::Pt main_ul(selector_width, pedia_ul.y + pedia_wh.y);
    const GG::Pt main_wh(main_width, ClientHeight() - main_ul.y);

    const GG::Pt palette_ul(selector_width + pedia_wh.x, pedia_ul.y);
    const GG::Pt palette_wh(main_width - pedia_wh.x, pedia_wh.y);

    static constexpr GG::Pt selector_ul(GG::X0, GG::Y0);
    const GG::Pt selector_wh(selector_width, ClientHeight());

    m_detail_panel-> InitSizeMove(pedia_ul,     pedia_ul + pedia_wh);
    m_main_panel->   InitSizeMove(main_ul,      main_ul + main_wh);
    m_part_palette-> InitSizeMove(palette_ul,   palette_ul + palette_wh);
    m_base_selector->InitSizeMove(selector_ul,  selector_ul + selector_wh);
}

void DesignWnd::ShowShipPartInEncyclopedia(const std::string& part)
{ m_detail_panel->SetShipPart(part); }

void DesignWnd::ShowShipHullInEncyclopedia(const std::string& ship_hull)
{ m_detail_panel->SetShipHull(ship_hull); }

void DesignWnd::ShowShipDesignInEncyclopedia(int design_id)
{ m_detail_panel->SetDesign(design_id); }

void DesignWnd::DesignChanged() {
    m_detail_panel->SetIncompleteDesign(m_main_panel->GetIncompleteDesign());
    m_base_selector->Reset();
}

void DesignWnd::DesignNameChanged() {
    m_detail_panel->SetIncompleteDesign(m_main_panel->GetIncompleteDesign());
    m_base_selector->Reset();
}

void DesignWnd::EnableOrderIssuing(bool enable)
{ m_base_selector->EnableOrderIssuing(enable); }
