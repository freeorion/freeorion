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


/// A helper class that stores both a connection and the
/// conditions in which it should be on.
struct HotkeyManager::ConditionalConnection {
    /// Block or unblocks the connection based on condition.
    void UpdateConnection() {
        if (connection.connected()) {
            if (!condition || condition())
                blocker.unblock();
            else
                blocker.block();
        }
    };

    ConditionalConnection(const boost::signals2::connection& conn, std::function<bool()> cond) :
        condition(cond),
        connection(conn),
        blocker(connection)
    {
        blocker.unblock();
    }

    /// The condition. If null, always on.
    std::function<bool()> condition;

    boost::signals2::connection connection;
    boost::signals2::shared_connection_block blocker;
};

/////////////////////////////////////////////////////////
// Hotkey
/////////////////////////////////////////////////////////
std::map<std::string, Hotkey>* Hotkey::s_hotkeys = nullptr;

void Hotkey::AddHotkey(const std::string& name, const std::string& description, GG::Key key, GG::Flags<GG::ModKey> mod) {
    if (!s_hotkeys)
        s_hotkeys = new std::map<std::string, Hotkey>;
    s_hotkeys->insert({name, Hotkey(name, description, key, mod)});
}

std::string Hotkey::HotkeyToString(GG::Key key, GG::Flags<GG::ModKey> mod) {
    std::ostringstream s;
    if (mod != GG::MOD_KEY_NONE) {
        s << mod;
        s << "+";
    }
    if (key > GG::GGK_UNKNOWN) {
        s << key;
    }
    return s.str();
}

std::set<std::string> Hotkey::DefinedHotkeys() {
    std::set<std::string> retval;
    if (s_hotkeys) {
        for (const auto& entry : *s_hotkeys)
        { retval.insert(entry.first); }
    }
    return retval;
}

std::string Hotkey::ToString() const
{ return HotkeyToString(m_key, m_mod_keys); }

std::pair<GG::Key, GG::Flags<GG::ModKey>> Hotkey::HotkeyFromString(const std::string& str) {
    if (str.empty())
        return {GG::GGK_NONE, GG::Flags<GG::ModKey>()};

    // Strip whitespace
    std::string copy = str;
    copy = std::string(copy.begin(), std::remove_if(copy.begin(), copy.end(), isspace));

    size_t plus = copy.find('+');
    bool has_modifier = plus != std::string::npos;

    GG::Flags<GG::ModKey> mod = GG::MOD_KEY_NONE;
    if (has_modifier) {
        // We have a modifier. Things get a little complex, since we need
        // to handle the |-separated flags:
        std::string m = copy.substr(0, plus);

        size_t found = 0;
        size_t prev = 0;

        try {
            while (true) {
                found = m.find('|', prev);
                std::string sub = m.substr(prev, found-prev);
                GG::ModKey cm = GG::FlagSpec<GG::ModKey>::instance().FromString(sub);
                mod |= cm;
                if (found == std::string::npos)
                    break;
                prev = found + 1;
            }
        } catch (...) {
            ErrorLogger() << "Unable make flag from string: " << str;
            return {GG::GGK_NONE, GG::Flags<GG::ModKey>()};
        }
    }

    std::string v = has_modifier ? copy.substr(plus+1) : copy;
    std::istringstream s(v);
    GG::Key key;
    s >> key;
    return std::pair<GG::Key, GG::Flags<GG::ModKey>>(key, mod);
}

void Hotkey::SetFromString(const std::string& str) {
    std::pair<GG::Key, GG::Flags<GG::ModKey>> km = HotkeyFromString(str);
    m_key = km.first;
    m_mod_keys = km.second;
}

void Hotkey::AddOptions(OptionsDB& db) {
    if (!s_hotkeys)
        return;
    for (const auto& entry : *s_hotkeys) {
        const Hotkey& hotkey = entry.second;
        std::string n = hotkey.m_name + ".hotkey";
        db.Add(n, hotkey.GetDescription(),
               hotkey.ToString(), Validator<std::string>());
    }
}

static void ReplaceInString(std::string& str, const std::string& what,
                            const std::string& replacement)
{
    size_t lst = 0;
    size_t l1 = what.size();
    size_t l2 = replacement.size();
    if (l1 == 0 && l2 == 0)
        return;                 // Nothing to do
    do {
        size_t t = str.find(what, lst);
        if(t == std::string::npos)
            return;
        str.replace(t, l1, replacement);
        t += l2;
    } while(true);
}

std::string Hotkey::PrettyPrint(GG::Key key, GG::Flags<GG::ModKey> mod) {
    std::string retval;
    if (mod & GG::MOD_KEY_CTRL)
        retval += "CTRL+";
    if (mod & GG::MOD_KEY_ALT)
        retval += "ALT+";
    if (mod & GG::MOD_KEY_SHIFT)
        retval += "SHIFT+";
    if (mod & GG::MOD_KEY_META)
        retval += "META+";

    std::ostringstream key_stream;
    key_stream << key;
    std::string key_string = key_stream.str();
    ReplaceInString(key_string, "GGK_", "");

    retval += key_string;
    return retval;
}

std::string Hotkey::PrettyPrint() const
{ return PrettyPrint(m_key, m_mod_keys); }

void Hotkey::ReadFromOptions(OptionsDB& db) {
    for (auto& entry : *s_hotkeys) {
        Hotkey& hotkey = entry.second;

        std::string options_db_name = hotkey.m_name + ".hotkey";
        if (!db.OptionExists(options_db_name)) {
            ErrorLogger() << "Hotkey::ReadFromOptions : no option for " << options_db_name;
            continue;
        }
        std::string option_string = db.Get<std::string>(options_db_name);

        std::pair<GG::Key, GG::Flags<GG::ModKey>> key_modkey_pair = {GG::GGK_NONE, GG::MOD_KEY_NONE};
        try {
            key_modkey_pair = HotkeyFromString(option_string);
        } catch (...) {
            ErrorLogger() << "Failed to read hotkey from string: " << option_string;
            continue;
        }

        if (key_modkey_pair.first == GG::GGK_NONE)
            continue;

        if (key_modkey_pair.first == GG::EnumMap<GG::Key>::BAD_VALUE) {
            ErrorLogger() << "Hotkey::ReadFromOptions : Invalid key spec: '"
                          << option_string << "' for hotkey " << hotkey.m_name;
            continue;
        }

        if (!IsTypingSafe(key_modkey_pair.first, key_modkey_pair.second)) {
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

std::string Hotkey::GetDescription() const
{ return m_description; }

Hotkey& Hotkey::PrivateNamedHotkey(const std::string& name) {
    std::string error_msg = "Hotkey::PrivateNamedHotkey error: no hotkey named: " + name;

    if (!s_hotkeys)
        throw std::runtime_error("Hotkey::PrivateNamedHotkey error: couldn't get hotkeys container.");

    auto i = s_hotkeys->find(name);
    if (i == s_hotkeys->end())
        throw std::invalid_argument(error_msg.c_str());

    return i->second;
}

bool Hotkey::IsTypingSafe(GG::Key key, GG::Flags<GG::ModKey> mod) {
    if (key >= GG::GGK_UP && key <= GG::GGK_PAGEDOWN)
        return false;
    if (mod & (GG::MOD_KEY_CTRL | GG::MOD_KEY_ALT | GG::MOD_KEY_META))
        return true;
    if (key >= GG::GGK_F1 && key <= GG::GGK_F15)
        return true;
    if (key == GG::GGK_TAB || key == GG::GGK_ESCAPE || key == GG::GGK_NONE)
        return true;
    return false;
}

bool Hotkey::IsTypingSafe() const
{ return IsTypingSafe(m_key, m_mod_keys); }

bool Hotkey::IsDefault() const
{ return m_key == m_key_default && m_mod_keys == m_mod_keys_default; }

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
{ Hotkey::SetHotkey(old_hotkey, GG::GGK_NONE, GG::Flags<GG::ModKey>()); }

//////////////////////////////////////////////////////////////////////
// InvisibleWindowCondition
//////////////////////////////////////////////////////////////////////
InvisibleWindowCondition::InvisibleWindowCondition(std::initializer_list<const GG::Wnd*> bl) :
    m_blacklist(bl)
{}

bool InvisibleWindowCondition::operator()() const {
    for (const auto& wnd : m_blacklist) {
        if (wnd->Visible())
            return false;
    }
    return true;
}


//////////////////////////////////////////////////////////////////////
// OrCondition
//////////////////////////////////////////////////////////////////////
OrCondition::OrCondition(std::initializer_list<std::function<bool()>> conditions) :
    m_conditions(conditions)
{}

bool OrCondition::operator()() const {
    for (auto cond : m_conditions) {
        if (cond())
            return true;
    }
    return false;
}


//////////////////////////////////////////////////////////////////////
// AndCondition
//////////////////////////////////////////////////////////////////////
AndCondition::AndCondition(std::initializer_list<std::function<bool()>> conditions) :
    m_conditions(conditions)
{}

bool AndCondition::operator()() const {
    for (auto cond : m_conditions) {
        if (!cond())
            return false;
    }
    return true;
}


//////////////////////////////////////////////////////////////////////
// HotkeyManager
//////////////////////////////////////////////////////////////////////
HotkeyManager* HotkeyManager::s_singleton = nullptr;

HotkeyManager::HotkeyManager()
{}

HotkeyManager::~HotkeyManager()
{}

HotkeyManager* HotkeyManager::GetManager() {
    if (!s_singleton)
        s_singleton = new HotkeyManager;
    return s_singleton;
}

void HotkeyManager::RebuildShortcuts() {
    for (const auto& con : m_internal_connections)
    { con.disconnect(); }
    m_internal_connections.clear();

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
                                             const boost::signals2::connection& conn,
                                             std::function<bool()> cond)
{
    ConditionalConnectionList& list = m_connections[name];
    list.push_back(ConditionalConnection(conn, cond));
}

GG::GUI::AcceleratorSignalType& HotkeyManager::NamedSignal(const std::string& name) {
    /// Unsure why GG::AcceleratorSignal implementation uses shared
    /// pointers. Maybe I should, too ?
    auto& sig = m_signals[name];
    if (!sig)
        sig = new GG::GUI::AcceleratorSignalType;
    return *sig;
}

bool HotkeyManager::ProcessNamedShortcut(const std::string& name, GG::Key key,
                                         GG::Flags<GG::ModKey> mod)
{
    // reject unsafe-for-typing key combinations while typing
    if (GG::GUI::GetGUI()->FocusWndAcceptsTypingInput() && !Hotkey::IsTypingSafe(key, mod))
        return false;

    // First update the connection state according to the current status.
    ConditionalConnectionList& conds = m_connections[name];
    for (auto i = conds.begin(); i != conds.end(); ++i) {
        i->UpdateConnection();
        if (!i->connection.connected())
            i = conds.erase(i);
    }

    // Then, return the value of the signal !
    GG::GUI::AcceleratorSignalType* sig = m_signals[name];
    return (*sig)();
}
