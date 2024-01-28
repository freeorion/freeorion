#ifndef _GameRules_h_
#define _GameRules_h_

#include "OptionsDB.h"
#include "Pending.h"
#include "GameRuleCategories.h"
#include "../universe/EnumsFwd.h"

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

struct FO_COMMON_API GameRule final : public OptionsDB::Option {
    enum class Type : int8_t {
        INVALID = -1,
        TOGGLE,
        INT,
        DOUBLE,
        STRING
    };

    [[nodiscard]] static constexpr Type RuleTypeForType(bool) noexcept { return Type::TOGGLE; }
    [[nodiscard]] static constexpr Type RuleTypeForType(int) noexcept { return Type::INT; }
    [[nodiscard]] static constexpr Type RuleTypeForType(Visibility) noexcept { return Type::INT; }
    [[nodiscard]] static constexpr Type RuleTypeForType(double) noexcept { return Type::DOUBLE; }
    [[nodiscard]] static inline Type RuleTypeForType(std::string) noexcept { return Type::STRING; }


    GameRule(Type type_, std::string name_, boost::any value_,
             boost::any default_value_, std::string description_,
             std::unique_ptr<ValidatorBase>&& validator_, bool engine_internal_, uint32_t rank_,
             std::string category_ = std::string());
    [[nodiscard]] bool IsInternal() const noexcept { return this->storable; }

    Type type = Type::INVALID;
    std::string category;
    uint32_t rank;
};

using GameRulesTypeMap = std::unordered_map<std::string, GameRule>;


/** Database of values that control how the game mechanics function. */
class FO_COMMON_API GameRules {
public:
    GameRules() = default;
    ~GameRules() = default;

    [[nodiscard]] bool Empty();
    [[nodiscard]] std::unordered_map<std::string, GameRule>::const_iterator begin();
    [[nodiscard]] std::unordered_map<std::string, GameRule>::const_iterator end();

    [[nodiscard]] bool RuleExists(const std::string& name);
    [[nodiscard]] bool RuleExists(const std::string& name, GameRule::Type type);
    [[nodiscard]] GameRule::Type GetType(const std::string& name);
    [[nodiscard]] bool RuleIsInternal(const std::string& name);

    /** returns the description string for rule \a rule_name, or throws
      * std::runtime_error if no such rule exists. */
    [[nodiscard]] const std::string& GetDescription(const std::string& rule_name);

    /** returns the validator for rule \a rule_name, or throws
      * std::runtime_error if no such rule exists. */
    [[nodiscard]] const ValidatorBase* GetValidator(const std::string& rule_name);

    /** returns all contained rules as map of name and value string. */
    [[nodiscard]] std::map<std::string, std::string> GetRulesAsStrings();

    /** returns collection of game rules sorted by their rank, chiefly for use in GUI ordering */
    [[nodiscard]] std::vector<const GameRule*> GetSortedByCategoryAndRank();

    template <typename T>
    [[nodiscard]] T Get(const std::string& name)
    {
        CheckPendingGameRules();

        TraceLogger() << "Requested rule named " << name << " of type " << typeid(T).name();


        auto it = m_game_rules.find(name);
        if (it == m_game_rules.end()) {
            ErrorLogger() << "GameRules::Get<>() : Attempted to get nonexistent rule \"" << name 
                          << "\". Returning data-type default value instead: " << T();
            return T();
        }

        if (it->second.value.type() != typeid(std::decay_t<T>)) {
            if constexpr (std::is_same_v<std::decay_t<T>, double>) {
                if (it->second.value.type() == typeid(int)) {
                    DebugLogger() << "GameRules::Get<>() : Requested value of type " << typeid(T).name()
                                  << " from rule of type " << it->second.value.type().name()
                                  << " ... getting as int instead";
                    try {
                        return boost::any_cast<int>(it->second.value);
                    } catch (const boost::bad_any_cast&) {
                        ErrorLogger() << "Getting as int failed";
                    }
                }
            }
            DebugLogger() << "GameRules::Get<>() : Requested value of type " << typeid(T).name()
                          << " from rule of type " << it->second.value.type().name()
                          << ". Returning data-type default value instead: " << T();
            return T();
        }

        try {
            return boost::any_cast<T>(it->second.value);
        } catch (const boost::bad_any_cast&) {
            ErrorLogger() << "GameRules::Get<>() : bad any cast getting value of game rule named: " << name
                          << " as type << " << typeid(T).name() << ". Returning default value of rule instead";
            try {
                return boost::any_cast<T>(it->second.default_value);
            } catch (const boost::bad_any_cast&) {
                ErrorLogger() << "GameRules::Get<>() : bad any cast getting default value of game rule named: "
                              << name << " that contains a value of type: " << it->second.value.type().name()
                              << ". Returning " << typeid(T).name() << " default value instead: " << T();
                return T();
            }
        }
    }

    /** Adds a rule, optionally with a custom validator.
        Adds option setup.rules.{RULE_NAME} to override default value and
        option setup.rules.server-locked.{RULE_NAME} to block rule changes from players */
    template <typename T>
    void Add(std::string name, std::string description, std::string category, T default_value,
             bool engine_internal, uint32_t rank, std::unique_ptr<ValidatorBase> validator = nullptr)
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

        GameRule rule{GameRule::RuleTypeForType(T()), name, value, value, std::move(description),
                      std::move(validator), engine_internal, rank, std::move(category)};
        m_game_rules.insert_or_assign(std::move(name), std::move(rule));
    }

    template <typename T, typename V> requires (std::is_convertible_v<V, Validator<T>>)
    void Add(std::string name, std::string description, std::string category, T default_value,
             bool engine_internal, uint32_t rank, V&& validator)
    {
        Add(std::move(name), std::move(description), std::move(category), std::move(default_value),
            engine_internal, rank, std::make_unique<V>(std::move(validator)));
    }

    template <typename T, typename V> requires (std::is_convertible_v<V, Validator<T>>)
        void Add(std::string name, std::string description, GameRuleCategories::GameRuleCategory category, T default_value,
            bool engine_internal, uint32_t rank, V&& validator)
    {
        Add(std::move(name), std::move(description), category, std::move(default_value),
            engine_internal, rank, std::make_unique<V>(std::move(validator)));
    }

    /** Adds a rule, optionally with a custom validator.
       Adds option setup.rules.{RULE_NAME} to override default value and
       option setup.rules.server-locked.{RULE_NAME} to block rule changes from players */
    template <typename T>
    void Add(std::string name, std::string description, GameRuleCategories::GameRuleCategory category, T default_value,
        bool engine_internal, uint32_t rank, std::unique_ptr<ValidatorBase> validator = nullptr)
    {
        Add(std::move(name), std::move(description), 
            category == GameRuleCategories::GameRuleCategory::GENERAL ? "" : std::string(to_string(category)),
            std::move(default_value), engine_internal, rank, std::move(validator));
    }

    /** Adds rules from the \p future. */
    void Add(Pending::Pending<GameRulesTypeMap>&& future);

    template <typename T>
    void Set(const std::string& name, T&& value)
    {
        CheckPendingGameRules();
        auto it = m_game_rules.find(name);
        if (it == m_game_rules.end())
            throw std::runtime_error("GameRules::Set<>() : Attempted to set nonexistent rule \"" + name + "\".");
        it->second.SetFromValue(std::forward<T>(value));
    }

    void SetFromStrings(const std::map<std::string, std::string>& names_values);

    /** Removes game rules that were added without being specified as
        engine internal. */
    void ClearExternalRules();

    /** Resets all rules to default values. */
    void ResetToDefaults();

private:
    void Add(GameRule&& rule);

    /** Assigns any m_pending_rules to m_game_rules. */
    void CheckPendingGameRules();

    /** Future rules being parsed by parser.  mutable so that it can
        be assigned to m_game_rules when completed.*/
    boost::optional<Pending::Pending<GameRulesTypeMap>> m_pending_rules = boost::none;

    GameRulesTypeMap m_game_rules;

    friend FO_COMMON_API GameRules& GetGameRules();
};


#endif
