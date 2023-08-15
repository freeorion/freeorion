/*
  Hotkeys.cpp: hotkeys (keyboard shortcuts)
  Copyright 2013 by Vincent Fourmond

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Hotkeys.h"

#include "../util/OptionsDB.h"
#include "../util/Logger.h"
#include "../util/ranges.h"


/////////////////////////////////////////////////////////
// HotkeyManager::ConditionalConnection
/////////////////////////////////////////////////////////
void HotkeyManager::ConditionalConnection::UpdateConnection() {
    if (connection.connected()) {
        if (!condition || condition())
            blocker.unblock();
        else
            blocker.block();
    }
};

HotkeyManager::ConditionalConnection::ConditionalConnection(boost::signals2::connection conn,
                                                            std::function<bool()> cond) :
    condition(std::move(cond)),
    connection(std::move(conn)),
    blocker(connection)
{ blocker.unblock(); }


/////////////////////////////////////////////////////////
// Hotkey
/////////////////////////////////////////////////////////
namespace {
    std::map<std::string, Hotkey> hotkeys;
}

void Hotkey::AddHotkey(const std::string& name, const std::string& description,
                       GG::Key key, GG::Flags<GG::ModKey> mod)
{
    auto inserted = hotkeys.emplace(name, Hotkey(name, description, key, mod)).second;
    if (!inserted)
        InfoLogger() << "Hotkey::AddHotkey skipped creating a new hotkey with name " << name;
}

std::string Hotkey::HotkeyToString(GG::Key key, GG::Flags<GG::ModKey> mod) {
    std::string retval;
    const std::size_t sz = ((mod != GG::MOD_KEY_NONE ? 1u : 0u) + (key > GG::Key::GGK_NONE ? 1u : 0u)) * 24; // guesstimate
    retval.reserve(sz);
    if (mod != GG::MOD_KEY_NONE)
        retval.append(GG::to_string(mod)).append("+");
    if (key > GG::Key::GGK_NONE)
        retval.append(to_string(key));
    return retval;
}

std::vector<std::string> Hotkey::DefinedHotkeys() {
    std::vector<std::string> retval;
    retval.reserve(hotkeys.size());
    std::transform(hotkeys.begin(), hotkeys.end(), std::back_inserter(retval),
                   [](const auto& h) { return h.first; });
    return retval;
}

std::string Hotkey::ToString() const
{ return HotkeyToString(m_key, m_mod_keys); }

std::pair<GG::Key, GG::Flags<GG::ModKey>> Hotkey::HotkeyFromString(const std::string& str) {
    if (str.empty())
        return {GG::Key::GGK_NONE, GG::Flags<GG::ModKey>()};

    // Strip whitespace
    std::string copy = str;
    copy = std::string(copy.begin(), std::remove_if(copy.begin(), copy.end(), isspace));
    std::string_view copy_view = copy;

    auto plus = copy.find('+');
    bool has_modifier = plus != std::string::npos;

    GG::Flags<GG::ModKey> mod = GG::MOD_KEY_NONE;
    if (has_modifier) {
        // We have a modifier. Things get a little complex, since we need
        // to handle the |-separated flags:
        auto m = copy_view.substr(0, plus);

        std::size_t found = 0;
        std::size_t prev = 0;

        try {
            while (true) {
                found = m.find('|', prev);
                auto sub = m.substr(prev, found - prev);
                GG::ModKey cm = GG::FlagSpec<GG::ModKey>::instance().FromString(sub);
                mod |= cm;
                if (found == std::string_view::npos)
                    break;
                prev = found + 1;
            }
        } catch (...) {
            ErrorLogger() << "Unable make flag from string: " << str;
            return {GG::Key::GGK_NONE, GG::Flags<GG::ModKey>()};
        }
    }

    auto v = has_modifier ? copy_view.substr(plus+1) : copy_view;
    GG::Key key = GG::KeyFromString(v, GG::Key::GGK_NONE);
    return {key, mod};
}

void Hotkey::SetFromString(const std::string& str) {
    auto km = HotkeyFromString(str);
    m_key = km.first;
    m_mod_keys = km.second;
}

void Hotkey::AddOptions(OptionsDB& db) {
    for (const auto& entry : hotkeys) {
        const Hotkey& hotkey = entry.second;
        db.Add(hotkey.m_name + ".hotkey", hotkey.GetDescription(), hotkey.ToString());
    }
}

static void ReplaceInString(std::string& str, const std::string& what,
                            const std::string& replacement)
{
    std::size_t lst = 0;
    auto l1 = what.size();
    auto l2 = replacement.size();
    if (l1 == 0 && l2 == 0)
        return;                 // Nothing to do
    do {
        auto t = str.find(what, lst);
        if(t == std::string::npos)
            return;
        str.replace(t, l1, replacement);
        t += l2;
    } while(true);
}

namespace {
    std::string PrettyPrint(GG::Key key, GG::Flags<GG::ModKey> mod) {
        std::string retval;
        if (mod & GG::MOD_KEY_CTRL)
            retval += "CTRL+";
        if (mod & GG::MOD_KEY_ALT)
            retval += "ALT+";
        if (mod & GG::MOD_KEY_SHIFT)
            retval += "SHIFT+";
        if (mod & GG::MOD_KEY_META)
            retval += "META+";

        static_assert(to_string(GG::Key::GGK_RIGHT) == "GGK_RIGHT");

        std::string key_string{to_string(key)};
        ReplaceInString(key_string, "GGK_", ""); // remove prefix

        retval += key_string;
        return retval;
    }

    constexpr bool IsTypingSafe(GG::Key key, GG::Flags<GG::ModKey> mod) noexcept {
        if (GG::Key::GGK_INSERT <= key && GG::Key::GGK_PAGEUP >= key)
            return false;
        if (GG::Key::GGK_END <= key && GG::Key::GGK_UP >= key)
            return false;
        if (mod & (GG::MOD_KEY_CTRL | GG::MOD_KEY_ALT | GG::MOD_KEY_META))
            return true;
        if (key >= GG::Key::GGK_F1 && key <= GG::Key::GGK_F12)
            return true;
        if (key >= GG::Key::GGK_F13 && key <= GG::Key::GGK_F24)
            return true;
        if (key == GG::Key::GGK_TAB || key == GG::Key::GGK_ESCAPE || key == GG::Key::GGK_NONE)
            return true;
        return false;
    }
}

std::string Hotkey::PrettyPrint() const
{ return ::PrettyPrint(m_key, m_mod_keys); }

void Hotkey::ReadFromOptions(OptionsDB& db) {
    for (auto& hotkey : hotkeys | range_values) {
        std::string options_db_name = hotkey.m_name + ".hotkey";
        if (!db.OptionExists(options_db_name)) {
            ErrorLogger() << "Hotkey::ReadFromOptions : no option for " << options_db_name;
            continue;
        }
        std::string option_string = db.Get<std::string>(options_db_name);

        std::pair<GG::Key, GG::Flags<GG::ModKey>> key_modkey_pair = {GG::Key::GGK_NONE, GG::MOD_KEY_NONE};
        try {
            key_modkey_pair = HotkeyFromString(option_string);
        } catch (...) {
            ErrorLogger() << "Failed to read hotkey from string: " << option_string;
            continue;
        }

        if (key_modkey_pair.first == GG::Key::GGK_NONE)
            continue;

        if (!::IsTypingSafe(key_modkey_pair.first, key_modkey_pair.second)) {
            DebugLogger() << "Hotkey::ReadFromOptions : Typing-unsafe key spec: '"
                          << option_string << "' for hotkey " << hotkey.m_name;
        }

        hotkey.m_key = key_modkey_pair.first;
        hotkey.m_mod_keys = key_modkey_pair.second;

        TraceLogger() << "Added hotkey '" << hotkey.m_key << "' with modifiers '"
                      << hotkey.m_mod_keys << "' for hotkey '" << hotkey.m_name << "'";
    }
}

Hotkey::Hotkey(const std::string& name, const std::string& description,
               GG::Key key, GG::Flags<GG::ModKey> mod) :
    m_name(name),
    m_description(description),
    m_key(key),
    m_key_default(key),
    m_mod_keys(mod),
    m_mod_keys_default(mod)
{}

const Hotkey& Hotkey::NamedHotkey(const std::string& name)
{ return PrivateNamedHotkey(name); }

Hotkey& Hotkey::PrivateNamedHotkey(const std::string& name) {
    std::string error_msg = "Hotkey::PrivateNamedHotkey error: no hotkey named: " + name;

    auto i = hotkeys.find(name);
    if (i == hotkeys.end())
        throw std::invalid_argument(error_msg.c_str());

    return i->second;
}

bool Hotkey::IsTypingSafe() const noexcept
{ return ::IsTypingSafe(m_key, m_mod_keys); }

void Hotkey::SetHotkey(const Hotkey& hotkey, GG::Key key, GG::Flags<GG::ModKey> mod) {
    Hotkey& hk = PrivateNamedHotkey(hotkey.m_name);
    hk.m_key = key;
    hk.m_mod_keys = GG::MassagedAccelModKeys(mod);

    GetOptionsDB().Set<std::string>(hk.m_name + ".hotkey", hk.ToString());
}

void Hotkey::ResetHotkey(const Hotkey& old_hotkey) {
    Hotkey& hk = PrivateNamedHotkey(old_hotkey.m_name);
    hk.m_key = hk.m_key_default;
    hk.m_mod_keys = hk.m_mod_keys_default;
    GetOptionsDB().Set<std::string>(hk.m_name + ".hotkey", hk.ToString());
}

void Hotkey::ClearHotkey(const Hotkey& old_hotkey)
{ Hotkey::SetHotkey(old_hotkey, GG::Key::GGK_NONE, GG::Flags<GG::ModKey>()); }


//////////////////////////////////////////////////////////////////////
// HotkeyManager
//////////////////////////////////////////////////////////////////////
namespace {
    HotkeyManager hkm;
}

HotkeyManager& HotkeyManager::GetManager()
{ return hkm; }

void HotkeyManager::RebuildShortcuts() {
    m_internal_connections.clear(); // should disconnect scoped connections

    /// @todo Disable the shortcuts that we've enabled so far ? Is it
    /// really necessary ? An unconnected signal should simply be
    /// ignored.

    // Now, build up again all the shortcuts
    GG::GUI* gui = GG::GUI::GetGUI();
    for (auto& entry : m_connections) {
        const Hotkey& hk = Hotkey::NamedHotkey(entry.first);

        gui->SetAccelerator(hk.m_key, hk.m_mod_keys);

        m_internal_connections.insert(gui->AcceleratorSignal(hk.m_key, hk.m_mod_keys).connect(
            boost::bind(&HotkeyManager::ProcessNamedShortcut, this, hk.m_name, hk.m_key, hk.m_mod_keys)));
    }
}

void HotkeyManager::AddConditionalConnection(const std::string& name,
                                             boost::signals2::connection conn,
                                             std::function<bool()> cond)
{ m_connections[name].emplace_back(std::move(conn), std::move(cond)); }

bool HotkeyManager::ProcessNamedShortcut(const std::string& name, GG::Key key,
                                         GG::Flags<GG::ModKey> mod)
{
    // reject unsafe-for-typing key combinations while typing
    if (GG::GUI::GetGUI()->FocusWndAcceptsTypingInput() && !::IsTypingSafe(key, mod))
        return false;

    // First update the connection state according to the current status.
    auto& conds = m_connections[name];
    std::for_each(conds.begin(), conds.end(), [](auto& c) { c.UpdateConnection(); });
    auto not_connected = [](const auto& c) { return !c.connection.connected(); };
    conds.erase(std::remove_if(conds.begin(), conds.end(), not_connected), conds.end());

    // Then, return the value of the signal !
    return m_signals[name]();
}
