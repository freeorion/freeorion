// -*- C++ -*-
#ifndef _Enable_Temporary_From_This_h_
#define _Enable_Temporary_From_This_h_

#include "TemporaryPtr.h"
#include "../util/Logger.h"

template <class T>
class EnableTemporaryFromThis {
public:
    TemporaryPtr<T> TemporaryFromThis() {
        if (!m_ptr)
            Logger().errorStream() << "TemporaryPtr from this was null!";

        return m_ptr;
    }

    TemporaryPtr<const T> TemporaryFromThis() const {
        if (!m_ptr)
            Logger().errorStream() << "TemporaryPtr from this was null!";

        return m_ptr;
    }

private:
    template <class Y> friend class TemporaryPtr;
    mutable TemporaryPtr<T> m_ptr;
};


#endif // _Enable_Temporary_From_This_h_
