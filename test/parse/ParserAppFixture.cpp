#include "ParserAppFixture.h"

#include "util/Directories.h"
#include <boost/test/unit_test.hpp>

#include <cstdlib>

namespace fs = std::filesystem;

ParserAppFixture::ParserAppFixture(bool test_scripting) :
    m_context(*this)
{
    BOOST_TEST_MESSAGE("Init parse tests " << boost::unit_test::framework::master_test_suite().argv[0]);
    InitDirs(boost::unit_test::framework::master_test_suite().argv[0], true);

    if (!m_python.IsPythonRunning()) {
        BOOST_REQUIRE(m_python.Initialize());
    }

#if defined(FREEORION_MACOSX)
    std::filesystem::path resource_dir = GetRootDataDir() / "default";
#else
    std::filesystem::path resource_dir = GetBinDir() / "default";
#endif

#if defined(FREEORION_WIN32)
    if (const wchar_t* resource_path_env = _wgetenv(L"FO_TEST_RESOURCE_PATH"))
        resource_dir = std::filesystem::path(resource_path_env);
#else
    if (const char* resource_path_env = std::getenv("FO_TEST_RESOURCE_PATH"))
        resource_dir = FilenameToPath(resource_path_env);
#endif

    GetOptionsDB().Set("resource.path", resource_dir);

    if (test_scripting) {
#if defined(FREEORION_MACOSX)
        m_scripting_dir = GetRootDataDir() / "test-scripting";
#else
        m_scripting_dir = fs::absolute(GetBinDir() / "test-scripting");
#endif
    } else {
        m_scripting_dir = resource_dir / "scripting";
    }
    BOOST_TEST_MESSAGE("Test scripting directory: " << m_scripting_dir);
    BOOST_REQUIRE(m_scripting_dir.is_absolute());
    BOOST_REQUIRE(fs::exists(m_scripting_dir));
    BOOST_REQUIRE(fs::is_directory(m_scripting_dir));

    m_python.SetModulesDirs({m_scripting_dir});
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
