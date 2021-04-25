#include "OptionsDB.h"

#include <codecvt>

#include "../util/OptionsDB.h"

using namespace godot;

void godot::OptionsDB::_register_methods() {
    register_method("_exists", &OptionsDB::_exists);
    register_method("_commit", &OptionsDB::_commit);
    register_method("_get_option_str", &OptionsDB::_get_option_str);
    register_method("_get_option_int", &OptionsDB::_get_option_int);
    register_method("_get_option_bool", &OptionsDB::_get_option_bool);
    register_method("_get_option_double", &OptionsDB::_get_option_double);

}

godot::OptionsDB::OptionsDB() {
}

godot::OptionsDB::~OptionsDB() {
}

void godot::OptionsDB::_init() {
}

bool godot::OptionsDB::_exists(String opt) const {
    std::string opt8 = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.to_bytes(opt.unicode_str());
    return ::GetOptionsDB().OptionExists(opt8);
}

void godot::OptionsDB::_commit() {
    ::GetOptionsDB().Commit();
}

String godot::OptionsDB::_get_option_str(String opt) const {
    std::string opt8 = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.to_bytes(opt.unicode_str());
    auto ret = ::GetOptionsDB().Get<std::string>(opt8);
    return String(ret.c_str());
}

int godot::OptionsDB::_get_option_int(String opt) const {
    std::string opt8 = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.to_bytes(opt.unicode_str());
    return ::GetOptionsDB().Get<int>(opt8);
}

bool godot::OptionsDB::_get_option_bool(String opt) const {
    std::string opt8 = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.to_bytes(opt.unicode_str());
    return ::GetOptionsDB().Get<bool>(opt8);
}

double godot::OptionsDB::_get_option_double(String opt) const {
    std::string opt8 = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.to_bytes(opt.unicode_str());
    return ::GetOptionsDB().Get<double>(opt8);
}

