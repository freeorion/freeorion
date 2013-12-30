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

#include <GG/DrawUtil.h>
#include <GG/MultiEdit.h>
#include <GG/WndEvent.h>
#include <GG/Layout.h>
#include <GG/GUI.h>

#include <algorithm>
#include <vector>
#include <set>

#include "Hotkeys.h"

#include "../util/OptionsDB.h"
#include "../util/i18n.h"

#include <sstream>


std::map<std::string, Hotkey> * Hotkey::s_hotkeys = NULL;

void Hotkey::AddHotkey(const std::string & name,
                       GG::Key key,
                       GG::Flags<GG::ModKey> mod)
{
    if (!s_hotkeys)
        s_hotkeys = new std::map<std::string, Hotkey>;
    s_hotkeys->insert(std::make_pair(name, Hotkey(name, key, mod)));
}

std::string Hotkey::HotkeyToString(GG::Key key,
                                   GG::Flags<GG::ModKey> mod)
{
    std::string ms;
    std::ostringstream s;
    if(mod != GG::MOD_KEY_NONE) {
        s << mod;
        s << "+";
    }
    s << key;
    ms = s.str();
    return ms;
}

std::set<std::string> Hotkey::DefinedHotkeys() {
    std::set<std::string> retval;
    if (s_hotkeys) {
        for (std::map<std::string, Hotkey>::iterator i = s_hotkeys->begin();
             i != s_hotkeys->end(); i++)
        { retval.insert(i->first); }
    }
    return retval;
}

std::string Hotkey::ToString() const
{ return HotkeyToString(m_key, m_mod_keys); }

std::pair<GG::Key, GG::Flags<GG::ModKey> > Hotkey::HotkeyFromString(const std::string & str) {
    size_t plus = str.find_first_of("+");
    std::string v = str;
    GG::Flags<GG::ModKey> mod = GG::MOD_KEY_NONE;
    if (plus != std::string::npos) {
        // We have a modifier. Things get a little complex, since we need
        // to handle the |-separated flags:
        std::string m = str.substr(0, plus);
        v = str.substr(plus);

        size_t found = 0;
        size_t prev = 0;
        while (true) {
            found = m.find(" | ", prev);
            std::string sub = m.substr(prev, found);
            GG::ModKey cm = GG::FlagSpec<GG::ModKey>::instance().FromString(sub);
            mod |= cm;
            if (found == std::string::npos)
                break;
            prev = found + 3;
        }
        v = str.substr(plus+1);
    }
    std::istringstream s(v);
    GG::Key key;
    s >> key;
    return std::pair<GG::Key, GG::Flags<GG::ModKey> >(key, mod);
}

void Hotkey::FromString(const std::string & str) {
    std::pair<GG::Key, GG::Flags<GG::ModKey> > km = HotkeyFromString(str);
    m_key = km.first;
    m_mod_keys = km.second;
}

void Hotkey::AddOptions(OptionsDB & db) {
    if (!s_hotkeys)
        return;
    for (std::map<std::string, Hotkey>::const_iterator i = s_hotkeys->begin();
         i != s_hotkeys->end(); i++)
    {
        const Hotkey & sb = i->second;
        std::string n = "UI.hotkeys.";
        n += sb.m_name;
        db.Add(n, UserString(UserStringForHotkey(sb.m_name)),
               sb.ToString(), Validator<std::string>());
    }
}

static void ReplaceInString(std::string & str, const std::string & what,
                            const std::string & replacement)
{
    size_t lst = 0;
    size_t l1 = what.size();
    size_t l2 = replacement.size();
    if(l1 == 0 && l2 == 0)
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
    std::string ret = HotkeyToString(key, mod);
    ReplaceInString(ret, "GGK_", "");

    std::string ks = HotkeyToString(key, GG::MOD_KEY_NONE);
    ReplaceInString(ks, "GGK_", "");
    if (mod == GG::MOD_KEY_CTRL)
        return "CTRL+" + ks;
    if (mod == GG::MOD_KEY_SHIFT)
        return "SHIFT+" + ks;
    if (mod == GG::MOD_KEY_ALT)
        return "ALT+" + ks;
    ReplaceInString(ret, "MOD_KEY_", "");

    return ret;
}

std::string Hotkey::PrettyPrint() const
{ return PrettyPrint(m_key, m_mod_keys); }

void Hotkey::ReadFromOptions(OptionsDB & db) {
    for (std::map<std::string, Hotkey>::iterator i = s_hotkeys->begin();
         i != s_hotkeys->end(); i++)
    {
        Hotkey * sb = &(i->second);
        std::string n = "UI.hotkeys.";
        n += sb->m_name;
        std::string s = db.Get<std::string>(n);
        try {
            std::pair<GG::Key, GG::Flags<GG::ModKey> > sc = HotkeyFromString(s);
            if (sc.first == GG::EnumMap<GG::Key>::BAD_VALUE)
                throw std::exception();
            sb->m_key = sc.first;
            sb->m_mod_keys = sc.second;
        }
        catch(...) {
            std::cerr << "Invalid key spec: '" << s << "', ignoring" << std::endl;
        }
    }
}

Hotkey::Hotkey(const std::string & name,
               GG::Key key,
               GG::Flags<GG::ModKey> mod) :
    m_name(name), m_key(key), m_mod_keys(mod)
{}

const Hotkey & Hotkey::NamedHotkey(const std::string & name)
{ return PrivateNamedHotkey(name); }

std::string Hotkey::UserStringForHotkey(const std::string & name) {
    std::string ret = "HOTKEY_" + name;
    std::transform(ret.begin(), ret.end(), ret.begin(), ::toupper);
    ReplaceInString(ret, ".", "_");
    return ret;
}

Hotkey & Hotkey::PrivateNamedHotkey(const std::string & name) {
    std::string error_msg = "Error: no hotkey named: " + name;

    if (!s_hotkeys)
        throw std::invalid_argument(error_msg.c_str());
    std::map<std::string, Hotkey>::iterator i = s_hotkeys->find(name);
    if (i == s_hotkeys->end())
        throw std::invalid_argument(error_msg.c_str());

    return i->second;
}


std::map<std::string, std::set<std::string> > Hotkey::ClassifyHotkeys() {
    std::map<std::string, std::set<std::string> > ret;
    if (s_hotkeys) {
        for (std::map<std::string, Hotkey>::iterator i = s_hotkeys->begin();
             i != s_hotkeys->end(); i++)
        {
            const std::string & hk_name = i->first;
            std::string section = "HOTKEYS_GENERAL";
            size_t j = hk_name.find('.');
            if (j != std::string::npos) {
                section = "HOTKEYS_" + hk_name.substr(0, j);
                std::transform(section.begin(), section.end(), 
                               section.begin(), ::toupper);
                if (section == "HOTKEYS_COMBAT")
                    section = "HOTKEYS_Z_COMBAT"; // make combat the
                                                  // last category
            }
            ret[section].insert(hk_name);
        }
    }
    return ret;
}


bool Hotkey::IsAlnum(GG::Key key, GG::Flags<GG::ModKey> mod) {
    // If something else than shift is pressed, it is not alphanumeric
    if (!(mod == 0 || mod == GG::MOD_KEY_LSHIFT || mod == GG::MOD_KEY_RSHIFT))
        return false;

    return ((key >= GG::GGK_a && key <= GG::GGK_z) ||
            (key >= GG::GGK_0 && key <= GG::GGK_9));
}

bool Hotkey::IsAlnum() const
{ return IsAlnum(m_key, m_mod_keys); }

void Hotkey::SetHotKey(const std::string & name, GG::Key key,
                       GG::Flags<GG::ModKey> mod)
{
    Hotkey & hk = PrivateNamedHotkey(name);
    std::string n = "UI.hotkeys.";
    n += hk.m_name;
    hk.m_key = key;
    hk.m_mod_keys = mod;
    GetOptionsDB().Set<std::string>(n, hk.ToString());
}

//////////////////////////////////////////////////////////////////////

InvisibleWindowCondition::InvisibleWindowCondition(GG::Wnd * w1, GG::Wnd * w2,
                                                   GG::Wnd * w3, GG::Wnd * w4)
{
    m_blacklist.push_back(w1);
    if (w2)
        m_blacklist.push_back(w2);
    if (w3)
        m_blacklist.push_back(w3);
    if (w4)
        m_blacklist.push_back(w4);
}

InvisibleWindowCondition::InvisibleWindowCondition(const std::list<GG::Wnd*> & bl) :
    m_blacklist(bl)
{}

bool InvisibleWindowCondition::IsActive() const {
    for (std::list<GG::Wnd*>::const_iterator i = m_blacklist.begin();
        i != m_blacklist.end(); i++)
    {
        if ((*i)->Visible())
            return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////

OrCondition::OrCondition(HotkeyCondition * c1, HotkeyCondition * c2,
                         HotkeyCondition * c3, HotkeyCondition * c4,
                         HotkeyCondition * c5, HotkeyCondition * c6,
                         HotkeyCondition * c7, HotkeyCondition * c8)
{
    m_conditions.push_back(c1);
    m_conditions.push_back(c2);
    if (c3)
        m_conditions.push_back(c3);
    if (c4)
        m_conditions.push_back(c4);
    if (c5)
        m_conditions.push_back(c5);
    if (c6)
        m_conditions.push_back(c6);
    if (c7)
        m_conditions.push_back(c7);
    if (c8)
        m_conditions.push_back(c8);
}

bool OrCondition::IsActive() const {
    for (std::list<HotkeyCondition*>::const_iterator i = m_conditions.begin();
         i != m_conditions.end(); i++)
    {
        if ((*i)->IsActive())
            return true;
    }
    return false;
}

OrCondition::~OrCondition() {
    for (std::list<HotkeyCondition*>::iterator i = m_conditions.begin();
         i != m_conditions.end(); i++)
    { delete *i; }
}

//////////////////////////////////////////////////////////////////////

void HotkeyManager::RebuildShortcuts() {
    for (std::set<boost::signals::connection>::iterator i = m_internal_connections.begin();
         i != m_internal_connections.end(); i++)
    { i->disconnect(); }
    m_internal_connections.clear();

    /// @todo Disable the shortcuts that we've enabled so far ? Is it
    /// really necessary ? An unconnected signal should simply be
    /// ignored.

    // Now, build up again all the shortcuts
    GG::GUI * gui = GG::GUI::GetGUI();
    for (Connections::iterator i = m_connections.begin(); i != m_connections.end(); i++) {
        const Hotkey & hk = Hotkey::NamedHotkey(i->first);
        gui->SetAccelerator(hk.m_key, hk.m_mod_keys);
        m_internal_connections.insert(GG::Connect(gui->AcceleratorSignal(hk.m_key, hk.m_mod_keys),
                                                  boost::bind(&HotkeyManager::ProcessNamedShortcut,
                                                              this, hk.m_name)));
    }
}

HotkeyManager * HotkeyManager::s_singleton = 0;

HotkeyManager * HotkeyManager::GetManager() {
    if (!s_singleton)
        s_singleton = new HotkeyManager;
    return s_singleton;
}

void HotkeyManager::AddConditionalConnection(const std::string & name,
                                             const boost::signals::connection & conn,
                                             HotkeyCondition * cond)
{
    ConditionalConnectionList & list = m_connections[name];
    list.push_back(ConditionalConnection(conn, cond));
}

GG::GUI::AcceleratorSignalType & HotkeyManager::NamedSignal(const std::string & name) {
    /// Unsure why GG::AcceleratorSignal implementation uses shared
    /// pointers. Maybe I should, too ?
    GG::GUI::AcceleratorSignalType * & sig = m_signals[name];
    if (!sig)
        sig = new GG::GUI::AcceleratorSignalType;
    return *sig;
}


bool HotkeyManager::ProcessNamedShortcut(const std::string & name) {
    if (m_disabled_alnum  && Hotkey::NamedHotkey(name).IsAlnum())
        return false;           // ignored

    // First update the connection state according to the current
    // status.
    ConditionalConnectionList & conds = m_connections[name];
    for (ConditionalConnectionList::iterator i = conds.begin();
         i != conds.end(); i++)
    {
        i->UpdateConnection();
        if (!i->connection.connected())
            i = conds.erase(i);
    }

    // Then, return the value of the signal !
    GG::GUI::AcceleratorSignalType * sig = m_signals[name];
    return (*sig)();
}

HotkeyManager::~HotkeyManager()
{}

HotkeyManager::HotkeyManager() : m_disabled_alnum(false)
{}

void HotkeyManager::DisableAlphaNumeric()
{ m_disabled_alnum = true; }

void HotkeyManager::EnableAlphaNumeric()
{ m_disabled_alnum = false; }
