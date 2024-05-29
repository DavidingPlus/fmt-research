#include <iostream>
#include <string>
#include <type_traits>

using stringType = std::string;

// 管理类型注册的泛化结构体
template <typename T>
struct toStringRegister
{
};

// 注册一个类型就是对该类型的toStringRegister做特化声明，并在结构体中创建type名称
#define TOSTRING_REGIST(U)     \
    template <>                \
    struct toStringRegister<U> \
    {                          \
        using type = U;        \
    };

// 对已注册的类型U，TOSTRING_FUNC替换为下列内容，作为类型U的toString函数的声明，其中形参名默认为val
#define TOSTRING_FUNC(U)                                      \
    typename std::enable_if<(isToStringRegistered<U>::value), \
                            stringType>::type                 \
    toString(const U &val)

// TOSTRING_DECL将上面两个宏的工作结合起来
#define TOSTRING_DECL(U) \
    TOSTRING_REGIST(U)   \
    TOSTRING_FUNC(U)

// void_t for CPP11
template <typename... T>
struct make_void
{
    using type = void;
};
template <typename... T>
using void_t = typename make_void<T...>::type;

// 特性，通过模板参数T对应的toStringRegister<T>中type名称是否被创建过，来检查T是否是已注册类型
template <typename T, typename = void>
struct isToStringRegistered : std::false_type
{
};

template <typename T>
struct isToStringRegistered<T, void_t<typename toStringRegister<T>::type>> : std::true_type
{
};

// 如果T不是已注册类型
template <typename T>
typename std::enable_if<!(isToStringRegistered<T>::value), stringType>::type
toString(const T &val)
{
    return std::string();
}

// 当T是已注册类型时，根据实际问题的需要，为不同的T特化模板
TOSTRING_DECL(int)
{
    return std::to_string(val);
}

TOSTRING_DECL(long)
{
    return std::to_string(val);
}

TOSTRING_DECL(short)
{
    return std::to_string(val);
}

TOSTRING_DECL(float)
{
    return std::to_string(val);
}

TOSTRING_DECL(double)
{
    return std::to_string(val);
}

TOSTRING_DECL(std::string)
{
    return val;
}

TOSTRING_DECL(const char*)
{
    return std::string(val);
}

