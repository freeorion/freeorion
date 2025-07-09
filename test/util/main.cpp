#define BOOST_TEST_MODULE "FreeOrion unit/coverage tests utilities"

#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_monitor.hpp>
#include <boost/test/output/compiler_log_formatter.hpp>

#include "util/Directories.h"

// Produces an GCC-ish message prefix for various frontends that understand gcc output.
struct gcc_log_formatter : 
    public boost::unit_test::output::compiler_log_formatter
{
    void print_prefix(std::ostream& output, boost::unit_test::const_string file_name, std::size_t line) override
    {
        output << file_name << ':' << line << ": error: ";
    }
};

struct boost_test_config
{
    boost_test_config()
    {
        boost::unit_test::unit_test_log.set_formatter(new gcc_log_formatter);
        BOOST_TEST_MESSAGE("Init util tests " << boost::unit_test::framework::master_test_suite().argv[0]);
        InitDirs(boost::unit_test::framework::master_test_suite().argv[0], true);
    }
};

BOOST_GLOBAL_FIXTURE(boost_test_config);
