// -*- C++ -*-
#ifndef _OptionValidators_h_
#define _OptionValidators_h_

#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <cmath>
#include <string>
#include <set>

// these are needed by the StepValidator
namespace details {
    template <class T> inline T mod (T dividend, T divisor)
    {  return (dividend % divisor); }

    template <> inline float mod<float>(float dividend, float divisor)
    { return std::fmod(dividend, divisor); }

    template <> inline double mod<double>(double dividend, double divisor)
    { return std::fmod(dividend, divisor); }

    template <> inline long double mod<long double>(long double dividend, long double divisor)
    { return std::fmod(dividend, divisor); }
}

/** base class for all OptionsDB validators. Simply provides the basic interface. */
struct ValidatorBase {
    virtual ~ValidatorBase() {}
    /** returns normally if \a str is a valid value, or throws otherwise */
    virtual boost::any Validate(const std::string& str) const = 0;

    /** returns the string representation of \a value */
    virtual std::string String(const boost::any& value) const = 0;

    /** returns a dynamically allocated copy of the object. */
    virtual ValidatorBase *Clone() const = 0;
};

/** determines if a string is a valid value for an OptionsDB option */
template <class T>
struct Validator : public ValidatorBase
{
    virtual boost::any Validate(const std::string& str) const
    { return boost::any(boost::lexical_cast<T>(str)); }

    virtual std::string String(const boost::any& value) const
    { return boost::lexical_cast<std::string>(boost::any_cast<T>(value)); }

    virtual Validator *Clone() const
    { return new Validator<T>(); }
};

/** a Validator that constrains the range of valid values */
template <class T>
struct RangedValidator : public Validator<T>
{
    RangedValidator(const T& min, const T& max) : m_min(min), m_max(max) {}

    virtual boost::any Validate(const std::string& str) const {
        T val = boost::lexical_cast<T>(str);
        if (val < m_min || val > m_max)
            throw boost::bad_lexical_cast();
        return boost::any(val);
    }

    virtual RangedValidator *Clone() const
    { return new RangedValidator<T>(m_min, m_max); }

    const T m_min;
    const T m_max;
};

/** a Validator that constrains valid values to certain step-values (eg: 0, 25, 50, ...).  The steps are assumed to
    begin at the validated type's default-constructed value, unless another origin is specified. */
template <class T>
struct StepValidator : public Validator<T>
{
    StepValidator(const T& step, const T& origin = T()) : m_step_size(step), m_origin(origin) {}

    virtual boost::any Validate(const std::string& str) const {
        T val = boost::lexical_cast<T>(str);
        if (std::abs(details::mod((val - m_origin), m_step_size)) > std::numeric_limits<T>::epsilon())
            throw boost::bad_lexical_cast();
        return boost::any(val);
    }

    virtual StepValidator *Clone() const
    { return new StepValidator<T>(m_step_size, m_origin); }

    const T m_step_size;
    const T m_origin;
};

/** a Validator similar to a StepValidator, but that further constrains the valid values to be within a certain range (eg: [25, 50, ..., 200]). */
template <class T>
struct RangedStepValidator : public Validator<T>
{
public:
    RangedStepValidator(const T& step, const T& min, const T& max) : m_step_size(step), m_origin(T()), m_min(min), m_max(max) {}
    RangedStepValidator(const T& step, const T& origin, const T& min, const T& max) : m_step_size (step), m_origin (origin), m_min (min), m_max (max) {}

    virtual boost::any Validate(const std::string& str) const {
        T val = boost::lexical_cast<T>(str);
        if (val < m_min || val > m_max ||
            std::abs(details::mod<T>(val - m_origin, m_step_size)) > std::numeric_limits<T>::epsilon() &&
            std::abs(m_step_size - details::mod<T>(val - m_origin, m_step_size)) > std::numeric_limits<T>::epsilon())
            throw boost::bad_lexical_cast();
        return boost::any(val);
    }

    virtual RangedStepValidator *Clone() const
    { return new RangedStepValidator<T>(m_step_size, m_origin, m_min, m_max); }

    const T m_step_size;
    const T m_origin;
    const T m_min;
    const T m_max;
};

/// a Validator that specifies a finite number of valid values.
/** Probably won't work well with floating point types. */
template <class T>
struct DiscreteValidator : public Validator<T>
{
    DiscreteValidator(const T& single_value) :
        m_values(&single_value, &single_value + 1)
    { }

    DiscreteValidator(const std::set<T>& values) :
        m_values(values)
    { }

    template <class iter>
    DiscreteValidator(iter start, iter finish) :
        m_values(start, finish)
    { }

    template <size_t N>
    DiscreteValidator(const T (&in)[N]) :
        m_values(in, in + N)
    { }

    virtual boost::any Validate(const std::string& str) const {
        T val = boost::lexical_cast<T>(str);

        if (m_values.find(val) == m_values.end())
            throw boost::bad_lexical_cast();

        return boost::any(val);
    }

    virtual DiscreteValidator* Clone() const
    { return new DiscreteValidator<T>(m_values); }

    /// Stores the list of vaild values.
    const std::set<T> m_values;
};

/// a Validator that performs a logical OR of two validators.
/** Stores and owns clones of the provided validators in boost::scoped_ptrs.
 *  Always calls m_validator_a->Validate(). Only calls m_validator_b->Validate()
 *  if the first one throws. */
template <class T>
struct OrValidator : public Validator<T>
{
    OrValidator(const Validator<T>& validator_a,
                const Validator<T>& validator_b) :
        m_validator_a(validator_a.Clone()),
        m_validator_b(validator_b.Clone())
    { }

    virtual boost::any Validate(const std::string& str) const {
        boost::any result;

        try {
            result = m_validator_a->Validate(str);
        } catch (boost::bad_lexical_cast&) {
            result = m_validator_b->Validate(str);
        }

        return result;
    }

    virtual OrValidator* Clone() const
    { return new OrValidator<T>(*m_validator_a.get(), *m_validator_b.get()); }

    const boost::scoped_ptr<Validator<T> > m_validator_a;
    const boost::scoped_ptr<Validator<T> > m_validator_b;
};

#endif // _OptionValidators_h_
