#ifndef _AIFramework_h_
#define _AIFramework_h_

#include "../../python/PythonBase.h"

#include <string>
#include <vector>

#ifdef FREEORION_MACOSX
// Bugfix for https://github.com/freeorion/freeorion/issues/1228

// The problem on OSX is that the boost/python/str.hpp redefines toupper() and
// similar functions if they are not already defined.

// This includes iostream before the boost/python/str.hpp to fix this issue.
// If the subsequent #include <boost/python/str.hpp> is removed then so can this workaround.
#include <iostream>
#endif

#include <boost/python/str.hpp>

class DiplomaticMessage;
struct DiplomaticStatusUpdateInfo;


/** @brief Class allowing AI to recieve basic game events.
 */
class PythonAI final : public PythonBase {
public:
    bool Initialize();

    void Start();

    /** Initializes AI Python imports. */
    bool InitImports() override;
    /** Initializes AI Python modules. */
    bool InitModules() override;

    /** @brief Call when the server has sent a new turn update.
     *
     * The AI subclass should review the new gamestate and send orders for
     * this turn.
     */
    void GenerateOrders();

    /** @brief Called when another player sends a chat message to this AI.
     *
     * The AI subclass should respond or react to the message in a meaningful
     * way.
     *
     * @param sender_id The player identifier representing the player, who sent
     *      the message.
     * @param msg The text body of the sent message.
     */
    void HandleChatMessage(int sender_id, const std::string& msg);

    /** @brief Called when another player sends a diplomatic message that
     *      affects this player
     *
     * The AI subclass should respond or react to the message in a meaningful
     * way.
     *
     * @param msg The diplomatic message sent.
     */
    void HandleDiplomaticMessage(const DiplomaticMessage& msg);

    /** @brief Called when two empires diplomatic status changes
     *
     * The AI subclass should respond or react to the change in a meaningful
     * way.
     *
     * @param u The diplomatic status changed.
     */
    void HandleDiplomaticStatusUpdate(const DiplomaticStatusUpdateInfo& u);

    /** @brief Called when a new game (not loaded) is started
     *
     * The AI subclass should clear its state and prepare to start for a new
     * game.
     */
    void StartNewGame();

    /** @brief Called when a game is loaded from a save
     *
     * The AI subclass should extract any state information stored in
     * @a save_state so it is able to continue generating orders when
     * asked to do so.
     *
     * @param save_state The serialized state information from a previous game
     *      run.
     */
    void ResumeLoadedGame(const std::string& save_state_string);

    /** @brief Called when the server is saving the game
     *
     * The AI should store any state information it will need to resume at any
     * later time, and return this information.
     *
     * @return The serialized state information of the game running.
     */
    [[nodiscard]] const std::string& GetSaveStateString() const;

private:
    // reference to imported Python AI module
    boost::python::object m_python_module_ai;
};


#endif
