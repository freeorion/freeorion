// -*- C++ -*-
#ifndef _InhibitableSignal_h_
#define _InhibitableSignal_h_

#include <boost/signal.hpp>
#include <boost/bind.hpp>


/** A class template for a type of signal that wraps a boost::signal so that its
  * emission can be controlled by an external boolean value. */
template <class T>
class InhibitableSignal
{
public:
    typedef T type;
    typedef typename T::slot_type slot_type;
    typedef typename T::group_type group_type;
    InhibitableSignal(const bool& inhibitor) : m_inhibitor(inhibitor) {}
//    InhibitableSignal(const InhibitableSignal& rhs) : m_inhibitor(rhs.m_inhibitor) {}

    void operator()()
    { if (!m_inhibitor) m_sig(); }

    boost::signals::connection
    connect(const slot_type& slot, boost::signals::connect_position at = boost::signals::at_back)
    { return m_sig.connect(slot, at); }

    boost::signals::connection
    connect(const group_type& group, const slot_type& slot, boost::signals::connect_position at = boost::signals::at_back)
    { return m_sig.connect(group, slot, at); }

private:
    const bool& m_inhibitor;
    type        m_sig;
};

namespace GG {
/** Connects an InhibitableSignal to a member function of a specific object that
  * has the same function signature, putting \a R  in slot group 0.  Slot call
  * groups are called in ascending order.
  * Overloads exist for const- and non-const- versions with 0 to 8 arguments.  8 
  * as picked as the max simply because boost::bind only supports up to 8 args as
  * of this writing. */
template <class C, class R, class T1, class T2> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (), C> >& sig, 
        R (T1::* fn) (), 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj), at);
}

template <class C, class R, class T1, class T2> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (), C> >& sig, 
        R (T1::* fn) () const, 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj), at);
}

template <class C, class R, class T1, class T2, class A1> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1), C> >& sig, 
        R (T1::* fn) (A1), 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj, _1), at);
}

template <class C, class R, class T1, class T2, class A1> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1), C> >& sig,
        R (T1::* fn) (A1) const, 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj, _1), at);
}

template <class C, class R, class T1, class T2, class A1, class A2> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2), C> >& sig, 
        R (T1::* fn) (A1, A2), 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj, _1, _2), at);
}

template <class C, class R, class T1, class T2, class A1, class A2> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2), C> >& sig, 
        R (T1::* fn) (A1, A2) const, 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj, _1, _2), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3), C> >& sig, 
        R (T1::* fn) (A1, A2, A3), 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj, _1, _2, _3), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3), C> >& sig, 
        R (T1::* fn) (A1, A2, A3) const, 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj, _1, _2, _3), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4), 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj, _1, _2, _3, _4), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4) const, 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj, _1, _2, _3, _4), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4, class A5> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4, A5), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4, A5), 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj, _1, _2, _3, _4, _5), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4, class A5> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4, A5), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4, A5) const, 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj, _1, _2, _3, _4, _5), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4, class A5, class A6> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4, A5, A6), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4, A5, A6), 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj, _1, _2, _3, _4, _5, _6), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4, class A5, class A6> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4, A5, A6), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4, A5, A6) const, 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj, _1, _2, _3, _4, _5, _6), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4, class A5, class A6, class A7> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4, A5, A6, A7), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4, A5, A6, A7), 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj, _1, _2, _3, _4, _5, _6, _7), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4, class A5, class A6, class A7> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4, A5, A6, A7), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4, A5, A6, A7) const, 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj, _1, _2, _3, _4, _5, _6, _7), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4, A5, A6, A7, A8), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4, A5, A6, A7, A8), 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj, _1, _2, _3, _4, _5, _6, _7, _8), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4, A5, A6, A7, A8), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4, A5, A6, A7, A8) const, 
        T2* obj, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(boost::bind(fn, obj, _1, _2, _3, _4, _5, _6, _7, _8), at);
}

/** Connects an InhibitableSignal to a member function of a specific object that
  * has the same function signature, putting \a R in slot group \a grp.  Slot
  * call groups are called in ascending order. 
  * Overloads exist for const- and non-const- versions with 0 to 8 arguments.  8
  * was picked as the max simply because boost::bind only supports up to 8 args
  * as of this writing. */
template <class C, class R, class T1, class T2> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (), C> >& sig, 
        R (T1::* fn) (), 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj), at);
}

template <class C, class R, class T1, class T2> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (), C> >& sig, 
        R (T1::* fn) () const, 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj), at);
}

template <class C, class R, class T1, class T2, class A1> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1), C> >& sig, 
        R (T1::* fn) (A1), 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj, _1), at);
}

template <class C, class R, class T1, class T2, class A1> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1), C> >& sig,
        R (T1::* fn) (A1) const, 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj, _1), at);
}

template <class C, class R, class T1, class T2, class A1, class A2> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2), C> >& sig, 
        R (T1::* fn) (A1, A2), 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2), at);
}

template <class C, class R, class T1, class T2, class A1, class A2> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2), C> >& sig, 
        R (T1::* fn) (A1, A2) const, 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3), C> >& sig, 
        R (T1::* fn) (A1, A2, A3), 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3), C> >& sig, 
        R (T1::* fn) (A1, A2, A3) const, 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4), 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4) const, 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4, class A5> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4, A5), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4, A5), 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4, _5), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4, class A5> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4, A5), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4, A5) const, 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4, _5), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4, class A5, class A6> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4, A5, A6), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4, A5, A6), 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4, _5, _6), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4, class A5, class A6> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4, A5, A6), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4, A5, A6) const, 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4, _5, _6), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4, class A5, class A6, class A7> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4, A5, A6, A7), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4, A5, A6, A7), 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4, _5, _6, _7), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4, class A5, class A6, class A7> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4, A5, A6, A7), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4, A5, A6, A7) const, 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4, _5, _6, _7), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4, A5, A6, A7, A8), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4, A5, A6, A7, A8), 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4, _5, _6, _7, _8), at);
}

template <class C, class R, class T1, class T2, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8> inline
boost::signals::connection 
Connect(InhibitableSignal<boost::signal<R (A1, A2, A3, A4, A5, A6, A7, A8), C> >& sig, 
        R (T1::* fn) (A1, A2, A3, A4, A5, A6, A7, A8) const, 
        T2* obj, 
        int grp, 
        boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4, _5, _6, _7, _8), at);
}

}

#endif // _InhibitableSignal_h_
