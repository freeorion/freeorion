// -*- C++ -*-
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


class OptionsDB;
/// A single hotkey, ie just a combination key+modifier that has been
/// given a certain name.
class Hotkey {

    /// The global hotkey storage
    static std::map<std::string, Hotkey> * s_hotkeys;

    Hotkey(const std::string & name,
           GG::Key key,
           GG::Flags<GG::ModKey> mod = GG::MOD_KEY_NONE);

    /// Returns a modifiable version of the hotkey. 
    static Hotkey & PrivateNamedHotkey(const std::string & name);

public:
    /// Internal name (code-like).
    ///
    /// Use namespaces. This name will be converted to a stringtable
    /// entry by prepending HOTKEY_, converting to uppercase and
    /// changing . into _.
    std::string m_name;

    /// Returns the name of the user string containing the description
    /// of the shortcut.
    static std::string UserStringForHotkey(const std::string & name);

    /// Key hotkey
    ///
    /// GG::GGK_RETURN set both return and keypad return
    GG::Key m_key;

    /// Modifier for the key.
    GG::Flags<GG::ModKey> m_mod_keys;

    /// Registers a hotkey name (ie the one used for storing in the
    /// database) along with a description and a default value.
    static void AddHotkey(const std::string & name, 
                          GG::Key key,
                          GG::Flags<GG::ModKey> mod = GG::MOD_KEY_NONE);


    /// Returns the name of all defined hotkeys
    static std::set<std::string> DefinedHotkeys();

    /// Returns the names of all defined hotkeys, classified by
    /// "sections" (ie "namespace"), converted into user string by
    /// naming it HOTKEYS_uppercase)
    static std::map<std::string, std::set<std::string> > ClassifyHotkeys();

    /// Returns the Hotkey of the given name, or raises an exception
    /// if there is no such hotkey.
    static const Hotkey & NamedHotkey(const std::string & name);

    /// Sets the value of the given named hotkey, and updates the
    /// corresponding option.
    static void SetHotKey(const std::string & name, GG::Key key,
                          GG::Flags<GG::ModKey> mod = GG::MOD_KEY_NONE);
  
    /// Returns a string that can later be parsed again with
    /// HotkeyFromString()
    static std::string HotkeyToString(GG::Key key, GG::Flags<GG::ModKey> mod);

    std::string ToString() const;

    /// Converts a string back to the pair key/modifier
    static std::pair<GG::Key, GG::Flags<GG::ModKey> > HotkeyFromString(const std::string & str);

    void FromString(const std::string & str);

    /// Adds the options to the database.
    ///
    /// This function must be called after the initialization of all the
    /// hotkey lists, which is probably why it should be called from
    /// main(), or something like that.
    static void AddOptions(OptionsDB& db);


    /// Parses the configuration back.
    static void ReadFromOptions(OptionsDB & db);

    /// Pretty print, ie transform into somethin that may look
    /// reasonably nice for the user, but won't be parseable anymore.
    static std::string PrettyPrint(GG::Key key, GG::Flags<GG::ModKey> mod);

    std::string PrettyPrint() const;

    /// Whether or not the given key combination is alphanumeric
    static bool IsAlnum(GG::Key key, GG::Flags<GG::ModKey> mod);

    /// Is the hotkey alphanumeric ?
    bool IsAlnum() const;
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
    HotkeyCondition() {;};
public:
    virtual bool IsActive() const = 0;
    virtual ~HotkeyCondition() {;};
};

/// On when the given window is visible
class VisibleWindowCondition : public HotkeyCondition {
protected:

    GG::Wnd * target;

public:
    VisibleWindowCondition(GG::Wnd * tg) : target(tg) {;};
    virtual bool IsActive() const {
        if(! target)
            return false;
        return target->Visible();
    };
};

/// On when the given windows are invisible 
class InvisibleWindowCondition : public HotkeyCondition {
protected:
    std::list<GG::Wnd*> m_blacklist;
public:
    InvisibleWindowCondition(GG::Wnd * w1, GG::Wnd * w2 = NULL, 
                             GG::Wnd * w3 = NULL,
                             GG::Wnd * w4 = NULL);
    InvisibleWindowCondition(const std::list<GG::Wnd*> & bl);

    virtual bool IsActive() const;
};

/// On when the given window is visible
class FocusWindowCondition : public HotkeyCondition {
protected:

    GG::Wnd * target;

public:
    FocusWindowCondition(GG::Wnd * tg) : target(tg) {;};
    virtual bool IsActive() const {
        if(! target)
            return false;
        GG::Wnd * foc = GG::GUI::GetGUI()->FocusWnd();
        return target == foc;
    };
};

template<class W> 
class FocusWindowIsA : public HotkeyCondition {
public:
    FocusWindowIsA() { ;};


    virtual bool IsActive() const {
        GG::Wnd * foc = GG::GUI::GetGUI()->FocusWnd();
        std::cout << "Focus: " << foc << std::endl;
        if(dynamic_cast<W*>(foc))
            return true;
        return false;
    };
};

class OrCondition : public HotkeyCondition {
protected:
    std::list<HotkeyCondition*> m_conditions;
public:
    OrCondition(HotkeyCondition * c1, HotkeyCondition * c2,
                HotkeyCondition * c3 = NULL,
                HotkeyCondition * c4 = NULL,
                HotkeyCondition * c5 = NULL,
                HotkeyCondition * c6 = NULL,
                HotkeyCondition * c7 = NULL,
                HotkeyCondition * c8 = NULL);

    virtual ~OrCondition();
    virtual bool IsActive() const;
};


/// An instance of this class is necessary for all classes that use
/// hotkeys. You must connect each hotkey you use to a given signal
/// manually, using the one of the Connect functions.
class HotkeyManager {


    /// A helper class that stores both a connection and the
    /// conditions in which it should be on.
    struct ConditionalConnection {

        /// The condition. If null, always on.
        boost::shared_ptr<HotkeyCondition> condition;

        boost::signals::connection connection;

        /// Block or unblocks the connection based on condition.
        void UpdateConnection() {
            if(connection.connected()) {
                bool active = true;
                if(condition)
                    active = condition->IsActive();
                connection.block(! active);
            }
        };

        ConditionalConnection(const boost::signals::connection & conn, 
                              HotkeyCondition * cond) : 
            condition(cond), connection(conn) {;};
    };

    typedef std::list<ConditionalConnection> ConditionalConnectionList;

    /// The list of connections !
    typedef std::map<std::string, ConditionalConnectionList> Connections;

    /// A set of connected shortcuts.
    Connections m_connections;

    /// Add the given conditional connection.
    void AddConditionalConnection(const std::string & name, 
                                  const boost::signals::connection & conn,
                                  HotkeyCondition * cond);

    /// The singleton instance
    static HotkeyManager * s_singleton;

    /// The constructor for the singleton. Private.
    HotkeyManager();

    /// Whether or not alnum shortcut are currently disabled
    bool m_disabled_alnum;
    
    /// Signals for each shortut name, created on demand.
    std::map<std::string, GG::GUI::AcceleratorSignalType * > m_signals;

    /// The shortcut processing function. Passed using boost::bind.
    bool ProcessNamedShortcut(const std::string & name);

    /// The connections hot key (real keypress) -> named hot key
    std::set<boost::signals::connection> m_internal_connections;

    /// Returns the signal for the given named accelerator, creating
    /// it if necessary.
    GG::GUI::AcceleratorSignalType & NamedSignal(const std::string & name);
    
    
public:

    /// Rebuilds all shortcuts/connections. Should be called again at
    /// the end of every function that registers new connections (just
    /// once, though cause calling it every time would get expensive),
    /// and every time as well a shortcut is changed.
    void RebuildShortcuts();

    /// Returns the singleton instance
    static HotkeyManager * GetManager();

    /// Connects a named shortcut to the target slot in the target
    /// instance.
    template<class T, class R> 
    void Connect(T * instance, R (T::*member)(), const std::string & name, 
                 HotkeyCondition * cond = NULL) {
        AddConditionalConnection(name, GG::Connect(NamedSignal(name), member, instance), cond);
    };

    ~HotkeyManager();


    /// Disables all alphanumeric accelerators
    void DisableAlphaNumeric();

    /// Reenables all alphanumeric accelerators
    void EnableAlphaNumeric();
};

#endif
