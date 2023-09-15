#include "GameRules.h"

namespace {
    std::vector<GameRulesFn>& GameRulesRegistry() {
        static std::vector<GameRulesFn> game_rules_registry;
        return game_rules_registry;
    }

    GameRules game_rules{};
}


/////////////////////////////////////////////
// Free Functions
/////////////////////////////////////////////
bool RegisterGameRules(GameRulesFn function) {
    GameRulesRegistry().push_back(function);
    return true;
}

GameRules& GetGameRules() {
    if (!GameRulesRegistry().empty()) {
        DebugLogger() << "Adding options rules";
        for (GameRulesFn fn : GameRulesRegistry())
            fn(game_rules);
        GameRulesRegistry().clear();
    }

    return game_rules;
}

namespace {
    constexpr auto category_ranks = GameRuleCategories::GameRuleCategoryValues();

    constexpr int8_t GetCategoryRank(std::string_view category) {
        if (category.empty())
            return static_cast<int8_t>(GameRuleCategories::GameRuleCategory::GENERAL);

        const auto it = std::find_if(category_ranks.begin(), category_ranks.end(),
                                     [category](auto entry) { return entry.second == category; });
        return static_cast<int8_t>(
            (it == category_ranks.end() ? GameRuleCategories::GameRuleCategory::UNDEFINED : it->first));
    }
}

/////////////////////////////////////////////////////
// GameRule
/////////////////////////////////////////////////////
GameRule::GameRule(Type type_, std::string name_, boost::any value_,
                   boost::any default_value_, std::string description_,
                   std::unique_ptr<ValidatorBase>&& validator_, bool engine_internal_, uint32_t rank_,
                   std::string category_) :
    OptionsDB::Option(static_cast<char>(0), std::move(name_), std::move(value_),
                      std::move(default_value_), std::move(description_),
                      std::move(validator_), engine_internal_, false, true, "setup.rules"),
    type(type_),
    category(std::move(category_)),
    rank(rank_)
{}


/////////////////////////////////////////////////////
// GameRules
/////////////////////////////////////////////////////
bool GameRules::Empty() {
    CheckPendingGameRules();
    return m_game_rules.empty();
}

std::unordered_map<std::string, GameRule>::const_iterator GameRules::begin() {
    CheckPendingGameRules();
    return m_game_rules.begin();
}

std::unordered_map<std::string, GameRule>::const_iterator GameRules::end() {
    CheckPendingGameRules();
    return m_game_rules.end();
}

bool GameRules::RuleExists(const std::string& name) {
    CheckPendingGameRules();
    return m_game_rules.contains(name);
}

bool GameRules::RuleExists(const std::string& name, GameRule::Type type) {
    if (type == GameRule::Type::INVALID)
        return false;
    CheckPendingGameRules();
    auto rule_it = m_game_rules.find(name);
    if (rule_it == m_game_rules.end())
        return false;
    return rule_it->second.type == type;
}

GameRule::Type GameRules::GetType(const std::string& name) {
    CheckPendingGameRules();
    auto rule_it = m_game_rules.find(name);
    if (rule_it == m_game_rules.end())
        return GameRule::Type::INVALID;
    return rule_it->second.type;
}

bool GameRules::RuleIsInternal(const std::string& name) {
    CheckPendingGameRules();
    auto rule_it = m_game_rules.find(name);
    if (rule_it == m_game_rules.end())
        return false;
    return rule_it->second.IsInternal();
}

const std::string& GameRules::GetDescription(const std::string& rule_name) {
    CheckPendingGameRules();
    auto it = m_game_rules.find(rule_name);
    if (it == m_game_rules.end())
        throw std::runtime_error(("GameRules::GetDescription(): No option called \"" + rule_name + "\" could be found.").c_str());
    return it->second.description;
}

const ValidatorBase* GameRules::GetValidator(const std::string& rule_name) {
    CheckPendingGameRules();
    auto it = m_game_rules.find(rule_name);
    if (it == m_game_rules.end())
        throw std::runtime_error(("GameRules::GetValidator(): No option called \"" + rule_name + "\" could be found.").c_str());
    return it->second.validator.get();
}

void GameRules::ClearExternalRules() {
    CheckPendingGameRules();
    auto it = m_game_rules.begin();
    while (it != m_game_rules.end()) {
        bool engine_internal = it->second.storable; // OptionsDB::Option member used to store if this option is engine-internal
        if (!engine_internal)
            it = m_game_rules.erase(it);
        else
            ++it;
    }
}

void GameRules::ResetToDefaults() {
    CheckPendingGameRules();
    for (auto& it : m_game_rules)
        it.second.SetToDefault();
}

std::map<std::string, std::string> GameRules::GetRulesAsStrings() {
    CheckPendingGameRules();
    std::map<std::string, std::string> retval;
    for (auto& [rule_name, rule_value] : m_game_rules)
        retval.emplace(rule_name, rule_value.ValueToString());
    return retval;
}

std::vector<const GameRule*> GameRules::GetSortedByCategoryAndRank() {
    CheckPendingGameRules();
    std::vector<const GameRule*> sorted_rules;
    sorted_rules.reserve(m_game_rules.size());
    std::transform(m_game_rules.begin(), m_game_rules.end(), std::back_inserter(sorted_rules),
                   [](const auto& rule_pair) { return &rule_pair.second; });
    std::sort(sorted_rules.begin(), sorted_rules.end(),
              [](const GameRule* lhs, const GameRule* rhs) {
                auto lhs_category_rank = GetCategoryRank(lhs->category);
                auto rhs_category_rank = GetCategoryRank(rhs->category);
                return lhs_category_rank == rhs_category_rank ?
                    lhs->rank < rhs->rank :
                    lhs_category_rank < rhs_category_rank;
    });
    return sorted_rules;
}
void GameRules::Add(Pending::Pending<GameRulesTypeMap>&& future)
{ m_pending_rules = std::move(future); }

namespace {
    /** Sets default validator, ensures option and sets value to value of the option. */
    template <typename T>
    void CheckValidatorAndAddRuleOption(GameRule& rule)
    {
        if (!rule.validator)
            rule.validator = std::make_unique<Validator<T>>();
        auto option_name = "setup.rules." + rule.name;
        if (!GetOptionsDB().OptionExists(option_name))
            GetOptionsDB().Add<T>(option_name, rule.description,
                                  boost::any_cast<T>(rule.default_value),
                                  rule.validator->Clone());
        rule.value = GetOptionsDB().Get<T>(option_name);
    }
}

void GameRules::Add(GameRule&& rule) {
    auto name{rule.name};

    auto it = m_game_rules.find(name);
    if (it != m_game_rules.end())
        throw std::runtime_error("GameRules::Add<>() : GameRule " + name + " was added twice.");

    if (!GetOptionsDB().OptionExists("setup.rules.server-locked." + name))
        GetOptionsDB().Add<bool>("setup.rules.server-locked." + name, rule.description, false);

    switch (rule.type) {
    case GameRule::Type::TOGGLE:    CheckValidatorAndAddRuleOption<bool>(rule);         break;
    case GameRule::Type::INT:       CheckValidatorAndAddRuleOption<int>(rule);          break;
    case GameRule::Type::DOUBLE:    CheckValidatorAndAddRuleOption<double>(rule);       break;
    case GameRule::Type::STRING:    CheckValidatorAndAddRuleOption<std::string>(rule);  break;
    default: {
        ErrorLogger() << "GameRules::Add(GameRule&&) unknown rule type. Skipping.";
        return;
    }
    }
    m_game_rules.insert_or_assign(std::move(name), std::move(rule));
}

void GameRules::SetFromStrings(const std::map<std::string, std::string>& names_values) {
    CheckPendingGameRules();
    DebugLogger() << "Setting Rules from Strings:";
    for (auto& [name, value] : names_values)
        DebugLogger() << "  " << name << " : " << value;

    ResetToDefaults();
    for (auto& [name, value] : names_values) {
        auto rule_it = m_game_rules.find(name);
        if (rule_it == m_game_rules.end()) {
            InfoLogger() << "GameRules::serialize received unrecognized rule: " << name;
            continue;
        }
        try {
            rule_it->second.SetFromString(value);
        } catch (const boost::bad_lexical_cast&) {
            ErrorLogger() << "Unable to set rule: " << name << " to value: " << value
                          << " - couldn't cast string to allowed value for this option";
        } catch (...) {
            ErrorLogger() << "Unable to set rule: " << name << " to value: " << value;
        }
    }

    DebugLogger() << "After Setting Rules:";
    for (auto& [name, value] : m_game_rules)
        DebugLogger() << "  " << name << " : " << value.ValueToString();
}

void GameRules::CheckPendingGameRules() {
    if (!m_pending_rules)
        return;

    auto parsed_new_rules = Pending::WaitForPending(m_pending_rules);
    if (!parsed_new_rules)
        return;

    for (auto& [name, game_rule] : *parsed_new_rules) {
        if (m_game_rules.contains(name)) {
            ErrorLogger() << "GameRules::Add<>() : GameRule " << name << " was added twice. Skipping ...";
            continue;
        }
        if (!game_rule.validator) {
            ErrorLogger() << "GameRules::Add<>() : GameRule " << name << " has no validator. Skipping ...";
            continue;
        }
        Add(std::move(game_rule));
    }

    DebugLogger() << "Registered and Parsed Game Rules:";
    for (auto& [name, value] : GetRulesAsStrings())
        DebugLogger() << " ... " << name << " : " << value;
}
