#include <iostream>
#include <vector>
#include <cstring>
#include <type_traits>

namespace fmt
{

    template <bool B, typename T = void>
    using enable_if_t = typename std::enable_if<B, T>::type;
    template <bool B, typename T, typename F>
    using conditional_t = typename std::conditional<B, T, F>::type;
    template <bool B>
    using bool_constant = std::integral_constant<bool, B>;

    class basicFormatArg;
    struct customValue
    {
        void *value = nullptr;
        void (*format)(void *arg);
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
        charType,
        last_integer_type = charType,
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

    enum
    {
        long_short = sizeof(long) == sizeof(int)
    };
    using long_type = conditional_t<long_short, int, long long>;
    using ulong_type = conditional_t<long_short, unsigned, unsigned long long>;

    struct argMapper
    {
        constexpr inline auto map(signed char val) -> int { return val; }
        constexpr inline auto map(unsigned char val) -> unsigned
        {
            return val;
        }
        constexpr inline auto map(short val) -> int { return val; }
        constexpr inline auto map(unsigned short val) -> unsigned
        {
            return val;
        }
        constexpr inline auto map(int val) -> int { return val; }
        constexpr inline auto map(unsigned val) -> unsigned { return val; }
        constexpr inline auto map(long val) -> long_type { return val; }
        constexpr inline auto map(unsigned long val) -> ulong_type
        {
            return val;
        }
        constexpr inline auto map(long long val) -> long long { return val; }
        constexpr inline auto map(unsigned long long val)
            -> unsigned long long
        {
            return val;
        }
        constexpr inline auto map(float val) -> float { return val; }
        constexpr inline auto map(double val) -> double { return val; }
        constexpr inline auto map(long double val) -> long double
        {
            return val;
        }

        constexpr inline auto map(char *val) -> const char *
        {
            return val;
        }
        constexpr inline auto map(const char *val) -> const char *
        {
            return val;
        }
        constexpr inline auto map(bool val) -> bool { return val; }
        constexpr inline auto map(void *val) -> const void * { return val; }
        constexpr inline auto map(const void *val) -> const void *
        {
            return val;
        }
        constexpr inline auto map(std::nullptr_t val) -> const void *
        {
            return val;
        }
    };

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
    template <typename Char>
    using std_string_view = std::basic_string_view<Char>;
#elif defined(FMT_USE_EXPERIMENTAL_STRING_VIEW)
    template <typename Char>
    using std_string_view = std::experimental::basic_string_view<Char>;
#else
    template <typename T>
    struct std_string_view
    {
    };
#endif

    template <typename Char>
    struct stringValue
    {
        const Char *data;
        size_t size;
    };

    template <typename T>
    constexpr inline auto const_check(T value) -> T
    {
        return value;
    }

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
    template <typename T, typename Char>
    struct type_constant : std::integral_constant<type, type::custom_type>
    {
    };
    template <typename T>
    using mapped_type_constant =
        type_constant<decltype(argMapper().map(std::declval<const T &>())), char>;

#define FMT_ENABLE_IF(...) enable_if_t<(__VA_ARGS__), int> = 0

    // basic string view
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

        inline basicStringView(const Char *s)
            : data_(s),
              size_(const_check(std::is_same<Char, char>::value &&
                                !fmt::is_constant_evaluated(true))
                        ? std::strlen(reinterpret_cast<const char *>(s))
                        : std::char_traits<Char>::length(s)) {}

        template <typename Traits, typename Alloc>
        constexpr basicStringView(
            const std::basic_string<Char, Traits, Alloc> &s) noexcept
            : data_(s.data()),
              size_(s.size()) {}

        template <typename S, FMT_ENABLE_IF(std::is_same<
                                            S, fmt::std_string_view<Char>>::value)>
        constexpr basicStringView(S s) noexcept : data_(s.data()),
                                                  size_(s.size()) {}

        constexpr auto data() const noexcept -> const Char * { return data_; }

        constexpr auto size() const noexcept -> size_t { return size_; }

        constexpr auto begin() const noexcept -> iterator { return data_; }
        constexpr auto end() const noexcept -> iterator { return data_ + size_; }

        constexpr auto operator[](size_t pos) const noexcept -> const Char &
        {
            return data_[pos];
        }

        void remove_prefix(size_t n) noexcept
        {
            data_ += n;
            size_ -= n;
        }

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

    using stringView = basicStringView<char>;

    struct monostate
    {
        constexpr monostate() {}
    };

    class value
    {
    private:
        template <typename T, typename Formatter>
        static void format_custom_arg(void *arg)
        {
            ;
        }

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
            stringValue<char> string;
            const void *pointer;
            customValue custom;
        };
        constexpr inline value() : no_value() {}
        constexpr inline value(int val) : int_value(val) {}
        constexpr inline value(unsigned val) : uint_value(val) {}
        constexpr inline value(long long val) : long_long_value(val) {}
        constexpr inline value(unsigned long long val) : ulong_long_value(val) {}
        constexpr inline value(float val) : float_value(val) {}
        constexpr inline value(double val) : double_value(val) {}
        inline value(long double val) : long_double_value(val) {}
        constexpr inline value(bool val) : bool_value(val) {}
        constexpr inline value(char val) : char_value(val) {}
        inline value(const char *val)
        {
            string.data = val;
            string.size = std::strlen(val);
        }
        inline value(basicStringView<char> val)
        {
            string.data = val.data();
            string.size = val.size();
        }
        inline value(const void *val) : pointer(val) {}
    };

    constexpr auto is_integral_type(type t) -> bool
    {
        return t > type::none_type && t <= type::last_integer_type;
    }
    constexpr auto is_arithmetic_type(type t) -> bool
    {
        return t > type::none_type && t <= type::last_numeric_type;
    }

    class basicFormatArg
    {
        friend class basicFormatArgs;
        template <typename T>
        friend auto makeArg(const T &value) -> basicFormatArg;

    public:
        // private:
        using charType = char;
        ::fmt::type type_;
        ::fmt::value value_;

    public:
        class handle
        {
        public:
            explicit handle(customValue custom) : custom_(custom) {}
            void format() const
            {
                custom_.format(custom_.value);
            }

        private:
            customValue custom_;
        };
        constexpr explicit operator bool() const noexcept
        {
            return type_ != ::fmt::type::none_type;
        }
        auto type() const -> fmt::type { return type_; }
        auto is_integral() const -> bool { return ::fmt::is_integral_type(type_); }
        auto is_arithmetic() const -> bool { return ::fmt::is_arithmetic_type(type_); }
    };

    template <typename T>
    auto makeArg(const T &value) -> basicFormatArg
    {
        basicFormatArg arg;
        arg.type_ = mapped_type_constant<T>::value;
        arg.value_ = argMapper().map(value);
        return arg;
    }

    template <typename T, typename Char, size_t NUM_ARGS>
    struct argData
    {
        T args_[1 + (NUM_ARGS != 0 ? NUM_ARGS : +1)];
        const int num_args = NUM_ARGS;
        template <typename... U>
        argData(const U &...init) : args_{init...} {}
        argData(const argData &other) = delete;
        auto args() const -> const T * { return args_; }
    };

    template <typename... Args>
    class formatArgStore
    {
    private:
        static const size_t num_args = sizeof...(Args);
        using valueType = basicFormatArg;
        argData<valueType, char, num_args> data_;
        friend class basicFormatArgs;

    public:
        template <typename... T>
        constexpr inline formatArgStore(T &&...args)
            : data_{makeArg<remove_cvref_t<T>>(std::forward<T>(args))...}
        {
        }
    };

    class basicFormatArgs
    {
    public:
        using size_type = int;
        using format_arg = basicFormatArg;

        constexpr basicFormatArgs(const format_arg *args,const int& num_args)
            : args_(args) ,numArgs(num_args){}

        template <typename... Args>
        constexpr inline basicFormatArgs(
            const formatArgStore<Args...> &store)
            : basicFormatArgs(store.data_.args(),store.data_.num_args) {}

        auto get(int id) const -> format_arg
        {
            return args_[id];
        }

        auto size() const -> int
        {
            return numArgs;
        }

    private:
        const format_arg *args_;
        const int numArgs;
    };

    template <typename... Args>
    auto makeFormatArgs(Args &&...args)
        -> formatArgStore<remove_cvref_t<Args>...>
    {
        return {std::forward<Args>(args)...};
    }

    inline auto vformat(stringView fmt, basicFormatArgs args)
        -> std::string
    {
        std::cout << args.get(0).value_.int_value << "\n"
        << args.get(1).value_.int_value << "\n"
        << args.get(2).value_.string.data << "\n"
        << args.size() << "\n";
        return std::string();
    }

    template <typename... T>
    inline auto format(const char *fmt, T &&...args)
        -> std::string
    {
        return vformat(stringView(fmt), fmt::makeFormatArgs(args...));
    }
}

int main()
{
    fmt::format("123", 114, 514, "Be One With Yuri!");
    return 0;
}
