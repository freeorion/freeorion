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
        if (first != last && !m_blocking)
            return m_combiner(first, last);
        else
            return result_type();
    }

private:
    const bool& m_blocking;
    inner_combiner m_combiner;
};

class Universe;

/** Re-implementation of blocking_combiner that can be assigned to. */
struct assignable_blocking_combiner {
    using InnerCombinerT = boost::signals2::optional_last_value<void>;
    using result_type = InnerCombinerT::result_type;

    assignable_blocking_combiner() = default;

    explicit assignable_blocking_combiner(std::function<bool()> is_blocking_func) :
        blocking(std::move(is_blocking_func))
    {}

    explicit assignable_blocking_combiner(const bool& b) :
        blocking([&b]() -> bool { return b; })
    {}

    explicit assignable_blocking_combiner(const Universe& universe);

    template <typename input_iterator>
    result_type operator()(input_iterator first, input_iterator last)
    {
        if (first != last && !blocking())
            inner_combiner(first, last);
    }

    std::function<bool()> blocking = []() -> bool { return false; };
    InnerCombinerT inner_combiner;
};


#endif
