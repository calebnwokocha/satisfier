#ifndef SATISFIER_HPP
#define SATISFIER_HPP

#include <stdbool.h>

#if defined(_WIN32) || defined(__CYGWIN__)
  #ifdef BUILD_SATISFIER
    #define SATISFIER_API __declspec(dllexport)
  #else
    #define SATISFIER_API __declspec(dllimport)
  #endif
#else
  #ifdef BUILD_SATISFIER
    #define SATISFIER_API __attribute__((visibility("default")))
  #else
    #define SATISFIER_API
  #endif
#endif

#ifdef __cplusplus
extern "C" {
    // Opaque forward declaration
    struct Logic;

    // Core C API: creation, deletion, binary AND/OR, unary NOT, and value access
    SATISFIER_API struct Logic* internal_new_logic(bool initial_value);
    SATISFIER_API void          internal_delete_logic(struct Logic* instance);
    SATISFIER_API struct Logic* internal_and(const struct Logic* left,
                                             const struct Logic* right);
    SATISFIER_API struct Logic* internal_or (const struct Logic* left,
                                             const struct Logic* right);
    SATISFIER_API struct Logic* internal_not(const struct Logic* left,
                                             const struct Logic* right);
    SATISFIER_API bool          internal_logic_value(const struct Logic* instance);
}

#include <utility>

namespace sat {

/// C++ RAII wrapper enabling two operand logic chaining
class Logic {
    private:
    ::Logic* ptr_;
    explicit Logic(::Logic* p) : ptr_(p) {}

    public:
    // construct from bool
    explicit Logic(bool v)
      : ptr_( internal_new_logic(v) ) {}

    // copy
    Logic(const Logic& o)
      : ptr_( internal_new_logic(o.value()) ) {}

    // move
    Logic(Logic&& o) noexcept : ptr_(o.ptr_) { o.ptr_ = nullptr; }

    // destructor
    ~Logic() {
        if (ptr_) internal_delete_logic(ptr_);
    }

    // assignment by copy and swap
    Logic& operator=(Logic o) noexcept {
        std::swap(ptr_, o.ptr_);
        return *this;
    }

    // two operand AND: prints warning if left is false, then computes both
    Logic And(const Logic& rhs) const {
        return Logic( internal_and(ptr_, rhs.ptr_) );
    }

    // two operand OR: conventional OR over both operands
    Logic Or(const Logic& rhs) const {
        return Logic( internal_or(ptr_, rhs.ptr_) );
    }

    // unary NOT: uses C API
    Logic Not(const Logic& rhs) const {
        return Logic( internal_not(ptr_, rhs.ptr_) );
    }

    // extract boolean value
    bool value() const {
        return internal_logic_value(ptr_);
    }
};

} // namespace sat
#endif // __cplusplus

#endif // SATISFIER_HPP
