/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/
#if 0
#include <GG/adobe/future/menu_system.hpp>
#include <GG/adobe/memory.hpp>

#include <GG/adobe/macintosh_carbon_safe.hpp>
#include <GG/adobe/macintosh_memory.hpp>
#include <GG/adobe/macintosh_string.hpp>

#include <GG/adobe/value.hpp>

#include <vector>
#include <list>
#include <cassert>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

const OSType app_creator_k      ('adbe');
const OSType menu_system_tag_k  ('mnsy');

/****************************************************************************************************/

struct menu_system_t::implementation_t
{
public:
    implementation_t();

    ~implementation_t();

    void            insert_menu(name_t name);
    void            insert_item(name_t   parent_name,
                                name_t   name,
                                boost::uint16_t cmd_key,
                                modifier_set_t  modifier_set);
    void            insert_separator(name_t parent_name);
    void            remove(name_t name);

    void            enable_menu_item(name_t name, bool enabled, bool force = false);
    void            mark_menu_item(name_t name, bool marked);

    connection_t    monitor_menu_item(name_t name, const item_callback_t& callback);

    static void     platform_initialization();

    struct component_t
    {
    public:
        component_t() :
            enabled_m(false),
            command_id_m(next_menu_command())
        { }

        component_t(const component_t& rhs) :
            name_m(rhs.name_m),
            ref_m(rhs.ref_m),
            enabled_m(rhs.enabled_m),
            //callback_set_m(rhs.callback_set_m),
            command_id_m(rhs.command_id_m)
        { }

        component_t& operator = (const component_t& rhs)
        {
            name_m = rhs.name_m;
            ref_m = rhs.ref_m;
            enabled_m = rhs.enabled_m;
            //callback_set_m = rhs.callback_set_m;
            const_cast<MenuCommand&>(command_id_m) = rhs.command_id_m;

            return *this;
        }

        name_t           name_m;
        value_t          ref_m;
        bool                    enabled_m;
        boost::signal<void ()>  callback_signal_m;
        const MenuCommand       command_id_m;

    private:
        ::MenuCommand next_menu_command()
        {
            static ::MenuCommand    cur_cmd_m(1000000);

            return ++cur_cmd_m;
        }
    };

    typedef std::list<component_t>      component_set_t;
    typedef component_set_t::iterator   component_iterator;

    auto_resource<MenuRef>   old_root_m;
    auto_resource<MenuRef>   root_m;
    component_set_t                 component_set_m;

private:
    inline component_iterator   find_iter(name_t name);
    component_t&                find(name_t name);
};

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

namespace {

/****************************************************************************************************/

std::string cfstring_to_string(CFStringRef x)
{
    if (x == 0) return std::string();

    const int                   pad(16); // must be at least one - seems like a good conservative size
    CFIndex                     max_size(::CFStringGetMaximumSizeForEncoding(::CFStringGetLength(x),
                                    kCFStringEncodingUTF8) + pad);
    std::vector<char>           buffer(static_cast<std::size_t>(max_size));

#ifndef NDEBUG
    Boolean result =
#endif

    ::CFStringGetCString(x, &buffer[0], max_size, kCFStringEncodingUTF8);

#ifndef NDEBUG
    assert(result);
#endif

    return std::string(&buffer[0]);
}

/****************************************************************************************************/

static pascal OSStatus menu_command(EventHandlerCallRef /*handler*/, EventRef event, void* /*data*/)
try
{
    HICommand                               command;
    adobe::auto_resource<MenuRef>           root(::AcquireRootMenu());
    adobe::menu_system_t::implementation_t* impl(0);
    UInt32                                  actual_size(0);

    ADOBE_REQUIRE_STATUS(::GetMenuItemProperty( root.get(), 0,
                                                adobe::app_creator_k,
                                                adobe::menu_system_tag_k,
                                                sizeof(impl),
                                                &actual_size, &impl));

    assert(actual_size == sizeof(impl));
    assert(impl);

    if( ::GetEventParameter( event, kEventParamDirectObject,
        typeHICommand, 0, sizeof( command ), 0, &command ) )
        return eventNotHandledErr;

    adobe::menu_system_t::implementation_t::component_iterator item =
        adobe::find_if( impl->component_set_m,
                        boost::bind(adobe::compare_members(
                                &adobe::menu_system_t::implementation_t::component_t::command_id_m,
                                std::equal_to< ::MenuCommand>()),
                            command.commandID,
                            _1));

    if (item == impl->component_set_m.end())
        return eventNotHandledErr;

    item->callback_signal_m();

    return noErr;
}
catch( ... )
{
    return eventNotHandledErr;
}

/****************************************************************************************************/

::MenuID next_menu_id()
{
    static ::MenuID id(12345);

    return ++id;
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

menu_system_t::menu_system_t() :
    object_m(new implementation_t())
    { implementation_t::platform_initialization(); }

menu_system_t::~menu_system_t()
    { delete object_m; }

void menu_system_t::insert_menu(name_t name)
    { object_m->insert_menu(name); }

void menu_system_t::insert_item(name_t   parent_name,
                                name_t   name,
                                boost::uint16_t cmd_key,
                                modifier_set_t  modifier_set)
    { object_m->insert_item(parent_name, name, cmd_key, modifier_set); }

void menu_system_t::insert_separator(name_t parent_name)
    { object_m->insert_separator(parent_name); }

void menu_system_t::remove(name_t name)
    { object_m->remove(name); }

void menu_system_t::enable_menu_item(name_t name, bool enabled)
    { object_m->enable_menu_item(name, enabled); }

void menu_system_t::mark_menu_item(name_t name, bool marked)
    { object_m->mark_menu_item(name, marked); }

menu_system_t::connection_t menu_system_t::monitor_menu_item(   name_t           name,
                                                                const item_callback_t&  callback)
    { return object_m->monitor_menu_item(name, callback); }

/****************************************************************************************************/

#if 0
#pragma mark -
#endif

/****************************************************************************************************/

menu_system_t::implementation_t::implementation_t() :
    old_root_m(::AcquireRootMenu())
{
    MenuRef new_root;

    menu_system_t::implementation_t* ptr(this);

    ADOBE_REQUIRE_STATUS(::CreateNewMenu(next_menu_id(), 0, &new_root));
    ADOBE_REQUIRE_STATUS(::SetMenuItemProperty( new_root, 0,
                                                app_creator_k,
                                                menu_system_tag_k,
                                                sizeof(ptr),
                                                &ptr));

    root_m.reset(new_root);

    ::SetRootMenu(new_root);

    ::InvalMenuBar();
    ::DrawMenuBar();
}

/****************************************************************************************************/

menu_system_t::implementation_t::~implementation_t()
{
    ::SetRootMenu(old_root_m.get());

    ::InvalMenuBar();
    ::DrawMenuBar();
}

/****************************************************************************************************/

void menu_system_t::implementation_t::platform_initialization()
{
    static EventTypeSpec                            hi_event = { kEventClassCommand, kHICommandFromMenu };
    static auto_resource<EventHandlerUPP>    hi_handler( ::NewEventHandlerUPP( &::menu_command ) );
    static bool                                     init(false);

    if (!init)
    {
        ::InstallApplicationEventHandler( hi_handler.get(), 1, &hi_event, 0, 0 );

        init = true;
    }
}

/****************************************************************************************************/

inline menu_system_t::implementation_t::component_iterator menu_system_t::implementation_t::find_iter(name_t name)
{
    return adobe::find_if(component_set_m,
                            boost::bind(adobe::compare_members(&component_t::name_m,
                                std::equal_to<name_t>()), name, _1));
}

/****************************************************************************************************/

menu_system_t::implementation_t::component_t& menu_system_t::implementation_t::find(name_t name)
{
    component_iterator item(find_iter(name));

    if (item == component_set_m.end()) throw std::runtime_error("Item not found.");

    return *item;
}

/****************************************************************************************************/

void menu_system_t::implementation_t::insert_menu(name_t name)
{
    MenuRef new_menu(0);

    ADOBE_REQUIRE_STATUS(::CreateNewMenu(next_menu_id(), 0, &new_menu));

    ADOBE_REQUIRE_STATUS(::SetMenuTitleWithCFString(new_menu, string_to_cfstring(name.get()).get()));

    ::InsertMenu(new_menu, 0);

    component_set_m.push_back(component_t());

    component_set_m.back().name_m = name;
    component_set_m.back().ref_m = value_t(new_menu);
}

/****************************************************************************************************/

void menu_system_t::implementation_t::insert_item(  name_t   parent_name,
                                                    name_t   name,
                                                    boost::uint16_t cmd_key,
                                                    modifier_set_t  modifier_set)
{
    component_t& parent(find(parent_name));

    assert (parent.ref_m.type() == typeid(::MenuRef));

    ::MenuRef parent_menu_ref(parent.ref_m.get< ::MenuRef>());

    component_set_m.push_back(component_t());
    component_set_m.back().name_m = name;

    UInt16  count(::CountMenuItems(parent_menu_ref) + 1);

    ADOBE_REQUIRE_STATUS(::InsertMenuItemTextWithCFString(
        parent_menu_ref,
        string_to_cfstring(name.get()).get(),
        count,
        kMenuItemAttrDisabled,
        component_set_m.back().command_id_m));

    if (cmd_key != boost::uint16_t(0))
    {
        ADOBE_REQUIRE_STATUS(::SetMenuItemCommandKey(parent_menu_ref, count, false, cmd_key));

        if (modifier_set != modifier_none_k)
        {
            UInt8 os_modifier_set(kMenuNoModifiers);

            if (modifier_set & modifier_option_k)   os_modifier_set |= kMenuOptionModifier;
            if (modifier_set & modifier_control_k)  os_modifier_set |= kMenuControlModifier;
            if (modifier_set & modifier_shift_k)    os_modifier_set |= kMenuShiftModifier;

            ADOBE_REQUIRE_STATUS(::SetMenuItemModifiers(parent_menu_ref, count, os_modifier_set));
        }
    }
}

/****************************************************************************************************/

void menu_system_t::implementation_t::insert_separator(name_t parent_name)
{
    component_t& parent(find(parent_name));

    assert (parent.ref_m.type() == typeid(::MenuRef));

    ::MenuRef parent_menu_ref(parent.ref_m.get< ::MenuRef>());

    ADOBE_REQUIRE_STATUS(::InsertMenuItemTextWithCFString(
        parent_menu_ref,
        CFSTR(""),
        999999,
        kMenuItemAttrSeparator,
        0));
}

/****************************************************************************************************/

void menu_system_t::implementation_t::remove(name_t name)
{
    component_iterator item(find_iter(name));

    if (item == component_set_m.end()) return;

    assert (::CountMenuItemsWithCommandID(0, item->command_id_m) > 0);

    MenuRef         found_menu_ref(0);
    MenuItemIndex   found_index(0);

    ADOBE_REQUIRE_STATUS(::GetIndMenuItemWithCommandID(0, item->command_id_m, 1, &found_menu_ref, &found_index));

    ::DeleteMenuItem(found_menu_ref, found_index);

    component_set_m.erase(item);
}

/****************************************************************************************************/

void menu_system_t::implementation_t::enable_menu_item(name_t name, bool enabled, bool force)
{
    component_t& item(find(name));

    if (item.enabled_m == enabled && !force) return;

    item.enabled_m = enabled;

    MenuRef         found_menu_ref(0);
    MenuItemIndex   found_index(0);

    assert (::CountMenuItemsWithCommandID(0, item.command_id_m) > 0);

    ADOBE_REQUIRE_STATUS(::GetIndMenuItemWithCommandID(0, item.command_id_m, 1, &found_menu_ref, &found_index));

    item.enabled_m ?
        ::EnableMenuItem(found_menu_ref, found_index) :
        ::DisableMenuItem(found_menu_ref, found_index);

    assert (IsMenuCommandEnabled(0, item.command_id_m) == item.enabled_m);
}

/****************************************************************************************************/

void menu_system_t::implementation_t::mark_menu_item(name_t name, bool marked)
{
    component_t& item(find(name));

    MenuRef         found_menu_ref(0);
    MenuItemIndex   found_index(0);

    assert (::CountMenuItemsWithCommandID(0, item.command_id_m) > 0);

    ADOBE_REQUIRE_STATUS(::GetIndMenuItemWithCommandID(0, item.command_id_m, 1, &found_menu_ref, &found_index));

    ::CheckMenuItem(found_menu_ref, found_index, marked);
}

/****************************************************************************************************/

menu_system_t::connection_t menu_system_t::implementation_t::monitor_menu_item(name_t name, const item_callback_t& callback)
{
    component_t& item(find(name));

    return item.callback_signal_m.connect(callback);
}


/****************************************************************************************************/

}// namespace adobe

/****************************************************************************************************/
// 0
#endif
