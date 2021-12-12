#ifndef _OptionValidators_h_
#define _OptionValidators_h_

#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>

#include <cmath>
#include <memory>
#include <set>
#include <string>
#include <vector>


// these are needed by the StepValidator
namespace details {
    template <typename T>
    inline T mod (T dividend, T divisor)
    { return (dividend % divisor); }

    template <>
    inline float mod<float>(float dividend, float divisor)
    { return std::fmod(dividend, divisor); }

    template <>
    inline double mod<double>(double dividend, double divisor)
    { return std::fmod(dividend, divisor); }

    template <>
    inline long double mod<long double>(long double dividend, long double divisor)
    { return std::fmod(dividend, divisor); }
}

/** Interface base class for all OptionsDB validators. Simply provides the basic interface. */
struct ValidatorBase {
    ValidatorBase() = default;
    ValidatorBase(ValidatorBase&& rhs) noexcept = default;
    virtual ~ValidatorBase() = default;

    /** returns normally if \a str is a valid value, or throws otherwise */
    virtual boost::any Validate(const std::string& str) const = 0;
    virtual boost::any Validate(std::string_view str) const = 0;

    /** returns the string representation of \a value */
    [[nodiscard]] virtual std::string String(const boost::any& value) const = 0;

    /** returns a dynamically allocated copy of the object. */
    [[nodiscard]] virtual std::unique_ptr<ValidatorBase> Clone() const = 0;
};

/** determines if a string is a valid value for an OptionsDB option */
template <typename T>
struct Validator : public ValidatorBase
{
    Validator() = default;
    Validator(Validator&& rhs) noexcept = default;
    ~Validator() override = default;

    boost::any Validate(const std::string& str) const override
    { return boost::any(boost::lexical_cast<T>(str)); }
    boost::any Validate(std::string_view str) const override
    { return boost::any(boost::lexical_cast<T>(str)); }

    [[nodiscard]] std::string String(const boost::any& value) const override
    {
        if constexpr (std::is_same_v<T, std::string>)
            return boost::any_cast<std::string>(value);
        else if constexpr (std::is_enum_v<T>)
            return std::string{to_string(boost::any_cast<T>(value))};
        else if constexpr (std::is_arithmetic_v<T>)
            return std::to_string(boost::any_cast<T>(value));
        else
            return boost::lexical_cast<std::string>(boost::any_cast<T>(value));
    }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() const override
    { return std::make_unique<Validator>(); }
};

FO_COMMON_API std::string ListToString(std::vector<std::string>&& input_list);
FO_COMMON_API std::vector<std::string> StringToList(std::string_view input_string);
FO_COMMON_API std::vector<std::string> StringToList(const char* input_string);
FO_COMMON_API std::vector<std::string> StringToList(const std::string& input_string);

template <>
struct Validator<std::vector<std::string>> : public ValidatorBase
{
    Validator() = default;
    Validator(Validator&& rhs) noexcept = default;
    ~Validator() override = default;

    boost::any Validate(const std::string& str) const override
    { return boost::any(StringToList(str)); }
    boost::any Validate(std::string_view str) const override
    { return boost::any(StringToList(str)); }

    [[nodiscard]] std::string String(const boost::any& value) const override
    { return ListToString(boost::any_cast<std::vector<std::string>>(value)); }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() const override
    { return std::make_unique<Validator<std::vector<std::string>>>(); }
};

/** a Validator that constrains the range of valid values */
template <typename T>
struct RangedValidator : public Validator<T>
{
    RangedValidator(const T& min, const T& max) : m_min(min), m_max(max) {}
    RangedValidator(RangedValidator&& rhs) noexcept = default;
    ~RangedValidator() override = default;

    boost::any Validate(const std::string& str) const override {
        T val = boost::lexical_cast<T>(str);
        if (val < m_min || val > m_max)
            throw boost::bad_lexical_cast();
        return boost::any(val);
    }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() const override
    { return std::make_unique<RangedValidator>(m_min, m_max); }

    T m_min;
    T m_max;
    static_assert(std::is_nothrow_move_constructible<T>::value);
};

/** a Validator that constrains valid values to certain step-values
    (eg: 0, 25, 50, ...).  The steps are assumed to begin at the
    validated type's default-constructed value, unless another origin
    is specified. */
template <typename T>
struct StepValidator : public Validator<T>
{
    StepValidator(const T& step, const T& origin = T()) : m_step_size(step), m_origin(origin) {}
    StepValidator(StepValidator&& rhs) noexcept = default;
    ~StepValidator() override = default;

    boost::any Validate(const std::string& str) const override {
        T val = boost::lexical_cast<T>(str);
        if (std::abs(details::mod((val - m_origin), m_step_size)) > std::numeric_limits<T>::epsilon())
            throw boost::bad_lexical_cast();
        return boost::any(val);
    }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() const override
    { return std::make_unique<StepValidator>(m_step_size, m_origin); }

    T m_step_size;
    T m_origin;
    static_assert(std::is_nothrow_move_constructible<T>::value);
};

/** a Validator similar to a StepValidator, but that further constrains the valid values to be within a certain range (eg: [25, 50, ..., 200]). */
template <typename T>
struct RangedStepValidator : public Validator<T>
{
public:
    RangedStepValidator(const T& step, const T& min, const T& max) : m_step_size(step), m_origin(T()), m_min(min), m_max(max) {}
    RangedStepValidator(const T& step, const T& origin, const T& min, const T& max) : m_step_size (step), m_origin (origin), m_min (min), m_max (max) {}
    RangedStepValidator(RangedStepValidator&& rhs) noexcept = default;
    ~RangedStepValidator() override = default;

    boost::any Validate(const std::string& str) const override {
        T val = boost::lexical_cast<T>(str);
        if ((val < m_min) || (val > m_max) ||
            ((std::abs(details::mod<T>(val - m_origin, m_step_size)) > std::numeric_limits<T>::epsilon()) &&
             (std::abs(m_step_size - details::mod<T>(val - m_origin, m_step_size)) > std::numeric_limits<T>::epsilon())))
            throw boost::bad_lexical_cast();
        return boost::any(val);
    }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() const override
    { return std::make_unique<RangedStepValidator>(m_step_size, m_origin, m_min, m_max); }

    T m_step_size;
    T m_origin;
    T m_min;
    T m_max;
    static_assert(std::is_nothrow_move_constructible<T>::value);
};

/// a Validator that specifies a finite number of valid values.
/** Probably won't work well with floating point types. */
template <typename T>
struct DiscreteValidator : public Validator<T>
{
    DiscreteValidator(T single_value) :
        m_values{std::move(single_value)}
    {}

    DiscreteValidator(std::set<T> values) :
        m_values(std::move(values))
    {}

    template <typename iter>
    DiscreteValidator(iter start, iter finish) :
        m_values(start, finish)
    {}

    DiscreteValidator(DiscreteValidator&& rhs) noexcept = default;

    ~DiscreteValidator() override = default;

    boost::any Validate(const std::string& str) const override {
        T val = boost::lexical_cast<T>(str);

        if (!m_values.count(val))
            throw boost::bad_lexical_cast();

        return boost::any(val);
    }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() const override
    { return std::make_unique<DiscreteValidator>(m_values); }

    /// Stores the list of vaild values.
    std::set<T> m_values;
    static_assert(std::is_nothrow_move_constructible<T>::value);
};

/// a Validator that performs a logical OR of two validators.
/** Stores and owns clones of the provided validators in std::unique_ptr.
 *  Always calls m_validator_a->Validate(). Only calls m_validator_b->Validate()
 *  if the first one throws. */
template <typename T>
struct OrValidator : public Validator<T>
{
    OrValidator(Validator<T>&& validator_a, Validator<T>&& validator_b) :
        m_validator_a{std::make_unique<Validator<T>>(std::move(validator_a))},
        m_validator_b{std::make_unique<Validator<T>>(std::move(validator_b))}
    {}

    OrValidator(std::unique_ptr<Validator<T>>&& validator_a,
                std::unique_ptr<Validator<T>>&& validator_b) :
        m_validator_a{std::move(validator_a)},
        m_validator_b{std::move(validator_b)}
    {}

    OrValidator(OrValidator&& rhs) noexcept = default;
    ~OrValidator() override = default;

    boost::any Validate(const std::string& str) const override {
        boost::any result;

        try {
            result = m_validator_a->Validate(str);
        } catch (const boost::bad_lexical_cast&) {
            result = m_validator_b->Validate(str);
        }

        return result;
    }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() const override {
        if (!m_validator_a || !m_validator_b)
            return nullptr;

        std::unique_ptr<ValidatorBase> up_base_a = m_validator_a->Clone();
        std::unique_ptr<ValidatorBase> up_base_b = m_validator_b->Clone();

        Validator<T>* val_a = static_cast<Validator<T>*>(up_base_a.release());
        Validator<T>* val_b = static_cast<Validator<T>*>(up_base_b.release());

        return std::make_unique<OrValidator<T>>(std::unique_ptr<Validator<T>>{val_a},
                                                std::unique_ptr<Validator<T>>{val_b});
    }

    std::unique_ptr<Validator<T>> m_validator_a;
    std::unique_ptr<Validator<T>> m_validator_b;
    static_assert(std::is_nothrow_move_constructible<std::unique_ptr<Validator<T>>>::value);
};


#endif
