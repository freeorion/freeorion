// -*- C++ -*-
#ifndef _Predicates_h_
#define _Predicates_h_

#ifndef _Universe_h_
#include "Universe.h"
#endif

#ifndef _System_h_
#include "System.h"
#endif

#ifndef _Planet_h_
#include "Planet.h"
#endif

#ifndef _Fleet_h_
#include "Fleet.h"
#endif


/** returns true iff \a obj is a System object. */
inline bool IsSystem(const UniverseObject* obj) {return dynamic_cast<const System*>(obj);}

/** returns true iff \a obj is a Planet object. */
inline bool IsPlanet(const UniverseObject* obj) {return dynamic_cast<const Planet*>(obj);}

/** returns true iff \a obj is a Fleet object. */
inline bool IsFleet(const UniverseObject* obj) {return dynamic_cast<const Fleet*>(obj);}

#endif // _Predicates_h_
