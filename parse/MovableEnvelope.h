#ifndef _MovableEnvelope_h_
#define _MovableEnvelope_h_

#include <memory>
#include <type_traits>

namespace parse { namespace detail {
    /** \p MovableEnvelope enables the boost::spirit parser to handle a
        \p T with move semantics.

        boost::spirit is designed only work with value semantics. The
        boost::phoenix actors only handle value semantics.  Types with move
        only semantics like unique_ptr will not work as expected.

        boost::spirit is designed to work with compile time polymorphism.  With
        run-time polymorphism using raw pointers to track heap objects, each
        time the parser backtracks it leaks memory.

        \p MovableEnvelope makes a \p unique_ptr<T> with move only semantics
        appear to have copy semantics, by moving it each time that it is copied.
        This allows boost::phoenix actors to handle it correctly if the movement is
        one way from creation at the parse location to consumption in a larger
        parsed component.

        \p MovableEnvelope is a work around.  It can be removed if
        boost::spirit supports move semantics, or the parser is changed to use
        compile time polymorphism.
    */
    template <typename T>
    class MovableEnvelope {
    public:

        /** \name Rule of Five constructors and operators.
            MovableEnvelope satisfies the rule of five with the following
            constructors and operator=().
         */ //@{

        // Default constructor
        MovableEnvelope() {}

        // Copy constructor
        // This leaves \p other in an emptied state
        MovableEnvelope(const MovableEnvelope& other) :
            MovableEnvelope(std::move(other.obj))
        {}

        // Move constructor
        MovableEnvelope(MovableEnvelope&& other) :
            MovableEnvelope(std::move(other.obj))
        {}

        // Move operator
        MovableEnvelope& operator= (MovableEnvelope&& other) {
            obj = std::move(other.obj);
            original_obj = other.original_obj;
            // Intentionally leave other.original_obj != other.obj.get()
            return *this;
        }

        // Copy operator
        // This leaves \p other in an emptied state
        MovableEnvelope& operator= (const MovableEnvelope& other) {
            obj = std::move(other.obj);
            original_obj = other.original_obj;
            // Intentionally leave other.original_obj != other.obj.get()
            return *this;
        }
        //@}

        virtual ~MovableEnvelope() {};

        /** \name Converting constructors and operators.
            MovableEnvelope allows conversion between compatible types with the following
            constructors and operators.
         */ //@{

        // nullptr constructor
        MovableEnvelope(std::nullptr_t) {}

        template <typename U,
                  typename std::enable_if<std::is_convertible<std::unique_ptr<U>,
                                                              std::unique_ptr<T>>::value>::type* = nullptr>
        explicit MovableEnvelope(std::unique_ptr<U>&& obj_) :
            obj(std::move(obj_)),
            original_obj(obj.get())
        {}

        // This takes ownership of obj_
        template <typename U,
                  typename std::enable_if<std::is_convertible<std::unique_ptr<U>,
                                                              std::unique_ptr<T>>::value>::type* = nullptr>
        explicit MovableEnvelope(U* obj_) :
            obj(obj_),
            original_obj(obj.get())
        {}

        // Converting copy constructor
        // This leaves \p other in an emptied state
        template <typename U,
                  typename std::enable_if<std::is_convertible<std::unique_ptr<U>,
                                                              std::unique_ptr<T>>::value>::type* = nullptr>
        MovableEnvelope(const MovableEnvelope<U>& other) :
            MovableEnvelope(std::move(other.obj))
        {}

        // Converting move constructor
        template <typename U,
                  typename std::enable_if<std::is_convertible<std::unique_ptr<U>,
                                                              std::unique_ptr<T>>::value>::type* = nullptr>
        MovableEnvelope(MovableEnvelope<U>&& other) :
            MovableEnvelope(std::move(other.obj))
        {}

        template <typename U,
                  typename std::enable_if<std::is_convertible<std::unique_ptr<U>,
                                                              std::unique_ptr<T>>::value>::type* = nullptr>
        MovableEnvelope& operator= (MovableEnvelope<U>&& other) {
            obj = std::move(other.obj);
            original_obj = other.original_obj;
            // Intentionally leave other.original_obj != other.obj.get()
            return *this;
        }

        // Copy operator
        // This leaves \p other in an emptied state
        template <typename U,
                  typename std::enable_if<std::is_convertible<std::unique_ptr<U>,
                                                              std::unique_ptr<T>>::value>::type* = nullptr>
        MovableEnvelope& operator= (const MovableEnvelope<U>& other) {
            obj = std::move(other.obj);
            original_obj = other.original_obj;
            // Intentionally leave other.original_obj != other.obj.get()
            return *this;
        }
        //@}

        // Return false if the original \p obj has been moved away from this
        // MovableEnvelope.
        bool IsEmptiedEnvelope() const
        { return (original_obj != obj.get());}

        /** OpenEnvelope returns the enclosed \p obj and throws
            std::runtime_error if \p safety does not equal the wrapped \p obj,
            which indicates that the wrapped pointer was move out of this \p
            MovableEnvelope.

            \p OpenEnvelope is a one-shot.  Calling OpenEnvelope a second time
            will throw, since the obj has already been removed.
        */

        std::unique_ptr<T> OpenEnvelope() const {
            if (IsEmptiedEnvelope())
                throw std::runtime_error(
                    "The parser attempted to extract the unique_ptr from a MovableEnvelope more than once.  \n"
                    "Until boost::spirit supports move semantics MovableEnvelope requires that unique_ptr be used once");
            return std::move(obj);
        }

    private:
        template <typename U>
        friend class MovableEnvelope;

        mutable std::unique_ptr<T> obj = nullptr;

        mutable T* original_obj = nullptr;
    };

    /** \p construct_movable is a functor that constructs a MovableEnvleope<T> */
    struct construct_movable {
        template <typename T>
        using result_type = MovableEnvelope<T>;

        template <typename T>
        result_type<T> operator() (T* obj) const
        { return MovableEnvelope<T>(obj); }

        template <typename T>
        result_type<T> operator() (std::unique_ptr<T>&& obj) const
        { return MovableEnvelope<T>(std::move(obj)); }

        template <typename T>
        result_type<T> operator() (std::unique_ptr<T>& obj) const
        { return MovableEnvelope<T>(std::move(obj)); }

        template <typename T>
        result_type<T> operator() (MovableEnvelope<T>&& obj) const
        { return MovableEnvelope<T>(std::move(obj)); }

        template <typename T>
        result_type<T> operator() (const MovableEnvelope<T>& obj) const
        { return MovableEnvelope<T>(obj); }
    };

    /** Free functions converting containers of MovableEnvlope to unique_ptrs. */
    template <typename T>
    std::vector<std::unique_ptr<T>> OpenEnvelopes(const std::vector<MovableEnvelope<T>>& envelopes) {
        std::vector<std::unique_ptr<T>> retval;
        for (auto&& envelope : envelopes)
            retval.push_back(envelope.OpenEnvelope());
        return retval;
    }

    /** \p deconstruct_movable is a functor that extracts the unique_ptr from a
        MovableEnvleope<T>.  This is a one time operation that empties the
        MovableEnvelop<T>.  It is typically done while calling the constructor
        from outside of boost::spirit that expects a unique_ptr<T>*/
    struct deconstruct_movable {
        template <typename T>
        std::unique_ptr<T> operator() (const MovableEnvelope<T>& obj) const
        { return obj.OpenEnvelope(); }

        template <typename T>
        std::vector<std::unique_ptr<T>> operator() (const std::vector<MovableEnvelope<T>>& objs) const
        { return OpenEnvelopes(objs); }
    };

    } // end namespace detail
} // end namespace parse

#endif // _MovableEnvelope_h_
