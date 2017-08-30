#ifndef TRIAL_PROTOCOL_DYNAMIC_VARIABLE_HPP
#define TRIAL_PROTOCOL_DYNAMIC_VARIABLE_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2017 Bjorn Reese <breese@users.sourceforge.net>
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <memory>
#include <iterator>
#include <string>
#include <vector>
#include <map>
#include <trial/protocol/core/detail/config.hpp>
#include <trial/protocol/core/detail/small_union.hpp>
#include <trial/protocol/dynamic/error.hpp>
#include <trial/protocol/dynamic/token.hpp>

namespace trial
{
namespace protocol
{
namespace dynamic
{

// Forward declarations

namespace detail
{

template <typename T, typename U, typename> struct overloader;
template <typename T, typename U, typename> struct iterator_overloader;
template <typename T, typename U, typename> struct operator_overloader;
template <typename C, typename T, typename> struct same_overloader;

} // namespace detail

enum nullable { null };
struct boolean {};
struct integer {};
struct number {};
struct string {};
template <typename CharT> struct basic_array;
template <typename CharT> struct basic_map;

template <typename CharT>
class basic_variable
{
    template <typename T> struct traits;
    template <typename T, typename = void> struct tag_traits;

public:
    using value_type = basic_variable<CharT>;
    using reference = typename std::add_lvalue_reference<value_type>::type;
    using const_reference = typename std::add_const<reference>::type;
    using size_type = std::size_t;
    using string_type = std::basic_string<CharT>;
    using array_type = std::vector<value_type>;
    using map_type = std::map<value_type, value_type>;
    using pair_type = typename map_type::value_type;

private:
    friend struct basic_array<CharT>;
    friend struct basic_map<CharT>;

    template <typename Derived, typename T>
    class iterator_base
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using key_type = typename std::add_const<typename map_type::key_type>::type;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = typename std::add_pointer<value_type>::type;
        using reference = typename std::add_lvalue_reference<value_type>::type;
        using const_reference = typename std::add_const<reference>::type;

        Derived& operator= (const Derived&);
        Derived& operator= (Derived&&);

        Derived& operator++ ();
        Derived operator++ (int);

        pointer operator-> ();

        bool operator== (const Derived&) const;
        bool operator!= (const Derived&) const;

    protected:
        friend class basic_variable<CharT>;

        using array_iterator = typename std::conditional<std::is_const<T>::value,
                                                         typename array_type::const_iterator,
                                                         typename array_type::iterator>::type;
        using map_iterator = typename std::conditional<std::is_const<T>::value,
                                                       typename map_type::const_iterator,
                                                       typename map_type::iterator>::type;

        iterator_base();
        iterator_base(const iterator_base&);
        iterator_base(iterator_base&&);
        iterator_base(pointer, bool = true);
        iterator_base(pointer, array_iterator);
        iterator_base(pointer, map_iterator);

        const_reference key() const;
        reference value();
        const_reference value() const;

    protected:
        using small_union = core::detail::small_union<sizeof(pointer),
                                                      pointer,
                                                      array_iterator,
                                                      map_iterator>;

        pointer scope;
        small_union current;
    };

public:
    class const_iterator;

    class iterator
        : public iterator_base<iterator, typename basic_variable<CharT>::value_type>
    {
        using super = iterator_base<iterator, typename basic_variable<CharT>::value_type>;

    public:
        using typename super::iterator_category;
        using typename super::key_type;
        using typename super::value_type;
        using typename super::difference_type;
        using typename super::pointer;
        using typename super::reference;
        using typename super::const_reference;

        iterator();
        iterator(const iterator& other);
        iterator(iterator&& other);

        iterator& operator= (const iterator& other);
        iterator& operator= (iterator&& other);

        const_reference key() const { return super::key(); }
        reference value() { return super::value(); }
        const_reference value() const { return super::value(); }
        reference operator* () { return super::value(); }
        const_reference operator* () const { return super::value(); }

    private:
        friend class basic_variable<CharT>;

        // Conversion from const_iterator to iterator is kept private
        iterator(const const_iterator& other);
        iterator(pointer p, bool e = true);
        iterator(pointer p, typename super::array_iterator);
        iterator(pointer p, typename super::map_iterator);
    };

    class const_iterator
        : public iterator_base<const_iterator, const typename basic_variable<CharT>::value_type>
    {
        using super = iterator_base<const_iterator, const typename basic_variable<CharT>::value_type>;

    public:
        using typename super::iterator_category;
        using typename super::key_type;
        using typename super::value_type;
        using typename super::difference_type;
        using typename super::pointer;
        using typename super::reference;
        using typename super::const_reference;

        const_iterator();
        const_iterator(const const_iterator& other);
        const_iterator(const_iterator&& other);
        // iterator is convertible to const_iterator
        const_iterator(const iterator& other);

        const_reference key() const { return super::key(); }
        const_reference value() const { return super::value(); }
        const_reference operator* () const { return super::value(); }

    private:
        friend class basic_variable<CharT>;
        template <typename T, typename U, typename> friend struct detail::iterator_overloader;

        const_iterator(pointer p, bool e = true);
    };

    class key_iterator
        : public iterator_base<key_iterator, const typename basic_variable<CharT>::value_type>
    {
        using super = iterator_base<key_iterator, const typename basic_variable<CharT>::value_type>;

    public:
        using typename super::iterator_category;
        using typename super::key_type;
        using typename super::value_type;
        using typename super::difference_type;
        using typename super::pointer;
        using typename super::reference;
        using typename super::const_reference;

        key_iterator();
        key_iterator(const key_iterator& other);
        key_iterator(key_iterator&& other);

        key_iterator& operator= (const key_iterator& other);
        key_iterator& operator= (key_iterator&& other);

        const_reference key() const;
        const_reference value() const { return super::value(); }
        const_reference operator* () { return key(); }

        key_iterator& operator++();

    private:
        friend class basic_variable<CharT>;

        key_iterator(pointer p, bool e = true);

    private:
        typename std::remove_const<value_type>::type index;
    };

    // Constructor
    basic_variable(const basic_variable<CharT>&);
    basic_variable(basic_variable<CharT>&&);
    template <typename T> basic_variable(T);
    // Initializer
    basic_variable(const std::initializer_list<value_type>&);
    // Null constructor
    basic_variable();
    basic_variable(const nullable&);
    // String constructor
    basic_variable(const CharT *);
    // Use factory instead
    basic_variable(typename basic_variable::array_type) = delete;
    basic_variable(typename basic_variable::map_type) = delete;
    basic_variable(typename basic_variable::pair_type) = delete;
 
    // Assignment

    basic_variable& operator= (const basic_variable&);
    basic_variable& operator= (basic_variable&&);
    template <typename T> basic_variable& operator= (T);
    basic_variable& operator= (nullable);
    basic_variable& operator= (const CharT *);

    // Addition / concatenation

    template <typename T> basic_variable& operator+= (const T&);
    basic_variable& operator+= (const basic_variable&);
    basic_variable& operator+= (const CharT *);

    // Accessor

    template <typename Tag> typename tag_traits<typename std::decay<Tag>::type>::type value() const;
    template <typename Tag> typename tag_traits<typename std::decay<Tag>::type>::type value(std::error_code&) const noexcept;
    template <typename R> explicit operator R() const;

    // Get stored value as type R.
    //
    // Precondition: same<R>() is true.
    template <typename R> R& unsafe_get() & noexcept;
    template <typename R> const R& unsafe_get() const & noexcept;

    explicit operator bool() const;

    basic_variable& operator[] (size_type) &;
    const basic_variable& operator[] (size_type) const &;

    basic_variable& operator[] (const typename map_type::key_type&) &;
    const basic_variable& operator[] (const typename map_type::key_type&) const &;

    iterator find(const basic_variable&) &;
    const_iterator find(const basic_variable&) const &;
    template <typename T> iterator find(const T&) &;
    template <typename T> const_iterator find(const T&) const &;

    size_type count(const basic_variable&) const;
    template <typename T> size_type count(const T&) const;

    // Type checker

    template <typename T> bool is() const noexcept;
    template <typename T> bool same() const noexcept;

    token::code::value code() const noexcept;
    token::symbol::value symbol() const noexcept;

    // Capacity

    bool empty() const noexcept;
    size_type size() const noexcept;

    // Modifiers

    void clear() noexcept;

    iterator insert(const basic_variable&);
    template <typename InputIterator>
    void insert(InputIterator begin, InputIterator end);

    iterator insert(const_iterator, const basic_variable&);
    template <typename InputIterator>
    void insert(const_iterator, InputIterator begin, InputIterator end);

    iterator erase(const_iterator);
    iterator erase(const_iterator, const_iterator);

    // Iteration

    iterator begin() &;
    const_iterator begin() const &;
    iterator end() &;
    const_iterator end() const &;

    key_iterator key_begin() const &;
    key_iterator key_end() const &;

private:
    bool is_pair() const;

private:
    template <typename T, typename U, typename> friend struct detail::overloader;
    template <typename T, typename U, typename> friend struct detail::iterator_overloader;
    template <typename T, typename U, typename> friend struct detail::operator_overloader;
    template <typename C, typename T, typename> friend struct detail::same_overloader;
    template <typename T> struct similar_visitor;

    using storage_type = core::detail::small_union<sizeof(double),
                                                   nullable,
                                                   bool,
                                                   signed char,
                                                   unsigned char,
                                                   signed short int,
                                                   unsigned short int,
                                                   signed int,
                                                   unsigned int,
                                                   signed long int,
                                                   unsigned long int,
                                                   signed long long int,
                                                   unsigned long long int,
                                                   float,
                                                   double,
                                                   long double,
                                                   string_type,
                                                   array_type,
                                                   map_type>;
    storage_type storage;
};

template <typename T, typename U>
basic_variable<T> operator+ (const basic_variable<T>&, const U&);

template <typename T>
basic_variable<T> operator+ (nullable, const basic_variable<T>&);

// Container comparison operators are noexcept from C++14

template <typename T, typename U>
bool operator== (const T&, const U&) TRIAL_PROTOCOL_CXX14(noexcept);

template <typename T, typename U>
bool operator!= (const T&, const U&) TRIAL_PROTOCOL_CXX14(noexcept);

template <typename T, typename U>
bool operator< (const T&, const U&) TRIAL_PROTOCOL_CXX14(noexcept);

template <typename CharT, typename U>
bool operator<= (const basic_variable<CharT>&, const U&) TRIAL_PROTOCOL_CXX14(noexcept);

template <typename CharT, typename U>
bool operator> (const basic_variable<CharT>&, const U&) TRIAL_PROTOCOL_CXX14(noexcept);

template <typename CharT, typename U>
bool operator>= (const basic_variable<CharT>&, const U&) TRIAL_PROTOCOL_CXX14(noexcept);

// Convenience

using variable = basic_variable<char>;
using map = basic_map<char>;
using array = basic_array<char>;

} // namespace dynamic
} // namespace protocol
} // namespace trial

#include <trial/protocol/dynamic/detail/variable.ipp>

#endif // TRIAL_PROTOCOL_DYNAMIC_VARIABLE_HPP
