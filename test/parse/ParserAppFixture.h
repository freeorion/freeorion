#ifndef _ParserAppFixture_h_
#define _ParserAppFixture_h_

#include "Empire/EmpireManager.h"
#include "Empire/Supply.h"
#include "universe/ScriptingContext.h"
#include "universe/Species.h"
#include "universe/Universe.h"
#include "util/AppInterface.h"
#include "util/MultiplayerCommon.h"
#include "util/PythonCommon.h"

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

    int EmpireID() const noexcept override;
    int CurrentTurn() const noexcept override;
    Universe& GetUniverse() noexcept override;
    const GalaxySetupData& GetGalaxySetupData() const noexcept override;
    Networking::ClientType GetEmpireClientType(int empire_id) const override;
    Networking::ClientType GetPlayerClientType(int player_id) const override;
    std::string GetVisibleObjectName(const UniverseObject& object) override;
    EmpireManager& Empires() noexcept override;
    Empire* GetEmpire(int empire_id) override;
    SpeciesManager& GetSpeciesManager() noexcept override;
    SupplyManager& GetSupplyManager() noexcept override;
    int EffectsProcessingThreads() const override;

    [[nodiscard]] ScriptingContext& GetContext() noexcept override { return m_context; };
    [[nodiscard]] const ScriptingContext& GetContext() const noexcept override { return m_context; };

protected:
    boost::filesystem::path m_test_scripting_dir;
    boost::filesystem::path m_default_scripting_dir;
    PythonCommon            m_python;

    // Gamestate...
    Universe                    m_universe;
    GalaxySetupData             m_galaxy_setup_data;
    EmpireManager               m_empires;
    SpeciesManager              m_species_manager;
    SupplyManager               m_supply_manager;
    // End Gamestate

    ScriptingContext            m_context;
};


#endif

