// -*- C++ -*-
#include <fstream>
#include <string>

#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>


std::string read_file(const std::string& filename)
{
    std::string retval;
    std::ifstream ifs(filename.c_str());
    int c;
    while ((c = ifs.get()) != std::ifstream::traits_type::eof()) {
        retval += c;
    }
    return retval;
}

namespace adobe { namespace version_1 {

std::ostream& operator<<(std::ostream& stream, const type_info_t& x)
{
    std::ostream_iterator<char> out(stream);
    serialize(x, out);
    return stream;
}

} }

void verbose_dump(const adobe::array_t& array, std::size_t indent = 0);
void verbose_dump(const adobe::dictionary_t& array, std::size_t indent = 0);

void verbose_dump(const adobe::array_t& array, std::size_t indent)
{
    if (array.empty()) {
        std::cout << std::string(4 * indent, ' ') << "[]\n";
        return;
    }

    std::cout << std::string(4 * indent, ' ') << "[\n";
    ++indent;
    for (adobe::array_t::const_iterator it = array.begin(); it != array.end(); ++it) {
        const adobe::any_regular_t& any = *it;
        if (any.type_info() == adobe::type_info<adobe::array_t>()) {
            verbose_dump(any.cast<adobe::array_t>(), indent);
        } else if (any.type_info() == adobe::type_info<adobe::dictionary_t>()) {
            verbose_dump(any.cast<adobe::dictionary_t>(), indent);
        } else {
            std::cout << std::string(4 * indent, ' ')
                      << "type: " << any.type_info() << " "
                      << "value: " << any << "\n";
        }
    }
    --indent;
    std::cout << std::string(4 * indent, ' ') << "]\n";
}

void verbose_dump(const adobe::dictionary_t& dictionary, std::size_t indent)
{
    if (dictionary.empty()) {
        std::cout << std::string(4 * indent, ' ') << "{}\n";
        return;
    }

    std::cout << std::string(4 * indent, ' ') << "{\n";
    ++indent;
    for (adobe::dictionary_t::const_iterator it = dictionary.begin(); it != dictionary.end(); ++it) {
        const adobe::pair<adobe::name_t, adobe::any_regular_t>& pair = *it;
        if (pair.second.type_info() == adobe::type_info<adobe::array_t>()) {
            std::cout << std::string(4 * indent, ' ') << pair.first << ",\n";
            verbose_dump(pair.second.cast<adobe::array_t>(), indent);
        } else if (pair.second.type_info() == adobe::type_info<adobe::dictionary_t>()) {
            std::cout << std::string(4 * indent, ' ') << pair.first << ",\n";
            verbose_dump(pair.second.cast<adobe::dictionary_t>(), indent);
        } else {
            std::cout << std::string(4 * indent, ' ')
                      << "(" << pair.first << ", "
                      << "type: " << pair.second.type_info() << " "
                      << "value: " << pair.second << ")\n";
        }
    }
    --indent;
    std::cout << std::string(4 * indent, ' ') << "}\n";
}
