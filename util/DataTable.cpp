#include "DataTable.h"

#include <boost/spirit.hpp>
#include <boost/spirit/dynamic/for.hpp>

#include <map>
#include <string>
#include <vector>

using namespace boost::spirit;

typedef scanner<file_iterator<> > Scanner;
typedef rule<Scanner> Rule;
typedef std::vector<std::vector<int> > DataTable;
typedef std::map<std::string, DataTable> DataTableMap;

namespace
{
    bool g_echo = false;

    struct InitIndex
    {
        InitIndex(int& i) : m_i(i) {}
        void operator()() const {m_i = 0;}
        int& m_i;
    };

    struct IndexLessThanN
    {
        IndexLessThanN(int& i, const int& n) : m_i(i), m_n(n) {}
        bool operator()() const {return m_i < m_n;}
        int& m_i;
        const int& m_n;
    };

    struct IncrIndex
    {
        IncrIndex(int& i) : m_i(i) {}
        void operator()() const {++m_i;}
        int& m_i;
    };

    struct InitializeTable
    {
        InitializeTable(DataTableMap& tables, const int& rows, const int& cols) :
            m_tables(tables), m_rows(rows), m_cols(cols)
            {}
        template <class Iter>
        void operator()(Iter first, Iter last) const
        {
            std::string table_name(first, last);
            m_tables[table_name].resize(m_rows, std::vector<int>(m_cols, 0));
        }
        DataTableMap& m_tables;
        const int& m_rows;
        const int& m_cols;
    };

    struct AssignToTable
    {
        AssignToTable(DataTableMap& tables, const std::string& table_name, const int& i, const int& j) :
            m_tables(tables), m_table_name(table_name), m_i(i), m_j(j)
            {}
        void operator()(int n) const
        {
            m_tables[m_table_name][m_i][m_j] = n;
        }
        DataTableMap& m_tables;
        const std::string& m_table_name;
        const int& m_i;
        const int& m_j;
    };

    void Echo (file_iterator<> first, file_iterator<> last)
    {
        if (g_echo)
            std::cout << std::string(first, last);
    }
}

void LoadDataTables(const std::string& filename, DataTableMap& tables, bool echo/* = false*/)
{
    if (echo)
        g_echo = true;

    file_iterator<> first(filename.c_str());
    file_iterator<> last = first.make_end();

    Scanner scan(first, last);

    int rows, cols;
    std::string table_name;
    int i, j;
    Rule comment = comment_p("#");
    Rule ignore = *(comment | space_p);
    Rule identifier = ignore >> alpha_p >> *(alnum_p | '_');
    Rule table_header = ignore >> '<' >> *blank_p >> int_p[assign(rows)] >> *blank_p >> 'x' >> *blank_p >> int_p[assign(cols)] >> *blank_p >> '>';
    Rule table = (table_header >>
        ignore >> (identifier[InitializeTable(tables, rows, cols)])[assign(table_name)] >>
        ignore >> *identifier >>
        for_p(InitIndex(i), IndexLessThanN(i, rows), IncrIndex(i)) [
            !identifier >>
            for_p(InitIndex(j), IndexLessThanN(j, cols), IncrIndex(j)) [
                ignore >> int_p[AssignToTable(tables, table_name, i, j)]
            ]
        ])[&Echo];

    (*table).parse(scan);

    g_echo = false;
}