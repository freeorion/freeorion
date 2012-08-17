/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/window_server.hpp>
#include <GG/adobe/future/widgets/headers/factory.hpp>

#include <GG/adobe/algorithm/for_each.hpp>
#include <GG/adobe/future/resources.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

window_server_t::window_server_t(sheet_t& sheet, behavior_t& behavior) :
    sheet_m(sheet),
    behavior_m(behavior),
    widget_factory_m(default_asl_widget_factory())
{ }

/*************************************************************************************************/

window_server_t::~window_server_t()
{
    adobe::for_each(window_list_m, delete_ptr<eve_client_holder*>());
}

/*************************************************************************************************/

eve_client_holder& window_server_t::top_client_holder()
{
    if (window_list_m.empty())
        throw std::runtime_error("No top view!");

    return *(window_list_m.back());
}

/*************************************************************************************************/

void window_server_t::push_back(const char* name, size_enum_t dialog_size)
{
    boost::filesystem::path     relative_path(window_list_m.back()->path_m.branch_path() / name);
    iterator                    window(window_list_m.insert(window_list_m.end(), NULL));
    boost::filesystem::path     file_name;

    try
    {
        file_name = find_resource(name);
    }
    catch (...)
    {
        file_name = relative_path;
    }

    boost::filesystem::ifstream stream(file_name);

    /*
    Update before attaching the window so that we can correctly capture contributing for reset.
    */
    
    sheet_m.update();

    window_list_m.back() = make_view(   name_t(file_name.string().c_str()),
                                        line_position_t::getline_proc_t(),
                                        stream,
                                        sheet_m,
                                        behavior_m,
                                        boost::bind(&window_server_t::dispatch_window_action,
                                            boost::ref(*this), window, _1, _2),
                                        dialog_size,
                                        default_widget_factory_proc_with_factory(widget_factory_m)).release();
    
    sheet_m.update(); // Force values to their correct states.

    window_list_m.back()->path_m = file_name;
    window_list_m.back()->eve_m.evaluate(eve_t::evaluate_nested);
    window_list_m.back()->show_window_m();
}

/*************************************************************************************************/

void window_server_t::push_back(std::istream&                                   data,
                                const boost::filesystem::path&                  path,
                                const line_position_t::getline_proc_t&   getline_proc,
                                size_enum_t                              dialog_size)
{
    /*
    Update before attaching the window so that we can correctly capture contributing for reset.
    */
    sheet_m.update();

    iterator window (window_list_m.insert(window_list_m.end(), NULL));

    //
    // REVISIT (ralpht): Where does this made-up filename get used? Does it need to be localized
    //  or actually be an existing file?
    //
    //  REVISIT (fbrereto) :    The file name is for error reporting purposes; see the const char*
    //                          push_back API where this is filled in. It should be valid, lest the
    //                          user not know the erroneous file.
    //
    window_list_m.back() = make_view(   name_t(path.string().c_str()),
                                        getline_proc,
                                        data,
                                        sheet_m,
                                        behavior_m,
                                        boost::bind(&window_server_t::dispatch_window_action,
                                            boost::ref(*this), window, _1, _2),
                                        dialog_size,
                                        default_widget_factory_proc_with_factory(widget_factory_m)).release();

    sheet_m.update(); // Force values to their correct states.

    window_list_m.back()->path_m = path;
    window_list_m.back()->eve_m.evaluate(eve_t::evaluate_nested);
    window_list_m.back()->show_window_m();
}

/*************************************************************************************************/

void window_server_t::erase(iterator window)
{
    delete *window;
    window_list_m.erase(window);
}

/*************************************************************************************************/

void window_server_t::set_action_fallback(action_fallback_proc_t proc)
{
    fallback_m = proc;
}

/*************************************************************************************************/

void window_server_t::dispatch_window_action(iterator window, name_t action, const any_regular_t& parameter)
{
    if (action == static_name_t("reset"))
    {
        sheet_m.set((*window)->contributing_m);
        sheet_m.update();
    }
    else if (action == static_name_t("dialog"))
    {
        push_back(parameter.cast<std::string>().c_str(), size_normal_s);
    }
    else if (action == static_name_t("cancel"))
    {
        sheet_m.set((*window)->contributing_m);
        sheet_m.update();

        general_deferred_proc_queue().insert(boost::bind(&window_server_t::erase, boost::ref(*this), window));
    }
    else if (action == static_name_t("ok"))
    {
        general_deferred_proc_queue().insert(boost::bind(&window_server_t::erase, boost::ref(*this), window));
    }

    if (fallback_m)
        fallback_m(action, parameter); 
}

/*************************************************************************************************/

void window_server_t::dispatch_action(name_t action, const any_regular_t& parameter)
{

    if (fallback_m)
        fallback_m(action, parameter);
}

/*************************************************************************************************/

#if 0
void window_server_t::set_back(const char* file_name, size_enum_t dialog_size)
{
    if (window_stack_m.size()) pop_back(false);
    push_back(file_name, dialog_size);
}
#endif

/****************************************************************************************************/

}

/****************************************************************************************************/
