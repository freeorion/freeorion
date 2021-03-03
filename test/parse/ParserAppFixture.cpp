#include "ParserAppFixture.h"

#include "util/Directories.h"
#include <boost/test/unit_test.hpp>

namespace fs = boost::filesystem;

ParserAppFixture::ParserAppFixture() {
    InitDirs(boost::unit_test::framework::master_test_suite().argv[0]);

    m_scripting_dir = fs::system_complete(GetBinDir() / "default-test" / "scripting");
    BOOST_TEST_MESSAGE("Test scripting directory: " << m_scripting_dir);
    BOOST_REQUIRE(m_scripting_dir.is_absolute());
    BOOST_REQUIRE(fs::exists(m_scripting_dir));
    BOOST_REQUIRE(fs::is_directory(m_scripting_dir));
}

int ParserAppFixture::CurrentTurn() const
{ return INVALID_GAME_TURN; }

Universe& ParserAppFixture::GetUniverse()
{ return m_universe; }

const GalaxySetupData& ParserAppFixture::GetGalaxySetupData() const
{ return m_galaxy_setup_data; }

Networking::ClientType ParserAppFixture::GetEmpireClientType(int empire_id) const
{ return Networking::ClientType::INVALID_CLIENT_TYPE; }

Networking::ClientType ParserAppFixture::GetPlayerClientType(int player_id) const
{ return Networking::ClientType::INVALID_CLIENT_TYPE; }

std::string ParserAppFixture::GetVisibleObjectName(std::shared_ptr<const UniverseObject> object) {
    if (!object) {
        ErrorLogger() << "ParserAppFixture::GetVisibleObjectName(): expected non null object pointer.";
        return std::string();
    }

    return object->Name();
}

EmpireManager& ParserAppFixture::Empires()
{ return m_empires; }

Empire* ParserAppFixture::GetEmpire(int empire_id)
{ return m_empires.GetEmpire(empire_id).get(); }

SpeciesManager& ParserAppFixture::GetSpeciesManager()
{ return m_species_manager; }

Species* ParserAppFixture::GetSpecies(const std::string& name)
{ return m_species_manager.GetSpecies(name); }

SupplyManager& ParserAppFixture::GetSupplyManager()
{ return m_supply_manager; }

ObjectMap& ParserAppFixture::EmpireKnownObjects(int empire_id) {
    return m_universe.EmpireKnownObjects(empire_id);
}

int ParserAppFixture::EffectsProcessingThreads() const
{ return 1; }
