#ifndef _MovableEnvelope_h_
#define _MovableEnvelope_h_

namespace parse {
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
        MovableEnvelope() : obj() {}

        explicit MovableEnvelope(std::unique_ptr<T>&& obj_) :
            obj(std::move(obj_)),
            original_obj(obj.get())
        {}

        // This take ownership of obj_
        explicit MovableEnvelope(T* obj_) :
            obj(obj_),
            original_obj(obj.get())
        {}

        // Copy constructor
        // This leaves \p other in an emptied state
        MovableEnvelope(const MovableEnvelope& other) :
            MovableEnvelope(std::move(other.obj))
        {}

        // Move constructor
        MovableEnvelope(MovableEnvelope&& other) :
            MovableEnvelope(std::move(other.obj))
        {}

        ~MovableEnvelope() {};

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
                throw std::runtime_error("A MovableEnvelope was opened after its obj was moved to another envelope.");
            return std::move(obj);
        }

    private:
        mutable std::unique_ptr<T> obj = nullptr;

        mutable T* original_obj = nullptr;
    };

    /** \p construct_movable is a function that constructs MovableEnvleopes<T> */
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
}

#endif // _MovableEnvelope_h_
