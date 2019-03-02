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
#include "../client/human/HumanClientApp.h"
#include "../universe/Condition.h"
#include "../universe/UniverseObject.h"
#include "../universe/ShipDesign.h"
#include "../universe/Enums.h"

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
//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <unordered_map>
#include <functional>

FO_COMMON_API extern const int INVALID_DESIGN_ID;

namespace {
    const std::string   PART_CONTROL_DROP_TYPE_STRING = "Part Control";
    const std::string   HULL_PARTS_ROW_DROP_TYPE_STRING = "Hull and Parts Row";
    const std::string   COMPLETE_DESIGN_ROW_DROP_STRING = "Complete Design Row";
    const std::string   SAVED_DESIGN_ROW_DROP_STRING = "Saved Design Row";
    const std::string   EMPTY_STRING = "";
    const std::string   DES_PEDIA_WND_NAME = "design.pedia";
    const std::string   DES_MAIN_WND_NAME = "design.edit";
    const std::string   DES_BASE_SELECTOR_WND_NAME = "design.selector";
    const std::string   DES_PART_PALETTE_WND_NAME = "design.parts";
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
    const std::string DESIGN_MANIFEST_PREFIX = "ShipDesignOrdering";
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

    boost::filesystem::path CreateSaveFileNameForDesign(const ShipDesign& design) {
        boost::filesystem::path designs_dir_path = GetDesignsDir();

        // Since there is no easy way to guarantee that an arbitrary design name with possibly
        // embedded decorator code is a safe file name, use the UUID. The users will never interact
        // with this filename.
        std::string file_name =
            DESIGN_FILENAME_PREFIX + boost::uuids::to_string(design.UUID()) + DESIGN_FILENAME_EXTENSION;

        return boost::filesystem::absolute(PathToString(designs_dir_path / file_name));
    }


    /** DisplayedShipDesignManager allows for the storage and manipulation of an
      * ordered list of design ids that are used to order the display of
      * ShipDesigns in the DesignWnd and the ProductionWnd. */
    class DisplayedShipDesignManager : public ShipDesignManager::Designs {
    public:
        DisplayedShipDesignManager()
        {}

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

        void InsertHullBefore(const std::string& id, const std::string& next_id = "");

        bool IsKnown(const int id) const;

        /** Return true if design \p id is obsolete or boost::none if \p id is not in
            the manager. */
        boost::optional<bool> IsObsolete(const int id) const;
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
        std::unordered_map<int,
                           std::pair<boost::optional<std::pair<bool, int>>,
                                     std::list<int>::const_iterator>> m_id_to_obsolete_and_loc;

        // An index from the hull name to the obsolescence state and the location in the
        // m_ordered_hull_ids list.
        std::unordered_map<std::string,
                           std::pair<std::pair<bool, int>,
                           std::list<std::string>::const_iterator>> m_hull_to_obsolete_and_loc;

        // A map from obsolete part name to the UI event count that changed it.
        std::unordered_map<std::string, int> m_obsolete_parts;
    };

    class SavedDesignsManager : public ShipDesignManager::Designs {
    public:
        SavedDesignsManager()
        {}

        const std::list<boost::uuids::uuid>& OrderedDesignUUIDs() const;
        std::vector<int> OrderedIDs() const override;

        void StartParsingDesignsFromFileSystem(bool is_new_game);
        void CheckPendingDesigns() const;

        const ShipDesign* GetDesign(const boost::uuids::uuid& uuid) const;

        void SaveManifest();

        std::list<boost::uuids::uuid>::const_iterator
        InsertBefore(const ShipDesign& design, std::list<boost::uuids::uuid>::const_iterator next);
        bool MoveBefore(const boost::uuids::uuid& moved_uuid, const boost::uuids::uuid& next_uuid);
        void Erase(const boost::uuids::uuid& erased_uuid);

    private:
        /** Save the design with the original filename or throw out_of_range. */
        void SaveDesign(const boost::uuids::uuid &uuid);

        /** SaveDesignConst allows CheckPendingDesigns to correct the designs
            in the saved directory.*/
        void SaveDesignConst(const boost::uuids::uuid &uuid) const;

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
                                   boost::hash<boost::uuids::uuid>>         m_saved_designs;

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

        const auto& current_ids = GetDisplayedDesignsManager().AllOrderedIDs();
        const auto is_same_design = [&design](const int id) {
            auto current_design = GetShipDesign(id);
            return current_design && *current_design == *design;
        };

        return std::none_of(current_ids.begin(), current_ids.end(), is_same_design);
    }

    /** Add \p design to the \p is_front of \p empire_id's list of current designs. */
    void AddSavedDesignToDisplayedDesigns(const boost::uuids::uuid& uuid, int empire_id,
                                          bool is_front = true)
    {
        const auto empire = GetEmpire(empire_id);
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
        auto new_current_design = *design;
        new_current_design.SetUUID(boost::uuids::random_generator()());

        auto order = std::make_shared<ShipDesignOrder>(empire_id, new_current_design);
        HumanClientApp::GetApp()->Orders().IssueOrder(order);

        auto& current_manager = GetDisplayedDesignsManager();
        const auto& all_ids = current_manager.AllOrderedIDs();
        const int before_id = (all_ids.empty() || !is_front) ? INVALID_OBJECT_ID : *all_ids.begin() ;
        current_manager.InsertBefore(order->DesignID(), before_id);
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

        const auto empire_id = HumanClientApp::GetApp()->EmpireID();

        manager.SetObsolete(design_id, obsolete);

        if (obsolete) {
            // make empire forget on the server
            HumanClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<ShipDesignOrder>(empire_id, design_id, true));
        } else {
            const auto design = GetShipDesign(design_id);
            if (!design) {
                ErrorLogger() << "Attempted to toggle obsolete state of design id "
                              << design_id << " which is unknown to the server";
                return;
            }

            //make known to empire on server
            HumanClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<ShipDesignOrder>(empire_id, design_id));
        }
    }

    /** Remove design from DisplayedDesigns. */
    void DeleteFromDisplayedDesigns(const int design_id) {
        auto& manager = GetDisplayedDesignsManager();

        //const auto empire_id = HumanClientApp::GetApp()->EmpireID();
        //const auto maybe_obsolete = manager.IsObsolete(design_id);
        //if (maybe_obsolete && !*maybe_obsolete)
        //    HumanClientApp::GetApp()->Orders().IssueOrder(
        //        std::make_shared<ShipDesignOrder>(empire_id, design_id, true));
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
        for (const auto uuid: m_ordered_uuids) {
            const auto& it = m_saved_designs.find(uuid);
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
                    path += DESIGN_FILENAME_EXTENSION;

            }


            for (auto& uuid: m_ordered_uuids)
                SaveDesignConst(uuid);
        }

        if (!m_is_new_game)
            return;

        m_is_new_game = false;

        // If requested on the first turn copy all of the saved designs to the client empire.
        if (GetOptionsDB().Get<bool>("resource.shipdesign.saved.enabled")) {
            const auto empire_id = HumanClientApp::GetApp()->EmpireID();
            TraceLogger() << "Adding saved designs to empire.";
            // assume the saved designs are preferred by the user: add them to the front.
            // note that this also ensures correct ordering.
            for (const auto& uuid : m_ordered_uuids)
                AddSavedDesignToDisplayedDesigns(uuid, empire_id, true); 
        }
    }

    const ShipDesign* SavedDesignsManager::GetDesign(const boost::uuids::uuid& uuid) const {
        CheckPendingDesigns();
        const auto& it = m_saved_designs.find(uuid);
        if (it == m_saved_designs.end())
            return nullptr;
        return it->second.first.get();
    }

    void SavedDesignsManager::SaveManifest()
    { SaveManifestConst(); }

    void SavedDesignsManager::SaveManifestConst() const {
        CheckPendingDesigns();
        boost::filesystem::path designs_dir_path = GetDesignsDir();

        std::string file_name = DESIGN_MANIFEST_PREFIX + DESIGN_FILENAME_EXTENSION;

        boost::filesystem::path file =
            boost::filesystem::absolute(PathToString(designs_dir_path / file_name));

        std::stringstream ss;
        ss << DESIGN_MANIFEST_PREFIX << "\n";
        for (const auto uuid: m_ordered_uuids)
            ss << "    uuid = \"" << uuid << "\"\n";
        WriteToFile(file, ss.str());
    }

    std::list<boost::uuids::uuid>::const_iterator SavedDesignsManager::InsertBefore(
        const ShipDesign& design,
        std::list<boost::uuids::uuid>::const_iterator next)
    {
        if (design.UUID() == boost::uuids::uuid{{0}}) {
            ErrorLogger() << "Ship design has a nil UUID for " << design.Name() << ". Not saving.";
            return next;
        }

        CheckPendingDesigns();
        if (m_saved_designs.count(design.UUID())) {
            // UUID already exists so this is a move.  Remove the old UUID location
            const auto existing_it = std::find(m_ordered_uuids.begin(), m_ordered_uuids.end(), design.UUID());
            if (existing_it != m_ordered_uuids.end())
                m_ordered_uuids.erase(existing_it);

        } else {
            // Add the new saved design.
            std::unique_ptr<ShipDesign> design_copy{boost::make_unique<ShipDesign>(design)};

            const auto save_path = CreateSaveFileNameForDesign(design);

            m_saved_designs.insert(std::make_pair(design.UUID(), std::make_pair(std::move(design_copy), save_path)));
            SaveDesign(design.UUID());
        }

        // Insert in the list.
        const auto retval = m_ordered_uuids.insert(next, design.UUID());
        SaveManifest();
        return retval;
    }

    bool SavedDesignsManager::MoveBefore(const boost::uuids::uuid& moved_uuid, const boost::uuids::uuid& next_uuid) {
        if (moved_uuid == next_uuid)
            return false;

        CheckPendingDesigns();
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

    void SavedDesignsManager::Erase(const boost::uuids::uuid& erased_uuid) {
        CheckPendingDesigns();
        const auto& saved_design_it = m_saved_designs.find(erased_uuid);
        if (saved_design_it != m_saved_designs.end()) {
            const auto& file = saved_design_it->second.second;
            boost::filesystem::remove(file);
            m_saved_designs.erase(erased_uuid);
        }

        const auto& uuid_it = std::find(m_ordered_uuids.begin(), m_ordered_uuids.end(), erased_uuid);
        m_ordered_uuids.erase(uuid_it);
    }

    void SavedDesignsManager::SaveDesign(const boost::uuids::uuid &uuid)
    { SaveDesignConst(uuid); }

    /** Save the design with the original filename or throw out_of_range..*/
    void SavedDesignsManager::SaveDesignConst(const boost::uuids::uuid &uuid) const {
        CheckPendingDesigns();
        const auto& design_and_filename = m_saved_designs.at(uuid);

        WriteToFile(design_and_filename.second, design_and_filename.first->Dump());
    }


    //////////////////////////////////////////////////
    // CurrentShipDesignsManager implementations
    //////////////////////////////////////////////////
    std::vector<int> DisplayedShipDesignManager::OrderedIDs() const {
        // Make sure that saved designs are included.
        // Only OrderedIDs is part of the Designs base class and
        // accessible outside this file.
        GetSavedDesignsManager().CheckPendingDesigns();

        // Remove all obsolete ids from the list
        std::vector<int> retval;
        std::copy_if(m_ordered_design_ids.begin(), m_ordered_design_ids.end(), std::back_inserter(retval),
                     [this](const int id) {
                         const auto maybe_obsolete = IsObsolete(id);
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

        m_id_to_obsolete_and_loc[id] = std::make_pair(boost::none, inserted_it);
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
            ErrorLogger() << "Hull name is empty()";
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

        m_hull_to_obsolete_and_loc[hull] =
            std::make_pair(std::make_pair(false, NextUIObsoleteEvent()), inserted_it);
    }

    bool DisplayedShipDesignManager::IsKnown(const int id) const
    { return m_id_to_obsolete_and_loc.count(id); }

    boost::optional<bool> DisplayedShipDesignManager::IsObsolete(const int id) const {
        // A non boost::none value for a specific design overrides the hull and part values
        auto it_id = m_id_to_obsolete_and_loc.find(id);

        // Unknown design
        if (it_id == m_id_to_obsolete_and_loc.end())
            return boost::none;

        const auto design = GetShipDesign(id);
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

        if (const auto maybe_obsolete_design = it_id->second.first) {
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
        if (it == m_id_to_obsolete_and_loc.end())
            return;

        it->second.first = std::make_pair(obsolete, NextUIObsoleteEvent());
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
        for (const auto& id_and_obsolete : design_ids_and_obsoletes) {
            const auto id = id_and_obsolete.first;
            const auto& obsolete = id_and_obsolete.second;
            if (m_id_to_obsolete_and_loc.count(id)) {
                ErrorLogger() << "DisplayedShipDesignManager::Load duplicate design id = " << id;
                continue;
            }
            if (obsolete && obsolete->first
                && (obsolete->second < 0 || obsolete->second >= m_obsolete_ui_event_count))
            {
                ErrorLogger() << "DisplayedShipDesignManager::Load design with id = " << id
                              << " has an obsolete_ui_event_count = " << obsolete->second
                              << " which does not satisfy 0 < obsolete_ui_event_count < m_obsolete_ui_event_count = "
                              << m_obsolete_ui_event_count;
            }
            m_ordered_design_ids.push_back(id);
            m_id_to_obsolete_and_loc[id] = std::make_pair(obsolete, --m_ordered_design_ids.end());
        }

        // Clear and load the ship hulls
        m_hull_to_obsolete_and_loc.clear();
        m_ordered_hulls.clear();
        for (const auto& name_and_obsolete : hulls_and_obsoletes) {
            const auto& name = name_and_obsolete.first;
            const auto& obsolete = name_and_obsolete.second;
            if (m_hull_to_obsolete_and_loc.count(name)) {
                ErrorLogger() << "DisplayedShipDesignManager::Load duplicate hull name = " << name;
                continue;
            }
            if (obsolete.first && (obsolete.second < 0 || obsolete.second >= m_obsolete_ui_event_count))
                ErrorLogger() << "DisplayedShipDesignManager::Load hull \"" << name
                              << "\" has an obsolete_ui_event_count = " << obsolete.second
                              << " which does not satisfy 0 < obsolete_ui_event_count < m_obsolete_ui_event_count = "
                              << m_obsolete_ui_event_count;
            m_ordered_hulls.push_back(name);
            m_hull_to_obsolete_and_loc[name] = std::make_pair(obsolete, --m_ordered_hulls.end());
        }

        // Clear and load the ship parts
        m_obsolete_parts = obsolete_parts;
        for (const auto& part_and_event_count : m_obsolete_parts) {
            const auto& name = part_and_event_count.first;
            const auto& count = part_and_event_count.second;
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
                design_ids_and_obsoletes.push_back({id, m_id_to_obsolete_and_loc.at(id).first});
            } catch (const std::out_of_range&) {
                ErrorLogger() << "DisplayedShipDesignManager::Save missing id = " << id;
                continue;
            }
        }

        hulls_and_obsoletes.clear();
        for (const auto name : m_ordered_hulls) {
            try {
               hulls_and_obsoletes.push_back({name, m_hull_to_obsolete_and_loc.at(name).first});
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

        AvailabilityManager(bool obsolete, bool available, bool unavailable);

        const DisplayedAvailabilies& GetAvailabilities() const { return m_availabilities; };
        bool GetAvailability(const Availability::Enum type) const;
        void SetAvailability(const Availability::Enum type, const bool state);
        void ToggleAvailability(const Availability::Enum type);

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
        DisplayedAvailabilies m_availabilities;
    };

    AvailabilityManager::AvailabilityManager(bool obsolete, bool available, bool unavailable) :
        m_availabilities{obsolete, available, unavailable}
    {}

    bool AvailabilityManager::GetAvailability(const Availability::Enum type) const {
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

    void AvailabilityManager::SetAvailability(const Availability::Enum type, const bool state) {
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

    void AvailabilityManager::ToggleAvailability(const Availability::Enum type)
    { SetAvailability(type, !GetAvailability(type)); }

    boost::optional<AvailabilityManager::DisplayedAvailabilies>
    AvailabilityManager::DisplayedDesignAvailability(const ShipDesign& design) const {
        int empire_id = HumanClientApp::GetApp()->EmpireID();
        const Empire* empire = GetEmpire(empire_id);  // may be nullptr
        bool available = empire ? empire->ShipDesignAvailable(design) : true;

        const auto& manager = GetDisplayedDesignsManager();
        const auto maybe_obsolete = manager.IsObsolete(design.ID());
        bool is_obsolete = maybe_obsolete && *maybe_obsolete;

        return DisplayedXAvailability(available, is_obsolete);
    }

    boost::optional<AvailabilityManager::DisplayedAvailabilies>
    AvailabilityManager::DisplayedHullAvailability(const std::string& id) const {
        int empire_id = HumanClientApp::GetApp()->EmpireID();
        const Empire* empire = GetEmpire(empire_id);  // may be nullptr
        bool available = empire ? empire->ShipHullAvailable(id) : true;

        const auto& manager = GetDisplayedDesignsManager();
        const auto obsolete = bool(manager.IsHullObsolete(id));

        return DisplayedXAvailability(available, obsolete);
    }

    boost::optional<AvailabilityManager::DisplayedAvailabilies>
    AvailabilityManager::DisplayedPartAvailability(const std::string& id) const {
        int empire_id = HumanClientApp::GetApp()->EmpireID();
        const Empire* empire = GetEmpire(empire_id);  // may be nullptr
        bool available = empire ? empire->ShipPartAvailable(id) : true;

        const auto& manager = GetDisplayedDesignsManager();
        const auto obsolete = bool(manager.IsPartObsolete(id));

        return DisplayedXAvailability(available, obsolete);
    }

    boost::optional<AvailabilityManager::DisplayedAvailabilies>
    AvailabilityManager::DisplayedXAvailability(bool available, bool obsolete) const {
        // TODO: C++17, Replace with structured binding auto [a, b, c] = m_availabilities;
        const bool showing_obsolete = std::get<Availability::Obsolete>(m_availabilities);
        const bool showing_available = std::get<Availability::Available>(m_availabilities);
        const bool showing_future = std::get<Availability::Future>(m_availabilities);

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
    m_displayed_designs(boost::make_unique<DisplayedShipDesignManager>()),
    m_saved_designs(boost::make_unique<SavedDesignsManager>())
{}

ShipDesignManager::~ShipDesignManager()
{}

void ShipDesignManager::StartGame(int empire_id, bool is_new_game) {
    auto empire = GetEmpire(empire_id);
    if (!empire) {
        ErrorLogger() << "Unable to initialize ShipDesignManager because empire id, " << empire_id << ", is invalid";
        return;
    }

    DebugLogger() << "ShipDesignManager initializing.";

    m_displayed_designs = boost::make_unique<DisplayedShipDesignManager>();
    auto displayed_designs = dynamic_cast<DisplayedShipDesignManager*>(m_displayed_designs.get());

    m_saved_designs = boost::make_unique<SavedDesignsManager>();
    auto saved_designs = dynamic_cast<SavedDesignsManager*>(m_saved_designs.get());
    saved_designs->StartParsingDesignsFromFileSystem(is_new_game);

    // Only setup saved and current designs for new games
    if (!is_new_game)
        return;

    // Initialize the hull ordering from the HullTypeManager
    for (const auto& name_and_type : GetHullTypeManager()) {
        const auto& hull_name = name_and_type.first;
        const auto& hull_type =  name_and_type.second;

        if (!hull_type || !hull_type->Producible())
            continue;
        displayed_designs->InsertHullBefore(hull_name);
    }

    // If requested initialize the current designs to all designs known by the empire
    if (GetOptionsDB().Get<bool>("resource.shipdesign.default.enabled")) {
        // While initializing a new game, before sending info to players, the
        // server should have added the default design ids to an empire's known
        // designs. Loop over these, and add them to "current" designs.
        DebugLogger() << "Add default designs to empire's current designs";
        const auto& ids = empire->ShipDesigns();
        std::set<int> ordered_ids(ids.begin(), ids.end());

        displayed_designs->InsertOrderedIDs(ordered_ids);
    }/* else {
        // Remove the default designs from the empire's current designs.
        DebugLogger() << "Remove default designs from empire";
        const auto ids = empire->ShipDesigns();
        for (const auto design_id : ids) {
            HumanClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<ShipDesignOrder>(empire_id, design_id, true));
        }
    }*/
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
    auto retval = m_displayed_designs.get();
    if (retval == nullptr) {
        ErrorLogger() << "ShipDesignManager m_displayed_designs was not correctly initialized "
                      << "with ShipDesignManager::GameStart().";
        m_displayed_designs = boost::make_unique<DisplayedShipDesignManager>();
        return m_displayed_designs.get();
    }
    return retval;
}

ShipDesignManager::Designs* ShipDesignManager::SavedDesigns() {
    auto retval = m_saved_designs.get();
    if (retval == nullptr) {
        ErrorLogger() << "ShipDesignManager m_saved_designs was not correctly initialized "
                      << "with ShipDesignManager::GameStart().";
        m_saved_designs = boost::make_unique<SavedDesignsManager>();
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
    /** \name Structors */ //@{
    PartControl(const PartType* part);
    //@}
    void CompleteConstruction() override;

    /** \name Accessors */ //@{
    const PartType*     Part() const { return m_part; }
    const std::string&  PartName() const { return m_part ? m_part->Name() : EMPTY_STRING; }
    //@}

    /** \name Mutators */ //@{
    void Render() override;
    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void SetAvailability(const AvailabilityManager::DisplayedAvailabilies& type);
    //@}

    mutable boost::signals2::signal<void (const PartType*, GG::Flags<GG::ModKey>)> ClickedSignal;
    mutable boost::signals2::signal<void (const PartType*, const GG::Pt& pt)> RightClickedSignal;
    mutable boost::signals2::signal<void (const PartType*)> DoubleClickedSignal;

private:
    std::shared_ptr<GG::StaticGraphic>  m_icon = nullptr;
    std::shared_ptr<GG::StaticGraphic>  m_background = nullptr;
    const PartType*                     m_part = nullptr;
};

PartControl::PartControl(const PartType* part) :
    GG::Control(GG::X0, GG::Y0, SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT, GG::INTERACTIVE),
    m_part(part)
{}

void PartControl::CompleteConstruction() {
    GG::Control::CompleteConstruction();
    if (!m_part)
        return;

    m_background = GG::Wnd::Create<GG::StaticGraphic>(PartBackgroundTexture(m_part), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_background->Resize(GG::Pt(SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT));
    m_background->Show();
    AttachChild(m_background);


    // position of part image centred within part control.  control is size of a slot, but the
    // part image is smaller
    GG::X part_left = (Width() - PART_CONTROL_WIDTH) / 2;
    GG::Y part_top = (Height() - PART_CONTROL_HEIGHT) / 2;

    //DebugLogger() << "PartControl::PartControl this: " << this << " part: " << part << " named: " << (part ? part->Name() : "no part");
    m_icon = GG::Wnd::Create<GG::StaticGraphic>(ClientUI::PartIcon(m_part->Name()), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
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

void PartControl::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ ClickedSignal(m_part, mod_keys); }

void PartControl::LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ DoubleClickedSignal(m_part); }

void PartControl::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
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

    /** \name Structors */ //@{
    PartsListBox(const AvailabilityManager& availabilities_state);
    //@}

    /** \name Accessors */ //@{
    const std::set<ShipPartClass>&  GetClassesShown() const;
    const AvailabilityManager&      AvailabilityState() const { return m_availabilities_state; }
    bool                            GetShowingSuperfluous() const { return m_show_superfluous_parts; }
    //@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;
    void AcceptDrops(const GG::Pt& pt, std::vector<std::shared_ptr<GG::Wnd>> wnds,
                     GG::Flags<GG::ModKey> mod_keys) override;

    PartGroupsType GroupAvailableDisplayableParts(const Empire* empire);
    void CullSuperfluousParts(std::vector<const PartType* >& this_group,
                              ShipPartClass pclass, int empire_id, int loc_id);
    void Populate();

    void ShowClass(ShipPartClass part_class, bool refresh_list = true);
    void ShowAllClasses(bool refresh_list = true);
    void HideClass(ShipPartClass part_class, bool refresh_list = true);
    void HideAllClasses(bool refresh_list = true);
    void ShowSuperfluousParts(bool refresh_list = true);
    void HideSuperfluousParts(bool refresh_list = true);
    //@}

    mutable boost::signals2::signal<void (const PartType*, GG::Flags<GG::ModKey>)>  PartTypeClickedSignal;
    mutable boost::signals2::signal<void (const PartType*)>                         PartTypeDoubleClickedSignal;
    mutable boost::signals2::signal<void (const PartType*, const GG::Pt& pt)>       PartTypeRightClickedSignal;
    mutable boost::signals2::signal<void (const std::string&)>                      ClearPartSignal;

protected:
    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const override;

private:
    std::set<ShipPartClass>     m_part_classes_shown;   // which part classes should be shown
    bool                        m_show_superfluous_parts;
    int                         m_previous_num_columns;
    const AvailabilityManager&  m_availabilities_state;
};

PartsListBox::PartsListBoxRow::PartsListBoxRow(GG::X w, GG::Y h, const AvailabilityManager& availabilities_state) :
    CUIListBox::Row(w, h, ""),    // drag_drop_data_type = "" implies not draggable row
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

    const auto part_type = part_control->Part();
    if (!part_type)
        return;

    auto new_part_control = GG::Wnd::Create<PartControl>(part_type);
    const auto parent = dynamic_cast<const PartsListBox*>(Parent().get());
    if (parent) {
        new_part_control->ClickedSignal.connect(
            parent->PartTypeClickedSignal);
        new_part_control->DoubleClickedSignal.connect(
            parent->PartTypeDoubleClickedSignal);
        new_part_control->RightClickedSignal.connect(
            parent->PartTypeRightClickedSignal);
    }

    // set availability shown
    auto shown = m_availabilities_state.DisplayedPartAvailability(part_type->Name());
    if (shown)
        new_part_control->SetAvailability(*shown);

    SetCell(ii, new_part_control);
}

PartsListBox::PartsListBox(const AvailabilityManager& availabilities_state) :
    CUIListBox(),
    m_part_classes_shown(),
    m_show_superfluous_parts(true),
    m_previous_num_columns(-1),
    m_availabilities_state(availabilities_state)
{
    ManuallyManageColProps();
    NormalizeRowsOnInsert(false);
    SetStyle(GG::LIST_NOSEL);
}

const std::set<ShipPartClass>& PartsListBox::GetClassesShown() const
{ return m_part_classes_shown; }

void PartsListBox::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    // maybe later do something interesting with docking
    CUIListBox::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size()) {
        // determine how many columns can fit in the box now...
        const GG::X TOTAL_WIDTH = Size().x - ClientUI::ScrollWidth();
        const int NUM_COLUMNS = std::max(1,
            Value(TOTAL_WIDTH / (SLOT_CONTROL_WIDTH + GG::X(PAD))));

        if (NUM_COLUMNS != m_previous_num_columns)
            Populate();
    }
}

/** Accept parts being discarded from the ship under design.*/
void PartsListBox::AcceptDrops(const GG::Pt& pt,
                               std::vector<std::shared_ptr<GG::Wnd>> wnds,
                               GG::Flags<GG::ModKey> mod_keys)
{
    // If ctrl is pressed then signal all parts of the same type to be cleared.
    if (!(GG::GUI::GetGUI()->ModKeys() & GG::MOD_KEY_CTRL))
        return;

    if (wnds.empty())
        return;

    const PartControl* control = boost::polymorphic_downcast<const PartControl*>(wnds.begin()->get());
    const PartType* part_type = control ? control->Part() : nullptr;
    if (!part_type)
        return;

    ClearPartSignal(part_type->Name());
}

PartGroupsType PartsListBox::GroupAvailableDisplayableParts(const Empire* empire) {
    PartGroupsType part_groups;
    // loop through all possible parts
    for (const auto& entry : GetPartTypeManager()) {
        const auto& part = entry.second;
        if (!part->Producible())
            continue;

        // check whether this part should be shown in list
        ShipPartClass part_class = part->Class();
        if (!m_part_classes_shown.count(part_class))
            continue;   // part of this class is not requested to be shown

        // Check if part satisfies availability and obsolecense
        auto shown = m_availabilities_state.DisplayedPartAvailability(part->Name());
        if (!shown)
            continue;

        for (ShipSlotType slot_type : part->MountableSlotTypes()) {
            part_groups[{part_class, slot_type}].push_back(part.get());
        }
    }
    return part_groups;
}

// Checks if the Location condition of the check_part totally contains the Location condition of ref_part
// i,e,, the ref_part condition is met anywhere the check_part condition is
bool LocationASubsumesLocationB(const Condition::ConditionBase* check_part_loc,
                                const Condition::ConditionBase* ref_part_loc)
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

bool PartALocationSubsumesPartB(const PartType* check_part, const PartType* ref_part) {
    static std::map<std::pair<std::string, std::string>, bool> part_loc_comparison_map;

    auto part_pair = std::make_pair(check_part->Name(), ref_part->Name());
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

    for (auto part_it = this_group.begin();
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
    const Empire* empire = GetEmpire(empire_id);  // may be nullptr

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
    PartGroupsType part_groups = GroupAvailableDisplayableParts(empire);

    // get empire id and location to use for cost and time comparisons
    int loc_id = INVALID_OBJECT_ID;
    if (empire) {
        auto location = GetUniverseObject(empire->CapitalID());
        loc_id = location ? location->ID() : INVALID_OBJECT_ID;
    }

    // if showing parts for a particular empire, cull redundant parts (if enabled)
    if (empire) {
        for (auto& part_group : part_groups) {
            ShipPartClass pclass = part_group.first.first;
            if (!m_show_superfluous_parts)
                CullSuperfluousParts(part_group.second, pclass, empire_id, loc_id);
        }
    }

    // now sort the parts within each group according to main stat, via weak
    // sorting in a multimap also, if a part was in multiple groups due to being
    // compatible with multiple slot types, ensure it is only displayed once
    std::set<const PartType*> already_added;
    for (auto& part_group : part_groups) {
        std::multimap<double, const PartType*> sorted_group;
        for (const PartType* part : part_group.second) {
            if (already_added.count(part))
                continue;
            already_added.insert(part);
            sorted_group.insert({GetMainStat(part), part});
        }

        // take the sorted parts and make UI elements (technically rows) for the PartsListBox
        for (auto& group : sorted_group) {
            const PartType* part = group.second;
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
            control->ClickedSignal.connect(
                PartsListBox::PartTypeClickedSignal);
            control->DoubleClickedSignal.connect(
                PartsListBox::PartTypeDoubleClickedSignal);
            control->RightClickedSignal.connect(
                PartsListBox::PartTypeRightClickedSignal);

            auto shown = m_availabilities_state.DisplayedPartAvailability(part->Name());
            if (shown)
                control->SetAvailability(*shown);

            cur_row->push_back(control);
        }
    }
    // add any incomplete rows
    if (cur_row)
        Insert(cur_row);

    // keep track of how many columns are present now
    m_previous_num_columns = NUM_COLUMNS;

    // If there are no parts add a prompt to suggest a solution.
    if (num_parts == 0)
        Insert(GG::Wnd::Create<PromptRow>(TOTAL_WIDTH,
                                          UserString("ALL_AVAILABILITY_FILTERS_BLOCKING_PROMPT")),
               begin(), false);

}

void PartsListBox::ShowClass(ShipPartClass part_class, bool refresh_list) {
    if (!m_part_classes_shown.count(part_class)) {
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
    /** \name Structors */ //@{
    PartPalette(const std::string& config_name);
    void CompleteConstruction() override;
    //@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

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
    //@}

    mutable boost::signals2::signal<void (const PartType*, GG::Flags<GG::ModKey>)> PartTypeClickedSignal;
    mutable boost::signals2::signal<void (const PartType*)> PartTypeDoubleClickedSignal;
    mutable boost::signals2::signal<void (const PartType*, const GG::Pt& pt)> PartTypeRightClickedSignal;
    mutable boost::signals2::signal<void ()> PartObsolescenceChangedSignal;
    mutable boost::signals2::signal<void (const std::string&)> ClearPartSignal;

private:
    void DoLayout();

    /** A part type click with ctrl obsoletes part. */
    void HandlePartTypeClicked(const PartType*, GG::Flags<GG::ModKey>);
    void HandlePartTypeRightClicked(const PartType*, const GG::Pt& pt);

    std::shared_ptr<PartsListBox>                               m_parts_list = nullptr;
    std::map<ShipPartClass, std::shared_ptr<CUIStateButton>>    m_class_buttons;
    std::shared_ptr<CUIStateButton>                             m_superfluous_parts_button = nullptr;

    // Holds the state of the availabilities filter.
    AvailabilityManager m_availabilities_state;
    std::tuple<std::shared_ptr<CUIStateButton>,
               std::shared_ptr<CUIStateButton>,
               std::shared_ptr<CUIStateButton>> m_availabilities_buttons;

};

DesignWnd::PartPalette::PartPalette(const std::string& config_name) :
    CUIWnd(UserString("DESIGN_WND_PART_PALETTE_TITLE"),
           GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE,
           config_name),
    m_availabilities_state(false, true, false)
{}

void DesignWnd::PartPalette::CompleteConstruction() {
    //TempUISoundDisabler sound_disabler;     // should be redundant with disabler in DesignWnd::DesignWnd.  uncomment if this is not the case
    SetChildClippingMode(ClipToClient);

    m_parts_list = GG::Wnd::Create<PartsListBox>(m_availabilities_state);
    AttachChild(m_parts_list);
    m_parts_list->PartTypeClickedSignal.connect(
        boost::bind(&DesignWnd::PartPalette::HandlePartTypeClicked, this, _1, _2));
    m_parts_list->PartTypeDoubleClickedSignal.connect(
        PartTypeDoubleClickedSignal);
    m_parts_list->PartTypeRightClickedSignal.connect(
        boost::bind(&DesignWnd::PartPalette::HandlePartTypeRightClicked, this, _1, _2));
    m_parts_list->ClearPartSignal.connect(ClearPartSignal);

    const PartTypeManager& part_manager = GetPartTypeManager();

    // class buttons
    for (ShipPartClass part_class = ShipPartClass(0); part_class != NUM_SHIP_PART_CLASSES; part_class = ShipPartClass(part_class + 1)) {
        // are there any parts of this class?
        bool part_of_this_class_exists = false;
        for (const auto& entry : part_manager) {
            if (const auto& part = entry.second) {
                if (part->Class() == part_class) {
                    part_of_this_class_exists = true;
                    break;
                }
            }
        }
        if (!part_of_this_class_exists)
            continue;

        m_class_buttons[part_class] = GG::Wnd::Create<CUIStateButton>(UserString(boost::lexical_cast<std::string>(part_class)), GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
        AttachChild(m_class_buttons[part_class]);
        m_class_buttons[part_class]->CheckedSignal.connect(
            boost::bind(&DesignWnd::PartPalette::ToggleClass, this, part_class, true));
    }

    // availability buttons
    // TODO: C++17, Collect and replace with structured binding auto [a, b, c] = m_availabilities;
    auto& m_obsolete_button = std::get<Availability::Obsolete>(m_availabilities_buttons);
    m_obsolete_button = GG::Wnd::Create<CUIStateButton>(UserString("PRODUCTION_WND_AVAILABILITY_OBSOLETE"), GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(m_obsolete_button);
    m_obsolete_button->CheckedSignal.connect(
        boost::bind(&DesignWnd::PartPalette::ToggleAvailability, this, Availability::Obsolete));
    m_obsolete_button->SetCheck(m_availabilities_state.GetAvailability(Availability::Obsolete));

    auto& m_available_button = std::get<Availability::Available>(m_availabilities_buttons);
    m_available_button = GG::Wnd::Create<CUIStateButton>(UserString("PRODUCTION_WND_AVAILABILITY_AVAILABLE"), GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(m_available_button);
    m_available_button->CheckedSignal.connect(
        boost::bind(&DesignWnd::PartPalette::ToggleAvailability, this, Availability::Available));
    m_available_button->SetCheck(m_availabilities_state.GetAvailability(Availability::Available));

    auto& m_unavailable_button = std::get<Availability::Future>(m_availabilities_buttons);
    m_unavailable_button = GG::Wnd::Create<CUIStateButton>(UserString("PRODUCTION_WND_AVAILABILITY_UNAVAILABLE"), GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(m_unavailable_button);
    m_unavailable_button->CheckedSignal.connect(
        boost::bind(&DesignWnd::PartPalette::ToggleAvailability, this, Availability::Future));
    m_unavailable_button->SetCheck(m_availabilities_state.GetAvailability(Availability::Future));

    // superfluous parts button
    m_superfluous_parts_button = GG::Wnd::Create<CUIStateButton>(UserString("PRODUCTION_WND_REDUNDANT"), GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(m_superfluous_parts_button);
    m_superfluous_parts_button->CheckedSignal.connect(
        boost::bind(&DesignWnd::PartPalette::ToggleSuperfluous, this, true));

    // default to showing nothing
    ShowAllClasses(false);
    ShowSuperfluous(false);
    Populate();

    CUIWnd::CompleteConstruction();

    DoLayout();
    SaveDefaultedOptions();
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
    const int NUM_AVAILABILITY_BUTTONS = 3;
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
    for (auto& entry : m_class_buttons) {
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
    m_parts_list->SizeMove(GG::Pt(GG::X0, BUTTON_EDGE_PAD + ROW_OFFSET*(row + 1)),
                           ClientSize() - GG::Pt(GG::X(BUTTON_SEPARATION), GG::Y(BUTTON_SEPARATION)));

    // place slot type buttons
    col = NUM_CLASS_BUTTONS_PER_ROW;
    row = 0;
    auto ul = GG::Pt(BUTTON_EDGE_PAD + col*COL_OFFSET, BUTTON_EDGE_PAD + row*ROW_OFFSET);
    auto lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_superfluous_parts_button->SizeMove(ul, lr);

    // a function to place availability buttons either in a single column below the
    // superfluous button or to complete a 2X2 grid left of the class buttons.
    auto place_avail_button_adjacent =
        [&col, &row, &num_non_class_buttons_per_row, NUM_CLASS_BUTTONS_PER_ROW,
         BUTTON_EDGE_PAD, COL_OFFSET, ROW_OFFSET, BUTTON_WIDTH, BUTTON_HEIGHT]
        (GG::Wnd* avail_btn)
        {
            if (num_non_class_buttons_per_row == 1)
                ++row;
            else {
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
    // TODO: C++17, Replace with structured binding auto [a, b, c] = m_availabilities;
    auto& m_obsolete_button = std::get<Availability::Obsolete>(m_availabilities_buttons);
    auto& m_available_button = std::get<Availability::Available>(m_availabilities_buttons);
    auto& m_unavailable_button = std::get<Availability::Future>(m_availabilities_buttons);

    place_avail_button_adjacent(m_obsolete_button.get());
    place_avail_button_adjacent(m_available_button.get());
    place_avail_button_adjacent(m_unavailable_button.get());
}

void DesignWnd::PartPalette::HandlePartTypeClicked(const PartType* part_type, GG::Flags<GG::ModKey> modkeys) {
    // Toggle obsolete for a control click.
    if (modkeys & GG::MOD_KEY_CTRL) {
        auto& manager = GetDisplayedDesignsManager();
        const auto obsolete = manager.IsPartObsolete(part_type->Name());
        manager.SetPartObsolete(part_type->Name(), !obsolete);

        PartObsolescenceChangedSignal();
        Populate();
    }
    else
        PartTypeClickedSignal(part_type, modkeys);
}

void DesignWnd::PartPalette::HandlePartTypeRightClicked(const PartType* part_type, const GG::Pt& pt) {
    // Context menu actions
    auto& manager = GetDisplayedDesignsManager();
    const auto& part_name = part_type->Name();
    auto is_obsolete = manager.IsPartObsolete(part_name);
    auto toggle_obsolete_design_action = [&manager, &part_name, is_obsolete, this]() {
        manager.SetPartObsolete(part_name, !is_obsolete);
        PartObsolescenceChangedSignal();
        Populate();
    };

    // create popup menu with a commands in it
    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

    const auto empire_id = HumanClientApp::GetApp()->EmpireID();
    if (empire_id != ALL_EMPIRES)
        popup->AddMenuItem(GG::MenuItem(
                               (is_obsolete
                                ? UserString("DESIGN_WND_UNOBSOLETE_PART")
                                : UserString("DESIGN_WND_OBSOLETE_PART")),
                               false, false, toggle_obsolete_design_action));

    popup->Run();

    PartTypeRightClickedSignal(part_type, pt);
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
    for (auto& entry : m_class_buttons)
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
    for (auto& entry : m_class_buttons)
        entry.second->SetCheck(false);
}

void DesignWnd::PartPalette::ToggleClass(ShipPartClass part_class, bool refresh_list) {
    if (part_class >= ShipPartClass(0) && part_class < NUM_SHIP_PART_CLASSES) {
        const auto& classes_shown = m_parts_list->GetClassesShown();
        if (!classes_shown.count(part_class))
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
    if (classes_shown.size() == NUM_SHIP_PART_CLASSES)
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
    static const std::string BASES_LIST_BOX_DROP_TYPE;

    /** \name Structors */ //@{
    BasesListBox(const AvailabilityManager& availabilities_state,
                 const boost::optional<std::string>& drop_type = boost::none,
                 const boost::optional<std::string>& empty_prompt = boost::none);
    //@}
    void CompleteConstruction() override;

    /** \name Accessors */ //@{
    //@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;
    void ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination) override;
    virtual void QueueItemMoved(const GG::ListBox::iterator& row_it, const GG::ListBox::iterator& original_position_it)
    {}
    void SetEmpireShown(int empire_id, bool refresh_list = true);
    virtual void Populate();
    //@}

    mutable boost::signals2::signal<void (int)>                 DesignSelectedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::vector<std::string>&)>
                                                                DesignComponentsSelectedSignal;
    mutable boost::signals2::signal<void (const boost::uuids::uuid&)>  SavedDesignSelectedSignal;

    mutable boost::signals2::signal<void (const ShipDesign*)>   DesignClickedSignal;
    mutable boost::signals2::signal<void (const HullType*)>     HullClickedSignal;
    mutable boost::signals2::signal<void (const ShipDesign*)>   DesignRightClickedSignal;

    class HullAndNamePanel : public GG::Control {
    public:
        HullAndNamePanel(GG::X w, GG::Y h, const std::string& hull, const std::string& name);

        void CompleteConstruction() override;
        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

        void Render() override
        {}

        void SetAvailability(const AvailabilityManager::DisplayedAvailabilies& type);
        void SetDisplayName(const std::string& name);

    private:
        std::shared_ptr<GG::StaticGraphic>  m_graphic = nullptr;
        std::shared_ptr<GG::Label>          m_name = nullptr;
    };

    class BasesListBoxRow : public CUIListBox::Row {
    public:
        BasesListBoxRow(GG::X w, GG::Y h, const std::string& hull, const std::string& name);

        void CompleteConstruction() override;
        void Render() override;

        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

        virtual void SetAvailability(const AvailabilityManager::DisplayedAvailabilies& type);
        virtual void SetDisplayName(const std::string& name);

    private:
        std::shared_ptr<HullAndNamePanel> m_hull_panel = nullptr;
    };

    class HullAndPartsListBoxRow : public BasesListBoxRow {
    public:
        HullAndPartsListBoxRow(GG::X w, GG::Y h, const std::string& hull,
                               const std::vector<std::string>& parts);
        void CompleteConstruction() override;
        const std::string&              Hull() const    { return m_hull_name; }
        const std::vector<std::string>& Parts() const   { return m_parts; }

    protected:
        std::string                     m_hull_name;
        std::vector<std::string>        m_parts;
    };

    class CompletedDesignListBoxRow : public BasesListBoxRow {
    public:
        CompletedDesignListBoxRow(GG::X w, GG::Y h, const ShipDesign& design);
        void CompleteConstruction() override;
        int DesignID() const { return m_design_id; }
    private:
        int m_design_id = INVALID_DESIGN_ID;
    };

protected:
    void ItemRightClickedImpl(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) override;

    /** An implementation of BasesListBox provides a PopulateCore to fill itself.*/
    virtual void PopulateCore() = 0;

    /** Reset the empty list prompt. */
    virtual void ResetEmptyListPrompt();

    /** If \p wnd is a valid dragged child return a replacement row.  Otherwise return nullptr. */
    virtual std::shared_ptr<Row> ChildrenDraggedAwayCore(const GG::Wnd* const wnd) = 0;

    /** \name Accessors for derived classes. */ //@{
    int EmpireID() const { return m_empire_id_shown; }

    const AvailabilityManager& AvailabilityState() const
    { return m_availabilities_state; }

    GG::Pt  ListRowSize();
    //@}

    virtual void BaseDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys)
    {}
    virtual void BaseLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys)
    {}
    virtual void BaseRightClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys)
    {}

private:
    void InitRowSizes();

    int                         m_empire_id_shown = ALL_EMPIRES;
    const AvailabilityManager&  m_availabilities_state;
    boost::signals2::connection m_empire_designs_changed_signal;
};

BasesListBox::HullAndNamePanel::HullAndNamePanel(GG::X w, GG::Y h, const std::string& hull, const std::string& name) :
    GG::Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS)
{
    SetChildClippingMode(ClipToClient);

    m_graphic = GG::Wnd::Create<GG::StaticGraphic>(ClientUI::HullIcon(hull),
                                                   GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC);
    m_graphic->Resize(GG::Pt(w, h));
    m_name = GG::Wnd::Create<CUILabel>(name, GG::FORMAT_WORDBREAK | GG::FORMAT_CENTER | GG::FORMAT_TOP);
}

void BasesListBox::HullAndNamePanel::CompleteConstruction() {
    GG::Control::CompleteConstruction();
    AttachChild(m_graphic);
    AttachChild(m_name);
}

void BasesListBox::HullAndNamePanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
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

void BasesListBox::HullAndNamePanel::SetDisplayName(const std::string& name) {
    m_name->SetText(name);
    m_name->Resize(GG::Pt(Width(), m_name->Height()));
}

BasesListBox::BasesListBoxRow::BasesListBoxRow(GG::X w, GG::Y h, const std::string& hull, const std::string& name) :
    CUIListBox::Row(w, h, BASES_LIST_BOX_DROP_TYPE)
{
    if (hull.empty()) {
        ErrorLogger() << "No hull name provided for ship row display.";
        return;
    }

    m_hull_panel = GG::Wnd::Create<HullAndNamePanel>(w, h, hull, name);

    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
}

void BasesListBox::BasesListBoxRow::CompleteConstruction() {
    CUIListBox::Row::CompleteConstruction();
    push_back(m_hull_panel);
}

void BasesListBox::BasesListBoxRow::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::Pt ul_adjusted_for_drop_indicator = GG::Pt(ul.x, ul.y + GG::Y(1));
    GG::Pt lr_adjusted_for_drop_indicator = GG::Pt(lr.x, lr.y - GG::Y(2));
    GG::FlatRectangle(ul_adjusted_for_drop_indicator, lr_adjusted_for_drop_indicator,
                      ClientUI::WndColor(),
                      (Disabled() ? DisabledColor(GG::CLR_WHITE) : GG::CLR_WHITE), 1);
}

void BasesListBox::BasesListBoxRow::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
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

void BasesListBox::BasesListBoxRow::SetDisplayName(const std::string& name) {
    if (m_hull_panel)
        m_hull_panel->SetDisplayName(name);
}

BasesListBox::HullAndPartsListBoxRow::HullAndPartsListBoxRow(GG::X w, GG::Y h, const std::string& hull,
                                                             const std::vector<std::string>& parts) :
    BasesListBoxRow(w, h, hull, UserString(hull)),
    m_hull_name(hull),
    m_parts(parts)
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

const std::string BasesListBox::BASES_LIST_BOX_DROP_TYPE = "BasesListBoxRow";

BasesListBox::BasesListBox(const AvailabilityManager& availabilities_state,
                           const boost::optional<std::string>& drop_type,
                           const boost::optional<std::string>& empty_prompt /*= boost::none*/) :
    QueueListBox(drop_type,
                 empty_prompt ? *empty_prompt : UserString("ADD_FIRST_DESIGN_DESIGN_QUEUE_PROMPT")),
    m_empire_id_shown(ALL_EMPIRES),
    m_availabilities_state(availabilities_state)
{}

void BasesListBox::CompleteConstruction() {
    QueueListBox::CompleteConstruction();

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
    const auto control = dynamic_cast<const GG::Control*>(wnd);
    if (!control)
        return;

    Row* original_row = boost::polymorphic_downcast<Row*>(*wnds.begin());
    iterator insertion_point = std::find_if(
        begin(), end(), [&original_row](const std::shared_ptr<Row>& xx){return xx.get() == original_row;});
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
        m_empire_designs_changed_signal = empire->ShipDesignsChangedSignal.connect(
                                            boost::bind(&BasesListBox::Populate, this));

    if (refresh_list)
        Populate();
}

void BasesListBox::Populate() {
    DebugLogger() << "BasesListBox::Populate";

    // Provide conditional reminder text when the list is empty
    if (AvailabilityState().GetAvailabilities() == AvailabilityManager::DisplayedAvailabilies(false, false, false))
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

void BasesListBox::ItemRightClickedImpl(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys)
{ this->BaseRightClicked(it, pt, modkeys); }


//////////////////////////////////////////////////
// BasesListBox derived classes                 //
//////////////////////////////////////////////////
class EmptyHullsListBox : public BasesListBox {
public:
    EmptyHullsListBox(const AvailabilityManager& availabilities_state,
                      const boost::optional<std::string>& drop_type = boost::none) :
        BasesListBox::BasesListBox(availabilities_state, drop_type, UserString("ALL_AVAILABILITY_FILTERS_BLOCKING_PROMPT"))
    {}

    void EnableOrderIssuing(bool enable = true) override;

protected:
    void PopulateCore() override;
    std::shared_ptr<Row> ChildrenDraggedAwayCore(const GG::Wnd* const wnd) override;
    void QueueItemMoved(const GG::ListBox::iterator& row_it, const GG::ListBox::iterator& original_position_it) override;

    void BaseDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) override;
    void BaseLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) override;
    void BaseRightClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) override;
};

class CompletedDesignsListBox : public BasesListBox {
public:
    CompletedDesignsListBox(const AvailabilityManager& availabilities_state,
                            const boost::optional<std::string>& drop_type = boost::none) :
        BasesListBox::BasesListBox(availabilities_state, drop_type)
    {};

protected:
    void PopulateCore() override;
    void ResetEmptyListPrompt() override;
    std::shared_ptr<Row> ChildrenDraggedAwayCore(const GG::Wnd* const wnd) override;
    void QueueItemMoved(const GG::ListBox::iterator& row_it, const GG::ListBox::iterator& original_position_it) override;
    void BaseDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) override;
    void BaseLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) override;
    void BaseRightClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) override;
};

class SavedDesignsListBox : public BasesListBox {
public:
    SavedDesignsListBox(const AvailabilityManager& availabilities_state,
                        const boost::optional<std::string>& drop_type = boost::none) :
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
    void QueueItemMoved(const GG::ListBox::iterator& row_it, const GG::ListBox::iterator& original_position_it) override;

    void BaseDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) override;
    void BaseLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) override;
    void BaseRightClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) override;
};

class MonstersListBox : public BasesListBox {
public:
    MonstersListBox(const AvailabilityManager& availabilities_state,
                    const boost::optional<std::string>& drop_type = boost::none) :
        BasesListBox::BasesListBox(availabilities_state, drop_type)
    {}

    void EnableOrderIssuing(bool enable = true) override;

protected:
    void PopulateCore() override;
    std::shared_ptr<Row> ChildrenDraggedAwayCore(const GG::Wnd* const wnd) override;

    void BaseDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) override;
    void BaseLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) override;
};

class AllDesignsListBox : public BasesListBox {
public:
    AllDesignsListBox(const AvailabilityManager& availabilities_state,
                      const boost::optional<std::string>& drop_type = boost::none) :
        BasesListBox::BasesListBox(availabilities_state, drop_type)
    {}

protected:
    void PopulateCore() override;
    std::shared_ptr<Row> ChildrenDraggedAwayCore(const GG::Wnd* const wnd) override;

    void BaseDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) override;
    void BaseLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) override;
};

void EmptyHullsListBox::PopulateCore() {
    ScopedTimer scoped_timer("EmptyHulls::PopulateCore");
    DebugLogger() << "EmptyHulls::PopulateCore EmpireID(): " << EmpireID();

    const GG::Pt row_size = ListRowSize();

    const auto& manager = GetDisplayedDesignsManager();

    auto hulls = manager.OrderedHulls();
    if (hulls.size() < GetHullTypeManager().size()) {
        ErrorLogger() << "EmptyHulls::PopulateCoreordered has fewer than expected entries...";
        for (auto& hull : GetHullTypeManager()) {
            auto it = std::find(hulls.begin(), hulls.end(), hull.first);
            if (it == hulls.end())
                hulls.push_back(hull.first);    // O(N^2) in loop, but I don't care...
        }
    }

    for (const auto& hull_name : hulls) {
        const auto& hull_type =  GetHullTypeManager().GetHullType(hull_name);

        if (!hull_type || !hull_type->Producible())
            continue;

        auto shown = AvailabilityState().DisplayedHullAvailability(hull_name);
        if (!shown)
            continue;
        const std::vector<std::string> empty_parts_vec;
        auto row = GG::Wnd::Create<HullAndPartsListBoxRow>(row_size.x, row_size.y, hull_name, empty_parts_vec);
        row->SetAvailability(*shown);
        Insert(row);
        row->Resize(row_size);
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
                const ShipDesign* design = GetShipDesign(design_id);
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
        for (auto it = universe.beginShipDesigns();
             it != universe.endShipDesigns(); ++it)
        {
            const ShipDesign* design = it->second;
            if (!design->Producible())
                continue;
            auto row = GG::Wnd::Create<CompletedDesignListBoxRow>(row_size.x, row_size.y, *design);
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

    for (auto it = universe.beginShipDesigns();
         it != universe.endShipDesigns(); ++it)
    {
        const ShipDesign* design = it->second;
        if (!design->IsMonster())
            continue;
        auto row = GG::Wnd::Create<CompletedDesignListBoxRow>(row_size.x, row_size.y, *design);
        Insert(row);
        row->Resize(row_size);
    }
}

void AllDesignsListBox::PopulateCore() {
    ScopedTimer scoped_timer("All::PopulateCore");

    const Universe& universe = GetUniverse();

    const GG::Pt row_size = ListRowSize();

    for (auto it = universe.beginShipDesigns();
         it != universe.endShipDesigns(); ++it)
    {
        const ShipDesign* design = it->second;
        auto row = GG::Wnd::Create<CompletedDesignListBoxRow>(row_size.x, row_size.y, *design);
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
    std::vector<std::string> empty_parts_vec;
    auto row =  GG::Wnd::Create<HullAndPartsListBoxRow>(row_size.x, row_size.y, hull_name, empty_parts_vec);

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
    const ShipDesign* design = GetShipDesign(design_id);
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
    const ShipDesign* design = GetShipDesign(design_id);
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
    const ShipDesign* design = GetShipDesign(design_id);
    if (!design) {
        ErrorLogger() << "Missing design with id " << design_id;
        return nullptr;
    }

    const auto row_size = ListRowSize();
    auto row = GG::Wnd::Create<CompletedDesignListBoxRow>(row_size.x, row_size.y, *design);
    return row;
}


void EmptyHullsListBox::EnableOrderIssuing(bool enable/* = true*/)
{ QueueListBox::EnableOrderIssuing(enable); }

void MonstersListBox::EnableOrderIssuing(bool)
{ QueueListBox::EnableOrderIssuing(false); }


void EmptyHullsListBox::BaseDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt,
                                          const GG::Flags<GG::ModKey>& modkeys)
{
    const auto hp_row = dynamic_cast<HullAndPartsListBoxRow*>(it->get());
    if (!hp_row)
        return;

    if (!hp_row->Hull().empty() || !hp_row->Parts().empty())
        DesignComponentsSelectedSignal(hp_row->Hull(), hp_row->Parts());
}

void CompletedDesignsListBox::BaseDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt,
                                                const GG::Flags<GG::ModKey>& modkeys)
{
    const auto cd_row = dynamic_cast<CompletedDesignListBoxRow*>(it->get());
    if (!cd_row || cd_row->DesignID() == INVALID_DESIGN_ID)
        return;

    DesignSelectedSignal(cd_row->DesignID());
}

void SavedDesignsListBox::BaseDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt,
                                            const GG::Flags<GG::ModKey>& modkeys)
{
    const auto sd_row = dynamic_cast<SavedDesignListBoxRow*>(it->get());

    if (!sd_row)
        return;
    SavedDesignSelectedSignal(sd_row->DesignUUID());
}

void MonstersListBox::BaseDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt,
                                        const GG::Flags<GG::ModKey>& modkeys)
{
    const auto cd_row = dynamic_cast<CompletedDesignListBoxRow*>(it->get());
    if (!cd_row || cd_row->DesignID() == INVALID_DESIGN_ID)
        return;

    DesignSelectedSignal(cd_row->DesignID());
}

void AllDesignsListBox::BaseDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt,
                                        const GG::Flags<GG::ModKey>& modkeys)
{
    const auto cd_row = dynamic_cast<CompletedDesignListBoxRow*>(it->get());
    if (!cd_row || cd_row->DesignID() == INVALID_DESIGN_ID)
        return;

    DesignSelectedSignal(cd_row->DesignID());
}


void EmptyHullsListBox::BaseLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt,
                                        const GG::Flags<GG::ModKey>& modkeys)
{
    const auto hull_parts_row = dynamic_cast<HullAndPartsListBoxRow*>(it->get());
    if (!hull_parts_row)
        return;
    const std::string& hull_name = hull_parts_row->Hull();
    const HullType* hull_type = GetHullType(hull_name);
    const std::vector<std::string>& parts = hull_parts_row->Parts();

    if (modkeys & GG::MOD_KEY_CTRL) {
        // Toggle hull obsolete
        auto& manager = GetDisplayedDesignsManager();
        const auto is_obsolete = manager.IsHullObsolete(hull_name);
        manager.SetHullObsolete(hull_name, !is_obsolete);
        Populate();
    }
    else if (hull_type && parts.empty())
        HullClickedSignal(hull_type);
}

void CompletedDesignsListBox::BaseLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt,
                                              const GG::Flags<GG::ModKey>& modkeys)
{
    const auto design_row = dynamic_cast<CompletedDesignListBoxRow*>(it->get());
    if (!design_row)
        return;
    int id = design_row->DesignID();
    const ShipDesign* design = GetShipDesign(id);
    if (!design)
        return;

    const auto& manager = GetDisplayedDesignsManager();

    if (modkeys & GG::MOD_KEY_CTRL && manager.IsKnown(id)) {
        const auto maybe_obsolete = manager.IsObsolete(id);
        bool is_obsolete = maybe_obsolete && *maybe_obsolete;
        SetObsoleteInDisplayedDesigns(id, !is_obsolete);
        Populate();

    } else {
        DesignClickedSignal(design);
    }
}

void SavedDesignsListBox::BaseLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt,
                                          const GG::Flags<GG::ModKey>& modkeys)
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

void MonstersListBox::BaseLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt,
                                      const GG::Flags<GG::ModKey>& modkeys)
{
    const auto design_row = dynamic_cast<CompletedDesignListBoxRow*>(it->get());
    if (!design_row)
        return;
    int id = design_row->DesignID();
    const ShipDesign* design = GetShipDesign(id);
    if (!design)
        return;

    DesignClickedSignal(design);
}

void AllDesignsListBox::BaseLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt,
                                        const GG::Flags<GG::ModKey>& modkeys)
{
    const auto design_row = dynamic_cast<CompletedDesignListBoxRow*>(it->get());
    if (!design_row)
        return;
    int id = design_row->DesignID();
    const ShipDesign* design = GetShipDesign(id);
    if (!design)
        return;

    DesignClickedSignal(design);
}


void EmptyHullsListBox::BaseRightClicked(GG::ListBox::iterator it, const GG::Pt& pt,
                                         const GG::Flags<GG::ModKey>& modkeys)
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

void CompletedDesignsListBox::BaseRightClicked(GG::ListBox::iterator it, const GG::Pt& pt,
                                               const GG::Flags<GG::ModKey>& modkeys)
{
    const auto design_row = dynamic_cast<CompletedDesignListBoxRow*>(it->get());
    if (!design_row)
        return;

    const auto design_id = design_row->DesignID();
    const auto design = GetShipDesign(design_id);
    if (!design)
        return;

    DesignRightClickedSignal(design);

    const auto empire_id = EmpireID();

    DebugLogger() << "BasesListBox::BaseRightClicked on design id : " << design_id;

    if (design->UUID() == boost::uuids::uuid{{0}})
        ErrorLogger() << "BasesListBox::BaseRightClicked Design UUID is null";

    // Context menu actions
    const auto& manager = GetDisplayedDesignsManager();
    const auto maybe_obsolete = manager.IsObsolete(design_id);
    bool is_obsolete = maybe_obsolete && *maybe_obsolete;
    auto toggle_obsolete_design_action = [&design_id, is_obsolete, this]() {
        SetObsoleteInDisplayedDesigns(design_id, !is_obsolete);
        Populate();
    };

    auto delete_design_action = [&design_id, this]() {
        DeleteFromDisplayedDesigns(design_id);
        Populate();
    };

    auto rename_design_action = [&empire_id, &design_id, design, &design_row]() {
        auto edit_wnd = GG::Wnd::Create<CUIEditWnd>(GG::X(350), UserString("DESIGN_ENTER_NEW_DESIGN_NAME"), design->Name());
        edit_wnd->Run();
        const std::string& result = edit_wnd->Result();
        if (!result.empty() && result != design->Name()) {
            HumanClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<ShipDesignOrder>(empire_id, design_id, result));
            design_row->SetDisplayName(design->Name());
        }
    };

    auto movetotop_design_action = [&design, this]() {
        GetDisplayedDesignsManager().MoveBefore(design->ID(), *GetDisplayedDesignsManager().OrderedIDs().begin());
        Populate();
    };

    auto movetobottom_design_action = [&design, this]() {
        GetDisplayedDesignsManager().MoveBefore(design->ID(), INVALID_DESIGN_ID);
        Populate();
    };

    auto save_design_action = [&design]() {
        auto saved_design = *design;
        saved_design.SetUUID(boost::uuids::random_generator()());
        GetSavedDesignsManager().InsertBefore(saved_design, GetSavedDesignsManager().OrderedDesignUUIDs().begin());
    };

    // toggle the option to add all saved designs at game start.
    const auto add_defaults = GetOptionsDB().Get<bool>("resource.shipdesign.default.enabled");
    auto toggle_add_default_designs_at_game_start_action = [add_defaults]() {
        GetOptionsDB().Set<bool>("resource.shipdesign.default.enabled", !add_defaults);
    };

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

void SavedDesignsListBox::BaseRightClicked(GG::ListBox::iterator it, const GG::Pt& pt,
                                           const GG::Flags<GG::ModKey>& modkeys)
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
        GetSavedDesignsManager().MoveBefore(design->UUID(), *GetSavedDesignsManager().OrderedDesignUUIDs().begin());
        Populate();
    };

    auto movetobottom_design_action = [&design, this]() {
        GetSavedDesignsManager().MoveBefore(design->UUID(), boost::uuids::uuid{{0}});
        Populate();
    };

    // add all saved designs
    auto add_all_saved_designs_action = [&manager, &empire_id]() {
        DebugLogger() << "BasesListBox::BaseRightClicked AddAllSavedDesignsToDisplayedDesigns";
        // add the items to the end of the existing list, in correct order
        // TODO: think about adding them at the front.
        auto design_uuids = manager.OrderedDesignUUIDs();
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


void EmptyHullsListBox::QueueItemMoved(const GG::ListBox::iterator& row_it,
                                       const GG::ListBox::iterator& original_position_it)
{
    const auto control = dynamic_cast<HullAndPartsListBoxRow*>(row_it->get());
    if (!control || !GetEmpire(EmpireID()))
        return;

    const std::string& hull_name = control->Hull();

    iterator insert_before_row = std::next(row_it);

    const auto insert_before_control = (insert_before_row == end()) ? nullptr :
        boost::polymorphic_downcast<const HullAndPartsListBoxRow*>(insert_before_row->get());
    std::string insert_before_hull = insert_before_control
        ? insert_before_control->Hull() : "";

    control->Resize(ListRowSize());

    GetDisplayedDesignsManager().InsertHullBefore(hull_name, insert_before_hull);
}

void CompletedDesignsListBox::QueueItemMoved(const GG::ListBox::iterator& row_it,
                                             const GG::ListBox::iterator& original_position_it)
{
    const auto control = dynamic_cast<BasesListBox::CompletedDesignListBoxRow*>(row_it->get());
    if (!control || !GetEmpire(EmpireID()))
        return;

    int design_id = control->DesignID();

    iterator insert_before_row = std::next(row_it);

    const auto insert_before_control = (insert_before_row == end()) ? nullptr :
        boost::polymorphic_downcast<const BasesListBox::CompletedDesignListBoxRow*>(insert_before_row->get());
    int insert_before_id = insert_before_control
        ? insert_before_control->DesignID() : INVALID_DESIGN_ID;

    control->Resize(ListRowSize());

    GetDisplayedDesignsManager().MoveBefore(design_id, insert_before_id);
}

void SavedDesignsListBox::QueueItemMoved(const GG::ListBox::iterator& row_it,
                                         const GG::ListBox::iterator& original_position_it)
{
    const auto control = dynamic_cast<SavedDesignsListBox::SavedDesignListBoxRow*>(row_it->get());
    if (!control)
        return;

    const auto& uuid = control->DesignUUID();

    iterator insert_before_row = std::next(row_it);

    const auto insert_before_control = (insert_before_row == end()) ? nullptr :
        boost::polymorphic_downcast<const SavedDesignsListBox::SavedDesignListBoxRow*>(insert_before_row->get());
    const auto& next_uuid = insert_before_control
        ? insert_before_control->DesignUUID() : boost::uuids::uuid{{0}};

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
    /** \name Structors */ //@{
    BaseSelector(const std::string& config_name);
    void CompleteConstruction() override;
    //@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void Reset();
    void ToggleAvailability(const Availability::Enum type);
    void SetEmpireShown(int empire_id, bool refresh_list);
    void EnableOrderIssuing(bool enable/* = true*/);
    //@}

    mutable boost::signals2::signal<void (int)>                         DesignSelectedSignal;
    mutable boost::signals2::signal<void (const std::string&, const std::vector<std::string>&)>
                                                                        DesignComponentsSelectedSignal;
    mutable boost::signals2::signal<void (const boost::uuids::uuid&)>   SavedDesignSelectedSignal;
    mutable boost::signals2::signal<void (const ShipDesign*)>           DesignClickedSignal;
    mutable boost::signals2::signal<void (const HullType*)>             HullClickedSignal;

    enum class BaseSelectorTab : std::size_t {Hull, Current, Saved, Monster, All};
    mutable boost::signals2::signal<void (const BaseSelectorTab)>       TabChangedSignal;

private:
    void DoLayout();

    std::shared_ptr<GG::TabWnd>                 m_tabs = nullptr;
    std::shared_ptr<EmptyHullsListBox>          m_hulls_list = nullptr;         // empty hulls on which a new design can be based
    std::shared_ptr<CompletedDesignsListBox>    m_designs_list= nullptr;        // designs this empire has created or learned how to make
    std::shared_ptr<SavedDesignsListBox>        m_saved_designs_list = nullptr; // designs saved to files
    std::shared_ptr<MonstersListBox>            m_monsters_list = nullptr;      // monster designs
    std::shared_ptr<AllDesignsListBox>          m_all_list = nullptr;           // all designs known to empire

    // Holds the state of the availabilities filter.
    AvailabilityManager                         m_availabilities_state{false, true, false};

    std::tuple<std::shared_ptr<CUIStateButton>, std::shared_ptr<CUIStateButton>,
               std::shared_ptr<CUIStateButton>> m_availabilities_buttons;
};

DesignWnd::BaseSelector::BaseSelector(const std::string& config_name) :
    CUIWnd(UserString("DESIGN_WND_STARTS"),
           GG::INTERACTIVE | GG::RESIZABLE | GG::ONTOP | GG::DRAGABLE | PINABLE,
           config_name)
{}

void DesignWnd::BaseSelector::CompleteConstruction() {
    // TODO: C++17, Collect and replace with structured binding auto [a, b, c] = m_availabilities;
    auto& m_obsolete_button = std::get<Availability::Obsolete>(m_availabilities_buttons);
    m_obsolete_button = GG::Wnd::Create<CUIStateButton>(UserString("PRODUCTION_WND_AVAILABILITY_OBSOLETE"),
                                                        GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(m_obsolete_button);
    m_obsolete_button->CheckedSignal.connect(
        boost::bind(&DesignWnd::BaseSelector::ToggleAvailability, this, Availability::Obsolete));
    m_obsolete_button->SetCheck(m_availabilities_state.GetAvailability(Availability::Obsolete));

    auto& m_available_button = std::get<Availability::Available>(m_availabilities_buttons);
    m_available_button = GG::Wnd::Create<CUIStateButton>(UserString("PRODUCTION_WND_AVAILABILITY_AVAILABLE"),
                                                         GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(m_available_button);
    m_available_button->CheckedSignal.connect(
        boost::bind(&DesignWnd::BaseSelector::ToggleAvailability, this, Availability::Available));
    m_available_button->SetCheck(m_availabilities_state.GetAvailability(Availability::Available));

    auto& m_unavailable_button = std::get<Availability::Future>(m_availabilities_buttons);
    m_unavailable_button = GG::Wnd::Create<CUIStateButton>(UserString("PRODUCTION_WND_AVAILABILITY_UNAVAILABLE"),
                                                           GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(m_unavailable_button);
    m_unavailable_button->CheckedSignal.connect(
        boost::bind(&DesignWnd::BaseSelector::ToggleAvailability, this, Availability::Future));
    m_unavailable_button->SetCheck(m_availabilities_state.GetAvailability(Availability::Future));

    m_tabs = GG::Wnd::Create<GG::TabWnd>(GG::X(5), GG::Y(2), GG::X(10), GG::Y(10), ClientUI::GetFont(),
                                         ClientUI::WndColor(), ClientUI::TextColor());
    m_tabs->TabChangedSignal.connect(boost::bind(&DesignWnd::BaseSelector::Reset, this));
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

    DoLayout();
    SaveDefaultedOptions();
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

    m_hulls_list->Populate();
    m_designs_list->Populate();
    m_saved_designs_list->Populate();
}

void DesignWnd::BaseSelector::EnableOrderIssuing(bool enable/* = true*/) {
    m_hulls_list->EnableOrderIssuing(enable);
    m_designs_list->EnableOrderIssuing(enable);
    m_saved_designs_list->EnableOrderIssuing(enable);
    m_monsters_list->EnableOrderIssuing(enable);
}

void DesignWnd::BaseSelector::DoLayout() {
    const GG::X LEFT_PAD(5);
    const GG::Y TOP_PAD(2);
    const GG::X AVAILABLE_WIDTH = ClientWidth() - 2*LEFT_PAD;
    const int BUTTON_SEPARATION = 3;
    const GG::X BUTTON_WIDTH = (AVAILABLE_WIDTH - 2*BUTTON_SEPARATION) / 3;
    const int PTS = ClientUI::Pts();
    const GG::Y BUTTON_HEIGHT(PTS * 2);

    GG::Y top(TOP_PAD);
    GG::X left(LEFT_PAD);

    // TODO: C++17, Replace with structured binding auto [a, b, c] = m_availabilities;
    auto& m_obsolete_button = std::get<Availability::Obsolete>(m_availabilities_buttons);
    auto& m_available_button = std::get<Availability::Available>(m_availabilities_buttons);
    auto& m_unavailable_button = std::get<Availability::Future>(m_availabilities_buttons);

    m_obsolete_button->SizeMove(GG::Pt(left, top), GG::Pt(left + BUTTON_WIDTH, top + BUTTON_HEIGHT));
    left = left + BUTTON_WIDTH + BUTTON_SEPARATION;
    m_available_button->SizeMove(GG::Pt(left, top), GG::Pt(left + BUTTON_WIDTH, top + BUTTON_HEIGHT));
    left = left + BUTTON_WIDTH + BUTTON_SEPARATION;
    m_unavailable_button->SizeMove(GG::Pt(left, top), GG::Pt(left + BUTTON_WIDTH, top + BUTTON_HEIGHT));
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
    /** \name Structors */ //@{
    SlotControl();
    SlotControl(double x, double y, ShipSlotType slot_type);
    //@}
    void CompleteConstruction() override;

    /** \name Accessors */ //@{
    ShipSlotType    SlotType() const;
    double          XPositionFraction() const;
    double          YPositionFraction() const;
    const PartType* GetPart() const;
    //@}

    /** \name Mutators */ //@{
    void StartingChildDragDrop(const GG::Wnd* wnd, const GG::Pt& offset) override;
    void CancellingChildDragDrop(const std::vector<const GG::Wnd*>& wnds) override;
    void AcceptDrops(const GG::Pt& pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys) override;
    void ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination) override;
    void DragDropEnter(const GG::Pt& pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                       GG::Flags<GG::ModKey> mod_keys) override;
    void DragDropLeave() override;

    void Render() override;
    void Highlight(bool actually = true);

    void SetPart(const std::string& part_name);         //!< used to programmatically set the PartControl in this slot.  Does not emit signal
    void SetPart(const PartType* part_type = nullptr);  //!< used to programmatically set the PartControl in this slot.  Does not emit signal
    //@}

    /** emitted when the contents of a slot are altered by the dragging
      * a PartControl in or out of the slot.  signal should be caught and the
      * slot contents set using SetPart accordingly */
    mutable boost::signals2::signal<void (const PartType*, bool)> SlotContentsAlteredSignal;

    mutable boost::signals2::signal<void (const PartType*, GG::Flags<GG::ModKey>)> PartTypeClickedSignal;

protected:
    bool EventFilter(GG::Wnd* w, const GG::WndEvent& event) override;
    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const override;

private:
    bool                                m_highlighted = false;
    ShipSlotType                        m_slot_type = INVALID_SHIP_SLOT_TYPE;
    double                              m_x_position_fraction = 0.4;    //!< position on hull image where slot should be shown, as a fraction of that image's size
    double                              m_y_position_fraction = 0.4;
    std::shared_ptr<PartControl>        m_part_control = nullptr;
    std::shared_ptr<GG::StaticGraphic>  m_background = nullptr;
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

    m_background = GG::Wnd::Create<GG::StaticGraphic>(SlotBackgroundTexture(m_slot_type), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_background->Resize(GG::Pt(SLOT_CONTROL_WIDTH, SLOT_CONTROL_HEIGHT));
    m_background->Show();
    AttachChild(m_background);

    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    // set up empty slot tool tip
    std::string title_text;
    if (m_slot_type == SL_EXTERNAL)
        title_text = UserString("SL_EXTERNAL");
    else if (m_slot_type == SL_INTERNAL)
        title_text = UserString("SL_INTERNAL");
    else if (m_slot_type == SL_CORE)
        title_text = UserString("SL_CORE");

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
    case GG::WndEvent::DragDropEnter:
    case GG::WndEvent::DragDropHere:
    case GG::WndEvent::CheckDrops:
    case GG::WndEvent::DragDropLeave:
    case GG::WndEvent::DragDroppedOn:
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

    for (DropsAcceptableIter it = first; it != last; ++it) {
        if (it->first->DragDropDataType() != PART_CONTROL_DROP_TYPE_STRING)
            continue;
        const auto part_control = boost::polymorphic_downcast<const PartControl* const>(it->first);
        const PartType* part_type = part_control->Part();
        if (part_type &&
            part_type->CanMountInSlotType(m_slot_type) &&
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

const PartType* SlotControl::GetPart() const {
    if (m_part_control)
        return m_part_control->Part();
    else
        return nullptr;
}

void SlotControl::StartingChildDragDrop(const GG::Wnd* wnd, const GG::Pt& offset) {
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

void SlotControl::AcceptDrops(const GG::Pt& pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys) {
    if (wnds.size() != 1)
        ErrorLogger() << "SlotControl::AcceptDrops given multiple wnds unexpectedly...";

    const auto wnd = *(wnds.begin());
    const PartControl* control = boost::polymorphic_downcast<const PartControl*>(wnd.get());
    const PartType* part_type = control ? control->Part() : nullptr;

    if (part_type)
        SlotContentsAlteredSignal(part_type, (mod_keys & GG::MOD_KEY_CTRL));
}

void SlotControl::ChildrenDraggedAway(const std::vector<GG::Wnd*>& wnds, const GG::Wnd* destination) {
    if (wnds.empty())
        return;
    const GG::Wnd* wnd = wnds.front();
    const auto part_control = dynamic_cast<const PartControl*>(wnd);
    if (part_control != m_part_control.get())
        return;
    DetachChildAndReset(m_part_control);
    SlotContentsAlteredSignal(nullptr, false);
}

void SlotControl::DragDropEnter(const GG::Pt& pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
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
{ SetPart(GetPartType(part_name)); }

void SlotControl::SetPart(const PartType* part_type) {
    // remove existing part control, if any
    DetachChildAndReset(m_part_control);

    // create new part control for passed in part_type
    if (part_type) {
        m_part_control = GG::Wnd::Create<PartControl>(part_type);
        AttachChild(m_part_control);
        m_part_control->InstallEventFilter(shared_from_this());

        // single click shows encyclopedia data
        m_part_control->ClickedSignal.connect(
            PartTypeClickedSignal);

        // double click clears slot
        m_part_control->DoubleClickedSignal.connect(
            [this](const PartType*){ this->SlotContentsAlteredSignal(nullptr, false); });
        SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

        // set part occupying slot's tool tip to say slot type
        std::string title_text;
        if (m_slot_type == SL_EXTERNAL)
            title_text = UserString("SL_EXTERNAL");
        else if (m_slot_type == SL_INTERNAL)
            title_text = UserString("SL_INTERNAL");
        else if (m_slot_type == SL_CORE)
            title_text = UserString("SL_CORE");

        m_part_control->SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
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
        std::string DisplayText() const
        { return m_is_in_stringtable ? UserString(m_text) : m_text; }

        /** Return the text as stored. */
        std::string StoredString() const
        { return m_text; }

        bool IsInStringtable() const
        { return m_is_in_stringtable; }

        private:
        const bool m_is_in_stringtable;
        const std::string m_text;
    };

    /** \name Structors */ //@{
    MainPanel(const std::string& config_name);
    void CompleteConstruction() override;
    //@}

    /** \name Accessors */ //@{
    /** If editing a current design return a ShipDesign* otherwise boost::none. */
    boost::optional<const ShipDesign*> EditingCurrentDesign() const;
    /** If editing a saved design return a ShipDesign* otherwise boost::none. */
    boost::optional<const ShipDesign*> EditingSavedDesign() const;

    const std::vector<std::string>      Parts() const;              //!< returns vector of names of parts in slots of current shown design.  empty slots are represented with empty stri
    const std::string&                  Hull() const;               //!< returns name of hull of current shown design
    bool                                IsDesignNameValid() const;  //!< checks design name validity
    /** Return a validated name and description.  If the design is a saved design then either both
        or neither will be stringtable values.*/
    std::pair<I18nString, I18nString>   ValidatedNameAndDescription() const;
    const I18nString                    ValidatedDesignName() const;//!< returns name currently entered for design or valid default
    const I18nString                    DesignDescription() const;  //!< returns description currently entered for design

    /** Returns a pointer to the design currently being modified (if any).  May
        return an empty pointer if not currently modifying a design. */
    std::shared_ptr<const ShipDesign>   GetIncompleteDesign() const;
    boost::optional<int>                GetReplacedDesignID() const;//!< returns ID of completed design selected to be replaced.

    /** If a design with the same hull and parts is registered with the empire then return the
        design, otherwise return boost::none. */
    boost::optional<const ShipDesign*>        CurrentDesignIsRegistered();
    //@}

    /** \name Mutators */ //@{
    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void AcceptDrops(const GG::Pt& pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys) override;

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void Sanitize();

    void SetPart(const std::string& part_name, unsigned int slot);   //!< puts specified part in specified slot.  does nothing if slot is out of range of available slots for current hull
    /** Sets the part in \p slot to \p part and emits and signal if requested.  Changes
        all similar parts if \p change_all_similar_parts. */
    void SetPart(const PartType* part, unsigned int slot, bool emit_signal = false, bool change_all_similar_parts = false);
    void SetParts(const std::vector<std::string>& parts);            //!< puts specified parts in slots.  attempts to put each part into the slot corresponding to its place in the passed vector.  if a part cannot be placed, it is ignored.  more parts than there are slots available are ignored, and slots for which there are insufficient parts in the passed vector are unmodified

    /** Attempts to add the specified part to the design, if possible.  will
      * first attempt to add part to an empty slot of the appropriate type, and
      * if no appropriate slots are available, may or may not move other parts
      * around within the design to open up a compatible slot in which to add
      * this part (and then add it).  may also do nothing. */
    void AddPart(const PartType* part);
    bool CanPartBeAdded(const PartType* part);

    void ClearParts();                                               //!< removes all parts from design.  hull is not altered
    /** Remove parts called \p part_name*/
    void            ClearPart(const std::string& part_name);

    /** Set the design hull \p hull_name, displaying appropriate background image and creating
        appropriate SlotControls.  If \p signal is false do not emit the the
        DesignChangedSignal(). */
    void SetHull(const std::string& hull_name, bool signal = true);
    void SetHull(const HullType* hull, bool signal = true);
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

    /** Add a design. */
    std::pair<int, boost::uuids::uuid> AddDesign();

    /** Replace an existing design.*/
    void ReplaceDesign();

    void HighlightSlotType(std::vector<ShipSlotType>& slot_types);   //!< renders slots of the indicated types differently, perhaps to indicate that that those slots can be drop targets for a particular part?

    /** Track changes in base type. */
    void HandleBaseTypeChange(const DesignWnd::BaseSelector::BaseSelectorTab base_type);
    //@}

    /** emitted when the design is changed (by adding or removing parts, not
      * name or description changes) */
    mutable boost::signals2::signal<void ()>                DesignChangedSignal;

    /** emitted when the design name is changed */
    mutable boost::signals2::signal<void ()>                DesignNameChangedSignal;

    /** propagates signals from contained SlotControls that signal that a part
      * has been clicked */
    mutable boost::signals2::signal<void (const PartType*, GG::Flags<GG::ModKey>)> PartTypeClickedSignal;

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

    const HullType*                             m_hull = nullptr;
    std::vector<std::shared_ptr<SlotControl>>   m_slots;
    boost::optional<int>                        m_replaced_design_id = boost::none;     // The design id if this design is replacable
    boost::optional<boost::uuids::uuid>         m_replaced_design_uuid = boost::none;   // The design uuid if this design is replacable

    /// Whether to add new designs to current or saved designs
    /// This tracks the last relevant selected tab in the base selector
    DesignWnd::BaseSelector::BaseSelectorTab    m_type_to_create = DesignWnd::BaseSelector::BaseSelectorTab::Current;

    mutable std::shared_ptr<ShipDesign>         m_incomplete_design = nullptr;

    std::shared_ptr<GG::StaticGraphic>          m_background_image = nullptr;
    std::shared_ptr<GG::Label>                  m_design_name_label = nullptr;
    std::shared_ptr<GG::Edit>                   m_design_name = nullptr;
    std::shared_ptr<GG::Label>                  m_design_description_label = nullptr;
    std::shared_ptr<GG::Edit>                   m_design_description = nullptr;
    std::shared_ptr<GG::Button>                 m_replace_button = nullptr;
    std::shared_ptr<GG::Button>                 m_confirm_button = nullptr;
    std::shared_ptr<GG::Button>                 m_clear_button = nullptr;
    bool                                        m_disabled_by_name = false; // if the design confirm button is currently disabled due to empty name
    bool                                        m_disabled_by_part_conflict = false;

    boost::signals2::connection                 m_empire_designs_changed_signal;
};

DesignWnd::MainPanel::MainPanel(const std::string& config_name) :
    CUIWnd(UserString("DESIGN_WND_MAIN_PANEL_TITLE"),
           GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE,
           config_name)
{}

void DesignWnd::MainPanel::CompleteConstruction() {
    SetChildClippingMode(ClipToClient);

    m_design_name_label = GG::Wnd::Create<CUILabel>(UserString("DESIGN_WND_DESIGN_NAME"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_design_name = GG::Wnd::Create<CUIEdit>(UserString("DESIGN_NAME_DEFAULT"));
    m_design_description_label = GG::Wnd::Create<CUILabel>(UserString("DESIGN_WND_DESIGN_DESCRIPTION"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_design_description = GG::Wnd::Create<CUIEdit>(UserString("DESIGN_DESCRIPTION_DEFAULT"));
    m_replace_button = Wnd::Create<CUIButton>(UserString("DESIGN_WND_UPDATE"));
    m_confirm_button = Wnd::Create<CUIButton>(UserString("DESIGN_WND_ADD_FINISHED"));
    m_clear_button = Wnd::Create<CUIButton>(UserString("DESIGN_WND_CLEAR"));

    m_replace_button->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_confirm_button->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

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
    m_replace_button->LeftClickedSignal.connect(DesignReplacedSignal);
    m_confirm_button->LeftClickedSignal.connect(DesignConfirmedSignal);
    DesignChangedSignal.connect(boost::bind(&DesignWnd::MainPanel::DesignChanged, this));
    DesignReplacedSignal.connect(boost::bind(&DesignWnd::MainPanel::ReplaceDesign, this));
    DesignConfirmedSignal.connect(boost::bind(&DesignWnd::MainPanel::AddDesign, this));

    DesignChanged(); // Initialize components that rely on the current state of the design.

    CUIWnd::CompleteConstruction();

    DoLayout();
    SaveDefaultedOptions();
}

boost::optional<const ShipDesign*> DesignWnd::MainPanel::EditingSavedDesign() const {
    // Is there a valid replaced_uuid that indexes a saved design?
    if (!m_replaced_design_uuid)
        return boost::none;

    const auto maybe_design = GetSavedDesignsManager().GetDesign(*m_replaced_design_uuid);
    if (!maybe_design)
        return boost::none;
    return maybe_design;
}

boost::optional<const ShipDesign*> DesignWnd::MainPanel::EditingCurrentDesign() const {
    // Is there a valid replaced_uuid that indexes a saved design?
    if (!m_replaced_design_id || !GetDisplayedDesignsManager().IsKnown(*m_replaced_design_id))
        return boost::none;

    const auto maybe_design = GetShipDesign(*m_replaced_design_id);
    if (!maybe_design)
        return boost::none;
    return maybe_design;
}

const std::vector<std::string> DesignWnd::MainPanel::Parts() const {
    std::vector<std::string> retval;
    for (const auto& slot : m_slots) {
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
    // All whitespace probably shouldn't be OK either.
    return !m_design_name->Text().empty();
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
        (UserStringExists(m_design_description->Text()) ? m_design_description->Text() :
         ((maybe_saved && (*maybe_saved)->LookupInStringtable()
           && (m_design_description->Text() == (*maybe_saved)->Description())) ? (*maybe_saved)->Description(false) : ""));

    // Are both the title and the description string table lookup values
    if (!name_index.empty() && !desc_index.empty())
        return std::make_pair(
            I18nString(true, name_index),
            I18nString(true, desc_index));

    return std::make_pair(
        I18nString(false, (IsDesignNameValid()) ? m_design_name->Text() : UserString("DESIGN_NAME_DEFAULT")),
        I18nString(false, m_design_description->Text()));
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

boost::optional<const ShipDesign*> DesignWnd::MainPanel::CurrentDesignIsRegistered() {
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const auto empire = GetEmpire(empire_id);
    if (!empire) {
        ErrorLogger() << "DesignWnd::MainPanel::CurrentDesignIsRegistered couldn't get the current empire.";
        return boost::none;
    }

    if (const auto& cur_design = GetIncompleteDesign()) {
        for (const auto design_id : empire->ShipDesigns()) {
            const auto ship_design = GetShipDesign(design_id);
            if (*ship_design == *cur_design.get())
                return ship_design;
        }
    }
    return boost::none;
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

void DesignWnd::MainPanel::Sanitize() {
    SetHull(nullptr, false);
    m_design_name->SetText(UserString("DESIGN_NAME_DEFAULT"));
    m_design_description->SetText(UserString("DESIGN_DESCRIPTION_DEFAULT"));
    // disconnect old empire design signal
    m_empire_designs_changed_signal.disconnect();
}

void DesignWnd::MainPanel::SetPart(const std::string& part_name, unsigned int slot)
{ SetPart(GetPartType(part_name), slot); }

void DesignWnd::MainPanel::SetPart(const PartType* part, unsigned int slot, bool emit_signal /* = false */, bool change_all_similar_parts /*= false*/) {
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
            for (auto& slot : m_slots) {
                // skip incompatible slots
                if (!part->CanMountInSlotType(slot->SlotType()))
                    continue;

                // skip different type parts
                const auto replaced_part = slot->GetPart();
                if (replaced_part && (replaced_part->Name() != original_part_name))
                    continue;

                slot->SetPart(part);
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
        for (const auto& slot : m_slots) {
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
        for (const auto& slot : m_slots) {
            const PartType* existing_part = slot->GetPart();
            if (!existing_part || existing_part->Class() != PC_FIGHTER_HANGAR)
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
        const PartType* existing_part = slot->GetPart();
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
{ SetHull(GetHullType(hull_name), signal); }

void DesignWnd::MainPanel::SetHull(const HullType* hull, bool signal) {
    m_hull = hull;
    DetachChild(m_background_image);
    m_background_image = nullptr;
    if (m_hull) {
        std::shared_ptr<GG::Texture> texture = ClientUI::HullTexture(hull->Name());
        m_background_image = GG::Wnd::Create<GG::StaticGraphic>(texture, GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC);
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
    m_design_description->SetText(ship_design->Description());

    bool suppress_design_changed_signal = true;
    SetHull(ship_design->GetHull(), !suppress_design_changed_signal);

    SetParts(ship_design->Parts());
    DesignChangedSignal();
}

void DesignWnd::MainPanel::SetDesign(int design_id)
{ SetDesign(GetShipDesign(design_id)); }

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
    m_design_description->SetText(desc);
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

void DesignWnd::MainPanel::Populate(){
    for (const auto& slot: m_slots)
        DetachChild(slot);
    m_slots.clear();

    if (!m_hull)
        return;

    const std::vector<HullType::Slot>& hull_slots = m_hull->Slots();

    for (std::vector<HullType::Slot>::size_type i = 0; i != hull_slots.size(); ++i) {
        const HullType::Slot& slot = hull_slots[i];
        auto slot_control = GG::Wnd::Create<SlotControl>(slot.x, slot.y, slot.type);
        m_slots.push_back(slot_control);
        AttachChild(slot_control);
        slot_control->SlotContentsAlteredSignal.connect(
            boost::bind(static_cast<void (DesignWnd::MainPanel::*)(const PartType*, unsigned int, bool, bool)>(&DesignWnd::MainPanel::SetPart), this, _1, i, true, _2));
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
        GG::Pt bg_ul = background_rect.UpperLeft();
        GG::Pt bg_lr = ClientSize();
        m_background_image->SizeMove(bg_ul, bg_lr);
        background_rect = m_background_image->RenderedArea();
    }

    // place slot controls over image of hull
    for (auto& slot : m_slots) {
        GG::X x(background_rect.Left() - slot->Width()/2 - ClientUpperLeft().x + slot->XPositionFraction() * background_rect.Width());
        GG::Y y(background_rect.Top() - slot->Height()/2 - ClientUpperLeft().y + slot->YPositionFraction() * background_rect.Height());
        slot->MoveTo(GG::Pt(x, y));
    }
}

void DesignWnd::MainPanel::DesignChanged() {
    m_replace_button->ClearBrowseInfoWnd();
    m_confirm_button->ClearBrowseInfoWnd();

    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
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
        const std::set<std::string>& hull_exclusions = m_hull->Exclusions();
        for (const std::string& part_name : Parts()) {
            if (part_name.empty())
                continue;
            if (hull_exclusions.count(part_name)) {
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
                if (already_seen_component_names.count(excluded_part)) {
                    m_disabled_by_part_conflict = true;
                    problematic_components.first = part_name;
                    problematic_components.second = excluded_part;
                    break;
                }
            }
            already_seen_component_names.insert(part_name);
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

    const auto new_design_name = ValidatedDesignName().DisplayText();

    // producible only matters for empire designs.
    // Monster designs can be edited as saved designs.
    bool producible = cur_design->Producible();

    // Current designs can not duplicate other designs, be already registered.
    const auto existing_design = CurrentDesignIsRegistered();

    const auto& replaced_saved_design = EditingSavedDesign();

    const auto& replaced_current_design = EditingCurrentDesign();

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
    const auto name_and_description = ValidatedNameAndDescription();
    const auto& name = name_and_description.first;
    const auto& description = name_and_description.second;

    if (ShipDesign* design = m_incomplete_design.get()) {
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
    const std::string& hull =           Hull();
    std::vector<std::string> parts =    Parts();

    const std::string& icon = m_hull ? m_hull->Icon() : EMPTY_STRING;

    const auto uuid = boost::uuids::random_generator()();

    // update stored design
    m_incomplete_design.reset();
    if (hull.empty())
        return;
    try {
        m_incomplete_design = std::make_shared<ShipDesign>(
            std::invalid_argument(""),
            name.StoredString(), description.StoredString(),
            CurrentTurn(), ClientApp::GetApp()->EmpireID(),
            hull, parts, icon, "", name.IsInStringtable(),
            false, uuid);
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

    if (dynamic_cast<const BasesListBox::BasesListBoxRow*>(first->first))
        first->second = true;
}

void DesignWnd::MainPanel::AcceptDrops(const GG::Pt& pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys) {
    if (wnds.size() != 1)
        ErrorLogger() << "DesignWnd::MainPanel::AcceptDrops given multiple wnds unexpectedly...";

    const auto& wnd = *(wnds.begin());
    if (!wnd)
        return;

    if (const auto completed_design_row = dynamic_cast<const BasesListBox::CompletedDesignListBoxRow*>(wnd.get())) {
        SetDesign(GetShipDesign(completed_design_row->DesignID()));
    }
    else if (const auto hullandparts_row = dynamic_cast<const BasesListBox::HullAndPartsListBoxRow*>(wnd.get())) {
        const std::string& hull = hullandparts_row->Hull();
        const std::vector<std::string>& parts = hullandparts_row->Parts();

        SetDesignComponents(hull, parts);
    }
    else if (const auto saved_design_row = dynamic_cast<const SavedDesignsListBox::SavedDesignListBoxRow*>(wnd.get())) {
        const auto& uuid = saved_design_row->DesignUUID();
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
        if (const HullType* hull = GetHullType(hull_name))
            icon = hull->Icon();

        auto new_uuid = boost::uuids::random_generator()();
        auto new_design_id = INVALID_DESIGN_ID;

        // create design from stuff chosen in UI
        ShipDesign design(std::invalid_argument(""),
                          name.StoredString(), description.StoredString(),
                          CurrentTurn(), ClientApp::GetApp()->EmpireID(),
                          hull_name, parts, icon, "some model", name.IsInStringtable(),
                          false, new_uuid);

        // If editing a saved design insert into saved designs
        if (m_type_to_create == DesignWnd::BaseSelector::BaseSelectorTab::Saved) {
            auto& manager = GetSavedDesignsManager();
            manager.InsertBefore(design, manager.OrderedDesignUUIDs().begin());
            new_uuid = *manager.OrderedDesignUUIDs().begin();

        // Otherwise insert into current empire designs
        } else {
            int empire_id = HumanClientApp::GetApp()->EmpireID();
            const Empire* empire = GetEmpire(empire_id);
            if (!empire) return {INVALID_DESIGN_ID, boost::uuids::uuid{{0}}};

            auto order = std::make_shared<ShipDesignOrder>(empire_id, design);
            HumanClientApp::GetApp()->Orders().IssueOrder(order);
            new_design_id = order->DesignID();

            auto& manager = GetDisplayedDesignsManager();
            const auto& all_ids = manager.AllOrderedIDs();
            manager.InsertBefore(new_design_id, all_ids.empty() ? INVALID_DESIGN_ID : *all_ids.begin());
        }

        DesignChangedSignal();

        DebugLogger() << "Added new design: " << design.Name();

        return std::make_pair(new_design_id, new_uuid);

    } catch (std::invalid_argument&) {
        ErrorLogger() << "DesignWnd::AddDesign tried to add an invalid ShipDesign";
        return {INVALID_DESIGN_ID, boost::uuids::uuid{{0}}};
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
            int empire_id = HumanClientApp::GetApp()->EmpireID();
            int replaced_id = (*(current_maybe_design ? current_maybe_design : existing_design))->ID();

            if (new_design_id == INVALID_DESIGN_ID) return;

            // Remove the old id from the Empire.
            const auto maybe_obsolete = manager.IsObsolete(replaced_id);
            bool is_obsolete = maybe_obsolete && *maybe_obsolete;
            if (!is_obsolete)
                HumanClientApp::GetApp()->Orders().IssueOrder(
                    std::make_shared<ShipDesignOrder>(empire_id, replaced_id, true));

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


//////////////////////////////////////////////////
// DesignWnd                                    //
//////////////////////////////////////////////////
DesignWnd::DesignWnd(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::ONTOP | GG::INTERACTIVE)
{}

void DesignWnd::CompleteConstruction() {
    GG::Wnd::CompleteConstruction();

    Sound::TempUISoundDisabler sound_disabler;
    SetChildClippingMode(ClipToClient);

    m_detail_panel = GG::Wnd::Create<EncyclopediaDetailPanel>(GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | PINABLE, DES_PEDIA_WND_NAME);
    m_main_panel = GG::Wnd::Create<MainPanel>(DES_MAIN_WND_NAME);
    m_part_palette = GG::Wnd::Create<PartPalette>(DES_PART_PALETTE_WND_NAME);
    m_base_selector = GG::Wnd::Create<BaseSelector>(DES_BASE_SELECTOR_WND_NAME);
    InitializeWindows();
    HumanClientApp::GetApp()->RepositionWindowsSignal.connect(
        boost::bind(&DesignWnd::InitializeWindows, this));

    AttachChild(m_detail_panel);

    AttachChild(m_main_panel);
    m_main_panel->PartTypeClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(const PartType*)>(&EncyclopediaDetailPanel::SetItem), m_detail_panel, _1));
    m_main_panel->HullTypeClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(const HullType*)>(&EncyclopediaDetailPanel::SetItem), m_detail_panel, _1));
    m_main_panel->DesignChangedSignal.connect(
        boost::bind(&DesignWnd::DesignChanged, this));
    m_main_panel->DesignNameChangedSignal.connect(
        boost::bind(&DesignWnd::DesignNameChanged, this));
    m_main_panel->CompleteDesignClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(int)>(&EncyclopediaDetailPanel::SetDesign), m_detail_panel, _1));
    //m_main_panel->Sanitize();

    AttachChild(m_part_palette);
    m_part_palette->PartTypeClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(const PartType*)>(&EncyclopediaDetailPanel::SetItem), m_detail_panel, _1));
    m_part_palette->PartTypeDoubleClickedSignal.connect(
        boost::bind(&DesignWnd::MainPanel::AddPart, m_main_panel, _1));
    m_part_palette->ClearPartSignal.connect(
        boost::bind(&DesignWnd::MainPanel::ClearPart, m_main_panel, _1));

    AttachChild(m_base_selector);

    m_base_selector->DesignSelectedSignal.connect(
        boost::bind(static_cast<void (MainPanel::*)(int)>(&MainPanel::SetDesign), m_main_panel, _1));
    m_base_selector->DesignComponentsSelectedSignal.connect(
        boost::bind(&MainPanel::SetDesignComponents, m_main_panel, _1, _2));
    m_base_selector->SavedDesignSelectedSignal.connect(
        boost::bind(static_cast<void (MainPanel::*)(const boost::uuids::uuid&)>(&MainPanel::SetDesign), m_main_panel, _1));

    m_base_selector->DesignClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(const ShipDesign*)>(&EncyclopediaDetailPanel::SetItem), m_detail_panel, _1));
    m_base_selector->HullClickedSignal.connect(
        boost::bind(static_cast<void (EncyclopediaDetailPanel::*)(const HullType*)>(&EncyclopediaDetailPanel::SetItem), m_detail_panel, _1));
    m_base_selector->TabChangedSignal.connect(boost::bind(&MainPanel::HandleBaseTypeChange, m_main_panel, _1));

    // Connect signals to re-populate when part obsolescence changes
    m_part_palette->PartObsolescenceChangedSignal.connect(
        boost::bind(&BaseSelector::Reset, m_base_selector));
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
    const GG::X selector_width = GG::X(300);
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

void DesignWnd::DesignChanged() {
    m_detail_panel->SetIncompleteDesign(m_main_panel->GetIncompleteDesign());
    m_base_selector->Reset();
}

void DesignWnd::DesignNameChanged() {
    m_detail_panel->SetIncompleteDesign(m_main_panel->GetIncompleteDesign());
    m_base_selector->Reset();
}

void DesignWnd::EnableOrderIssuing(bool enable/* = true*/)
{ m_base_selector->EnableOrderIssuing(enable); }
