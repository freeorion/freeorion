#ifndef _GameRules_h_
#define _GameRules_h_

#include "OptionsDB.h"
#include "Pending.h"

class GameRules;

/////////////////////////////////////////////
// Free Functions
/////////////////////////////////////////////
using GameRulesFn = void (*) (GameRules&); ///< the function signature for functions that add Rules to the GameRules (void (GameRules&))

/** adds \a function to a vector of pointers to functions that add Rules to
  * the GameRules.  This function returns a boolean so that it can be used to
  * declare a dummy static variable that causes \a function to be registered as
  * a side effect (e.g. at file scope:
  * "bool unused_bool = RegisterGameRules(&foo)"). */
FO_COMMON_API bool RegisterGameRules(GameRulesFn function);

/** returns the single instance of the GameRules class */
FO_COMMON_API GameRules& GetGameRules();

/** Database of values that control how the game mechanics function. */
class FO_COMMON_API GameRules {
public:
    enum class Type : int {
        INVALID = -1,
        TOGGLE,
        INT,
        DOUBLE,
        STRING
    };

    [[nodiscard]] static constexpr Type RuleTypeForType(bool)
    { return Type::TOGGLE; }
    [[nodiscard]] static constexpr Type RuleTypeForType(int)
    { return Type::INT; }
    [[nodiscard]] static constexpr Type RuleTypeForType(double)
    { return Type::DOUBLE; }
    [[nodiscard]] static inline Type RuleTypeForType(std::string)
    { return Type::STRING; }

    struct FO_COMMON_API GameRule : public OptionsDB::Option {
        GameRule() = default;
        GameRule(Type type_, std::string name_, boost::any value_,
                 boost::any default_value_, std::string description_,
                 std::unique_ptr<ValidatorBase>&& validator_, bool engine_internal_,
                 std::string category_ = std::string());
        [[nodiscard]] bool IsInternal() const { return this->storable; }

        Type type = Type::INVALID;
        std::string category;
    };

    using GameRulesTypeMap = std::unordered_map<std::string, GameRule>;

    [[nodiscard]] bool Empty() const;
    [[nodiscard]] std::unordered_map<std::string, GameRule>::const_iterator begin() const;
    [[nodiscard]] std::unordered_map<std::string, GameRule>::const_iterator end() const;
    [[nodiscard]] std::unordered_map<std::string, GameRule>::iterator begin();
    [[nodiscard]] std::unordered_map<std::string, GameRule>::iterator end();

    [[nodiscard]] bool RuleExists(const std::string& name) const;
    [[nodiscard]] bool RuleExists(const std::string& name, Type type) const;
    [[nodiscard]] Type GetType(const std::string& name) const;
    [[nodiscard]] bool RuleIsInternal(const std::string& name) const;

    /** returns the description string for rule \a rule_name, or throws
      * std::runtime_error if no such rule exists. */
    [[nodiscard]] const std::string& GetDescription(const std::string& rule_name) const;

    /** returns the validator for rule \a rule_name, or throws
      * std::runtime_error if no such rule exists. */
    [[nodiscard]] const ValidatorBase* GetValidator(const std::string& rule_name) const;

    /** returns all contained rules as map of name and value string. */
    [[nodiscard]] std::map<std::string, std::string> GetRulesAsStrings() const;

    template <typename T>
    [[nodiscard]] T Get(const std::string& name) const
    {
        CheckPendingGameRules();
        auto it = m_game_rules.find(name);
        if (it == m_game_rules.end())
            throw std::runtime_error("GameRules::Get<>() : Attempted to get nonexistent rule \"" + name + "\".");
        try {
            return boost::any_cast<T>(it->second.value);
        } catch (const boost::bad_any_cast&) {
            ErrorLogger() << "bad any cast converting value of game rule named: " << name << ". Returning default value instead";
            try {
                return boost::any_cast<T>(it->second.default_value);
            } catch (const boost::bad_any_cast&) {
                ErrorLogger() << "bad any cast converting default value of game rule named: " << name << ". Returning data-type default value instead: " << T();
                return T();
            }
        }
    }

    /** Adds a rule, optionally with a custom validator.
        Adds option setup.rules.{RULE_NAME} to override default value and
        option setup.rules.server-locked.{RULE_NAME} to block rule changes from players */
    template <typename T>
    void Add(std::string name, std::string description, std::string category, T default_value,
             bool engine_internal, std::unique_ptr<ValidatorBase> validator = nullptr)
    {
        CheckPendingGameRules();

        if (!validator)
            validator = std::make_unique<Validator<T>>();

        auto it = m_game_rules.find(name);
        if (it != m_game_rules.end())
            throw std::runtime_error("GameRules::Add<>() : GameRule " + name + " was added twice.");

        if (!GetOptionsDB().OptionExists("setup.rules.server-locked." + name))
            GetOptionsDB().Add<bool>("setup.rules.server-locked." + name, description, false);

        if (!GetOptionsDB().OptionExists("setup.rules." + name))
            GetOptionsDB().Add<T>("setup.rules." + name, description, std::move(default_value),
                                  validator->Clone());

        T value = GetOptionsDB().Get<T>("setup.rules." + name);

        DebugLogger() << "Added game rule named " << name << " with default value " << value;

        GameRule&& rule{RuleTypeForType(T()), name, value, value, std::move(description),
                        std::move(validator), engine_internal, std::move(category)};
        m_game_rules.emplace(std::move(name), std::move(rule));
    }

    template <typename T>
    void Add(std::string name, std::string description, std::string category, T default_value,
             bool engine_internal, Validator<T>&& validator) // validator should be wrapped in unique_ptr
    {
        Add(std::move(name), std::move(description), std::move(category), std::move(default_value),
            engine_internal, std::make_unique<Validator<T>>(std::move(validator)));
    }

    /** Adds rules from the \p future. */
    void Add(Pending::Pending<GameRules>&& future);

    template <typename T>
    void Set(const std::string& name, T value)
    {
        CheckPendingGameRules();
        auto it = m_game_rules.find(name);
        if (it == m_game_rules.end())
            throw std::runtime_error("GameRules::Set<>() : Attempted to set nonexistent rule \"" + name + "\".");
        it->second.SetFromValue(std::move(value));
    }

    void SetFromStrings(const std::map<std::string, std::string>& names_values);

    /** Removes game rules that were added without being specified as
        engine internal. */
    void ClearExternalRules();

    /** Resets all rules to default values. */
    void ResetToDefaults();

private:
    /** Assigns any m_pending_rules to m_game_rules. */
    void CheckPendingGameRules() const;

    /** Future rules being parsed by parser.  mutable so that it can
        be assigned to m_game_rules when completed.*/
    mutable boost::optional<Pending::Pending<GameRules>> m_pending_rules = boost::none;

    mutable GameRulesTypeMap m_game_rules;

    friend FO_COMMON_API GameRules& GetGameRules();
};


#endif
