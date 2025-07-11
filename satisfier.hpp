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
    struct Formula;

    // Core C API: creation, deletion, binary AND, OR, NOT, and value access
    SATISFIER_API struct Formula* internal_new_formula(bool initial_value, const char* name);
    SATISFIER_API void          internal_delete_formula(struct Formula* instance);
    SATISFIER_API struct Formula* internal_and(const struct Formula* left,
                                             const struct Formula* right);
    SATISFIER_API struct Formula* internal_or (const struct Formula* left,
                                             const struct Formula* right);
    SATISFIER_API struct Formula* internal_not(const struct Formula* left,
                                             const struct Formula* right);
    SATISFIER_API bool          internal_formula_value(const struct Formula* instance);
}

#include <utility>

namespace satisfy {

/// C++ RAII wrapper enabling two operand formula chaining
class Formula {
    private:
    ::Formula* ptr_;
    explicit Formula(::Formula* p) : ptr_(p) {}

    public:
    // construct from bool
    Formula(bool value, const char* name)
      : ptr_( internal_new_formula(value, name)) {}

    // copy
     Formula(const Formula& o)
      : ptr_( internal_new_formula(o.value(), nullptr) ) {}

    // move
    Formula(Formula&& o) noexcept : ptr_(o.ptr_) { o.ptr_ = nullptr; }

    // destructor
    ~Formula() {
        if (ptr_) internal_delete_formula(ptr_);
    }

    // assignment by copy and swap
    Formula& operator=(Formula o) noexcept {
        std::swap(ptr_, o.ptr_);
        return *this;
    }

    // two operand AND: prints warning if left is false, then computes both
    Formula And(const Formula& rhs) const {
        return Formula( internal_and(ptr_, rhs.ptr_), nullptr);
    }

    // two operand OR: conventional OR over both operands
    Formula Or(const Formula& rhs) const {
        return Formula( internal_or(ptr_, rhs.ptr_), nullptr);
    }

    // unary NOT: uses C API
    Formula Not(const Formula& rhs) const {
        return Formula( internal_not(ptr_, rhs.ptr_), nullptr);
    }

    // extract boolean value
    bool value() const {
        return internal_formula_value(ptr_);
    }
};

#define suppose_literal(var, val) satisfy::Formula var((val), #var)

} // namespace sat
#endif // __cplusplus

#endif // SATISFIER_HPP
