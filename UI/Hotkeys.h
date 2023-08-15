/*
  Hotkeys.h: hotkeys (keyboard shortcuts) 
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
#ifndef _Hotkey_h_
#define _Hotkey_h_

#include <GG/GUI.h>
#include <GG/Wnd.h>

#include <boost/signals2/shared_connection_block.hpp>

#include <functional>


class OptionsDB;

/// A single hotkey, ie just a combination key+modifier that has been
/// given a certain name.
class Hotkey {
    Hotkey(const std::string& name, const std::string& description,
           GG::Key key, GG::Flags<GG::ModKey> mod = GG::MOD_KEY_NONE);

    /// Returns a modifiable version of the hotkey. 
    static Hotkey& PrivateNamedHotkey(const std::string& name);

public:
    auto& GetName() const noexcept { return m_name; }
    auto& GetDescription() const noexcept { return m_description; }
    auto  GetKey() const noexcept { return m_key; }
    auto  GetKeyDefault() const noexcept { return m_key_default; }
    auto  GetModKeys() const noexcept { return m_mod_keys; }
    auto  GetModKeysDefault() const noexcept { return m_mod_keys_default; }

    /// Registers a hotkey name (ie the one used for storing in the
    /// database) along with a description and a default value.
    static void AddHotkey(const std::string& name, const std::string& description,
                          GG::Key key, GG::Flags<GG::ModKey> mod = GG::MOD_KEY_NONE);

    /// Returns the name of all defined hotkeys
    static std::vector<std::string> DefinedHotkeys();

    /// Returns the Hotkey of the given name, or raises an exception
    /// if there is no such hotkey.
    static const Hotkey& NamedHotkey(const std::string& name);

    /// Sets the value of the given named hotkey, and updates the
    /// corresponding option.
    static void SetHotkey(const Hotkey& hotkey, GG::Key key,
                          GG::Flags<GG::ModKey> mod = GG::MOD_KEY_NONE);

    /// Sets the value of the given named hotkey to its default,
    /// and updates the corresponding option.
    static void ResetHotkey(const Hotkey& hotkey);

    /// Sets the value of the given named hotkey to no key
    static void ClearHotkey(const Hotkey& hotkey);

    /// Returns a string that can later be parsed again with
    /// HotkeyFromString()
    static std::string HotkeyToString(GG::Key key, GG::Flags<GG::ModKey> mod);

    std::string ToString() const;

    /// Converts a string back to the pair key/modifier
    static std::pair<GG::Key, GG::Flags<GG::ModKey>> HotkeyFromString(const std::string& str);

    void SetFromString(const std::string& str);

    /// Adds hotkey-related options to the options data base
    ///
    /// This function must be called after the initialization of all the
    /// hotkey lists, which is probably why it should be called from
    /// main(), or something like that.
    static void AddOptions(OptionsDB& db);
    static void ReadFromOptions(OptionsDB& db);

    /// Pretty print, ie transform into something that may look
    /// reasonably nice for the user, but won't be parseable anymore.
    std::string PrettyPrint() const;

    [[nodiscard]] bool IsTypingSafe() const noexcept;  /// Is the hotkey safe to recognize while typing text?
    [[nodiscard]] bool IsDefault() const noexcept { return m_key == m_key_default && m_mod_keys == m_mod_keys_default; };

private:
    std::string           m_name; /// Internal name (code-like).
    std::string           m_description;  /// The human readable description of what this Hot does.
    GG::Key               m_key;
    GG::Key               m_key_default;
    GG::Flags<GG::ModKey> m_mod_keys;
    GG::Flags<GG::ModKey> m_mod_keys_default;

    friend class HotkeyManager;
};


/// On only when no modal Wnds are open
inline bool NoModalWndsOpenCondition() {
    return !GG::GUI::GetGUI()->ModalWndsOpen();
};

/// On when the given window is visible
class VisibleWindowCondition {
public:
    VisibleWindowCondition(const GG::Wnd* tg) :
        target(tg)
    {}

    bool operator()() const { return target && target->Visible(); };

private:
    const GG::Wnd* target = nullptr;
};

/// On when the given windows are invisible
template <std::size_t N>
class InvisibleWindowCondition {
public:
    InvisibleWindowCondition(std::array<const GG::Wnd*, N> bl) :
        m_blacklist(std::move(bl))
    {}

    bool operator()() const
    { return std::none_of(m_blacklist.begin(), m_blacklist.end(), [](auto* w) { return w->Visible(); }); }

private:
    std::array<const GG::Wnd*, N> m_blacklist;
};

/// On when the given window is visible
class FocusWindowCondition {
public:
    FocusWindowCondition(const GG::Wnd* tg) :
        target(tg)
    {}

    bool operator()() const {
        const auto& foc = GG::GUI::GetGUI()->FocusWnd();
        return target && target == foc.get();
    };

private:
    const GG::Wnd* target = nullptr;
};

template <typename W>
class FocusWindowIsA {
public:
    bool operator()() const {
        const auto foc = GG::GUI::GetGUI()->FocusWnd();
        return dynamic_cast<const W*>(foc.get());
    };
};

template <std::size_t N>
class OrCondition {
public:
    using BoolFunc = std::function<bool()>;

    OrCondition(std::array<BoolFunc, N> conditions) :
        m_conditions(std::move(conditions))
    {}

    template <typename... Args>
    OrCondition(Args&&... args) :
        m_conditions{std::forward<Args>(args)...}
    {}

    bool operator()() const
    { return std::any_of(m_conditions.begin(), m_conditions.end(), [](auto& cond) { return cond(); }); }

private:
    std::array<BoolFunc, N> m_conditions;
};

template<typename... Args>
OrCondition(Args&&...) -> OrCondition<sizeof...(Args)>;


template <std::size_t N>
class AndCondition {
public:
    using BoolFunc = std::function<bool()>;

    AndCondition(std::array<BoolFunc, N> conditions) :
        m_conditions(std::move(conditions))
    {}

    template <typename... Args>
    AndCondition(Args&&... args) :
        m_conditions{std::forward<Args>(args)...}
    {}

    bool operator()() const
    { return std::all_of(m_conditions.begin(), m_conditions.end(), [](auto& cond) { return cond(); }); }

private:
    std::array<BoolFunc, N> m_conditions;
};

template<typename... Args>
AndCondition(Args&&...) -> AndCondition<sizeof...(Args)>;


class HotkeyManager {
public:
    /// Rebuilds all shortcuts/connections. Should be called again at
    /// the end of every function that registers new connections (just
    /// once, though cause calling it every time would get expensive),
    /// and every time as well a shortcut is changed.
    void RebuildShortcuts();

    /// Returns the singleton instance
    static HotkeyManager& GetManager();

    /// Connects a named shortcut to the target slot in the target instance.
    void Connect(std::function<bool()> func, const std::string& name, std::function<bool()> cond = nullptr)
    { AddConditionalConnection(name, m_signals[name].connect(func), cond); };

    HotkeyManager() = default;

private:
    /// The shortcut processing function. Passed using boost::bind.
    bool ProcessNamedShortcut(const std::string& name, GG::Key key, GG::Flags<GG::ModKey> mod);

    /// Add the given conditional connection.
    void AddConditionalConnection(const std::string& name, boost::signals2::connection conn, std::function<bool()> cond);

    struct ConditionalConnection {
        ConditionalConnection(boost::signals2::connection conn, std::function<bool()> cond);

        void UpdateConnection(); ///< Block or unblocks the connection based on condition.
        std::function<bool()> condition; ///< The condition. If null, always on.

        boost::signals2::scoped_connection connection;
        boost::signals2::shared_connection_block blocker;
    };
    using Connections = std::map<std::string, std::vector<ConditionalConnection>>;

    Connections                                           m_connections;
    std::map<std::string, GG::GUI::AcceleratorSignalType> m_signals;
    std::set<boost::signals2::scoped_connection>          m_internal_connections;
};

#endif
