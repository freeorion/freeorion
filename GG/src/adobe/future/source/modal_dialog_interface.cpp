/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#define ADOBE_DLL_SAFE 0

#include <GG/Wnd.h>

#include <GG/adobe/config.hpp>
#include <GG/adobe/future/modal_dialog_interface.hpp>
#include <GG/adobe/adam_evaluate.hpp>
#include <GG/adobe/adam_parser.hpp>
#include <GG/adobe/future/assemblage.hpp>
#include <GG/adobe/future/resources.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/keyboard.hpp>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

std::string getline(std::istream& stream)
{
    std::string result;

    result.reserve(128);

    while (true)
    {
        int c(stream.get());

        if (c == EOF || c == '\n')
        {
            break;
        }
        else if (c == '\r')
        {
            c = stream.get();

            if (c != '\n')
                stream.putback(c);

            break;
        }

        result += static_cast<char>(c);
    }

    return result;
}

/****************************************************************************************************/

std::string mdi_error_getline(std::istream& layout_definition, name_t, std::streampos line_start_position)
{
    std::streampos old_pos(layout_definition.tellg());

    layout_definition.clear();

    layout_definition.seekg(line_start_position);

    std::string result(getline(layout_definition));

    layout_definition.seekg(old_pos);

    return result;
}

/****************************************************************************************************/

modal_dialog_t::modal_dialog_t() :
    display_options_m(dialog_display_s),
    parent_m(platform_display_type()),
    root_behavior_m(false),
    defer_view_close_m(false)
{ }

/****************************************************************************************************/

dialog_result_t modal_dialog_t::go(std::istream& layout, std::istream& sheet)
{
    resource_context_t res_context(working_directory_m);

    assemblage_t assemblage;

    vm_lookup_m.attach_to(sheet_m);
    vm_lookup_m.attach_to(sheet_m.machine_m);

    result_m = dialog_result_t();

    //
    // Parse the proprty model stream
    //

    try
    {
        parse( sheet, line_position_t( "Proprty model sheet definition" ), bind_to_sheet( sheet_m ) );
    }
    catch (const stream_error_t& error)
    {
        throw std::logic_error(format_stream_error(sheet, error));
    }
    catch (...)
    {
        throw; // here for completeness' sake
    }

    //
    // REVISIT (2006/09/28, fbrereto): The sheet initializers don't run until the first update().
    //                                 To keep them from blowing away your sets in a couple of
    //                                 lines we need to add an update before the first set()
    //                                 on the sheet (this will get fixed with the proprty model
    //                                 library rewrite so mark it with a revisit comment).
    //

    sheet_m.update();

    //
    // Set up the sheet with initial inputs, etc.
    //

    sheet_m.set(input_m);
    sheet_m.set(record_m);

    need_ui_m = false;

    sheet_m.update();

    //
    // Save off the contributing set for potential reset action later
    //

    contributing_m = sheet_m.contributing();

    //
    // Set up the callback functions now so the contributing mark is meaningful, then update again
    //

    try
    {
        name_t result_cell(static_name_t("result"));

        attach_view(assemblage, result_cell, *this, sheet_m);

        sheet_m.monitor_invariant_dependent(result_cell, boost::bind(&modal_dialog_t::monitor_invariant, boost::ref(*this), _1));
        sheet_m.monitor_contributing(result_cell, sheet_m.contributing(), boost::bind(&modal_dialog_t::monitor_record, boost::ref(*this), _1));
    }
    catch (...)
    {
        // result cell wasn't found. While this isn't a deal-breaker, it's not
        // going to do much in the way of getting results from this dialog.
    }

    sheet_m.update();

    if (display_options_m == dialog_never_display_s && need_ui_m)
        throw std::runtime_error("handle_dialog: Invalid command parameters and UI not permitted.");

    if ((display_options_m == dialog_no_display_s && need_ui_m) ||
        display_options_m == dialog_display_s)
    {
        line_position_t::getline_proc_t getline_proc(new line_position_t::getline_proc_impl_t(boost::bind(&mdi_error_getline, boost::ref(layout), _1, _2)));

        view_m.reset( make_view( static_name_t( "eve definition" ),
                                        getline_proc,
                                        layout,
                                        sheet_m,
                                        root_behavior_m,
                                        boost::bind(&modal_dialog_t::latch_callback, boost::ref(*this), _1, _2),
                                        size_normal_s,
                                        default_widget_factory_proc(),
                                        parent_m).release()
                                        );

        // Set up the view's sheet with display state values, etc.
        //
        view_m->layout_sheet_m.set(display_state_m);

        sheet_m.update();

        //
        // Show the GUI.
        //
        view_m->eve_m.evaluate(eve_t::evaluate_nested);
        view_m->show_window_m();

        platform_display_type dlg = view_m->root_display_m;
        dlg->Run();

#if ADOBE_PLATFORM_MAC

        HIViewRef                            cntl(view_m->root_display_m);
        WindowRef                            owner(::GetControlOwner(cntl));
        auto_resource< ::EventLoopTimerUPP > loop_upp(::NewEventLoopTimerUPP(purge_closed_windows));
        auto_resource< ::EventLoopTimerRef > idle_timer_ref;
        ::EventLoopTimerRef                  temp_timer_ref;
    
        if (::InstallEventLoopTimer(::GetMainEventLoop(),
                                    1,
                                    .01,
                                    loop_upp.get(),
                                    this,
                                    &temp_timer_ref) != noErr)
            throw std::runtime_error("InstallEventLoopTimer");     

        idle_timer_ref.reset(temp_timer_ref);

        raw_key_handler_t keyboard_event_monitor;

        keyboard_event_monitor.window_m = owner;
        keyboard_event_monitor.handler_m.insert(kEventClassKeyboard, kEventRawKeyDown);
//        keyboard_event_monitor.handler_m.insert(kEventClassKeyboard, kEventRawKeyUp);
        keyboard_event_monitor.handler_m.install(::GetApplicationEventTarget());

        ::ShowWindow(owner);
        ::ActivateWindow(owner, true);
        ::BringToFront(owner);

        ::RunAppModalLoopForWindow(owner);

#elif ADOBE_PLATFORM_WIN

        HWND cntl(view_m->root_display_m);

        MSG  msg;

        while ( ::GetMessage( &msg, 0, 0, 0 ) )
        {
            try
            {
                bool special_key_handled(false);

                if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN ||
                    msg.message == WM_KEYUP   || msg.message == WM_SYSKEYUP)
                {
                    special_key_handled = keyboard_t::get().dispatch(key_type(msg.wParam),
                                                                     msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN,
                                                                     modifier_state(),
                                                                     any_regular_t(cntl));
                }

                if(special_key_handled == false && ::IsDialogMessage(cntl, &msg) == false)
                {
                    //
                    // Pass the message through to the application.
                    //
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }
            }
            catch ( const std::exception& error )
            {
                std::cerr << "Exception: " << error.what() << std::endl;
            }
            catch ( ... )
            {
                std::cerr << "Exception: Unknown" << std::endl;
            }

            end_dialog();
        }

#endif

        result_m.display_state_m = view_m->layout_sheet_m.contributing();

        view_m.reset(0);

    }

    return result_m;
}

/****************************************************************************************************/

bool modal_dialog_t::end_dialog()
{
    if (defer_view_close_m == false)
        return false;

#if ADOBE_PLATFORM_MAC

    HIViewRef cntl(view_m->root_display_m);
    WindowRef owner(::GetControlOwner(cntl));

    ::QuitAppModalLoopForWindow(owner);

#elif ADOBE_PLATFORM_WIN

    ::PostQuitMessage(0);

#endif

    return true;
}

/****************************************************************************************************/

void modal_dialog_t::latch_callback(name_t action, const any_regular_t& value)
try
{
    assert(view_m);
    assert(callback_m);

    if (action == static_name_t("reset"))
    {
        sheet_m.set(contributing_m);
        sheet_m.update();
    }
    else if (callback_m(action, value))
    {
        result_m.terminating_action_m = action;

        defer_view_close_m = true;
    }
}
catch(const std::exception& error)
{
    std::cerr << "Exception (modal_dialog_t::latch_callback) : " << error.what() << std::endl;
}
catch(...)
{
    std::cerr << "Unknown exception (modal_dialog_t::latch_callback)" << std::endl;
}

/****************************************************************************************************/

void modal_dialog_t::display(const model_type& value)
{
    result_m.command_m = value.cast<dictionary_t>();
}

/****************************************************************************************************/

void modal_dialog_t::monitor_record(const dictionary_t& record_info)
{
    result_m.record_m = record_info;
}

/****************************************************************************************************/

void modal_dialog_t::monitor_invariant(bool is_set)
{
    need_ui_m = !is_set;
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
