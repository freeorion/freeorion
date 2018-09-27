# Handlers allow to add debug features to AI without modifying existing code.

## Attach handler

  - Create python file
  - Add call of `register_pre_handler` or `register_post_handler` on file import
  - Add handler to config file (section: `main`, key: `handler` space separated paths to python files)
    path can be absolute, single_name(in same folder as config file) or relative form AI folder. Backslashes should be escaped.
  - run game with param `--ai-config <path to config file>`

  If game freezes on start check log for error.


## Existing handlers:
  - `python\AI\freeorion_tools\charts_handler.py`:
    Debug prints required for charts. Started by default
  - `python\handlers\inspect_freeOrionAIInterface.py`:
    Code that create stub for `freeOrionAIInterface`. Must be launched with single AI player.
  - `python\handlers\inspect_universe_generation.py` Code that create stub for `freeorion.pyi`

## Implementation
    Only AI handlers implementation is present now (`python\AI\freeorion_tools\handlers.py`),
    need to add universe generation and events implementation too.
