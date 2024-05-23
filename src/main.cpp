#include <iostream>
#include <vector>
#include <type_traits>

namespace fmt
{
    template <typename... T>
    constexpr void ignore_unused(const T &...) {}

    constexpr inline auto is_constant_evaluated(bool default_value = false) noexcept -> bool
    {
#ifdef __cpp_lib_is_constant_evaluated
        ignore_unused(default_value);
        return std::is_constant_evaluated();
#else
        return default_value;
#endif
    }

#if defined(FMT_USE_STRING_VIEW)
template <typename Char> using std_string_view = std::basic_string_view<Char>;
#elif defined(FMT_USE_EXPERIMENTAL_STRING_VIEW)
template <typename Char>
using std_string_view = std::experimental::basic_string_view<Char>;
#else
template <typename T> struct std_string_view {};
#endif

    template <typename T>
    constexpr inline auto const_check(T value) -> T
    {
        return value;
    }

    template <bool B, typename T = void>
    using enable_if_t = typename std::enable_if<B, T>::type;
    template <bool B, typename T, typename F>
    using conditional_t = typename std::conditional<B, T, F>::type;
    template <bool B>
    using bool_constant = std::integral_constant<bool, B>;
    template <typename T>
    using remove_reference_t = typename std::remove_reference<T>::type;
    template <typename T>
    using remove_const_t = typename std::remove_const<T>::type;
    template <typename T>
    using remove_cvref_t = typename std::remove_cv<remove_reference_t<T>>::type;
    template <typename T>
    struct type_identity
    {
        using type = T;
    };
    template <typename T>
    using type_identity_t = typename type_identity<T>::type;

#define FMT_ENABLE_IF(...) enable_if_t<(__VA_ARGS__), int> = 0

//basic string view
    template <typename Char>
    class basicStringView
    {
    private:
        const Char *data_;
        size_t size_;

    public:
        using value_type = Char;
        using iterator = const Char *;

        constexpr basicStringView() noexcept : data_(nullptr), size_(0) {}

        /** Constructs a string reference object from a C string and a size. */
        constexpr basicStringView(const Char *s, size_t count) noexcept
            : data_(s),
              size_(count) {}

        /**
          \rst
          Constructs a string reference object from a C string computing
          the size with ``std::char_traits<Char>::length``.
          \endrst
         */

        inline basicStringView(const Char *s)
            : data_(s),
              size_(const_check(std::is_same<Char, char>::value &&
                                !fmt::is_constant_evaluated(true))
                        ? std::strlen(reinterpret_cast<const char *>(s))
                        : std::char_traits<Char>::length(s)) {}

        /** Constructs a string reference from a ``std::basic_string`` object. */
        template <typename Traits, typename Alloc>
        constexpr basicStringView(
            const std::basic_string<Char, Traits, Alloc> &s) noexcept
            : data_(s.data()),
              size_(s.size()) {}

        template <typename S, FMT_ENABLE_IF(std::is_same<
                                            S, fmt::std_string_view<Char>>::value)>
        constexpr basicStringView(S s) noexcept : data_(s.data()),
                                                  size_(s.size()) {}

        /** Returns a pointer to the string data. */
        constexpr auto data() const noexcept -> const Char * { return data_; }

        /** Returns the string size. */
        constexpr auto size() const noexcept -> size_t { return size_; }

        constexpr auto begin() const noexcept -> iterator { return data_; }
        constexpr auto end() const noexcept -> iterator { return data_ + size_; }

        constexpr auto operator[](size_t pos) const noexcept -> const Char &
        {
            return data_[pos];
        }

        constexpr void remove_prefix(size_t n) noexcept
        {
            data_ += n;
            size_ -= n;
        }

        // Lexicographically compare this string reference to other.
        auto compare(basicStringView other) const -> int
        {
            size_t str_size = size_ < other.size_ ? size_ : other.size_;
            int result = std::char_traits<Char>::compare(data_, other.data_, str_size);
            if (result == 0)
                result = size_ == other.size_ ? 0 : (size_ < other.size_ ? -1 : 1);
            return result;
        }

        friend auto operator==(basicStringView lhs,
                               basicStringView rhs)
            -> bool
        {
            return lhs.compare(rhs) == 0;
        }
        friend auto operator!=(basicStringView lhs, basicStringView rhs) -> bool
        {
            return lhs.compare(rhs) != 0;
        }
        friend auto operator<(basicStringView lhs, basicStringView rhs) -> bool
        {
            return lhs.compare(rhs) < 0;
        }
        friend auto operator<=(basicStringView lhs, basicStringView rhs) -> bool
        {
            return lhs.compare(rhs) <= 0;
        }
        friend auto operator>(basicStringView lhs, basicStringView rhs) -> bool
        {
            return lhs.compare(rhs) > 0;
        }
        friend auto operator>=(basicStringView lhs, basicStringView rhs) -> bool
        {
            return lhs.compare(rhs) >= 0;
        }
    };



    struct monostate
    {
        constexpr monostate() {}
    };

    enum class type
    {
        none_type,
        // Integer types should go first,
        int_type,
        uint_type,
        long_long_type,
        ulong_long_type,
        int128_type,
        uint128_type,
        bool_type,
        char_type,
        last_integer_type = char_type,
        // followed by floating-point types.
        float_type,
        double_type,
        long_double_type,
        last_numeric_type = long_double_type,
        cstring_type,
        string_type,
        pointer_type,
        custom_type
    };

    class value
    {
    private:
        /* data */
    public:
        union
        {
            monostate no_value;
            int int_value;
            unsigned uint_value;
            long long long_long_value;
            unsigned long long ulong_long_value;
            bool bool_value;
            char char_value;
            float float_value;
            double double_value;
            long double long_double_value;
            const void *pointer;
            customValue custom;
        };
        value(/* args */) { ; }
        ~value() { ; }
    };
    constexpr auto is_integral_type(type t) -> bool
    {
        return t > type::none_type && t <= type::last_integer_type;
    }
    constexpr auto is_arithmetic_type(type t) -> bool
    {
        return t > type::none_type && t <= type::last_numeric_type;
    }

    class basicFormatarg
    {
    private:
        using charType = char;
        ::fmt::type type_;

    public:
        void visit();
        class handle
        {
        };
        constexpr explicit operator bool() const noexcept
        {
            return type_ != ::fmt::type::none_type;
        }
        auto type() const -> fmt::type { return type_; }
        auto is_integral() const -> bool { return fmt::is_integral_type(type_); }
        auto is_arithmetic() const -> bool { return fmt::is_arithmetic_type(type_); }
    };

    struct customValue
    {
        using fmtFunc = void(void *);
        void *value = nullptr;
        fmtFunc format;
    };
}

template <typename T>
void formatImpl(T *pArg)
{
    ;
}

template <>
void formatImpl(int *pArg)
{
    std::cout << "[------------Int received-------------]" << *pArg;
}

template <>
void formatImpl(float *pArg)
{
    std::cout << "[------------Float received-------------]" << *pArg;
}

template <typename T>
void formatFunc(void *pArg)
{
    formatImpl<T>(static_cast<T *>(pArg));
}

template <typename T>
struct arg_t
{
    using fmtFunc = void(void *);
    arg_t(void *a) : pArg(a), format(formatFunc<T>) {}
    /* data */
    void *pArg = nullptr;
    fmtFunc format;
};

template <typename T>
std::vector<T> argStore(T arg)
{
    std::vector<arg_t> res;
    res.append(arg_t((void *)arg));
}

template <typename T, typename... Args>
std::vector<T> f(T t, Args... args)
{
    std::vector<T> res = f(args...);
    res.prepend(t);
    return res;
}

int main()
{
    auto vec = f(1, 2, 3);

    std::cout << vec.size();

    std::cout;

    for (auto it = vec.begin(); it != vec.end(); ++it)
    {
        std::cout << *it;
    }

    return 0;
}
