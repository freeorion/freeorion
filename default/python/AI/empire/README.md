# Empire info module

This module contains code that responsible for holding useful information about empire objects.

## Implementation details. 

Files in this module wildly use encapsulation principle.

Data container itself is hidden from outer scope and access to it should be provided via functions.
This approach allows changing internal implementation and add new functionality 
without changing code which uses this module. 
Also, we could keуp this module as simple as required for current code.

Modules usually don't have any AI dependencies, so they could be easily used from any part of AI code.

Each module have fucntion to set data and functions to get data. This gives us a temporal coupling. 
Using any getter before all setters calls will lead to unexpected behaviour. Be aware of it. 

To achieve encapsulation the following is done:
The main data structure is wrapped to the `@cache_for_current_turn`. 
So basically you will get new value only once per turn. 
This approach require a few lines of code it is hard to forget to make a cleanup before each turn.
This might be a problem when writing unittests. Probably will need new decorator for this.
This function has private scope and should not be used outside that module.  
It returns mutable object, which is mutated by setters.

For example:
```python
@cache_for_current_turn
def _get_foos() -> list:
    return []

def set_foo(val):
    _get_foos().append(val)
```

Getters are public functions which return or pretend to return immutable result.

```python
def get_foo() -> typing.Sequence:
    return _get_foos()
```

Technically this example returns list, 
but we use annotation with supertype which is immutable. 
It won’t prevent collection modification in runtime, but will show warning in IDE.
Creating defensive copy might be complicated for some cases.














