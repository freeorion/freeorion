#ifndef _AIClientApp_h_
#define _AIClientApp_h_

#include "../ClientApp.h"

#include <memory>
#include <vector>


class PythonAI;

/** the application framework for an AI player FreeOrion client.*/
class AIClientApp final : public ClientApp {
public:
    AIClientApp() = delete;
    explicit AIClientApp(const std::vector<std::string>& args);
    AIClientApp(const AIClientApp&) = delete;
    AIClientApp(AIClientApp&&) = delete;
    ~AIClientApp() override;

    const AIClientApp& operator=(const AIClientApp&) = delete;
    AIClientApp& operator=(const AIClientApp&&) = delete;

    //! Executes main event handler
    void Run();
    void ExitApp(int code = 0); ///< does basic clean-up, then calls exit(); callable from anywhere in user code via GetApp()
    void SetPlayerName(std::string player_name) noexcept { m_player_name = std::move(player_name); }

    [[nodiscard]] [[noreturn]] int SelectedSystemID() const override { throw std::runtime_error{"AI client cannot access selected object ID"}; }
    [[nodiscard]] [[noreturn]] int SelectedPlanetID() const override { throw std::runtime_error{"AI client cannot access selected object ID"}; }
    [[nodiscard]] [[noreturn]] int SelectedFleetID() const override { throw std::runtime_error{"AI client cannot access selected object ID"}; }
    [[nodiscard]] [[noreturn]] int SelectedShipID() const override { throw std::runtime_error{"AI client cannot access selected object ID"}; }
    [[nodiscard]] int              EffectsProcessingThreads() const override;

    /** @brief Return the player name of this client
     *
     * @return An UTF-8 encoded and NUL terminated string containing the player
     *      name of this client.
     */
    [[nodiscard]] const auto& PlayerName() const noexcept { return m_player_name; }

    static AIClientApp* GetApp() noexcept { return static_cast<AIClientApp*>(s_app); }
    const PythonAI*     GetAI() noexcept { return m_AI.get(); }

private:
    void ConnectToServer();
    void InitializePythonAI();
    void StartPythonAI();
    void HandlePythonAICrash();
    void HandleMessage(const Message& msg);


    /** Implementation of AI logic. */
    std::unique_ptr<PythonAI> m_AI;

    std::string m_player_name;
    int         m_max_aggression = 0;
};


#endif
