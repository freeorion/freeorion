from analytics.jar import CallInfo


def base_reported(call_info: CallInfo) -> str:
  res = "OK" if call_info.result else "Fail"
  return f'[{call_info.name}]: {call_info.args}: {res}'
