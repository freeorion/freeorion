#ifndef _EffectPythonParser_h_
#define _EffectPythonParser_h_

#include <memory>

#include "../universe/Effect.h"
#include "../universe/UnlockableItem.h"

namespace boost::python {
    class dict;
}

struct effect_wrapper {
    effect_wrapper(std::shared_ptr<Effect::Effect>&& ref)
        : effect(std::move(ref))
    { }

    effect_wrapper(const std::shared_ptr<Effect::Effect>& ref)
        : effect(ref)
    { }

    const std::shared_ptr<const Effect::Effect> effect;
};

struct effect_group_wrapper {
    effect_group_wrapper(std::shared_ptr<Effect::EffectsGroup>&& ref)
        : effects_group(std::move(ref))
    { }

    effect_group_wrapper(const std::shared_ptr<Effect::EffectsGroup>& ref)
        : effects_group(ref)
    { }

    const std::shared_ptr<const Effect::EffectsGroup> effects_group;
};

struct unlockable_item_wrapper {
    unlockable_item_wrapper(UnlockableItem&& item_)
        : item(std::move(item_))
    { }

    unlockable_item_wrapper(const UnlockableItem& item_)
        : item(item_)
    { }

    const UnlockableItem item;
};

void RegisterGlobalsEffects(boost::python::dict& globals);

#endif

