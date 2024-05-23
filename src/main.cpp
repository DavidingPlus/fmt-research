#include <iostream>
#include <vector>


namespace fmt
{
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
