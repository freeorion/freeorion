#ifndef _ShipDesign_h_
#define _ShipDesign_h_


#include <boost/functional/hash.hpp>
#include <boost/optional/optional.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include "ConstantsFwd.h"
#include "EnumsFwd.h"
#include "../util/Pending.h"
#include <unordered_map>

class Universe;
struct ScriptingContext;

/** ParsedShipDesign holds the results of a parsed ship design which can be
    converted to a ShipDesign. */
struct FO_COMMON_API ParsedShipDesign {
    ParsedShipDesign(std::string&& name, std::string&& description,
                     int designed_on_turn, int designed_by_empire,
                     std::string&& hull, std::vector<std::string>&& parts,
                     std::string&& icon, std::string&& model,
                     bool name_desc_in_stringtable = false, bool monster = false,
                     boost::uuids::uuid uuid = boost::uuids::nil_uuid());

    std::string                 m_name;
    std::string                 m_description;
    boost::uuids::uuid          m_uuid = boost::uuids::nil_uuid();

    int                         m_designed_on_turn = INVALID_GAME_TURN;
    int                         m_designed_by_empire = ALL_EMPIRES;

    std::string                 m_hull;
    std::vector<std::string>    m_parts;

    std::string                 m_icon;
    std::string                 m_3D_model;

    bool                        m_is_monster = false;
    bool                        m_name_desc_in_stringtable = false;
};

class FO_COMMON_API ShipDesign {
public:
    /** The ShipDesign() constructor constructs invalid designs and is only used by boost
        serialization. */
    ShipDesign();

    /** The public ShipDesign constructor will only construct valid ship
        designs, as long as the ShipHullManager has at least one hull.

        If \p should_throw is not boost::none and the passed in parameters (\p
        hull and \p parts) would result in an invalid design it generates an
        explicit log message showing the FOCS corresponding to the passed in
        parameters and the FOCS corresponding to a corrected valid design and
        then throws std::invalid_argument.  This can be used to test design
        validity and provide an explcit log message.

        should_throw is not used but it is a literal reminder that
        std::invalid_argument should be caught.

        If \p should_throw is boost::none it will correct the errors in
        parameters, print a log message and return a valid ship design.  The
        only exception is if there are no ship hull in the HullManager.  Then
        the constructor will not throw, but will return an invalid ship design
        with a empty "" hull.
    */
    ShipDesign(const boost::optional<std::invalid_argument>& should_throw,
               std::string name, std::string description,
               int designed_on_turn, int designed_by_empire, std::string hull,
               std::vector<std::string> parts,
               std::string icon, std::string model,
               bool name_desc_in_stringtable = false, bool monster = false,
               boost::uuids::uuid uuid = boost::uuids::nil_uuid());

    /** Convert a parsed ship design and do any required verification. */
    ShipDesign(const ParsedShipDesign& design);

    [[nodiscard]] int ID() const noexcept { return m_id; }    ///< returns id number of design
    /** returns name of design.  if \a stringtable_lookup is true and the
      * design was constructed specifying name_desc_in_stringtable true,
      * the name string is looked up in the stringtable before being returned.
      * otherwise, the raw name string is returned. */
    [[nodiscard]] const std::string& Name(bool stringtable_lookup = true) const;
    void SetName(std::string name) noexcept;

    /** Return the UUID. */
    [[nodiscard]] boost::uuids::uuid UUID() const noexcept { return m_uuid; }

    /** returns description of design.  if \a stringtable_lookup is true and
      * the design was constructed specifying name_desc_in_stringtable true,
      * the description string is looked up in the stringtable before being
      * returned.  otherwise, the raw name string is returned. */
    [[nodiscard]] const std::string& Description(bool stringtable_lookup = true) const;
    void SetDescription(const std::string& description);

    [[nodiscard]] int DesignedOnTurn() const noexcept   { return m_designed_on_turn; };    ///< returns turn on which design was created
    [[nodiscard]] int DesignedByEmpire() const noexcept { return m_designed_by_empire; };  ///< returns id of empire that created this design

    [[nodiscard]] bool  ProductionCostTimeLocationInvariant() const;          ///< returns true if the production cost and time are invariant (does not depend on) the location
    [[nodiscard]] float ProductionCost(int empire_id, int location_id, const ScriptingContext& context) const; ///< returns the total cost to build a ship of this design
    [[nodiscard]] float PerTurnCost(int empire_id, int location_id, const ScriptingContext& context) const;    ///< returns the maximum per-turn number of production points that can be spent on building a ship of this design
    [[nodiscard]] int   ProductionTime(int empire_id, int location_id, const ScriptingContext& context) const; ///< returns the time in turns it takes to build a ship of this design
    [[nodiscard]] bool  Producible() const noexcept { return m_producible; }  ///< returns whether this design is producible by players and appears on the production screen list

    [[nodiscard]] float Speed() const noexcept          { return m_speed; }                 ///< returns design speed along starlanes

    [[nodiscard]] float Structure() const noexcept      { return m_structure; }             ///< returns the max structure of this design
    [[nodiscard]] float Shields() const noexcept        { return m_shields; }               ///< returns the max shields of this design
    [[nodiscard]] float Fuel() const noexcept           { return m_fuel; }                  ///< returns the max fuel capacity of this design
    [[nodiscard]] float Detection() const noexcept      { return m_detection; }             ///< returns the detection ability of this design
    [[nodiscard]] float ColonyCapacity() const noexcept { return m_colony_capacity; }       ///< returns the colonization capacity of this design
    [[nodiscard]] float TroopCapacity() const noexcept  { return m_troop_capacity; }        ///< returns the troop capacity of this design
    [[nodiscard]] float Stealth() const noexcept        { return m_stealth; }               ///< returns the stealth of this design

    [[nodiscard]] float IndustryGeneration() const noexcept   { return m_industry_generation; }   ///< returns the industry output from this ship design
    [[nodiscard]] float ResearchGeneration() const noexcept   { return m_research_generation; }   ///< returns the research output from this ship design
    [[nodiscard]] float InfluenceGeneration() const noexcept  { return m_influence_generation; }  ///< returns the influence output from this ship design
    [[nodiscard]] bool  IsProductionLocation() const noexcept { return m_is_production_location;} ///< returns true if this ship design can be a production location

    [[nodiscard]] bool  CanColonize() const;                                                     ///< returns true iff the design has colonisation parts, or false otherwise
    [[nodiscard]] bool  HasTroops() const noexcept        { return (m_troop_capacity > 0.0f); }
    [[nodiscard]] bool  CanBombard() const noexcept       { return m_can_bombard; }
    [[nodiscard]] bool  IsArmed() const noexcept          { return m_is_armed; }
    [[nodiscard]] bool  HasDirectWeapons() const noexcept { return m_has_direct_weapons; }
    [[nodiscard]] bool  HasFighters() const noexcept      { return m_has_fighters; }
    [[nodiscard]] bool  IsMonster() const noexcept        { return m_is_monster; }

    [[nodiscard]] float Attack() const;
    [[nodiscard]] float AdjustedAttack(float shield) const;
    [[nodiscard]] float Defense() const;

    [[nodiscard]] const auto&              Hull() const noexcept           { return m_hull; }      ///< returns name of hull on which design is based
    [[nodiscard]] const auto&              Parts() const noexcept          { return m_parts; }     ///< returns vector of names of all parts in this design, with position in vector corresponding to slot positions
    [[nodiscard]] std::vector<std::string> Parts(ShipSlotType slot_type) const;            ///< returns vector of names of parts in slots of indicated type in this design, unrelated to slot positions
    [[nodiscard]] std::vector<std::string> Weapons() const;                                ///< returns vector of names of weapon parts in, unrelated to slot positions

    [[nodiscard]] const auto&              Tags() const noexcept           { return m_tags; };
    [[nodiscard]] bool                     HasTag(std::string_view tag) const
    { return std::any_of(m_tags.begin(), m_tags.end(), [tag](const auto& t) { return t == tag; }); }

    [[nodiscard]] auto& Icon() const noexcept                { return m_icon; }      ///< returns filename for small-size icon graphic for design
    [[nodiscard]] auto& Model() const noexcept               { return m_3D_model; }  ///< returns filename of 3D model that represents ships of design
    [[nodiscard]] bool  LookupInStringtable() const noexcept { return m_name_desc_in_stringtable; }

    //! Returns number of parts in this ship design, indexed by ShipPart name
    [[nodiscard]] auto& ShipPartCount() const noexcept { return m_num_ship_parts; }
    [[nodiscard]] int   PartCount() const;

    /** returns number of parts in this ship design, indexed by ShipPartClass */
    [[nodiscard]] auto& PartClassCount() const noexcept { return m_num_part_classes; }

    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const;           ///< returns a data file format representation of this object

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    [[nodiscard]] uint32_t GetCheckSum() const;

    [[nodiscard]] bool ProductionLocation(int empire_id, int location_id, const ScriptingContext& context) const;   ///< returns true iff the empire with ID empire_id can produce this design at the location with location_id

    void SetID(int id);                                                  ///< sets the ID number of the design to \a id .  Should only be used by Universe class when inserting new design into Universe.
    void SetUUID(boost::uuids::uuid uuid) { m_uuid = uuid; }
    void Rename(std::string name) noexcept { m_name = std::move(name); } ///< renames this design to \a name
    void SetMonster(const bool is_monster) noexcept { m_is_monster = is_monster; }

    /** Return true if \p hull and \p parts would make a valid design. */
    [[nodiscard]] static bool ValidDesign(const std::string& hull, const std::vector<std::string>& parts);

private:
    /** Return a valid hull and parts pair iff the \p hull and \p parts vectors
      * would not make a valid ShipDesign.
      * Also pad parts with "" if it is shorter than the \p hull number of slots.
      * Otherwise (ie. when the input hull and parts do make a valid design), return none.
      * If \p produce_log is true then produce log messages. */
    [[nodiscard]] static boost::optional<std::pair<std::string, std::vector<std::string>>>
        MaybeInvalidDesign(std::string hull, std::vector<std::string> parts, bool produce_log);

    /** Force design invariants to be true. If design invariants are not begin
      * met and \p produce_log is true provide an explicit log message about how it
      * was corrected and throw std::invalid_argument if \p should_throw is not
      * none.
      *
      * \p should_throw is not used but it is a literal reminder that
      * std::invalid_argument should be caught. */
    void ForceValidDesignOrThrow(const boost::optional<std::invalid_argument>& should_throw, bool produce_log);

    void BuildStatCaches();

    int                      m_id = INVALID_DESIGN_ID;

    std::string              m_name;
    std::string              m_description;
    boost::uuids::uuid       m_uuid = boost::uuids::nil_uuid();

    int                      m_designed_on_turn = INVALID_GAME_TURN;
    int                      m_designed_by_empire = ALL_EMPIRES;

    std::string              m_hull;
    std::vector<std::string> m_parts;

    std::string              m_icon;
    std::string              m_3D_model;

    // initialized by BuildStatCaches based on parts and hull tags. tags data does not need to be serialized.
    std::string                   m_tags_concatenated;
    std::vector<std::string_view> m_tags;

    float   m_detection = 0.0f; // TODO: const all these?
    float   m_colony_capacity = 0.0f;
    float   m_troop_capacity = 0.0f;
    float   m_stealth = 0.0f;
    float   m_fuel = 0.0f;
    float   m_shields = 0.0f;
    float   m_structure = 0.0f;
    float   m_speed = 0.0f;
    float   m_research_generation = 0.0f;
    float   m_industry_generation = 0.0f;
    float   m_influence_generation = 0.0f;
    std::map<std::string, int>      m_num_ship_parts;
    std::map<ShipPartClass, int>    m_num_part_classes;
    bool    m_is_production_location = false;
    bool    m_producible = false;
    bool    m_has_direct_weapons = false;
    bool    m_has_fighters = false;
    bool    m_is_armed = false;
    bool    m_can_bombard = false;

    bool    m_is_monster = false;
    bool    m_name_desc_in_stringtable = false;

    template <typename Archive>
    friend void serialize(Archive&, ShipDesign&, unsigned int const);
};

///< Returns true if the two designs have the same hull and parts.
[[nodiscard]] inline bool operator==(const ShipDesign& first, const ShipDesign& second) {
    return first.Hull() == second.Hull() &&
        first.ShipPartCount() == second.ShipPartCount();
}


class FO_COMMON_API PredefinedShipDesignManager {
public:
    using ParsedShipDesignsType = std::pair<
        std::vector<std::pair<std::unique_ptr<ParsedShipDesign>, boost::filesystem::path>>, // designs_and_paths,
        std::vector<boost::uuids::uuid> // ordering
    >;

    /** Return pointers the ShipDesigns in order.*/
    [[nodiscard]] std::vector<const ShipDesign*> GetOrderedShipDesigns() const;

    /** Return pointers the ShipDesigns in order.*/
    [[nodiscard]] std::vector<const ShipDesign*> GetOrderedMonsterDesigns() const;

    /** Returns the ID for the design in the Universe for the predefined design
      * with the specified \a name.  If there is generic design available for
      * the specified \a name, then INVALID_DESIGN_ID is returned. */
    [[nodiscard]] int GetDesignID(const std::string& name) const;

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    [[nodiscard]] uint32_t GetCheckSum() const;

    /** Adds designs in this manager to the universe with the design creator
      * left as no empire. */
    void AddShipDesignsToUniverse(Universe& universe) const;

    /** Returns the predefined ShipDesign with the name \a name.  If no such
      * ship design exists, 0 is returned instead. */
    [[nodiscard]] static PredefinedShipDesignManager& GetPredefinedShipDesignManager();

    /** Sets ship design types to the future value of \p pending_designs
        found in \p subdir. */
    FO_COMMON_API void SetShipDesignTypes(Pending::Pending<ParsedShipDesignsType>&& pending_designs);

    /** Sets monster design types to the future value of \p
        pending_design_types found in \p subdir. */
    FO_COMMON_API void SetMonsterDesignTypes(Pending::Pending<ParsedShipDesignsType>&& pending_designs);

private:
    /** Assigns any m_pending_designs. */
    void CheckPendingDesignsTypes() const;

    /** Future ship design type being parsed by parser.  mutable so that it can
        be assigned to m_ship design_types when completed.*/
    mutable boost::optional<Pending::Pending<ParsedShipDesignsType>> m_pending_designs = boost::none;
    mutable boost::optional<Pending::Pending<ParsedShipDesignsType>> m_pending_monsters = boost::none;

    mutable std::unordered_map<boost::uuids::uuid, std::unique_ptr<ShipDesign>,
                               boost::hash<boost::uuids::uuid>> m_designs;

    mutable std::unordered_map<std::string, boost::uuids::uuid> m_name_to_ship_design;
    mutable std::unordered_map<std::string, boost::uuids::uuid> m_name_to_monster_design;
    // ids of designs from this manager that have been added to the universe with no empire as the creator
    mutable std::unordered_map<std::string, int> m_design_generic_ids;

    mutable std::vector<boost::uuids::uuid> m_ship_ordering;
    mutable std::vector<boost::uuids::uuid> m_monster_ordering;
};

/** returns the singleton predefined ship design manager type manager */
[[nodiscard]] FO_COMMON_API PredefinedShipDesignManager& GetPredefinedShipDesignManager();


/** Load all ship designs in \p parsed and return a tuple is_error, the map
    from uuid to ship design and path and the ship ordering from the
    manifest. */
[[nodiscard]] FO_COMMON_API std::tuple<
    bool,
    std::unordered_map<boost::uuids::uuid,
                       std::pair<std::unique_ptr<ShipDesign>, boost::filesystem::path>,
                       boost::hash<boost::uuids::uuid>>,
    std::vector<boost::uuids::uuid>>
LoadShipDesignsAndManifestOrderFromParseResults(PredefinedShipDesignManager::ParsedShipDesignsType& parsed);


#endif
