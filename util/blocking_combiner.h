#ifndef _blocking_combiner_h_
#define _blocking_combiner_h_

/** Implementation of a combiner to block or unblock signals emitted by
 * boost::signals calls. Sample usage:
 *
 * bool the_control = true;
 *
 * boost::signals<void (), blocking_combiner<boost::last_value<void>>> the_signal(blocking_combiner<boost::last_value<void>(the_control));
 *
 * the_signal(); // signal is propagated.
 * the_control = true; // blocking future signals
 * the_signal(); // signal is blocked.
 * the_control = false; // propagate future signals
 * the_signal(); // signal is propagated.
 *
 */
template <typename inner_combiner>
struct blocking_combiner
{
    typedef typename inner_combiner::result_type result_type;

    blocking_combiner(const bool& blocking): m_blocking(blocking), m_combiner() {}

    template <typename input_iterator>
    result_type operator()(input_iterator first, input_iterator last)
    {
        if(first != last && !m_blocking)
            return m_combiner(first, last);
        else
            return result_type();
    }

private:
    const bool& m_blocking;
    inner_combiner m_combiner;
};

#endif // _blocking_combiner_h_
