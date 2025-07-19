#ifndef SATISFIER_HPP
  #define SATISFIER_HPP

  #include <stdbool.h>
  #include <utility>

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

        // Core C API: creation, deletion, AND, OR, NOT, and value access
        SATISFIER_API Formula* internal_new_formula(bool initial_value, const char* name);
        SATISFIER_API void          internal_delete_formula(Formula* instance);
        SATISFIER_API Formula* internal_and(Formula* left, Formula* right);
        SATISFIER_API Formula* internal_or (Formula* left, Formula* right);
        SATISFIER_API Formula* internal_not(Formula* operand);
        SATISFIER_API bool          internal_formula_value(const Formula* instance);
    }

    namespace Satisfy {

      /// C++ RAII wrapper enabling two operand formula chaining
      class Formula {
          private:
            ::Formula* ptr_;
            explicit Formula(::Formula* p) noexcept : ptr_(p) {}

          public:
            // construct from bool
            Formula(bool value, const char* name)
              : ptr_( internal_new_formula(value, name)) {}

            Formula(const Formula&) = delete;
            Formula& operator=(const Formula&) = delete;

            Formula(Formula&& o) noexcept : ptr_(o.ptr_) { o.ptr_ = nullptr; }
            Formula& operator=(Formula&& o) noexcept {
                std::swap(ptr_, o.ptr_);
                return *this;
            }

            // destructor
            ~Formula() {
                if (ptr_) internal_delete_formula(ptr_);
            }

            // two operand AND: prints warning if left is false, then computes both
            Formula And(const Formula& rhs) const {
                return Formula( internal_and(ptr_, rhs.ptr_));
            }

            // two operand OR: conventional OR over both operands
            Formula Or(const Formula& rhs) const {
                return Formula( internal_or(ptr_, rhs.ptr_));
            }

             // allow Not() to access private ptr_
            friend Formula Not(const Formula& operand);

            // extract boolean value
            bool Value() const {
                return internal_formula_value(ptr_);
            }
        };

        // unary NOT: free function (not chainable)
        inline Formula Not(const Formula& operand) {
            Formula temp = Formula( internal_not(operand.ptr_) );
            return temp;
        }

        #define Suppose_literal(var, val) Satisfy::Formula var((val), #var)

    } // namespace Satisfy

  #endif // __cplusplus

#endif // SATISFIER_HPP
