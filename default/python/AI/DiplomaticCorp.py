"""Handle diplomatic messages and response determination."""
import freeOrionAIInterface as fo
import random
from logging import debug

from aistate_interface import get_aistate
from character.character_module import Aggression
from character.character_strings_module import possible_greetings
from common.fo_typing import EmpireId
from common.option_tools import get_option_dict
from freeorion_tools.translation import UserStringList

gang_up_turn = int(get_option_dict().get("gang_up_turn", "99999"))


def check_gang_up():
    if fo.currentTurn() == gang_up_turn:
        for empire_id in fo.allEmpireIDs():
            player_id = fo.empirePlayerID(empire_id)
            if not fo.playerIsAI(player_id):
                fo.sendChatMessage(player_id, "Let's gang up to destroy this arrogant player!")
            elif empire_id > fo.empireID():
                offer = fo.diplomaticMessage(empire_id, fo.empireID(), fo.diplomaticMessageType.peaceProposal)
                debug(f"Sending diplomatic message to empire {empire_id} of type {offer.type}")
                fo.sendDiplomaticMessage(offer)


def get_diplomatic_status(empire_id: EmpireId) -> fo.diplomaticStatus:
    return fo.getDiplomaticStatus(fo.empireID(), empire_id)


def handle_pregame_chat(sender_player_id, message_txt):
    if fo.playerIsAI(sender_player_id):
        return
    possible_acknowledgments = UserStringList("AI_PREGAME_ACKNOWLEDGEMENTS__LIST")
    acknowledgement = random.choice(possible_acknowledgments)
    debug(
        "Acknowledging pregame chat with initial message (from %d choices): '%s'"
        % (len(possible_acknowledgments), acknowledgement)
    )
    fo.sendChatMessage(sender_player_id, acknowledgement)


class DiplomaticCorp:
    def __init__(self):
        self.be_chatty = True

    def handle_diplomatic_message(self, message):  # noqa: C901
        """Handle a diplomatic message update from the server,
        such as if another player declares war, accepts peace, or cancels a proposed peace treaty.
        :param message: message.recipient and message.sender are respective empire IDs
        :return:
        """
        debug(
            "Received diplomatic {} message from {} to {}.".format(
                message.type,
                fo.getEmpire(message.sender),
                "me" if message.recipient == fo.empireID() else fo.getEmpire(message.recipient),
            )
        )
        # TODO: remove the following early return once proper support for third party diplomatic history is added
        if message.recipient != fo.empireID():
            return
        aistate = get_aistate()
        if message.type == fo.diplomaticMessageType.alliesProposal:
            aistate.log_alliance_request(message.sender, message.recipient)
            proposal_sender_player = fo.empirePlayerID(message.sender)
            attitude = aistate.character.attitude_to_empire(message.sender, aistate.diplomatic_logs)
            possible_acknowledgments = []
            aggression = aistate.character.get_trait(Aggression)
            if fo.currentTurn() >= gang_up_turn and fo.playerIsAI(fo.empirePlayerID(message.sender)):
                accept_proposal = True
            elif aggression.key == fo.aggression.beginner:
                accept_proposal = True
            elif aggression.key == fo.aggression.turtle:
                accept_proposal = attitude > 0
            elif aggression.key == fo.aggression.cautious:
                accept_proposal = attitude > 5
            else:  # aggression typical or greater
                accept_proposal = False
            if aggression.key <= fo.aggression.typical:
                possible_acknowledgments = UserStringList("AI_ALLIANCE_PROPOSAL_ACKNOWLEDGEMENTS_MILD_LIST")
                if accept_proposal:
                    possible_replies = UserStringList("AI_ALLIANCE_PROPOSAL_RESPONSES_YES_MILD_LIST")
                else:
                    possible_replies = UserStringList("AI_ALLIANCE_PROPOSAL_RESPONSES_NO_MILD_LIST")
            else:
                possible_acknowledgments = UserStringList("AI_ALLIANCE_PROPOSAL_ACKNOWLEDGEMENTS_HARSH_LIST")
                if accept_proposal:
                    possible_replies = UserStringList("AI_ALLIANCE_PROPOSAL_RESPONSES_YES_HARSH_LIST")
                else:
                    possible_replies = UserStringList("AI_ALLIANCE_PROPOSAL_RESPONSES_NO_HARSH_LIST")
            acknowledgement = random.choice(possible_acknowledgments)
            reply_text = random.choice(possible_replies)
            debug(
                "Acknowledging proposal with initial message (from %d choices): '%s'"
                % (len(possible_acknowledgments), acknowledgement)
            )
            fo.sendChatMessage(proposal_sender_player, acknowledgement)
            if accept_proposal:
                diplo_reply = fo.diplomaticMessage(
                    message.recipient, message.sender, fo.diplomaticMessageType.acceptAlliesProposal
                )
                debug(f"Sending diplomatic message to empire {message.sender} of type {diplo_reply.type}")
                fo.sendDiplomaticMessage(diplo_reply)
            debug(
                "sending chat to player %d of empire %d, message body: '%s'"
                % (proposal_sender_player, message.sender, reply_text)
            )
            fo.sendChatMessage(proposal_sender_player, reply_text)
        if message.type == fo.diplomaticMessageType.peaceProposal:
            aistate.log_peace_request(message.sender, message.recipient)
            proposal_sender_player = fo.empirePlayerID(message.sender)
            attitude = aistate.character.attitude_to_empire(message.sender, aistate.diplomatic_logs)
            possible_acknowledgments = []
            aggression = aistate.character.get_trait(Aggression)
            if aggression.key <= fo.aggression.typical:
                possible_acknowledgments = UserStringList("AI_PEACE_PROPOSAL_ACKNOWLEDGEMENTS_MILD_LIST")
                if attitude > 0:
                    possible_replies = UserStringList("AI_PEACE_PROPOSAL_RESPONSES_YES_MILD_LIST")
                else:
                    possible_replies = UserStringList("AI_PEACE_PROPOSAL_RESPONSES_NO_MILD_LIST")
            else:
                possible_acknowledgments = UserStringList("AI_PEACE_PROPOSAL_ACKNOWLEDGEMENTS_HARSH_LIST")
                if attitude > 0:
                    possible_replies = UserStringList("AI_PEACE_PROPOSAL_RESPONSES_YES_HARSH_LIST")
                else:
                    possible_replies = UserStringList("AI_PEACE_PROPOSAL_RESPONSES_NO_HARSH_LIST")
            acknowledgement = random.choice(possible_acknowledgments)
            reply_text = random.choice(possible_replies)
            debug(
                "Acknowledging proposal with initial message (from %d choices): '%s'"
                % (len(possible_acknowledgments), acknowledgement)
            )
            fo.sendChatMessage(proposal_sender_player, acknowledgement)
            if fo.currentTurn() >= gang_up_turn and fo.playerIsAI(fo.empirePlayerID(message.sender)):
                reply = fo.diplomaticMessage(
                    message.recipient, message.sender, fo.diplomaticMessageType.acceptPeaceProposal
                )
                fo.sendDiplomaticMessage(reply)
                allies = fo.diplomaticMessage(
                    message.recipient, message.sender, fo.diplomaticMessageType.alliesProposal
                )
                fo.sendDiplomaticMessage(allies)
            elif attitude > 0:
                diplo_reply = fo.diplomaticMessage(
                    message.recipient, message.sender, fo.diplomaticMessageType.acceptPeaceProposal
                )
                debug(f"Sending diplomatic message to empire {message.sender} of type {diplo_reply.type}")
                fo.sendDiplomaticMessage(diplo_reply)
            debug(
                "sending chat to player %d of empire %d, message body: '%s'"
                % (proposal_sender_player, message.sender, reply_text)
            )
            fo.sendChatMessage(proposal_sender_player, reply_text)
        elif message.type == fo.diplomaticMessageType.warDeclaration:
            # note: apparently this is currently (normally?) sent not as a warDeclaration,
            # but as a simple diplomatic_status_update to war
            aistate.log_war_declaration(message.sender, message.recipient)

    @staticmethod
    def get_first_turn_greet_message():
        greets = possible_greetings(get_aistate().character)
        # no such entry
        if len(greets) == 1 and greets[0] == "?":
            greets = UserStringList("AI_FIRST_TURN_GREETING_BEGINNER")
        return random.choice(greets)

    def handle_diplomatic_status_update(self, status_update: fo.diplomaticStatusUpdate):
        """Handle an update about the diplomatic status between players, which may
        or may not include this player."""
        debug(
            f"Received diplomatic status update to {status_update.status} about empire {status_update.empire1} and empire {status_update.empire2}"
        )
        if status_update.empire2 == fo.empireID() and status_update.status == fo.diplomaticStatus.war:
            get_aistate().log_war_declaration(status_update.empire1, status_update.empire2)

    def handle_midgame_chat(self, sender_player_id, message_txt):
        debug("Midgame chat received from player %d, message: %s" % (sender_player_id, message_txt))
        if fo.playerIsAI(sender_player_id) or not self.be_chatty:
            return
        if "BE QUIET" in message_txt.upper():
            possible_acknowledgments = UserStringList("AI_BE_QUIET_ACKNOWLEDGEMENTS__LIST")
            acknowledgement = random.choice(possible_acknowledgments)
            debug(
                "Acknowledging 'Be Quiet' chat request with initial message (from %d choices): '%s'"
                % (len(possible_acknowledgments), acknowledgement)
            )
            fo.sendChatMessage(sender_player_id, acknowledgement)
            self.be_chatty = False
            return
        if random.random() > 0.25:
            return
        possible_acknowledgments = UserStringList("AI_MIDGAME_ACKNOWLEDGEMENTS__LIST")
        acknowledgement = random.choice(possible_acknowledgments)
        debug(
            "Acknowledging midgame chat with initial message (from %d choices): '%s'"
            % (len(possible_acknowledgments), acknowledgement)
        )
        fo.sendChatMessage(sender_player_id, acknowledgement)
        self.be_chatty = False


class BeginnerDiplomaticCorp(DiplomaticCorp):
    pass


class ManiacalDiplomaticCorp(DiplomaticCorp):
    def __init__(self):
        DiplomaticCorp.__init__(self)
        self.be_chatty = False
