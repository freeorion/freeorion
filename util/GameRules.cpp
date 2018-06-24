#include "GameRules.h"

namespace {
    std::vector<GameRulesFn>& GameRulesRegistry() {
        static std::vector<GameRulesFn> game_rules_registry;
        return game_rules_registry;
    }
}


/////////////////////////////////////////////
// Free Functions
/////////////////////////////////////////////
bool RegisterGameRules(GameRulesFn function) {
    GameRulesRegistry().push_back(function);
    return true;
}

GameRules& GetGameRules() {
    static GameRules game_rules;
    if (!GameRulesRegistry().empty()) {
        DebugLogger() << "Adding options rules";
        for (GameRulesFn fn : GameRulesRegistry())
            fn(game_rules);
        GameRulesRegistry().clear();
    }

    return game_rules;
}


/////////////////////////////////////////////////////
// GameRules
/////////////////////////////////////////////////////
GameRules::Rule::Rule() :
    OptionsDB::Option()
{}

GameRules::Rule::Rule(Type type_, const std::string& name_, const boost::any& value_,
                      const boost::any& default_value_, const std::string& description_,
                      const ValidatorBase *validator_, bool engine_internal_,
                      const std::string& category_) :
    OptionsDB::Option(static_cast<char>(0), name_, value_, default_value_,
                      description_, validator_, engine_internal_, false, true, "setup.rules"),
    type(type_),
    category(category_)
{}

GameRules::GameRules()
{}

bool GameRules::Empty() const {
    CheckPendingGameRules();
    return m_game_rules.empty();
}

std::unordered_map<std::string, GameRules::Rule>::const_iterator GameRules::begin() const {
    CheckPendingGameRules();
    return m_game_rules.begin();
}

std::unordered_map<std::string, GameRules::Rule>::const_iterator GameRules::end() const {
    CheckPendingGameRules();
    return m_game_rules.end();
}

bool GameRules::RuleExists(const std::string& name) const {
    CheckPendingGameRules();
    return m_game_rules.count(name);
}

bool GameRules::RuleExists(const std::string& name, Type type) const {
    if (type == Type::INVALID)
        return false;
    CheckPendingGameRules();
    auto rule_it = m_game_rules.find(name);
    if (rule_it == m_game_rules.end())
        return false;
    return rule_it->second.type == type;
}

GameRules::Type GameRules::GetType(const std::string& name) const {
    CheckPendingGameRules();
    auto rule_it = m_game_rules.find(name);
    if (rule_it == m_game_rules.end())
        return Type::INVALID;
    return rule_it->second.type;
}

bool GameRules::RuleIsInternal(const std::string& name) const {
    CheckPendingGameRules();
    auto rule_it = m_game_rules.find(name);
    if (rule_it == m_game_rules.end())
        return false;
    return rule_it->second.IsInternal();
}

const std::string& GameRules::GetDescription(const std::string& rule_name) const {
    CheckPendingGameRules();
    auto it = m_game_rules.find(rule_name);
    if (it == m_game_rules.end())
        throw std::runtime_error(("GameRules::GetDescription(): No option called \"" + rule_name + "\" could be found.").c_str());
    return it->second.description;
}

std::shared_ptr<const ValidatorBase> GameRules::GetValidator(const std::string& rule_name) const {
    CheckPendingGameRules();
    auto it = m_game_rules.find(rule_name);
    if (it == m_game_rules.end())
        throw std::runtime_error(("GameRules::GetValidator(): No option called \"" + rule_name + "\" could be found.").c_str());
    return it->second.validator;
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

std::vector<std::pair<std::string, std::string>> GameRules::GetRulesAsStrings() const {
    CheckPendingGameRules();
    std::vector<std::pair<std::string, std::string>> retval;
    for (const auto& rule : m_game_rules)
        retval.push_back({rule.first, rule.second.ValueToString()});
    return retval;
}

void GameRules::Add(Pending::Pending<GameRules>&& future)
{ m_pending_rules = std::move(future); }

void GameRules::SetFromStrings(const std::vector<std::pair<std::string, std::string>>& names_values) {
    CheckPendingGameRules();
    DebugLogger() << "Setting Rules from Strings:";
    for (const auto& entry : names_values)
        DebugLogger() << "  " << entry.first << " : " << entry.second;

    ResetToDefaults();
    for (auto& entry : names_values) {
        auto rule_it = m_game_rules.find(entry.first);
        if (rule_it == m_game_rules.end()) {
            InfoLogger() << "GameRules::serialize received unrecognized rule: " << entry.first;
            continue;
        }
        try {
            rule_it->second.SetFromString(entry.second);
        } catch (const boost::bad_lexical_cast& e) {
            ErrorLogger() << "Unable to set rule: " << entry.first << " to value: " << entry.second << " - couldn't cast string to allowed value for this option";
        } catch (...) {
            ErrorLogger() << "Unable to set rule: " << entry.first << " to value: " << entry.second;
        }
    }

    DebugLogger() << "After Setting Rules:";
    for (const auto& entry : m_game_rules)
        DebugLogger() << "  " << entry.first << " : " << entry.second.ValueToString();
}

void GameRules::CheckPendingGameRules() const {
    if (!m_pending_rules)
        return;

    auto parsed = Pending::WaitForPending(m_pending_rules);
    if (!parsed)
        return;

    auto new_rules = std::move(*parsed);
    for (const auto& rule : new_rules) {
        const auto& name = rule.first;
        if (m_game_rules.count(name)) {
            ErrorLogger() << "GameRules::Add<>() : Rule " << name << " was added twice. Skipping ...";
            continue;
        }
        m_game_rules[name] = rule.second;
    }

    DebugLogger() << "Registered and Parsed Game Rules:";
    for (const auto& entry : GetRulesAsStrings())
        DebugLogger() << " ... " << entry.first << " : " << entry.second;
}
