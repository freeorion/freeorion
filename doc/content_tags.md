Content Definition Tags  {#content_tags}
=======================

Content entries may have tags associated with them to help filter them
for later effects.

Tags are used to easily filter for entries, without relying on the specific
entries names.

These filters are applied in various areas, such as:

 * displaying only certain parts in the design window
 * not showing buildings unless other criteria are met
 * only affecting certain ships within a system.


Documenting tags
----------------

To document content tags you can use the custom `@content_tag` Doxygen
keyword:

```
\@content_tag{TAG_NAME} brief description
```

To document C++ defined tags use regular comments, however keep in
mind, that each documentation need to be unique within the symbol
scope to properly be registered Doxygen.

```cpp
// \@content_tag{TAG_ONE} Documentation for tag one
int SomeFunction(std::string tag = "TAG_ONE");

// \@content_tag{TAG_HARD_WORK} Documentation for tag hard work
const std::string TAG_HARD_WORK = "TAG_HARD_WORK";

// \@content_tag{TAG_LESS_WORK} Documentation for tag less work
const std::string TAG_LESS_WORK = "TAG_LESS_WORK";

void OneFunction(UniverseObject& obj) {
    if (obj.HasTag(TAG_HARD_WORK))
        DoHardWork(obj);
    else if (obj.HasTag(TAG_LESS_WORK))
        DoLessWork(obj);
}

void AnotherFunc(UniverseObject& obj) {
    // This will be overwritten by TAG_BAD_OVERWRITE
    // \@content_tag{TAG_BAD_LOST} Documentation will be overwritten
    if (obj.HasTag("TAG_BAD_LOST"))
        DoHardWork();
    // \@content_tag{TAG_BAD_OVERWRITE} Documentation will occur twice
    else if (obj.HasTag("TAG_BAD_OVERWRITE"))
        DoLessWork();
}
```

With Python, docstring comments do not support special commands, instead use the hash form #

```py
def AnotherFunc(some_object):
    # \@content_tag{TAG_DO_WORK} Documentaion for "do work" tag
    if universe.getObject(some_object).hasTag("TAG_DO_WORK"):
        do_work
```

For FOCS use the special c style form of triple slashes ///

```
EffectsGroup
    scope = And [
        Ship
        /// \@content_tag{CTRL_SAMPLE2} Sample 2
        Not HasTag name = "CTRL_SAMPLE2"
        WithinDistance distance = Source.Size condition = Source
    ]
    effects = ...
```


Content Definition Tag Listing
------------------------------

The following list of availble tags is auto generated, it may not be
visible outside of Doxygen generated content.
