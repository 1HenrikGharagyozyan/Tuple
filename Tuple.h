#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

// ------------------ tuple_leaf -------------------
// A single tuple element holder.
// Each tuple_leaf<I, T> stores the I-th element of type T.
template<std::size_t I, typename T>
struct tuple_leaf
{
    using value_type = T;

    T value; // The actual stored value for this element

    constexpr tuple_leaf() = default;

    template<typename U, typename = std::enable_if_t<std::is_constructible_v<T, U&&>>>
    constexpr explicit tuple_leaf(U&& v) noexcept(std::is_nothrow_constructible_v<T, U&&>)
        : value(std::forward<U>(v)) {
    }

    constexpr tuple_leaf(const tuple_leaf&) = default;
    constexpr tuple_leaf(tuple_leaf&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;

    constexpr tuple_leaf& operator=(const tuple_leaf&) = default;
    constexpr tuple_leaf& operator=(tuple_leaf&&) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

    constexpr T& get() & noexcept { return value; }
    constexpr const T& get() const& noexcept { return value; }
    constexpr T&& get() && noexcept { return std::move(value); }
    constexpr const T&& get() const&& noexcept { return std::move(value); }
};

// ------------------ tuple_impl -------------------
// A base implementation that inherits from tuple_leaf<Is, Ts>...
// This structure actually stores all tuple elements by multiple inheritance.
template<typename IndexSeq, typename... Ts>
struct tuple_impl;

template<std::size_t... Is, typename... Ts>
struct tuple_impl<std::index_sequence<Is...>, Ts...> : tuple_leaf<Is, Ts>...
{
    using self_type = tuple_impl<std::index_sequence<Is...>, Ts...>;

    constexpr tuple_impl() = default;

    template<typename... Us, typename = std::enable_if_t<(sizeof...(Us) == sizeof...(Ts))>>
    constexpr explicit tuple_impl(Us&&... args)
        : tuple_leaf<Is, Ts>(std::forward<Us>(args))... {}

    constexpr tuple_impl(const self_type&) = default;
    constexpr tuple_impl(self_type&&) noexcept((std::is_nothrow_move_constructible_v<Ts> && ...)) = default;

    constexpr self_type& operator=(const self_type&) = default;
    constexpr self_type& operator=(self_type&&) noexcept((std::is_nothrow_move_assignable_v<Ts> && ...)) = default;
};

// ------------------ tuple -------------------
// The public tuple class.
// Inherits from tuple_impl to store all elements and provide a clean interface.
template<typename... Ts>
struct tuple : tuple_impl<std::index_sequence_for<Ts...>, Ts...>
{
    using base_type = tuple_impl<std::index_sequence_for<Ts...>, Ts...>;
    using base_type::base_type; // inherit all constructors from base_type
};

// ------------------ tuple_element -------------------
// Metafunction to get the I-th type from a tuple<Ts...>.
template<std::size_t I, typename Tuple>
struct tuple_element;

template<std::size_t I, typename T, typename... Ts>
struct tuple_element<I, tuple<T, Ts...>> : tuple_element<I - 1, tuple<Ts...>> {};

template<typename T, typename... Ts>
struct tuple_element<0, tuple<T, Ts...>> { using type = T; };

template<std::size_t I, typename Tuple>
using tuple_element_t = typename tuple_element<I, Tuple>::type;

// ------------------ get<I> -------------------
// Returns a reference to the I-th element of the tuple.
template<std::size_t I, typename... Ts>
constexpr tuple_element_t<I, tuple<Ts...>>& get(tuple<Ts...>& t) noexcept
{
    return static_cast<tuple_leaf<I, tuple_element_t<I, tuple<Ts...>>>&>(t).get();
}

template<std::size_t I, typename... Ts>
constexpr const tuple_element_t<I, tuple<Ts...>>& get(const tuple<Ts...>& t) noexcept
{
    return static_cast<const tuple_leaf<I, tuple_element_t<I, tuple<Ts...>>>&>(t).get();
}

template<std::size_t I, typename... Ts>
constexpr tuple_element_t<I, tuple<Ts...>>&& get(tuple<Ts...>&& t) noexcept
{
    return static_cast<tuple_leaf<I, tuple_element_t<I, tuple<Ts...>>>&&>(t).get();
}

template<std::size_t I, typename... Ts>
constexpr const tuple_element_t<I, tuple<Ts...>>&& get(const tuple<Ts...>&& t) noexcept
{
    return static_cast<const tuple_leaf<I, tuple_element_t<I, tuple<Ts...>>>&&>(t).get();
}

// ------------------ tuple_size -------------------
// Compile-time number of elements in a tuple.
template<typename T>
struct tuple_size;

template<typename... Ts>
struct tuple_size<tuple<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template<typename T>
inline constexpr std::size_t tuple_size_v = tuple_size<T>::value;

// ------------------ make_tuple -------------------
// Helper function that constructs a tuple with decayed types.
template<typename... Ts>
constexpr auto make_tuple(Ts&&... args)
{
    using Tuple = tuple<std::decay_t<Ts>...>;
    return Tuple(std::forward<Ts>(args)...);
}

// ------------------ operator== -------------------
// Compares two tuples element-by-element.
template<typename... Ts, std::size_t... Is>
constexpr bool tuple_equal_impl(const tuple<Ts...>& a, const tuple<Ts...>& b, std::index_sequence<Is...>)
{
    return ((get<Is>(a) == get<Is>(b)) && ...);
}

template<typename... Ts>
constexpr bool operator==(const tuple<Ts...>& a, const tuple<Ts...>& b)
{
    return tuple_equal_impl(a, b, std::index_sequence_for<Ts...>{});
}
