from __future__ import print_function

from functools import wraps

handlers = {
    # 'function_name': [
    #     ['pre handlers list'],
    #     ['post handlers list']
    # ]
}


def _register(function_name, handler, is_post_handler):
    handlers.setdefault(function_name, [[], []])[is_post_handler].append(handler)
    print('Register "%s" %s "%s" execution' % (
        handler.__name__,
        'after' if is_post_handler else 'before',
        function_name
    ))


def register_pre_handler(function_name, handler):
    """
    Register pre handler for function, handler must accept same number of arguments as function.
    Handler must not modify any arguments.
    """
    _register(function_name, handler, False)


def register_post_handler(function_name, handler):
    """
    Register post handler for function,
        handler must accept function result as first argument and all other function arguments after that.
    Handler must not modify any arguments.
    """
    _register(function_name, handler, True)


def listener(funct):
    @wraps(funct)
    def wrapper(*args, **kwargs):
        pre, post = handlers.get(funct.__name__, [[], []])
        [x(*args, **kwargs) for x in pre]
        res = funct(*args)
        [x(res, *args, **kwargs) for x in post]
        return res
    return wrapper
