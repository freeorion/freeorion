#include "ParserAppFixture.h"

#include "util/Directories.h"
#include <boost/test/unit_test.hpp>

namespace fs = boost::filesystem;

ParserAppFixture::ParserAppFixture() {
    InitDirs(boost::unit_test::framework::master_test_suite().argv[0]);

    BOOST_REQUIRE(m_python.Initialize());

    m_scripting_dir = fs::system_complete(GetBinDir() / "test-scripting");
    BOOST_TEST_MESSAGE("Test scripting directory: " << m_scripting_dir);
    BOOST_REQUIRE(m_scripting_dir.is_absolute());
    BOOST_REQUIRE(fs::exists(m_scripting_dir));
    BOOST_REQUIRE(fs::is_directory(m_scripting_dir));

    GetOptionsDB().Set<std::string>("resource.path", PathToString(GetBinDir() / "default"));
}

int ParserAppFixture::EmpireID() const noexcept
{ return ALL_EMPIRES; }

int ParserAppFixture::CurrentTurn() const noexcept
{ return INVALID_GAME_TURN; }

Universe& ParserAppFixture::GetUniverse() noexcept
{ return m_universe; }

const GalaxySetupData& ParserAppFixture::GetGalaxySetupData() const noexcept
{ return m_galaxy_setup_data; }

Networking::ClientType ParserAppFixture::GetEmpireClientType(int empire_id) const
{ return Networking::ClientType::INVALID_CLIENT_TYPE; }

Networking::ClientType ParserAppFixture::GetPlayerClientType(int player_id) const
{ return Networking::ClientType::INVALID_CLIENT_TYPE; }

std::string ParserAppFixture::GetVisibleObjectName(const UniverseObject& object)
{ return object.Name(); }

EmpireManager& ParserAppFixture::Empires() noexcept
{ return m_empires; }

Empire* ParserAppFixture::GetEmpire(int empire_id)
{ return m_empires.GetEmpire(empire_id).get(); }

SpeciesManager& ParserAppFixture::GetSpeciesManager() noexcept
{ return m_species_manager; }

SupplyManager& ParserAppFixture::GetSupplyManager() noexcept
{ return m_supply_manager; }

int ParserAppFixture::EffectsProcessingThreads() const
{ return 1; }
