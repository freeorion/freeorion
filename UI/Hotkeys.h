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

#include <GG/DrawUtil.h>
#include <GG/MultiEdit.h>
#include <GG/Wnd.h>
#include <GG/WndEvent.h>
#include <GG/Layout.h>
#include <GG/GUI.h>
#include <boost/signals2/shared_connection_block.hpp>


class OptionsDB;

/// A single hotkey, ie just a combination key+modifier that has been
/// given a certain name.
class Hotkey {
    /// The global hotkey storage
    static std::map<std::string, Hotkey>* s_hotkeys;

    Hotkey(const std::string& name, const std::string& description,
           GG::Key key, GG::Flags<GG::ModKey> mod = GG::MOD_KEY_NONE);

    /// Returns a modifiable version of the hotkey. 
    static Hotkey& PrivateNamedHotkey(const std::string& name);

public:
    /// Internal name (code-like).
    std::string m_name;

    /// The human readable description of what this Hot does.
    std::string m_description;

    /// Returns the name of the user string containing the description
    /// of the shortcut.
    std::string GetDescription() const;

    /// Registers a hotkey name (ie the one used for storing in the
    /// database) along with a description and a default value.
    static void AddHotkey(const std::string& name, const std::string& description,
                          GG::Key key, GG::Flags<GG::ModKey> mod = GG::MOD_KEY_NONE);

    /// Returns the name of all defined hotkeys
    static std::set<std::string> DefinedHotkeys();

    /// Returns the names of all defined hotkeys, classified by
    /// "sections" (ie "namespace"), converted into user string by
    /// naming it HOTKEYS_uppercase)
    static std::map<std::string, std::set<std::string> > ClassifyHotkeys();

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
    static std::pair<GG::Key, GG::Flags<GG::ModKey> > HotkeyFromString(const std::string& str);

    void SetFromString(const std::string& str);

    /// Adds hotkey-related options to the options data base
    ///
    /// This function must be called after the initialization of all the
    /// hotkey lists, which is probably why it should be called from
    /// main(), or something like that.
    static void AddOptions(OptionsDB& db);
    /// 
    static void ReadFromOptions(OptionsDB& db);

    /// Pretty print, ie transform into something that may look
    /// reasonably nice for the user, but won't be parseable anymore.
    static std::string PrettyPrint(GG::Key key, GG::Flags<GG::ModKey> mod);
    std::string PrettyPrint() const;

    /// Whether or not the given key combination is safe to recognize while typing text
    static bool IsTypingSafe(GG::Key key, GG::Flags<GG::ModKey> mod);

    bool IsTypingSafe() const;  /// Is the hotkey safe to recognize while typing text?
    bool IsDefault() const;     /// Is the hotkey set to its default value ?

    GG::Key                 m_key;
    GG::Key                 m_key_default;
    GG::Flags<GG::ModKey>   m_mod_keys;
    GG::Flags<GG::ModKey>   m_mod_keys_default;
};

/// A simple functor returning a boolean value. The children will be
/// used to check simply if a condition is fullfilled as a prelude to
/// activate a signal.
///
/// @todo Write functions that make it easier to create complex
/// or-stuff ?
class HotkeyCondition {
protected:
    friend class HotkeyManager;
    HotkeyCondition() {}

public:
    virtual bool IsActive() const = 0;
    virtual ~HotkeyCondition() {}
};

/// On only when no modal Wnds are open
class NoModalWndsOpenCondition : public HotkeyCondition {
public:
    NoModalWndsOpenCondition() {};

    virtual bool IsActive() const {
        return !GG::GUI::GetGUI()->ModalWndsOpen();
    };
};

/// On when the given window is visible
class VisibleWindowCondition : public HotkeyCondition {
protected:
    const GG::Wnd* target;

public:
    VisibleWindowCondition(const GG::Wnd* tg) :
        target(tg)
    {}
    virtual bool IsActive() const {
        if (!target)
            return false;
        return target->Visible();
    };
};

/// On when the given windows are invisible
class InvisibleWindowCondition : public HotkeyCondition {
protected:
    std::list<const GG::Wnd*> m_blacklist;

public:
    InvisibleWindowCondition(const GG::Wnd* w1,     const GG::Wnd* w2 = 0,
                             const GG::Wnd* w3 = 0, const GG::Wnd* w4 = 0);
    InvisibleWindowCondition(const std::list<const GG::Wnd*>& bl);

    virtual bool IsActive() const;
};

/// On when the given window is visible
class FocusWindowCondition : public HotkeyCondition {
protected:
    const GG::Wnd* target;

public:
    FocusWindowCondition(const GG::Wnd* tg) :
        target(tg)
    {}
    virtual bool IsActive() const {
        if (!target)
            return false;
        const GG::Wnd* foc = GG::GUI::GetGUI()->FocusWnd();
        return target == foc;
    };
};

template<class W>
class FocusWindowIsA : public HotkeyCondition {
public:
    FocusWindowIsA() {};

    virtual bool IsActive() const {
        const GG::Wnd* foc = GG::GUI::GetGUI()->FocusWnd();
        //std::cout << "Focus: " << foc << std::endl;
        if (dynamic_cast<const W*>(foc))
            return true;
        return false;
    };
};

class OrCondition : public HotkeyCondition {
protected:
    std::list<HotkeyCondition*> m_conditions;
public:
    OrCondition(HotkeyCondition* c1, HotkeyCondition* c2,
                HotkeyCondition* c3 = 0,
                HotkeyCondition* c4 = 0,
                HotkeyCondition* c5 = 0,
                HotkeyCondition* c6 = 0,
                HotkeyCondition* c7 = 0,
                HotkeyCondition* c8 = 0);

    virtual ~OrCondition();
    virtual bool IsActive() const;
};

class AndCondition : public HotkeyCondition {
protected:
    std::list<HotkeyCondition*> m_conditions;
public:
    AndCondition(HotkeyCondition* c1, HotkeyCondition* c2,
                 HotkeyCondition* c3 = 0,
                 HotkeyCondition* c4 = 0,
                 HotkeyCondition* c5 = 0,
                 HotkeyCondition* c6 = 0,
                 HotkeyCondition* c7 = 0,
                 HotkeyCondition* c8 = 0);

    virtual ~AndCondition();
    virtual bool IsActive() const;
};

class HotkeyManager {
public:
    ~HotkeyManager();

    /// Rebuilds all shortcuts/connections. Should be called again at
    /// the end of every function that registers new connections (just
    /// once, though cause calling it every time would get expensive),
    /// and every time as well a shortcut is changed.
    void RebuildShortcuts();

    /// Returns the singleton instance
    static HotkeyManager* GetManager();

    /// Connects a named shortcut to the target slot in the target instance.
    template <class T, class R>
    void Connect(T* instance, R (T::*member)(), const std::string& name, HotkeyCondition* cond = 0)
    { AddConditionalConnection(name, GG::Connect(NamedSignal(name), member, instance), cond); };

    void Connect(boost::function<bool()> func, const std::string& name, HotkeyCondition* cond = 0)
    { AddConditionalConnection(name, GG::Connect(NamedSignal(name), func), cond); };


private:
    HotkeyManager();

    /// The shortcut processing function. Passed using boost::bind.
    bool ProcessNamedShortcut(const std::string& name, GG::Key key, GG::Flags<GG::ModKey> mod);

    /// Returns the signal for the given named accelerator, creating
    /// it if necessary.
    GG::GUI::AcceleratorSignalType& NamedSignal(const std::string& name);

    /// Add the given conditional connection.
    void AddConditionalConnection(const std::string& name,
                                  const boost::signals2::connection& conn,
                                  HotkeyCondition* cond);

    struct ConditionalConnection;
    typedef std::list<ConditionalConnection> ConditionalConnectionList;
    typedef std::map<std::string, ConditionalConnectionList> Connections;

    /// A set of connected shortcuts.
    Connections             m_connections;
    std::map<std::string, GG::GUI::AcceleratorSignalType*>
                            m_signals;
    std::set<boost::signals2::connection>
                            m_internal_connections;
    static HotkeyManager*   s_singleton;
};

#endif
