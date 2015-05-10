"""Handle diplomatic messages and response determination."""

import random

import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
import AIstate
from freeorion_tools import UserString, UserStringList, chat_on_error, print_error, chat_human, ppstring

def handle_pregame_chat(sender_id, message_txt):
    possible_acknowledgments = UserStringList("AI_PREGAME_ACKNOWLEDGEMENTS__LIST")
    acknowledgement = random.choice(possible_acknowledgments)
    print ("Acknowledging pregame chat with initial message (from %d choices): '%s'" %
          (len(possible_acknowledgments), acknowledgement))
    fo.sendChatMessage(fo.empirePlayerID(sender_id), acknowledgement)

class DiplomaticCorp(object):

    @chat_on_error
    def handle_diplomatic_message(self, message):  # pylint: disable=invalid-name
        """Handle a diplomatic message update from the server,
        such as if another player declares war, accepts peace, or cancels a proposed peace treaty.
        :param message:
        :return:
        """
        print "Received diplomatic %s message from empire %s to empire %s" % (message.type, message.sender, message.recipient)
        print "my empire id: %s" % fo.empireID()
        if message.recipient != fo.empireID():
            return
        reply_sender = message.recipient
        reply_recipient = message.sender
        if message.type == fo.diplomaticMessageType.peaceProposal:
            foAI.foAIstate.log_peace_request(message.sender, message.recipient)
            proposal_sender_player = fo.empirePlayerID(message.sender)
            suffix = "MILD" if foAI.foAIstate.aggression <= fo.aggression.typical else "HARSH"
            possible_acknowledgments = UserStringList("AI_PEACE_PROPOSAL_ACKNOWLEDGEMENTS_" + suffix + "_LIST")
            acknowledgement = random.choice(possible_acknowledgments)
            print ("Acknowledging proposal with initial message (from %d choices): '%s'" %
                  (len(possible_acknowledgments), acknowledgement))
            fo.sendChatMessage(proposal_sender_player, acknowledgement)
            attitude = self.evaluate_diplomatic_attitude(message.sender)
            if attitude > 0:
                reply_text = random.choice(UserStringList("AI_PEACE_PROPOSAL_RESPONSES_YES_" + suffix + "_LIST"))
                diplo_reply = fo.diplomaticMessage(reply_sender, reply_recipient, fo.diplomaticMessageType.acceptProposal)
                print "Sending diplomatic message to empire %s of type %s" % (reply_recipient, diplo_reply.type)
                fo.sendDiplomaticMessage(diplo_reply)
            else:
                reply_text = random.choice(UserStringList("AI_PEACE_PROPOSAL_RESPONSES_NO_" + suffix + "_LIST"))
            print ("sending chat to player %d of empire %d, message body: '%s'" %
                  (proposal_sender_player, reply_recipient, reply_text))
            fo.sendChatMessage(proposal_sender_player, reply_text)
        elif message.type == fo.diplomaticMessageType.warDeclaration:
            # note: apparently this is currently (normally?) sent not as a warDeclaration,
            # but as a simple diplomatic_status_update to war
            foAI.foAIstate.log_war_declaration(message.sender, message.recipient)

    @chat_on_error
    def handle_diplomatic_status_update(self, status_update):  # pylint: disable=invalid-name
        """Handle an update about the diplomatic status between players, which may
        or may not include this player."""
        print ("Received diplomatic status update to %s about empire %s and empire %s" %
              (status_update.status, status_update.empire1, status_update.empire2))
        if (status_update.empire2 == fo.empireID() and status_update.status == fo.diplomaticStatus.war):
            foAI.foAIstate.log_war_declaration(status_update.empire1, status_update.empire2)

    @chat_on_error
    def evaluate_diplomatic_attitude(self, other_empire_id):
        """Evaluate this empire's diplomatic attitude regarding the other empire.
        :param other_empire_id: integer
        :return: a numeric rating, currently in the range [-10 : 10]
        """

        # TODO: consider proximity, competitive needs, relations with other empires, past history with this empire, etc.
        # in the meantime, somewhat random
        log_index = (other_empire_id, fo.empireID())
        num_peace_requests = len(foAI.foAIstate.diplomatic_logs.get('peace_requests', {}).get(log_index, []))
        num_war_declarations = len(foAI.foAIstate.diplomatic_logs.get('war_declarations', {}).get(log_index, []))
        # Too many requests for peace irritate the AI, as do any war declarations
        irritation = (foAI.foAIstate.aggression * (2.0 + num_peace_requests/10.0 + 2.0 * num_war_declarations) + 0.5)
        attitude = 10 * random.random() - irritation
        return min(10, max(-10, attitude))

    def handle_midgame_chat(self, sender_id, message_txt):
        possible_acknowledgments = UserStringList("AI_MIDGAME_ACKNOWLEDGEMENTS__LIST")
        acknowledgement = random.choice(possible_acknowledgments)
        print ("Acknowledging midgame chat with initial message (from %d choices): '%s'" %
              (len(possible_acknowledgments), acknowledgement))
        fo.sendChatMessage(fo.empirePlayerID(sender_id), acknowledgement)

class BeginnerDiplomaticCorp(DiplomaticCorp):
    def evaluate_diplomatic_attitude(self, other_empire_id):
        return 9

class ManiacalDiplomaticCorp(DiplomaticCorp):
    def evaluate_diplomatic_attitude(self, other_empire_id):
        return -9
