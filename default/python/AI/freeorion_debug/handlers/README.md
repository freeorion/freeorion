# Handlers allow to add debug features to AI with out modifying existing code.

## Attach handler

  - Create python file
  - Add call of `register_pre_handler` or `register_post_handler` on file import
  - Add handler to config file (section: `main`, key: `handler` space separated paths to python files)
    path can be absolute, single_name(in same folder as config file) or relative form AI folder. Backslashes should be escaped.
  - run game with param `--ai-config <path to config file>`

  If game freezes on stat check log for error.


## Existing handlers:
  - `freeorion_debug/handlers/charts_handler`:
    Debug prints required for charts. Started by default
  - `freeorion_debug/handlers/inspect_freeOrionAIInterface.py`:
    Code that create stub for `freeOrionAIInterface`. Must be launched with single AI player.
