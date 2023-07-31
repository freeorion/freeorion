#ifndef _OptionValidators_h_
#define _OptionValidators_h_

#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>

#include <cmath>
#include <memory>
#include <set>
#include <string>
#include <vector>


/** Interface base class for all OptionsDB validators. Simply provides the basic interface. */
struct ValidatorBase {
    virtual ~ValidatorBase() = default;

    /** returns normally if \a str is a valid value, or throws otherwise */
    virtual boost::any Validate(const std::string& str) const = 0;
    virtual boost::any Validate(std::string_view str) const = 0;

    /** returns the string representation of \a value */
    [[nodiscard]] virtual std::string String(const boost::any& value) const = 0;

    /** returns a dynamically allocated copy of the object. */
    [[nodiscard]] virtual std::unique_ptr<ValidatorBase> Clone() const & = 0;

    /** returns a dynamically allocated copy of the object. */
    [[nodiscard]] virtual std::unique_ptr<ValidatorBase> Clone() && = 0;
};

FO_COMMON_API std::string ListToString(std::vector<std::string> input_list);
FO_COMMON_API std::vector<std::string> StringToList(std::string_view input_string);
FO_COMMON_API std::vector<std::string> StringToList(const char* input_string);
FO_COMMON_API std::vector<std::string> StringToList(const std::string& input_string);

/** determines if a string is a valid value for an OptionsDB option */
template <typename T>
struct Validator : public ValidatorBase
{
    boost::any Validate(const std::string& str) const override {
        if constexpr (std::is_same_v<T, std::vector<std::string>>)
            return boost::any(StringToList(str));
        else if constexpr (std::is_same_v<T, std::string>)
            return boost::any(std::string{str});
        else
            return boost::any(boost::lexical_cast<T>(str));
    }

    boost::any Validate(std::string_view str) const override {
        if constexpr (std::is_same_v<T, std::vector<std::string>>)
            return boost::any(StringToList(str));
        else if constexpr (std::is_same_v<T, std::string>)
            return boost::any(std::string{str});
        else
            return boost::any(boost::lexical_cast<T>(str));
    }

    [[nodiscard]] std::string String(const boost::any& value) const override {
        if constexpr (std::is_same_v<T, std::string>) {
            if (value.type() == typeid(std::string))
                return boost::any_cast<std::string>(value);
            else if (value.type() == typeid(const char*))
                return std::string{boost::any_cast<const char*>(value)};
            else if (value.type() == typeid(std::string_view))
                return std::string{boost::any_cast<std::string_view>(value)};

        } else if constexpr (std::is_enum_v<T>) {
            if (value.type() == typeid(T))
                return std::string{to_string(boost::any_cast<T>(value))};

        } else if constexpr (std::is_arithmetic_v<T>) {
            if (value.type() == typeid(T))
                return std::to_string(boost::any_cast<T>(value));

        } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
            if (value.type() == typeid(T))
                return ListToString(boost::any_cast<std::vector<std::string>>(value));

        } else {
            if (value.type() == typeid(T))
                return boost::lexical_cast<std::string>(boost::any_cast<T>(value));
        }
        return "";
    }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() const & override
    { return std::make_unique<Validator<T>>(); }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() && override
    { return std::make_unique<Validator<T>>(); }
};

/** a Validator that constrains the range of valid values */
template <typename T>
struct RangedValidator final : public Validator<T>
{
    RangedValidator(T min, T max) :
        m_min(min),
        m_max(max)
    {
        if (max < min)
            throw std::invalid_argument("RangedValidator given max < min");
    }
    RangedValidator(RangedValidator&& rhs) noexcept = default;

    boost::any Validate(const std::string& str) const override {
        T val = boost::lexical_cast<T>(str);
        if (val < m_min || val > m_max)
            throw boost::bad_lexical_cast();
        return boost::any(val);
    }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() const & override
    { return std::make_unique<RangedValidator>(m_min, m_max); }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() && override
    { return std::make_unique<RangedValidator>(std::move(*this)); }

    const T m_min;
    const T m_max;
    static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>);
};

/** a Validator that constrains valid values to certain step-values
    (eg: 0, 25, 50, ...).  The steps are assumed to begin at the
    validated type's default-constructed value, unless another origin
    is specified. */
template <typename T>
struct StepValidator final : public Validator<T>
{
    StepValidator(T step, T origin = 0) :
        m_step_size(step),
        m_origin(origin)
    {
        if (m_step_size <= 0)
            throw std::invalid_argument("StepValidator constructed with step <= 0");
    }
    StepValidator(StepValidator&& rhs) noexcept = default;

    boost::any Validate(const std::string& str) const override {
        const T val = boost::lexical_cast<T>(str);
        const T diff = val - m_origin;
        if constexpr (std::is_integral_v<T>) {
            if (diff % m_step_size != T(0))
                throw boost::bad_lexical_cast();

        } else {
            static constexpr T epsilon = std::numeric_limits<T>::epsilon();
            if (std::abs(std::fmod(diff, m_step_size)) > epsilon)
                throw boost::bad_lexical_cast();
        }
        return boost::any(val);
    }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() const & override
    { return std::make_unique<StepValidator>(m_step_size, m_origin); }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() && override
    { return std::make_unique<StepValidator>(std::move(*this)); }

    const T m_step_size;
    const T m_origin;
    static_assert(std::is_arithmetic_v<T>);
};

/** a Validator similar to a StepValidator, but that further constrains the valid values to be within a certain range (eg: [25, 50, ..., 200]). */
template <typename T>
struct RangedStepValidator final : public Validator<T>
{
public:
    RangedStepValidator(T step, T min, T max) :
        m_step_size(step),
        m_origin(T()),
        m_min(min),
        m_max(max)
    {
        if (m_step_size <= 0)
            throw std::invalid_argument("RangedStepValidator constructed with step <= 0");
        if (max < min)
            throw std::invalid_argument("RangedStepValidator given max < min");
    }

    RangedStepValidator(T step, T origin, T min, T max) :
        m_step_size (step),
        m_origin(origin),
        m_min(min),
        m_max(max)
    {
        if (m_step_size <= 0)
            throw std::invalid_argument("RangedStepValidator constructed with step <= 0");
        if (max < min)
            throw std::invalid_argument("RangedStepValidator given max < min");
    }

    RangedStepValidator(RangedStepValidator&& rhs) noexcept = default;

    boost::any Validate(const std::string& str) const override {
        const T val = boost::lexical_cast<T>(str);
        if ((val < m_min) || (val > m_max))
            throw boost::bad_lexical_cast();

        const T diff = val - m_origin;

        if constexpr (std::is_integral_v<T>) {
            if (diff % m_step_size != T(0))
                throw boost::bad_lexical_cast();

        } else {
            static constexpr T epsilon = std::numeric_limits<T>::epsilon();
            const T remainder = std::fmod(diff, m_step_size);
            if ((std::abs(remainder) > epsilon) &&
                (std::abs(m_step_size - remainder) > epsilon))
            { throw boost::bad_lexical_cast(); }
        }

        return boost::any(val);
    }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() const & override
    { return std::make_unique<RangedStepValidator>(m_step_size, m_origin, m_min, m_max); }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() && override
    { return std::make_unique<RangedStepValidator>(std::move(*this)); }

    const T m_step_size;
    const T m_origin;
    const T m_min;
    const T m_max;
    static_assert(std::is_arithmetic_v<T>);
};

/// a Validator that specifies a finite number of valid values.
/** Probably won't work well with floating point types. */
template <typename T>
struct DiscreteValidator final : public Validator<T>
{
    explicit DiscreteValidator(T single_value) :
        m_values{std::move(single_value)}
    {}

    explicit DiscreteValidator(std::vector<T> values) :
        m_values(std::move(values))
    {}

    explicit DiscreteValidator(const std::vector<std::string_view>& values) :
        m_values(values.begin(), values.end())
    {}

    explicit DiscreteValidator(const std::vector<const char*>& values) :
        m_values(values.begin(), values.end())
    {}

    template <std::size_t N>
    explicit DiscreteValidator(const std::array<const char*, N>& values) :
        m_values(values.begin(), values.end())
    {}

    boost::any Validate(const std::string& str) const override {
        if constexpr (std::is_same_v<std::string, T>) {
            if (std::any_of(m_values.begin(), m_values.end(), [&str](const auto& v) { return str == v; }))
                return boost::any(str);
        } else {
            T val = boost::lexical_cast<T>(str);
            if (std::any_of(m_values.begin(), m_values.end(), [val](auto v) { return val == v; }))
                return boost::any(val);
        }
        throw boost::bad_lexical_cast();
    }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() const & override
    { return std::make_unique<DiscreteValidator>(m_values); }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() && override
    { return std::make_unique<DiscreteValidator>(std::move(m_values)); }

    /// Stores the list of vaild values.
    std::vector<T> m_values;
    static_assert(std::is_arithmetic_v<T> || std::is_same_v<std::string, T>);
};

/// a Validator that performs a logical OR of two validators.
/** Stores and owns clones of the provided validators in std::unique_ptr.
 *  Always calls m_validator_a->Validate(). Only calls m_validator_b->Validate()
 *  if the first one throws. */
template <typename T>
struct OrValidator final : public Validator<T>
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

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() const & override {
        if (!m_validator_a || !m_validator_b)
            return nullptr;

        std::unique_ptr<Validator<T>> val_a{static_cast<Validator<T>*>(m_validator_a->Clone().release())};
        std::unique_ptr<Validator<T>> val_b{static_cast<Validator<T>*>(m_validator_b->Clone().release())};

        return std::make_unique<OrValidator<T>>(std::move(val_a), std::move(val_b));
    }

    [[nodiscard]] std::unique_ptr<ValidatorBase> Clone() && override
    { return std::make_unique<OrValidator<T>>(std::move(*this)); }

    std::unique_ptr<Validator<T>> m_validator_a;
    std::unique_ptr<Validator<T>> m_validator_b;
    static_assert(std::is_nothrow_move_constructible_v<std::unique_ptr<Validator<T>>>);
};


#endif
