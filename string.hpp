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
#ifndef STRING_HPP
#define STRING_HPP

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

class str_const
{
 private:
  const char* const p_;
  const std::size_t sz_;

  static constexpr bool equal(std::size_t sz, const char* x, const char* y)
  {
    return (sz == 0) ? true : (x[0] == y[0]) && equal(sz - 1, x + 1, y + 1);
  }

  static constexpr bool less(std::size_t xsz, std::size_t ysz, const char* x,
                             const char* y)
  {
    return (xsz == 0 && ysz == 0)
               ? false
               : (xsz == 0) ? true : (ysz == 0)
                                         ? false
                                         : (static_cast<uint8_t>(x[0]) <
                                            static_cast<uint8_t>(y[0]))
                                               ? true
                                               : (static_cast<uint8_t>(y[0]) <
                                                  static_cast<uint8_t>(x[0]))
                                                     ? false
                                                     : less(xsz - 1, ysz - 1,
                                                            x + 1, y + 1);
  }

 public:
  template <std::size_t N>
  constexpr str_const(const char (&a)[N])
      : p_(a), sz_(N - 1)
  {
  }

  constexpr char operator[](std::size_t n) const
  {
    return n < sz_ ? p_[n] : throw std::out_of_range("str_const out of range");
  }
  constexpr std::size_t size() const { return sz_; }
  constexpr const char* data() const { return p_; }

  // This is an awkward to use operator[], but which is strictly compile time
  template <std::size_t N, std::size_t I>
  static constexpr char get(const str_const& x)
  {
    static_assert(I < N, "");
    return x.data()[I];
  }

  friend constexpr bool operator==(const str_const& x, const str_const& y)
  {
    return (x.size() == y.size()) && equal(x.size(), x.data(), y.data());
  }

  friend constexpr bool operator<(const str_const& x, const str_const& y)
  {
    return less(x.size(), y.size(), x.data(), y.data());
  }

  friend constexpr bool operator!=(const str_const& x, const str_const& y)
  {
    return !(x == y);
  }
  friend constexpr bool operator>(const str_const& x, const str_const& y)
  {
    return y < x;
  }
  friend constexpr bool operator<=(const str_const& x, const str_const& y)
  {
    return x < y || x == y;
  }
  friend constexpr bool operator>=(const str_const& x, const str_const& y)
  {
    return y < x || x == y;
  }
};

#endif  // STRING_HPP
