// -*- C++ -*-
/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@gmail.com */

/* Note that this file intentionally has no include guards.  This is because it is intended to be included multiple
   times, as in SignalsAndSlots.h. */

#if GG_SIGNALS_NUM_ARGS == 0
#define GG_SIGNALS_COMMA_IF_NONZERO_ARGS
#else
#define GG_SIGNALS_COMMA_IF_NONZERO_ARGS ,
#endif

#define GG_SIGNALS_FORWARDER_NAME BOOST_PP_CAT(GG_SIGNALS_FORWARDER_BASE_NAME, GG_SIGNALS_NUM_ARGS)

namespace GG {

namespace detail {
template <class C, class R GG_SIGNALS_COMMA_IF_NONZERO_ARGS
          GG_SIGNALS_SIGNAL_TEMPLATE_PARMS>
struct GG_SIGNALS_FORWARDER_NAME
{
    GG_SIGNALS_FORWARDER_NAME(boost::signals2::signal<R (GG_SIGNALS_SIGNAL_TEMPLATE_ARGS), C>& sig_) : sig(sig_) {}
    R operator()(GG_SIGNALS_SIGNAL_PARMS) {sig(GG_SIGNALS_SIGNAL_ARGS);}
    boost::signals2::signal<R (GG_SIGNALS_SIGNAL_TEMPLATE_ARGS), C>& sig;
};
} // namespace detail

/** connects a signal to a member function of a specific object that has the same function signature, putting \a R in
    slot group 0.  Slot call groups are called in ascending order.  Overloads exist for const- and non-const- versions
    with 0 to 8 arguments.  8 was picked as the max simply because boost::bind only supports up to 8 args as of this
    writing. */
template <class C, class R, class T1, class T2 GG_SIGNALS_COMMA_IF_NONZERO_ARGS
          GG_SIGNALS_SIGNAL_TEMPLATE_PARMS> inline
boost::signals2::connection
Connect(boost::signals2::signal<R (GG_SIGNALS_SIGNAL_TEMPLATE_ARGS), C>& sig,
        R (T1::* fn) (GG_SIGNALS_SIGNAL_TEMPLATE_ARGS),
        T2 obj,
        boost::signals2::connect_position at = boost::signals2::at_back)
{
    return sig.connect(boost::bind(fn, obj GG_SIGNALS_COMMA_IF_NONZERO_ARGS GG_SIGNALS_BIND_ARGS), at);
}

/** connects a signal to a const member function of a specific object that has the same function signature, putting \a R
    in slot group 0.  Slot call groups are called in ascending order.  Overloads exist for const- and non-const-
    versions with 0 to 8 arguments.  8 was picked as the max simply because boost::bind only supports up to 8 args as of
    this writing. */
template <class C, class R, class T1, class T2 GG_SIGNALS_COMMA_IF_NONZERO_ARGS
          GG_SIGNALS_SIGNAL_TEMPLATE_PARMS> inline
boost::signals2::connection
Connect(boost::signals2::signal<R (GG_SIGNALS_SIGNAL_TEMPLATE_ARGS), C>& sig,
        R (T1::* fn) (GG_SIGNALS_SIGNAL_TEMPLATE_ARGS) const,
        T2 obj,
        boost::signals2::connect_position at = boost::signals2::at_back)
{
    return sig.connect(boost::bind(fn, obj GG_SIGNALS_COMMA_IF_NONZERO_ARGS GG_SIGNALS_BIND_ARGS), at);
}

/** connects a signal to a member function of a specific object that has the same function signature, putting \a R in
    slot group \a grp.  Slot call groups are called in ascending order. Overloads exist for const- and non-const-
    versions with 0 to 8 arguments.  8 was picked as the max simply because boost::bind only supports up to 8 args as of
    this writing. */
template <class C, class R, class T1, class T2 GG_SIGNALS_COMMA_IF_NONZERO_ARGS
          GG_SIGNALS_SIGNAL_TEMPLATE_PARMS> inline
boost::signals2::connection
Connect(boost::signals2::signal<R (GG_SIGNALS_SIGNAL_TEMPLATE_ARGS), C>& sig,
        R (T1::* fn) (GG_SIGNALS_SIGNAL_TEMPLATE_ARGS),
        T2 obj,
        int grp,
        boost::signals2::connect_position at = boost::signals2::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj GG_SIGNALS_COMMA_IF_NONZERO_ARGS GG_SIGNALS_BIND_ARGS), at);
}

/** connects a signal to a const member function of a specific object that has the same function signature, putting \a R
    in slot group \a grp.  Slot call groups are called in ascending order. Overloads exist for const- and non-const-
    versions with 0 to 8 arguments.  8 was picked as the max simply because boost::bind only supports up to 8 args as of
    this writing. */
template <class C, class R, class T1, class T2 GG_SIGNALS_COMMA_IF_NONZERO_ARGS
          GG_SIGNALS_SIGNAL_TEMPLATE_PARMS> inline
boost::signals2::connection
Connect(boost::signals2::signal<R (GG_SIGNALS_SIGNAL_TEMPLATE_ARGS), C>& sig,
        R (T1::* fn) (GG_SIGNALS_SIGNAL_TEMPLATE_ARGS) const,
        T2 obj,
        int grp,
        boost::signals2::connect_position at = boost::signals2::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj GG_SIGNALS_COMMA_IF_NONZERO_ARGS GG_SIGNALS_BIND_ARGS), at);
}

/** connects a signal to another signal of the same signature, establishing signal-forwarding. \a sig1 places \a sig2 in
    its slot group 0. */
template <class C, class R GG_SIGNALS_COMMA_IF_NONZERO_ARGS
          GG_SIGNALS_SIGNAL_TEMPLATE_PARMS> inline
boost::signals2::connection
Connect(boost::signals2::signal<R (GG_SIGNALS_SIGNAL_TEMPLATE_ARGS), C>& sig1,
        boost::signals2::signal<R (GG_SIGNALS_SIGNAL_TEMPLATE_ARGS), C>& sig2,
        boost::signals2::connect_position at = boost::signals2::at_back)
{
    typedef typename detail::GG_SIGNALS_FORWARDER_NAME<C, R GG_SIGNALS_COMMA_IF_NONZERO_ARGS GG_SIGNALS_SIGNAL_TEMPLATE_ARGS> Forwarder;
    return sig1.connect(Forwarder(sig2), at);
}

/** connects a signal to another signal of the same signature, establishing signal-forwarding. \a sig1 places \a sig2 in
    its slot group \a grp. */
template <class C, class R GG_SIGNALS_COMMA_IF_NONZERO_ARGS
          GG_SIGNALS_SIGNAL_TEMPLATE_PARMS> inline
boost::signals2::connection
Connect(boost::signals2::signal<R (GG_SIGNALS_SIGNAL_TEMPLATE_ARGS), C>& sig1,
        boost::signals2::signal<R (GG_SIGNALS_SIGNAL_TEMPLATE_ARGS), C>& sig2,
        int grp,
        boost::signals2::connect_position at = boost::signals2::at_back)
{
    typedef typename detail::GG_SIGNALS_FORWARDER_NAME<C, R GG_SIGNALS_COMMA_IF_NONZERO_ARGS GG_SIGNALS_SIGNAL_TEMPLATE_ARGS> Forwarder;
    return sig1.connect(grp, Forwarder(sig2), at);
}

} //namespace GG

#undef GG_SIGNALS_COMMA_IF_NONZERO_ARGS
#undef GG_SIGNALS_FORWARDER_NAME
