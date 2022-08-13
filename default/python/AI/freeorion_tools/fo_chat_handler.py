import freeOrionAIInterface as fo

from common.option_tools import get_option_dict
from DiplomaticCorp import DiplomaticCorp, handle_pregame_chat
from freeorion_tools.chat_handler import DebugChatHandler, NormalChatHandler

_chat_handler = None  # Optional[_ChatHandler]


def configure_debug_chat(empire_id: int) -> None:
    global _chat_handler
    if get_option_dict().get("allow_debug_chat", False):
        _chat_handler = DebugChatHandler(empire_id)
    else:
        _chat_handler = NormalChatHandler()


def process_chat_message(sender_id: None, message_text: str, diplomatic_corp: DiplomaticCorp):
    # We got our first massages before we initialize AI
    if _chat_handler is None:
        handle_pregame_chat(sender_id, message_text)
        return
    handled = _chat_handler.process_message(sender_id, message_text)
    if not handled and not (fo.playerIsHost(sender_id) and _chat_handler.debug_active):
        diplomatic_corp.handle_midgame_chat(sender_id, message_text)
