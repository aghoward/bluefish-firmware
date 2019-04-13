#pragma once

#include <utility.h>
#include <type_traits.h>

namespace detail {
    template <typename ... T>
    using void_t = void;

    template <typename T, typename = void>
    struct supports_size : std::false_type {};

    template <typename T>
    struct supports_size<T, void_t<decltype(std::declval<T>().size())>>
        : std::true_type {};

    template <typename T>
    static constexpr bool supports_size_v =
        supports_size<std::remove_cvref_t<T>>::value;
}

template <typename T, typename = std::enable_if_t<detail::supports_size_v<T>>>
auto size(T&& item) -> decltype(std::declval<T>().size())
{
    return item.size();
}

template <typename T, typename = std::enable_if_t<!detail::supports_size_v<T>>>
auto size(T&&) -> decltype(sizeof(T))
{
    return sizeof(T);
}
