#ifndef _ParserAppFixture_h_
#define _ParserAppFixture_h_

#include "Empire/EmpireManager.h"
#include "Empire/Supply.h"
#include "universe/Species.h"
#include "universe/Universe.h"
#include "util/AppInterface.h"
#include "util/MultiplayerCommon.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

class ParserAppFixture : public IApp {
public:
    ParserAppFixture();

    ParserAppFixture(const ParserAppFixture&) = delete;
    ParserAppFixture(ParserAppFixture&&) = delete;
    ~ParserAppFixture() override = default;

    const ParserAppFixture& operator=(const ParserAppFixture&) = delete;
    ParserAppFixture& operator=(ParserAppFixture&&) = delete;

    int CurrentTurn() const override;

    Universe& GetUniverse() override;

    const GalaxySetupData& GetGalaxySetupData() const override;

    Networking::ClientType GetEmpireClientType(int empire_id) const override;

    Networking::ClientType GetPlayerClientType(int player_id) const override;

    std::string GetVisibleObjectName(std::shared_ptr<const UniverseObject> object) override;

    EmpireManager& Empires() override;

    Empire* GetEmpire(int empire_id) override;

    SpeciesManager& GetSpeciesManager() override;
    Species* GetSpecies(const std::string& name) override;

    SupplyManager& GetSupplyManager() override;

    ObjectMap& EmpireKnownObjects(int empire_id) override;

    int EffectsProcessingThreads() const override;
protected:
    boost::filesystem::path m_scrpiting_dir;

    // Gamestate...
    Universe                    m_universe;
    GalaxySetupData             m_galaxy_setup_data;
    EmpireManager               m_empires;
    SpeciesManager              m_species_manager;
    SupplyManager               m_supply_manager;
    // End Gamestate
};


#endif

