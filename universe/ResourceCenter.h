#ifndef _ResourceCenter_h_
#define _ResourceCenter_h_


#include <boost/signals2/signal.hpp>
#include "EnumsFwd.h"
#include "ConstantsFwd.h"
#include "../util/Export.h"


class Empire;
class Meter;
class UniverseObject;
class ObjectMap;
struct ScriptingContext;

/** The ResourceCenter class is an abstract base class for anything in the
  * FreeOrion gamestate that generates resources (minerals, etc.).  Most
  * likely, such an object will also be a subclass of UniverseObject.
  *
  * Planet is the most obvious class to inherit ResourceCenter, but other
  * classes could be made from it as well (e.g., a trade-ship or mining vessel,
  * or a non-Planet UniverseObject- and PopCenter- derived object of some
  * sort. */
class FO_COMMON_API ResourceCenter : virtual public std::enable_shared_from_this<UniverseObject> {
public:
    ResourceCenter();
    ResourceCenter(const ResourceCenter& rhs);
    virtual ~ResourceCenter();

    const auto&                      Focus() const noexcept { return m_focus; }
    int                              TurnsSinceFocusChange(int current_turn) const;
    virtual std::vector<std::string> AvailableFoci(const ScriptingContext&) const { return {}; }
    virtual bool                     FocusAvailable(std::string_view, const ScriptingContext&) const { return false; }
    virtual const std::string&       FocusIcon(std::string_view focus_name, const ScriptingContext& context) const;
    std::string                      Dump(uint8_t ntabs = 0) const;

    virtual const Meter*             GetMeter(MeterType type) const = 0;             ///< implementation should return the requested Meter, or 0 if no such Meter of that type is found in this object

    /** the state changed signal object for this ResourceCenter */
    mutable boost::signals2::signal<void ()> ResourceCenterChangedSignal;

    void Copy(const ResourceCenter& copied_object, Visibility vis);
    void Copy(const ResourceCenter& copied_object);

    void SetFocus(const std::string& focus, const ScriptingContext& context);
    void ClearFocus(int current_turn);
    void UpdateFocusHistory();

    virtual Meter* GetMeter(MeterType type) = 0;    ///< implementation should return the requested Meter, or 0 if no such Meter of that type is found in this object

    /** Resets the meters, etc. This should be called when a ResourceCenter is
        wiped out due to starvation, etc. */
    virtual void Reset(ObjectMap&);

protected:
    void Init();    ///< initialization that needs to be called by derived class after derived class is constructed

    void ResourceCenterResetTargetMaxUnpairedMeters();
    void ResourceCenterClampMeters();

private:
    std::string m_focus;
    int         m_last_turn_focus_changed = INVALID_GAME_TURN;
    std::string m_focus_turn_initial;
    int         m_last_turn_focus_changed_turn_initial = INVALID_GAME_TURN;

    virtual void AddMeter(MeterType meter_type) = 0; ///< implementation should add a meter to the object so that it can be accessed with the GetMeter() functions

    template <typename Archive>
    friend void serialize(Archive&, ResourceCenter&, unsigned int const);
};


#endif
