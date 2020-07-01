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

    static inline constexpr Type RuleTypeForType(bool dummy)
    { return Type::TOGGLE; }
    static inline constexpr Type RuleTypeForType(int dummy)
    { return Type::INT; }
    static inline constexpr Type RuleTypeForType(double dummy)
    { return Type::DOUBLE; }
    static inline Type RuleTypeForType(std::string dummy)
    { return Type::STRING; }

    struct FO_COMMON_API Rule : public OptionsDB::Option {
        Rule();
        Rule(Type type_, const std::string& name_, const boost::any& value_,
             const boost::any& default_value_, const std::string& description_,
             const ValidatorBase *validator_, bool engine_internal_,
             const std::string& category_ = "");
        bool IsInternal() const { return this->storable; }

        Type type = Type::INVALID;
        std::string category = "";
    };

    using GameRulesTypeMap = std::unordered_map<std::string, Rule>;

    GameRules();

    /** \name Accessors */ //@{
    bool Empty() const;
    std::unordered_map<std::string, Rule>::const_iterator begin() const;
    std::unordered_map<std::string, Rule>::const_iterator end() const;

    bool RuleExists(const std::string& name) const;
    bool RuleExists(const std::string& name, Type type) const;
    Type GetType(const std::string& name) const;
    bool RuleIsInternal(const std::string& name) const;

    /** returns the description string for rule \a rule_name, or throws
      * std::runtime_error if no such rule exists. */
    const std::string& GetDescription(const std::string& rule_name) const;

    /** returns the validator for rule \a rule_name, or throws
      * std::runtime_error if no such rule exists. */
    std::shared_ptr<const ValidatorBase> GetValidator(const std::string& rule_name) const;

    /** returns all contained rules as map of name and value string. */
    std::map<std::string, std::string> GetRulesAsStrings() const;

    template <typename T>
    T Get(const std::string& name) const
    {
        CheckPendingGameRules();
        auto it = m_game_rules.find(name);
        if (it == m_game_rules.end())
            throw std::runtime_error("GameRules::Get<>() : Attempted to get nonexistent rule \"" + name + "\".");
        try {
            return boost::any_cast<T>(it->second.value);
        } catch (const boost::bad_any_cast& e) {
            ErrorLogger() << "bad any cast converting value of game rule named: " << name << ". Returning default value instead";
            try {
                return boost::any_cast<T>(it->second.default_value);
            } catch (const boost::bad_any_cast&) {
                ErrorLogger() << "bad any cast converting default value of game rule named: " << name << ". Returning data-type default value instead: " << T();
                return T();
            }
        }
    }
    //@}

    /** Adds a rule, optionally with a custom validator.
        Adds option setup.rules.{RULE_NAME} to override default value and
        option setup.rules.server-locked.{RULE_NAME} to block rule changes from players */
    template <typename T>
    void Add(const std::string& name, const std::string& description,
             const std::string& category, T default_value,
             bool engine_interal, const ValidatorBase& validator = Validator<T>())
    {
        CheckPendingGameRules();
        auto it = m_game_rules.find(name);
        if (it != m_game_rules.end())
            throw std::runtime_error("GameRules::Add<>() : Rule " + name + " was added twice.");
        if (!GetOptionsDB().OptionExists("setup.rules.server-locked." + name)) {
            GetOptionsDB().Add<bool>("setup.rules.server-locked." + name, description, false);
        }
        if (!GetOptionsDB().OptionExists("setup.rules." + name)) {
            GetOptionsDB().Add<T>("setup.rules." + name, description, default_value,
                                  validator);
        }
        T value = GetOptionsDB().Get<T>("setup.rules." + name);
        m_game_rules[name] = Rule(RuleTypeForType(T()), name, value, value, description,
                                  validator.Clone(), engine_interal, category);
        DebugLogger() << "Added game rule named " << name << " with default value " << value;
    }

    /** Adds rules from the \p future. */
    void Add(Pending::Pending<GameRules>&& future);

    template <typename T>
    void Set(const std::string& name, const T& value)
    {
        CheckPendingGameRules();
        auto it = m_game_rules.find(name);
        if (it == m_game_rules.end())
            throw std::runtime_error("GameRules::Set<>() : Attempted to set nonexistent rule \"" + name + "\".");
        it->second.SetFromValue(value);
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

#endif // _GameRules_h_
