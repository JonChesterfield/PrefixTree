/*
 * This file is part of PrefixTree
 *
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016 Jon Chesterfield
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef PREFIX_H
#define PREFIX_H

#include "string.hpp"

#include <cstddef>
#include <cstdio>
#include <algorithm>
#include <tuple>
#include <cassert>
#include <type_traits>

namespace prefix
{
struct external
{
};

template <bool B>
struct force_bool;

template <>
struct force_bool<true>
{
  static constexpr bool value = true;
};

template <>
struct force_bool<false>
{
  static constexpr bool value = false;
};

template <typename T>
class member
{
 public:
  const str_const key;
  const T value;

  template <std::size_t N>
  constexpr member(const char (&a)[N], T t)
      : key(a), value(t)
  {
  }

  friend constexpr bool operator==(const member& x, const member& y)
  {
    return x.key == y.key && x.value == y.value;
  }
  friend constexpr bool operator!=(const member& x, const member& y)
  {
    return !(x == y);
  }
};

template <>
class member<external>
{
 public:
  const str_const key;

  template <std::size_t N>
  constexpr member(const char (&a)[N])
      : key(a)
  {
  }

  friend constexpr bool operator==(const member& x, const member& y)
  {
    return x.key == y.key;
  }
  friend constexpr bool operator!=(const member& x, const member& y)
  {
    return !(x == y);
  }
};

// The iterator is very rough and ready, i.e. untested
template <typename M, typename V>
class iterator_impl
    : public std::iterator<std::input_iterator_tag, M, std::ptrdiff_t, V*, V&>
{
 private:
  typedef iterator_impl self_type;

 public:
  constexpr iterator_impl() : ptr(nullptr) {}
  constexpr iterator_impl(iterator_impl const& other) : ptr(other.ptr) {}
  constexpr iterator_impl(const M* m) : ptr(m) {}

  friend bool operator==(self_type x, self_type y) { return x.ptr == y.ptr; }
  friend bool operator!=(self_type x, self_type y) { return !(x == y); }

  const V& operator*() { return ptr->value; }

  const V* operator->() { return &(operator*()); }

  self_type operator+(std::ptrdiff_t n)
  {
    self_type tmp(*this);
    tmp.ptr += n;
    return tmp;
  }
  self_type& operator+=(std::ptrdiff_t n)
  {
    ptr += n;
    return *this;
  }

  self_type& operator++()
  {
    ++ptr;
    return *this;
  }

  self_type operator++(int)
  {
    self_type tmp(*this);
    ++(*this);
    return tmp;
  }

 private:
  const M* ptr;
};

template <typename T, typename V = external>
class crtp
{
 public:
  typedef str_const key_type;
  typedef V value_type;
  using element = member<value_type>;
  using iterator = iterator_impl<element, value_type>;

  // Access string key in table by index
  template <std::size_t I>
  static constexpr key_type get()
  {
    static_assert(I < size(), "Get index out of bounds");
    return T::table[I].key;
  }

  static constexpr key_type get(std::size_t i)
  {
    return i < size()
               ? T::table[i].key
               : throw std::out_of_range("get element index out of range");
  }

  // Get size of table
  static constexpr std::size_t size()
  {
    return std::extent<decltype(T::table)>{};
  }

  // Get size of largest key in table
  template <std::size_t L, std::size_t U>
  static constexpr std::size_t max_key_size()
  {
    return get<max_key_size_impl<L, U, L>()>().size();
  }

  // Is the table ordered? Required for lookup
  template <std::size_t L, std::size_t U>
  static constexpr bool ordered()
  {
    return ordered_impl<L, U>();
  }

  // Returned if lookup_index fails
  static constexpr std::size_t fail() { return size(); }

  // Lookup index in table that matches key
  template <typename U>
  static constexpr typename std::enable_if<std::is_same<U, const char*>::value,
                                           std::size_t>::type
  lookup_index(U key)
  {
    return lookup_index_impl(key);
  }

  template <std::size_t N>
  constexpr static std::size_t lookup_index(const char (&key)[N])
  {
    return lookup_index_impl(str_const(key).data());
  }

  constexpr static std::size_t lookup_index_impl(const char* key)
  {
    static_assert(ordered<0, size()>(), "Table is not ordered - cannot search");
    return scan<0, size(), 0, max_key_size<0, size()>()>(key);
  }

  static constexpr iterator begin() { return iterator(&T::table[0]); }

  static constexpr iterator end() { return begin() + size(); }

  // Lookup index when using external storage
  template <typename U>
  static typename std::enable_if<std::is_same<U, external>::value,
                                 std::size_t>::type
  lookup_impl(const char* key)
  {
    return lookup_index(key);
  }

  // Lookup reference (should be iterator) when using internal storage
  template <typename U>
  static typename std::enable_if<!std::is_same<U, external>::value,
                                 iterator>::type
  lookup_impl(const char* key)
  {
    const std::size_t index = lookup_index(key);
    if (index < size())
      {
        return begin() + index;
      }
    return end();
  }

  // Lookup index for external storage or reference for internal storage
  static auto lookup(const char* key) -> decltype(lookup_impl<value_type>(key))
  {
    return lookup_impl<value_type>(key);
  }

#ifndef PREFIX_TESTING_ACCESS
 private:
#else
#undef PREFIX_TESTING_ACCESS
#endif

  // Wrap the slightly nasty str_const::get interface
  template <std::size_t P, std::size_t I>
  static constexpr char getchar()
  {
    // Get character at position P in table, index I
    static_assert(I < get<P>().size(), "");
    return str_const::get<get<P>().size(), I>(get<P>());
  }

  template <std::size_t L, std::size_t U, std::size_t M>
  static constexpr typename std::enable_if<(L == U), std::size_t>::type
  max_key_size_impl()
  {
    static_assert(in_bound<M>, "M out of bounds");
    return M;
  }

  template <std::size_t L, std::size_t U, std::size_t M>
  static constexpr typename std::enable_if<(L + 1 == U), std::size_t>::type
  max_key_size_impl()
  {
    static_assert(in_bound<M>, "M out of bounds");
    return get<L>().size() < get<M>().size() ? M : L;
  }

  template <std::size_t L, std::size_t U, std::size_t M>
  static constexpr typename std::enable_if<(L + 2 == U), std::size_t>::type
  max_key_size_impl()
  {
    static_assert(in_bound<M>, "M out of bounds");

    return (get<L>().size() < get<L + 1>().size())
               ? get<L + 1>().size() < get<M>().size() ? M : L + 1
               : get<L>().size() < get<M>().size() ? M : L;
  }

  template <std::size_t L, std::size_t U, std::size_t M>
  static constexpr typename std::enable_if<(L + 2 < U), std::size_t>::type
  max_key_size_impl()
  {
    return (get<max_key_size_impl<L, (U + L) / 2 + 1, M>()>().size() <
            get<max_key_size_impl<(U + L) / 2, U, M>()>().size())
               ? max_key_size_impl<(U + L) / 2, U, M>()
               : max_key_size_impl<L, (U + L) / 2 + 1, M>();
  }

  template <std::size_t X>
  static constexpr bool in_bound()
  {
    return X < size();
  }

  struct lessthan
  {
    static constexpr bool cmp(char x, char y) { return x < y; }
  };

  struct morethan
  {
    static constexpr bool cmp(char x, char y) { return y < x; }
  };

  template <typename CMP, std::size_t L, std::size_t U, std::size_t I,
            std::size_t F, std::size_t M>
  static constexpr typename std::enable_if<(L == U), std::size_t>::type
  limiting_char()
  {
    return M;
  }

  template <typename CMP, std::size_t L, std::size_t U, std::size_t I,
            std::size_t F, std::size_t M>
  static constexpr typename std::enable_if<(L + 1 == U), std::size_t>::type
  limiting_char()
  {
    return (I >= get(L).size())
               ? M
               : (M == F) ? L : CMP::cmp(get(L)[I], get(M)[I]) ? L : M;
  }

  template <typename CMP, std::size_t L, std::size_t U, std::size_t I,
            std::size_t F, std::size_t M, std::size_t X, std::size_t Y>
  static constexpr std::size_t limiting_char_binary()
  {
    return (X == F) ? Y : (Y == F) ? X : CMP::cmp(get(X)[I], get(Y)[I]) ? X : Y;
  }

  template <typename CMP, std::size_t L, std::size_t U, std::size_t I,
            std::size_t F, std::size_t M>
  static constexpr typename std::enable_if<(L + 2 == U), std::size_t>::type
  limiting_char()
  {
    return limiting_char_binary<CMP, L, U, I, F, M,
                                limiting_char<CMP, L, L + 1, I, F, M>(),
                                limiting_char<CMP, L + 1, U, I, F, M>()>();
  }

  template <typename CMP, std::size_t L, std::size_t U, std::size_t I,
            std::size_t F, std::size_t M>
  static constexpr typename std::enable_if<(L + 2 < U), std::size_t>::type
  limiting_char()

  {
    return limiting_char_binary<
        CMP, L, U, I, F, M, limiting_char<CMP, L, (L + U) / 2 + 1, I, F, M>(),
        limiting_char<CMP, (L + U) / 2, U, I, F, M>()>();
  }

  template <std::size_t L, std::size_t U, std::size_t I>
  static constexpr char smallest_char()
  {
    static_assert(I < max_key_size<L, U>(), "I is too large for this range");
    return getchar<limiting_char<lessthan, L, U, I, size(), size()>(), I>();
  }

  template <std::size_t L, std::size_t U, std::size_t I>
  static constexpr char largest_char()
  {
    static_assert(I < max_key_size<L, U>(), "I is too large for this range");
    return getchar<limiting_char<morethan, L, U, I, size(), size()>(), I>();
  }

  template <std::size_t L, std::size_t U, std::size_t I, char C>
  static constexpr typename std::enable_if<(L == U), bool>::type contains_char()
  {
    return false;
  }

  template <std::size_t L, std::size_t U, std::size_t I, char C>
  static constexpr typename std::enable_if<(L + 1 == U), bool>::type
  contains_char()
  {
    return (I >= get(L).size()) ? false : (get(L)[I] == C);
  }

  template <std::size_t L, std::size_t U, std::size_t I, char C>
  static constexpr typename std::enable_if<(L + 2 == U), bool>::type
  contains_char()
  {
    return contains_char<L, L + 1, I, C>() || contains_char<L + 1, U, I, C>();
  }

  template <std::size_t L, std::size_t U, std::size_t I, char C>
  static constexpr typename std::enable_if<(L + 2 < U), bool>::type
  contains_char()
  {
    return contains_char<L, (L + U) / 2 + 1, I, C>() ||
           contains_char<(L + U) / 2, U, I, C>();
  }

  template <std::size_t L, std::size_t U>
  static constexpr typename std::enable_if<(L == U), bool>::type ordered_impl()
  {
    return true;
  }

  template <std::size_t L, std::size_t U>
  static constexpr typename std::enable_if<(L + 1 == U), bool>::type
  ordered_impl()
  {
    static_assert(in_bound<L>(), "L out of bounds");
    static_assert(U == 0 || in_bound<U - 1>(), "U out of bounds");
    return true;
  }

  template <std::size_t L, std::size_t U>
  static constexpr typename std::enable_if<(L + 2 == U), bool>::type
  ordered_impl()
  {
    static_assert(in_bound<L>(), "L out of bounds");
    static_assert(in_bound<L + 1>(), "L+1 out of bounds");
    static_assert(U == 0 || in_bound<U - 1>(), "U out of bounds");

    return get<L>() < get<L + 1>();
  }

  template <std::size_t L, std::size_t U>
  static constexpr typename std::enable_if<(L + 2 < U), bool>::type
  ordered_impl()
  {
    static_assert(in_bound<L>(), "L out of bounds");
    static_assert(U == 0 || in_bound<U - 1>(), "U out of bounds");

    return ordered_impl<L, (L + U) / 2 + 1>() &&
           get<(L + U) / 2>() < get<(L + U) / 2 + 1>() &&
           ordered_impl<((L + U) / 2), U>();
  }

  template <std::size_t L, std::size_t U, std::size_t I, char C>
  static constexpr typename std::enable_if<(L == U), std::size_t>::type
  find_upper_bound()
  {
    return fail();
  }

  template <std::size_t L, std::size_t U, std::size_t I, char C>
  static constexpr typename std::enable_if<(L + 1 == U), std::size_t>::type
  find_upper_bound()
  {
    return (contains_char<L, U, I, C>()) ? U : fail();
  }

  template <std::size_t L, std::size_t U, std::size_t I, char C>
  static constexpr typename std::enable_if<(L + 1 < U), std::size_t>::type
  find_upper_bound()
  {
    return contains_char<(L + U) / 2, U, I, C>()
               ? find_upper_bound<(L + U) / 2, U, I, C>()
               : contains_char<L, (L + U) / 2, I, C>()
                     ? find_upper_bound<L, (L + U) / 2, I, C>()
                     : fail();
  }

  template <std::size_t L, std::size_t U, std::size_t I, char C>
  static constexpr typename std::enable_if<(L == U), std::size_t>::type
  find_lower_bound()
  {
    return fail();
  }

  template <std::size_t L, std::size_t U, std::size_t I, char C>
  static constexpr typename std::enable_if<(L + 1 == U), std::size_t>::type
  find_lower_bound()
  {
    return (contains_char<L, U, I, C>()) ? L : fail();
  }

  template <std::size_t L, std::size_t U, std::size_t I, char C>
  static constexpr typename std::enable_if<(L + 1 < U), std::size_t>::type
  find_lower_bound()
  {
    return (contains_char<L, (L + U) / 2, I, C>())
               ? find_lower_bound<L, (L + U) / 2, I, C>()
               : (contains_char<(L + U) / 2, U, I, C>())
                     ? find_lower_bound<(L + U) / 2, U, I, C>()
                     : fail();
  }

  // Failure base case, including inverted bounsd => empty
  template <std::size_t L, std::size_t U, std::size_t I, std::size_t IMAX>
  static constexpr typename std::enable_if<((L >= U) || (I > IMAX)),
                                           std::size_t>::type
  scan(const char*)
  {
    return fail();
  }

  // Narrowed down to a single element at L
  // Asking to look at the next character beyond L's string means success
  template <std::size_t L, std::size_t U, std::size_t I, std::size_t IMAX,
            std::size_t LSZ>
  static constexpr typename std::enable_if<(I >= LSZ), std::size_t>::type
  scan_single(const char*)
  {
    static_assert(L + 1 == U, "");
    static_assert(I <= IMAX, "");
    return L;
  }

  // Still looking at more characters
  template <std::size_t L, std::size_t U, std::size_t I, std::size_t IMAX,
            std::size_t LSZ>
  static constexpr typename std::enable_if<(I < LSZ), std::size_t>::type
  scan_single(const char* key)
  {
    static_assert(L + 1 == U, "");
    static_assert(I <= IMAX, "");
    static_assert(I < LSZ, "");

    return getchar<L, I>() == key[I] ? scan<L, U, I + 1, IMAX>(key) : fail();
  }

  template <std::size_t L, std::size_t U, std::size_t I, std::size_t IMAX>
  static constexpr typename std::enable_if<((L + 1 == U) && (I <= IMAX)),
                                           std::size_t>::type
  scan(const char* key)
  {
    return scan_single<L, U, I, IMAX, get<L>().size()>(key);
  }

  // Reducing bounds
  template <std::size_t L, std::size_t U, std::size_t I, std::size_t IMAX,
            char C>
  static constexpr typename std::enable_if<(I > IMAX), std::size_t>::type n(
      const char*)
  {
    return fail();
  }

  template <std::size_t L, std::size_t U, std::size_t I, std::size_t IMAX,
            char C>
  static constexpr typename std::enable_if<(I <= IMAX), std::size_t>::type n(
      const char* key)
  {
    return scan<find_lower_bound<L, U, I, C>(), find_upper_bound<L, U, I, C>(), I + 1,
                IMAX>(key);
  }

  template <std::size_t L, std::size_t U, std::size_t I, char C>
  static constexpr bool candidate(const char* key)
  {
    return force_bool<contains_char<L, U, I, C>()>::value && key[I] == C;
  }

  template <std::size_t L, std::size_t U, std::size_t I, std::size_t IMAX,
            char SC, char MC>
  static constexpr std::size_t switch_lookup(const char* key)
  {
#define SWITCH_CASE(X) candidate<L, U, I, X>(key) ? n<L, U, I, IMAX, X>(key):
    return /* constexpr style switch statement */
        SWITCH_CASE('\x00') SWITCH_CASE('\x01') SWITCH_CASE('\x02')
        SWITCH_CASE('\x03') SWITCH_CASE('\x04') SWITCH_CASE('\x05')
        SWITCH_CASE('\x06') SWITCH_CASE('\x07') SWITCH_CASE('\x08')
        SWITCH_CASE('\x09') SWITCH_CASE('\x0a') SWITCH_CASE('\x0b')
        SWITCH_CASE('\x0c') SWITCH_CASE('\x0d') SWITCH_CASE('\x0e')
        SWITCH_CASE('\x0f') SWITCH_CASE('\x10') SWITCH_CASE('\x11')
        SWITCH_CASE('\x12') SWITCH_CASE('\x13') SWITCH_CASE('\x14')
        SWITCH_CASE('\x15') SWITCH_CASE('\x16') SWITCH_CASE('\x17')
        SWITCH_CASE('\x18') SWITCH_CASE('\x19') SWITCH_CASE('\x1a')
        SWITCH_CASE('\x1b') SWITCH_CASE('\x1c') SWITCH_CASE('\x1d')
        SWITCH_CASE('\x1e') SWITCH_CASE('\x1f') SWITCH_CASE('\x20')
        SWITCH_CASE('\x21') SWITCH_CASE('\x22') SWITCH_CASE('\x23')
        SWITCH_CASE('\x24') SWITCH_CASE('\x25') SWITCH_CASE('\x26')
        SWITCH_CASE('\x27') SWITCH_CASE('\x28') SWITCH_CASE('\x29')
        SWITCH_CASE('\x2a') SWITCH_CASE('\x2b') SWITCH_CASE('\x2c')
        SWITCH_CASE('\x2d') SWITCH_CASE('\x2e') SWITCH_CASE('\x2f')
        SWITCH_CASE('\x30') SWITCH_CASE('\x31') SWITCH_CASE('\x32')
        SWITCH_CASE('\x33') SWITCH_CASE('\x34') SWITCH_CASE('\x35')
        SWITCH_CASE('\x36') SWITCH_CASE('\x37') SWITCH_CASE('\x38')
        SWITCH_CASE('\x39') SWITCH_CASE('\x3a') SWITCH_CASE('\x3b')
        SWITCH_CASE('\x3c') SWITCH_CASE('\x3d') SWITCH_CASE('\x3e')
        SWITCH_CASE('\x3f') SWITCH_CASE('\x40') SWITCH_CASE('\x41')
        SWITCH_CASE('\x42') SWITCH_CASE('\x43') SWITCH_CASE('\x44')
        SWITCH_CASE('\x45') SWITCH_CASE('\x46') SWITCH_CASE('\x47')
        SWITCH_CASE('\x48') SWITCH_CASE('\x49') SWITCH_CASE('\x4a')
        SWITCH_CASE('\x4b') SWITCH_CASE('\x4c') SWITCH_CASE('\x4d')
        SWITCH_CASE('\x4e') SWITCH_CASE('\x4f') SWITCH_CASE('\x50')
        SWITCH_CASE('\x51') SWITCH_CASE('\x52') SWITCH_CASE('\x53')
        SWITCH_CASE('\x54') SWITCH_CASE('\x55') SWITCH_CASE('\x56')
        SWITCH_CASE('\x57') SWITCH_CASE('\x58') SWITCH_CASE('\x59')
        SWITCH_CASE('\x5a') SWITCH_CASE('\x5b') SWITCH_CASE('\x5c')
        SWITCH_CASE('\x5d') SWITCH_CASE('\x5e') SWITCH_CASE('\x5f')
        SWITCH_CASE('\x60') SWITCH_CASE('\x61') SWITCH_CASE('\x62')
        SWITCH_CASE('\x63') SWITCH_CASE('\x64') SWITCH_CASE('\x65')
        SWITCH_CASE('\x66') SWITCH_CASE('\x67') SWITCH_CASE('\x68')
        SWITCH_CASE('\x69') SWITCH_CASE('\x6a') SWITCH_CASE('\x6b')
        SWITCH_CASE('\x6c') SWITCH_CASE('\x6d') SWITCH_CASE('\x6e')
        SWITCH_CASE('\x6f') SWITCH_CASE('\x70') SWITCH_CASE('\x71')
        SWITCH_CASE('\x72') SWITCH_CASE('\x73') SWITCH_CASE('\x74')
        SWITCH_CASE('\x75') SWITCH_CASE('\x76') SWITCH_CASE('\x77')
        SWITCH_CASE('\x78') SWITCH_CASE('\x79') SWITCH_CASE('\x7a')
        SWITCH_CASE('\x7b') SWITCH_CASE('\x7c') SWITCH_CASE('\x7d')
        SWITCH_CASE('\x7e') SWITCH_CASE('\x7f') SWITCH_CASE('\x80')
        SWITCH_CASE('\x81') SWITCH_CASE('\x82') SWITCH_CASE('\x83')
        SWITCH_CASE('\x84') SWITCH_CASE('\x85') SWITCH_CASE('\x86')
        SWITCH_CASE('\x87') SWITCH_CASE('\x88') SWITCH_CASE('\x89')
        SWITCH_CASE('\x8a') SWITCH_CASE('\x8b') SWITCH_CASE('\x8c')
        SWITCH_CASE('\x8d') SWITCH_CASE('\x8e') SWITCH_CASE('\x8f')
        SWITCH_CASE('\x90') SWITCH_CASE('\x91') SWITCH_CASE('\x92')
        SWITCH_CASE('\x93') SWITCH_CASE('\x94') SWITCH_CASE('\x95')
        SWITCH_CASE('\x96') SWITCH_CASE('\x97') SWITCH_CASE('\x98')
        SWITCH_CASE('\x99') SWITCH_CASE('\x9a') SWITCH_CASE('\x9b')
        SWITCH_CASE('\x9c') SWITCH_CASE('\x9d') SWITCH_CASE('\x9e')
        SWITCH_CASE('\x9f') SWITCH_CASE('\xa0') SWITCH_CASE('\xa1')
        SWITCH_CASE('\xa2') SWITCH_CASE('\xa3') SWITCH_CASE('\xa4')
        SWITCH_CASE('\xa5') SWITCH_CASE('\xa6') SWITCH_CASE('\xa7')
        SWITCH_CASE('\xa8') SWITCH_CASE('\xa9') SWITCH_CASE('\xaa')
        SWITCH_CASE('\xab') SWITCH_CASE('\xac') SWITCH_CASE('\xad')
        SWITCH_CASE('\xae') SWITCH_CASE('\xaf') SWITCH_CASE('\xb0')
        SWITCH_CASE('\xb1') SWITCH_CASE('\xb2') SWITCH_CASE('\xb3')
        SWITCH_CASE('\xb4') SWITCH_CASE('\xb5') SWITCH_CASE('\xb6')
        SWITCH_CASE('\xb7') SWITCH_CASE('\xb8') SWITCH_CASE('\xb9')
        SWITCH_CASE('\xba') SWITCH_CASE('\xbb') SWITCH_CASE('\xbc')
        SWITCH_CASE('\xbd') SWITCH_CASE('\xbe') SWITCH_CASE('\xbf')
        SWITCH_CASE('\xc0') SWITCH_CASE('\xc1') SWITCH_CASE('\xc2')
        SWITCH_CASE('\xc3') SWITCH_CASE('\xc4') SWITCH_CASE('\xc5')
        SWITCH_CASE('\xc6') SWITCH_CASE('\xc7') SWITCH_CASE('\xc8')
        SWITCH_CASE('\xc9') SWITCH_CASE('\xca') SWITCH_CASE('\xcb')
        SWITCH_CASE('\xcc') SWITCH_CASE('\xcd') SWITCH_CASE('\xce')
        SWITCH_CASE('\xcf') SWITCH_CASE('\xd0') SWITCH_CASE('\xd1')
        SWITCH_CASE('\xd2') SWITCH_CASE('\xd3') SWITCH_CASE('\xd4')
        SWITCH_CASE('\xd5') SWITCH_CASE('\xd6') SWITCH_CASE('\xd7')
        SWITCH_CASE('\xd8') SWITCH_CASE('\xd9') SWITCH_CASE('\xda')
        SWITCH_CASE('\xdb') SWITCH_CASE('\xdc') SWITCH_CASE('\xdd')
        SWITCH_CASE('\xde') SWITCH_CASE('\xdf') SWITCH_CASE('\xe0')
        SWITCH_CASE('\xe1') SWITCH_CASE('\xe2') SWITCH_CASE('\xe3')
        SWITCH_CASE('\xe4') SWITCH_CASE('\xe5') SWITCH_CASE('\xe6')
        SWITCH_CASE('\xe7') SWITCH_CASE('\xe8') SWITCH_CASE('\xe9')
        SWITCH_CASE('\xea') SWITCH_CASE('\xeb') SWITCH_CASE('\xec')
        SWITCH_CASE('\xed') SWITCH_CASE('\xee') SWITCH_CASE('\xef')
        SWITCH_CASE('\xf0') SWITCH_CASE('\xf1') SWITCH_CASE('\xf2')
        SWITCH_CASE('\xf3') SWITCH_CASE('\xf4') SWITCH_CASE('\xf5')
        SWITCH_CASE('\xf6') SWITCH_CASE('\xf7') SWITCH_CASE('\xf8')
        SWITCH_CASE('\xf9') SWITCH_CASE('\xfa') SWITCH_CASE('\xfb')
        SWITCH_CASE('\xfc') SWITCH_CASE('\xfd') SWITCH_CASE('\xfe')
#undef SWITCH_CASE
        fail();
  }

  template <std::size_t L, std::size_t U, std::size_t I, std::size_t IMAX,
            char SC, char MC>
  static constexpr typename std::enable_if<(SC == MC), std::size_t>::type
  scan_narrow(const char* key)
  {
    static_assert(L + 1 < U, "Bounds wrong");
    static_assert(I <= IMAX, "");
    return (key[I] != SC) ? fail() : n<L, U, I, IMAX, SC>(key);
  }

  template <std::size_t L, std::size_t U, std::size_t I, std::size_t IMAX,
            char SC, char MC>
  static constexpr typename std::enable_if<(SC != MC), std::size_t>::type
  scan_narrow(const char* key)
  {
    static_assert(L + 1 < U, "Bounds wrong");
    static_assert(I <= IMAX, "");

    return switch_lookup<L, U, I, IMAX, smallest_char<L, U, I>(),
                         largest_char<L, U, I>()>(key);
  }

  template <std::size_t L, std::size_t U, std::size_t I, std::size_t IMAX>
  static constexpr typename std::enable_if<((L + 1 < U) && (I <= IMAX)),
                                           std::size_t>::type
  scan(const char* key)
  {
    static_assert(L < U, "Bounds wrong");
    // Scan narrow hand-optimises the case where smallest_char == largest_char
    return scan_narrow<L, U, I, IMAX, smallest_char<L, U, I>(),
                       largest_char<L, U, I>()>(key);
  }
};
}

#endif  // PREFIX_H
